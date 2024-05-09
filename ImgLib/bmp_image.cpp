#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

PACKED_STRUCT_BEGIN BitmapFileHeader {
    char sign1 = 'B';
    char sign2 = 'M';
    uint32_t size = 0;
    uint32_t reserved_space = 0;
    uint32_t padding = 54;
}
PACKED_STRUCT_END

PACKED_STRUCT_BEGIN BitmapInfoHeader {
    uint32_t size = 40;
    int32_t width = 0;
    int32_t height = 0;
    uint16_t layers = 1;
    uint16_t bit_per_pixel = 24;
    uint32_t compression = 0;
    uint32_t byte_count = 0;
    int32_t horizontal_resolution = 11811;
    int32_t vertical_resolution = 11811;
    int32_t used_colors = 0;
    int32_t meaningful_colors = 0x1000000;
}
PACKED_STRUCT_END

static int GetBMPStride(int w) {
    return 4 * ((w * 3 + 3) / 4);
}

bool SaveBMP(const Path& file, const Image& image) {
    ofstream out(file, ios::binary);
    
    const int w = image.GetWidth();
    const int h = image.GetHeight();
    int stride = GetBMPStride(w);
    
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    
    file_header.size = file_header.padding + h * stride;
    info_header.width = w;
    info_header.height = h;
    info_header.byte_count = h * stride;
    
    out.write(reinterpret_cast<const char*>(&file_header), 14);
    out.write(reinterpret_cast<const char*>(&info_header), 40);
    
    std::vector<char> buff(stride);
    
    for (int y = h - 1; y >= 0; --y) {
        const Color* line = image.GetLine(y);
        for (int x = 0; x < w; ++x) {
            buff[x * 3 + 0] = static_cast<char>(line[x].b);
            buff[x * 3 + 1] = static_cast<char>(line[x].g);
            buff[x * 3 + 2] = static_cast<char>(line[x].r);
        }
        out.write(buff.data(), stride);
    }
    
    return out.good();
}

Image LoadBMP(const Path& file) {
    ifstream ifs(file, ios::binary);
    
    BitmapFileHeader file_header;
    BitmapInfoHeader info_header;
    
    ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
    ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));
    
    int w = info_header.width;
    int h = info_header.height;
    int stride = GetBMPStride(w);
    
    Image result(w, h, Color::Black());
    std::vector<char> buff(stride);
    
    for (int y = h - 1; y >= 0; --y) {
        Color* line = result.GetLine(y);
        ifs.read(buff.data(), stride);

        for (int x = 0; x < w; ++x) {
            line[x].b = static_cast<byte>(buff[x * 3 + 0]);
            line[x].g = static_cast<byte>(buff[x * 3 + 1]);
            line[x].r = static_cast<byte>(buff[x * 3 + 2]);
        }
    }

    return result;
}

}  // namespace img_lib
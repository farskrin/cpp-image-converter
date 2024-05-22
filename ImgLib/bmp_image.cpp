#include "bmp_image.h"
#include "pack_defines.h"

#include <array>
#include <fstream>
#include <string_view>

using namespace std;

namespace img_lib {

    PACKED_STRUCT_BEGIN BitmapFileHeader{
        // поля заголовка Bitmap File Header
        std::array<char, 2> sign = {'B', 'M'};
        uint32_t header_data_size = 0;          //headers_offset + data; where data = stride * height;
        uint32_t reserved_space = 0;
        uint32_t headers_offset = 54;           //header1 + header2; (14 + 40 byte from condition)
    }PACKED_STRUCT_END

    PACKED_STRUCT_BEGIN BitmapInfoHeader{
        // поля заголовка Bitmap Info Header
        uint32_t header_size = 40;      //size of header2 (40 byte from conditon)
        uint32_t width = 0;
        uint32_t height = 0;
        uint16_t layers = 1;            //one RGB layer
        uint16_t bit_per_pixel = 24;    //condition
        uint32_t compression = 0;       //no compression
        uint32_t data = 0;              //(bytes) data = stride * height;
        int32_t vertical_resolution = 11811;     //pixel per meter
        int32_t horizontal_resolution = 11811;   //pixel per meter
        int32_t usign_colors = 0;             //undefined
        int32_t significant_colors = 0x1000000;
    }
    PACKED_STRUCT_END

    // функция вычисления отступа по ширине
    static int GetBMPStride(int w) {
        return 4 * ((w * 3 + 3) / 4);
    }

    // напишите эту функцию
    bool SaveBMP(const Path& file, const Image& image) {    
        ofstream ofs(file, ios::binary);
        const int width = image.GetWidth();
        const int height = image.GetHeight();
        const int stride = GetBMPStride(width);
        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;

        const uint32_t data = stride * height;
        file_header.header_data_size = file_header.headers_offset + data;

        info_header.width = width;
        info_header.height = height;
        info_header.data = data;

        ofs.write(reinterpret_cast<const char*>(&file_header), 14);
        ofs.write(reinterpret_cast<const char*>(&info_header), 40);

        std::vector<char> buff(stride);
        for (int y = height - 1; y >= 0; --y) {
            const Color* line = image.GetLine(y);
            for (int x = 0; x < width; ++x) {
                buff[x * 3 + 0] = static_cast<char>(line[x].b);
                buff[x * 3 + 1] = static_cast<char>(line[x].g);
                buff[x * 3 + 2] = static_cast<char>(line[x].r);
            }
            ofs.write(buff.data(), stride);
        }

        return ofs.good();
    }

    // напишите эту функцию
    Image LoadBMP(const Path& file) {
        ifstream ifs(file, ios::binary);
        BitmapFileHeader file_header;
        BitmapInfoHeader info_header;
        ifs.read(reinterpret_cast<char*>(&file_header), sizeof(BitmapFileHeader));
        ifs.read(reinterpret_cast<char*>(&info_header), sizeof(BitmapInfoHeader));

        const int width = info_header.width;
        const int height = info_header.height;
		const int stride = GetBMPStride(width);

        Image result(width, height, Color::Black());     //TODO: w

        std::vector<char> buff(stride);
        for (int y = height - 1; y >= 0; --y) {
            Color* line = result.GetLine(y);
            ifs.read(buff.data(), stride);

            for (int x = 0; x < width; ++x) {
                line[x].b = static_cast<byte>(buff[x * 3 + 0]);
                line[x].g = static_cast<byte>(buff[x * 3 + 1]);
                line[x].r = static_cast<byte>(buff[x * 3 + 2]);
            }
        }

        return result;
    }

}  // namespace img_lib
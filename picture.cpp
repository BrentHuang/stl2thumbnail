/*
Copyright (C) 2017  Paul Kremer

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "picture.h"
//#include <iostream>

static Byte floatToByte(float v)
{
    v = std::max(0.0f, std::min(v, 1.0f));
    return Byte(v * 255.0f);
}

Picture::Picture(size_t width, size_t height, const char* bg_pic_file_path, int depth)
    : m_width(width), m_height(height), m_bg_pic_file_path(), m_depth(depth)
{
    if (bg_pic_file_path != nullptr)
    {
        m_bg_pic_file_path.assign(bg_pic_file_path);
    }

    m_stride = m_width * m_depth;
    m_buffer.resize(m_height * m_stride);
}

Byte* Picture::data()
{
    return m_buffer.data();
}

int Picture::save(const std::string& file_path)
{
    FILE* fp = fopen(file_path.c_str(), "wb");
    if (nullptr == fp)
    {
        return -1;
    }

    png_structp png_ptr  = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
    if (nullptr == png_ptr)
    {
        fclose(fp);
        return -1;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (nullptr == info_ptr)
    {
        png_destroy_write_struct(&png_ptr, nullptr);
        fclose(fp);
        return -1;
    }

    png_init_io(png_ptr, fp);
    png_set_IHDR(png_ptr, info_ptr, m_width, m_height,
                 8, (4 == m_depth) ? PNG_COLOR_TYPE_RGBA : PNG_COLOR_TYPE_RGB,
                 PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
    png_write_info(png_ptr, info_ptr);

    for (size_t y = 0; y < m_height; ++y)
    {
        png_write_row(png_ptr, &m_buffer[y * m_stride]);
    }

    png_write_end(png_ptr, nullptr);

    png_free_data(png_ptr, info_ptr, PNG_FREE_ALL, -1);
    png_destroy_write_struct(&png_ptr, &info_ptr);
    fclose(fp);

    return 0;
}

void Picture::setRGB(size_t x, size_t y, Byte r, Byte g, Byte b, Byte a)
{
    if (x >= m_width || y >= m_height)
    {
        return;
    }

    m_buffer[y * m_stride + x * m_depth + 0] = r;
    m_buffer[y * m_stride + x * m_depth + 1] = g;
    m_buffer[y * m_stride + x * m_depth + 2] = b;

    if (4 == m_depth)
    {
        m_buffer[y * m_stride + x * m_depth + 3] = a;
    }
}

void Picture::setRGB(size_t x, size_t y, float r, float g, float b, float a)
{
    if (x >= m_width || y >= m_height)
    {
        return;
    }

    m_buffer[y * m_stride + x * m_depth + 0] = floatToByte(r);
    m_buffer[y * m_stride + x * m_depth + 1] = floatToByte(g);
    m_buffer[y * m_stride + x * m_depth + 2] = floatToByte(b);

    if (4 == m_depth)
    {
        m_buffer[y * m_stride + x * m_depth + 3] = floatToByte(a);
    }
}

void Picture::setBackground()
{
    FILE* fp = nullptr;
    png_structp png_ptr = nullptr;
    png_infop info_ptr = nullptr;
    bool success = false;

    do
    {
        fp = fopen(m_bg_pic_file_path.c_str(), "rb");
        if (nullptr == fp)
        {
            break;
        }

        png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
        if (nullptr == png_ptr)
        {
            break;
        }

        info_ptr = png_create_info_struct( png_ptr );
        if (nullptr == info_ptr)
        {
            break;
        }

        setjmp(png_jmpbuf(png_ptr));

        /* 读取PNG_BYTES_TO_CHECK个字节的数据 */
        const int PNG_BYTES_TO_CHECK = 4;
        char buf[PNG_BYTES_TO_CHECK] = "";

        int ret = fread(buf, 1, PNG_BYTES_TO_CHECK, fp);
        if (ret < PNG_BYTES_TO_CHECK) /* 若读到的数据并没有PNG_BYTES_TO_CHECK个字节 */
        {
            break;
        }

        /* 检测数据是否为PNG的签名 */
        ret = png_sig_cmp((png_bytep) buf, (png_size_t) 0, PNG_BYTES_TO_CHECK);
        if (ret != 0) /* 如果不是PNG的签名，则说明该文件不是PNG文件 */
        {
            break;
        }

        rewind(fp); /* 复位文件指针 */

        png_init_io(png_ptr, fp);
        png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, nullptr); /* 读取PNG图片信息和像素数据 */

        const png_byte color_type = png_get_color_type(png_ptr, info_ptr);
        const png_uint_32 w = png_get_image_width(png_ptr, info_ptr);
        const png_uint_32 h = png_get_image_height(png_ptr, info_ptr);
        if (w != m_width || h != m_height)
        {
            break;
        }

        /* 获取图像的所有行像素数据，row_pointers里边就是rgba数据 */
        png_bytep* row_pointers = png_get_rows(png_ptr, info_ptr);
        if (nullptr == row_pointers)
        {
            break;
        }

        setBg(color_type, row_pointers, m_backgroundColor);
        success = true;
    } while (0);

    if (!success)
    {
        fill(m_backgroundColor.x, m_backgroundColor.y, m_backgroundColor.z, m_backgroundColor.w); // 设置背景色
    }

    if (png_ptr != nullptr)
    {
        if (info_ptr != nullptr)
        {
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        }
        else
        {
            png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        }
    }

    if (fp != nullptr)
    {
        fclose(fp);
    }
}

//size_t Picture::size() const
//{
//    return m_size;
//}

void Picture::fill(float r, float g, float b, float a)
{
    for (size_t y = 0; y < m_height; ++y)
    {
        for (size_t x = 0; x < m_width; ++x)
        {
            setRGB(x, y, r, g, b, a);
        }
    }
}

void Picture::setBg(png_byte color_type, png_bytep* row_pointers, const Vec4& bg_color)
{
    /* 根据不同的色彩类型进行相应处理 */
    switch (color_type)
    {
        case PNG_COLOR_TYPE_RGB_ALPHA:
        {
            png_uint_32 pos = 0;

            for (png_uint_32 y = 0; y < m_height; ++y)
            {
                for (png_uint_32 x = 0; x < m_width * 4; )
                {
                    /* 以下是RGBA数据，需要自己补充代码，保存RGBA数据 */
                    m_buffer[pos++] = row_pointers[y][x++]; // red
                    m_buffer[pos++] = row_pointers[y][x++]; // green
                    m_buffer[pos++] = row_pointers[y][x++]; // blue

                    if (4 == m_depth)
                    {
                        m_buffer[pos++] = row_pointers[y][x++]; // alpha
                    }
                }
            }

//            for ( y = 0; y < h; ++y )
//            {
//                for ( x = 0; x < w * 4; )
//                {
//                    /* 以下是RGBA数据，需要自己补充代码，保存RGBA数据 */
//                    /* 目标内存 */ = row_pointers[y][x++]; // red
//                    /* 目标内存 */ = row_pointers[y][x++]; // green
//                    /* 目标内存 */ = row_pointers[y][x++]; // blue
//                    /* 目标内存 */ = row_pointers[y][x++]; // alpha
//                }
//            }
        }
        break;

        case PNG_COLOR_TYPE_RGB:
        {
            png_uint_32 pos = 0;

            for (png_uint_32 y = 0; y < m_height; ++y)
            {
                for (png_uint_32 x = 0; x < m_width * 3; )
                {
                    /* 以下是RGBA数据，需要自己补充代码，保存RGBA数据 */
                    m_buffer[pos++] = row_pointers[y][x++]; // red
                    m_buffer[pos++] = row_pointers[y][x++]; // green
                    m_buffer[pos++] = row_pointers[y][x++]; // blue

                    if (4 == m_depth)
                    {
                        m_buffer[pos++] = 255; // alpha
                    }
                }
            }
//            for ( y = 0; y < h; ++y )
//            {
//                for ( x = 0; x < w * 3; )
//                {
//                    /* 目标内存 */ = row_pointers[y][x++]; // red
//                    /* 目标内存 */ = row_pointers[y][x++]; // green
//                    /* 目标内存 */ = row_pointers[y][x++]; // blue
//                }
//            }
        }
        break;

        /* 其它色彩类型的图像就不读了 */

        default:
        {
            fill(bg_color.x, bg_color.y, bg_color.z, bg_color.w); // 设置背景色
        }
        break;
    }
}

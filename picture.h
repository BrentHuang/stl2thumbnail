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

#pragma once

#include <string>
#include <vector>
#include <png.h> // sudo apt-get install libpng-dev
#include "vec4.h"

using Byte   = unsigned char;
using Buffer = std::vector<Byte>;

class Picture
{
public:
    explicit Picture(size_t width, size_t height, const char* bg_pic_file_path = nullptr, int depth = 4); // depth=3: rgb depth=4: rgba

    Byte* data();
    int save(const std::string& file_path);
    void setRGB(size_t x, size_t y, Byte r, Byte g, Byte b, Byte a = 255);
    void setRGB(size_t x, size_t y, float r, float g, float b, float a = 1.0f);
    void setBackground();
//    size_t size() const;

private:
    void fill(float r, float g, float b, float a);
    void setBg(png_byte color_type, png_bytep* row_pointers, const Vec4& bg_color);

private:
    size_t m_width = 0;
    size_t m_height = 0;

    std::string m_bg_pic_file_path;
    Vec4 m_backgroundColor = { 211 / 255.f, 218 / 255.f, 224 / 255.f, 1.0f }; // 背景色，灰色。alpha值为0表示透明，为1表示不透明，值越小越透明

    int m_depth  = 4; // rgba
    size_t m_stride = 0;
    Buffer m_buffer;
};

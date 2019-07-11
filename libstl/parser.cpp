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

#include "parser.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include "helpers.h"

// STL format specifications: http://www.fabbers.com/tech/STL_Format

namespace stl
{
Parser::Parser()
{
}

Parser::~Parser()
{
}

int Parser::parseFile(Mesh& mesh, const std::string& file_path) const
{
    std::ifstream stream(file_path, std::ifstream::in | std::ifstream::binary);
    if (!stream)
    {
//        throw ("Cannot open file");
        return -1;
    }

    if (isBinaryFormat(stream))
    {
        struct stat stat_buf;
        stat(file_path.c_str(), &stat_buf);

        return parseBinary(mesh, stream, stat_buf.st_size);
    }

    return parseAscii(mesh, stream);
}

bool Parser::isBinaryFormat(std::ifstream& in) const
{
    // Note: A file starting with "solid" is no indicator for having an ASCII file
    // Some exporters put "solid <name>" in the binary header

    std::string line;
    getTrimmedLine(in, line); // skip potential string: solid <name>
    getTrimmedLine(in, line); // has to start with "facet" otherwise it is a binary file

    // return to begin of file
    in.clear();
    in.seekg(0, in.beg);

    return line.substr(0, 5) != "facet";
}

int Parser::parseBinary(Mesh& mesh, std::ifstream& in, size_t file_size) const
{
    // skip header
    in.seekg(80); // 文件起始的80个字节是文件头，用于存贮零件名

    // get the number of triangles in the stl 紧接着用4个字节的整数来描述模型的三角面片个数
    const uint32_t triangleCount = readU32(in);
    if ((84 + 50 * triangleCount) != file_size)
    {
        return -1;
    }

    mesh.reserve(triangleCount); // 太大了内存可能会爆掉

    // parse triangles 后面逐个给出每个三角面片的几何信息
    size_t i;

    for (i = 0; i < triangleCount && in; ++i)
    {
        Triangle triangle;

        if (readBinaryTriangle(triangle, in) != 0)
        {
            return -1;
        }

        mesh.emplace_back(triangle);
    }

    return 0;
}

int Parser::parseAscii(Mesh& mesh, std::ifstream& in) const
{
    // solid name
    std::string line;
    std::getline(in, line);
    if (line.substr(0, 5) != "solid")
    {
        return -1;
    }

    while (in)
    {
        Triangle triangle;

        if (readAsciiTriangle(triangle, in) != 0)
        {
            return -1;
        }

        mesh.emplace_back(triangle);
    }

    return 0;
}

uint32_t Parser::readU32(std::ifstream& in) const
{
    uint32_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

uint16_t Parser::readU16(std::ifstream& in) const
{
    uint16_t v;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

float Parser::readFloat(std::ifstream& in) const
{
    float v;
    in.read(reinterpret_cast<char*>(&v), sizeof(v));
    return v;
}

int Parser::readVector3(Vec3& v, std::ifstream& in) const
{
    v.x = readFloat(in);
    v.y = readFloat(in);
    v.z = -readFloat(in);

    return 0;
}

int Parser::readAsciiTriangle(Triangle& triangle, std::ifstream& in) const
{
    std::string line;

    getTrimmedLine(in, line);
    int n = std::sscanf(line.c_str(), "facet normal %e %e %e", &triangle.normal.x, &triangle.normal.y, &triangle.normal.z);
    if (n != 3)
    {
        if (0 == line.length() || "endsolid" == line.substr(0, 8))
        {
            // 要么是endpoint，要么是文件结束
            return 0;
        }

        return -1;
    }

    getTrimmedLine(in, line); // outer loop

    for (size_t i = 0; i < 3; ++i)
    {
        getTrimmedLine(in, line);
        n = std::sscanf(line.c_str(), "vertex %e %e %e", &triangle.vertices[i].x, &triangle.vertices[i].y, &triangle.vertices[i].z);
        if (n != 3)
        {
            return -1;
        }
    }

    getTrimmedLine(in, line); // endloop
    getTrimmedLine(in, line); // endfacet

    return 0;
}

int Parser::readBinaryTriangle(Triangle& triangle, std::ifstream& in) const
{
    // 每个三角面片占用固定的50个字节，依次是3个4字节浮点数(角面片的法矢量)，
    // 3个4字节浮点数(1个顶点的坐标)，3个4字节浮点数(2个顶点的坐标)，3个4字节浮点数(3个顶点的坐标)，
    // 最后2个字节用来描述三角面片的属性信息
    readVector3(triangle.normal, in);
    triangle.normal.normalize();

    readVector3(triangle.vertices[1], in);
    readVector3(triangle.vertices[0], in);
    readVector3(triangle.vertices[2], in);
    readU16(in); // attributes

    // some stl files have garbage normals
    // we recalculate them here in case they are NaN
    if (std::isnan(triangle.normal.x) || std::isnan(triangle.normal.y) || std::isnan(triangle.normal.z))
    {
        triangle.normal = triangle.calcNormal().normalize();
    }

    return 0;
}
} // namespace

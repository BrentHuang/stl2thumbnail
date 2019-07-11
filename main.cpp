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

#include <iostream>
#include <parser.h>

#include "args.hxx"
#include "backends/raster/backend.h"
#include "picture.h"

// mkdir build
// cd build
// cmake ..
// make
// ./stl2thumbnail ../cube.stl ./crub -s750x600

int main(int argc, char** argv)
{
    // command line
    args::ArgumentParser parser("Creates thumbnails from STL files", "");
    args::HelpFlag help(parser, "help", "Display this help menu", { 'h', "help" });

    args::Group group(parser, "This group is all exclusive:", args::Group::Validators::All);
    args::Positional<std::string> in(group, "in", "The stl filename");
    args::Positional<std::string> out(group, "out", "The thumbnail picture filename prefix");
    args::ValueFlag<std::string> picSize(group, "widthxheight", "The thumbnail size", { 's' });

    try
    {
        parser.ParseCLI(argc, argv);
    }
    catch (args::Help)
    {
        std::cout << parser;
        return 0;
    }
    catch (args::ParseError e)
    {
        std::cerr << e.what() << std::endl;
        std::cerr << parser;
        return 1;
    }
    catch (args::ValidationError)
    {
        std::cout << parser;
        return 0;
    }

    std::string size = picSize.Get();
    std::cout << size << std::endl;

    unsigned width, height;
    std::sscanf(size.c_str(), "%ux%u", &width, &height);

    // parse STL
    stl::Parser stlParser;
    Mesh mesh;
    try
    {
        stlParser.parseFile(mesh, in.Get());
    }
    catch (...)
    {
        std::cerr << "Cannot parse file " << in.Get();
        return 1;
    }

    std::cout << "Triangles: " << mesh.size() << std::endl;

    const int PIC_COUNT = 4;
    const Vec3 view_pos[PIC_COUNT] = {{ -1.f, -1.f, 1.f }, { 1.f, -1.f, 1.f }, { 1.f, 1.f, -1.f }, { -1.f, 1.f, -1.f }};

    for (int i = 0; i < PIC_COUNT; ++i)
    {
        // render using raster backend
        RasterBackend backend(width, height);
        Picture pic(width, height);
        backend.render(pic, mesh, view_pos[i]);

        // save to disk
        std::string png_file_path(out.Get());
        png_file_path += "-";
        png_file_path.append(std::to_string(i + 1));
        png_file_path += ".png";
        pic.save(png_file_path);
    }

    return 0;
}

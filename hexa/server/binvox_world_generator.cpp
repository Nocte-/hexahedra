//---------------------------------------------------------------------------
// server/binvox_world_generator.cpp
//
// This file is part of Hexahedra.
//
// Hexahedra is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License as published
// by the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Affero General Public License for more details.
//
// You should have received a copy of the GNU Affero General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.
//
// Copyright 2012, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "binvox_world_generator.hpp"

#include <cmath>
#include <cstring>
#include <fstream>
#include <map>
#include <stdexcept>
#include <sstream>
#include <string>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <zlib.h>

#include <hexa/block_types.hpp>

namespace fs = boost::filesystem;
using namespace boost::property_tree;

namespace hexa {

struct binvox_world_generator::impl
{
    fs::path         file_;
    world_vector     to_origin_;

    std::vector<uint8_t> voxels;

    uint32_t      width;
    uint32_t      depth;
    uint32_t      height;

    impl(fs::path file, world_coordinates origin = world_center)
        : file_       (file)
        , to_origin_  (origin)
    {
        std::ifstream input (file.string(), std::ios::binary);

        // read header
        //
        std::string line;
        input >> line;  // #binvox
        if (line != "#binvox")
        {
            std::cout << "Error: first line reads [" << line << "] instead of [#binvox]" << std::endl;
            return;
        }
        int version;
        input >> version;
        std::cout << "reading binvox version " << version << std::endl;

        depth = 0;
        int done = 0;
        while(input.good() && !done)
        {
            input >> line;
            if (line.compare("data") == 0)
                done = 1;
            else if (line.compare("dim") == 0)
                input >> depth >> height >> width;
            else
            {
                std::cout << "  unrecognized keyword [" << line << "], skipping" << std::endl;
                char c;
                do
                {  // skip until end of line
                    c = input.get();
                }
                while(input.good() && (c != '\n'));
            }
        }

        if (!done)
        {
            std::cout << "  error reading header" << std::endl;
            width = height = depth = 0;
            return;
        }
        if (depth == 0)
        {
            std::cout << "  missing dimensions in header" << std::endl;
            width = height = depth = 0;
            return;
        }

        int size = width * height * depth;
        voxels.resize(size);

        // read voxel data
        //
        uint8_t value;
        uint8_t count;
        int index = 0;
        int end_index = 0;
        int nr_voxels = 0;

        input.unsetf(std::ios::skipws);  // need to read every byte now (!)
        input >> value;  // read the linefeed char

        while((end_index < size) && input.good())
        {
            input >> value >> count;

            if (input.good())
            {
                end_index = index + count;
                if (end_index > size)
                    break;

                for(int i=index; i < end_index; i++)
                    voxels[i] = value;

                if (value)
                    nr_voxels += count;

                index = end_index;
            }  // if file still ok
        }  // while

        std::cout << "Set up binvox with " << width << " x " << height << " x " << depth << std::endl;
    }

    void generate (const chunk_coordinates& pos, chunk& dest) const
    {
        world_rel_coordinates l ((pos * chunk_size) - to_origin_);

        if (l.x <= -chunk_size || l.y <= -chunk_size || l.z <= -chunk_size)
            return;

        if (l.x > (int)width || l.y > (int)depth || l.z > (int)height)
            return;

        uint32_t c (0);
        for (int32_t z (l.z); z < l.z + chunk_size; ++z)
        {
        for (int32_t y (l.y); y < l.y + chunk_size; ++y)
        {
        for (int32_t x (l.x); x < l.x + chunk_size; ++x)
        {
            if (   x >= 0 && y >= 0 && z >= 0
                && x < (int)width && y < (int)depth && z < (int)height)
            {
                if (voxels[z + y * height + x * height * width])
                    dest[c] = 32;
            }
            ++c;
        }
        }
        }
    }

    chunk_height estimate_height (map_coordinates xy) const
    {
        map_rel_coordinates l ((xy * chunk_size) - map_coordinates(to_origin_));

        if (l.x <= -chunk_size || l.y <= -chunk_size)
            return undefined_height;

        if (l.x > (int)width || l.y > (int)depth)
            return undefined_height;

        uint32_t tz (height + to_origin_.z);
        return tz / chunk_size + 1;
    }

};

binvox_world_generator::binvox_world_generator (world& w, const ptree& conf)
    : terrain_generator_i (w, conf)
    , pimpl_ (new impl(conf.get<std::string>("path")))
{ }

binvox_world_generator::~binvox_world_generator()
{ }

void binvox_world_generator::generate(chunk_coordinates pos, chunk& dest)
{
    pimpl_->generate(pos, dest);
}

chunk_height
binvox_world_generator::estimate_height (map_coordinates xy) const
{
    return pimpl_->estimate_height(xy);
}
} // namespace hexa


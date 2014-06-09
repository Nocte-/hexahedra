//---------------------------------------------------------------------------
// hexa/client/render_surface.cpp
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

#include "render_surface.hpp"

namespace hexa
{

namespace
{

/** Optimize a single layer of tiles.
 *  This function tries to find rectangles of identical tiles in a 16x16
 *  grid.  It is a greedy algorithm, so it is built for speed rather than
 *  finding the optimal solution. */
optimized_render_surface::layer optimize_greedy(render_surface::layer in)
{
    // Max-size is set to 15, since the current OpenGL3 renderer only
    // supports quad sizes up to 15.  Not quite elegant, but it'll have
    // to do for now.
    const unsigned int max_size(chunk_size - 1);

    optimized_render_surface::layer result;
    size_t search(0);

    while (true) {
        // Find a suitable starting position
        optimized_render_surface::element rect;
        for (; search < chunk_area; ++search) {
            if (in[search].texture != 0) {
                rect = in[search];
                break;
            }
        }

        // If no more tiles could be found, we're done
        if (search == chunk_area)
            break;

        // Expand the rectangle horizontally until we encounter a different
        // tile.
        map_index pos(search % chunk_size, search >> cnkshift);

        while (pos.x + rect.size.x < max_size
               && rect == in[pos + map_index(rect.size.x, 0)]) {
            ++rect.size.x;
            ++search;
        }

        // Same thing, this time vertically.
        while (pos.y + rect.size.y < max_size) {
            bool expand(true);
            for (int x(0); expand && x < rect.size.x; ++x) {
                if (rect != in[pos + map_index(x, rect.size.y)])
                    expand = false;
            }

            if (!expand)
                break;

            ++rect.size.y;
        }

        // Remove the tiles from the original surface, so we don't process
        // them twice.
        for (int y(pos.y); y < pos.y + rect.size.y; ++y) {
            for (int x(pos.x); x < pos.x + rect.size.x; ++x)
                in(x, y).texture = 0;
        }

        // Add the rectagle to the result and continue.
        result[pos] = rect;
    }
    return result;
}

} // anonymous namespace

//---------------------------------------------------------------------------

optimized_render_surface optimize_greedy(const render_surface& in)
{
    optimized_render_surface result;
    for (int i(0); i < 6; ++i) {
        for (auto& j : in.dirs[i])
        // for (uint8_t j (0); j < chunk_size; ++j)
        {
            ///\todo Is this a memory leak in boost::flat_map?
            // result.dirs[i].emplace(j, optimize_greedy(in.dirs[i][j]));
            // result.dirs[i].insert(std::make_pair(j,
            // optimize_greedy(in.dirs[i][j])));
            result.dirs[i].insert(
                std::make_pair(j.first, optimize_greedy(j.second)));
        }
    }

    return result;
}

} // namespace hexa

//---------------------------------------------------------------------------
/// \file   client/render_surface.hpp
/// \brief  Combined object of surface and light data, ready for rendering.
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

#pragma once

#include <array>
#include <functional>
#include <boost/range/algorithm.hpp>
//#include <boost/container/flat_map.hpp>
#include <hexa/flat_map.hpp>
#include <hexa/area_data.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/matrix.hpp>

using namespace boost::range;

namespace hexa {

template <class t>
class slice
{
    typedef std::vector<t>  array_t;
    array_t buf_;

public:
    typedef typename array_t::value_type        value_type;
    typedef typename array_t::iterator          iterator;
    typedef typename array_t::const_iterator    const_iterator;

public:
    slice() : buf_(chunk_area) { }

    iterator       begin()       { return buf_.begin(); }
    const_iterator begin() const { return buf_.begin(); }
    iterator       end()         { return buf_.end();   }
    const_iterator end() const   { return buf_.end();   }
    bool           empty() const { return buf_.empty(); }
    void           clear()       { fill(buf_, t()); }

    value_type& operator[](map_index pos)
        { return operator()(pos.x, pos.y); }

    value_type& operator[](size_t pos)
        { assert(pos < size()); return buf_[pos]; }

    value_type  operator[](map_index pos) const
        { return operator()(pos.x, pos.y); }

    value_type  operator[](size_t pos) const
        { assert(pos < size()); return buf_[pos]; }

    value_type& operator()(uint8_t x, uint8_t y)
    {
        assert (x < chunk_size);
        assert (y < chunk_size);
        return buf_[x + y * chunk_size];
    }

    value_type  operator()(uint8_t x, uint8_t y) const
    {
        assert (x < chunk_size);
        assert (y < chunk_size);
        return buf_[x + y * chunk_size];
    }

    void resize(size_t dummy)
        { assert (dummy == chunk_area); }

    size_t size() const
        { return chunk_area; }
};

/** An intermediate representation of a chunk surface.
 *  This is a combination of a surface and a light map, with the textures
 *  already looked up in the client's table.  If supported by the renderer,
 *  this object can be further processed into an \a optimized_render_surface.
 */
class render_surface
{
public:
    struct element
    {
        element(): texture(0), light_sun(0), light_amb(0), light_art(0) { }

        uint16_t    texture;
        uint8_t     light_sun;
        uint8_t     light_amb;
        uint8_t     light_art;
    };

    element& operator() (chunk_index i, uint8_t dir)
    {
        chunk_index p (0,0,0);

        switch (dir)
        {
            case 0: p = chunk_index( i.y, i.z, i.x); break;
            case 1: p = chunk_index( 15 - i.y, i.z, 15 - i.x); break;
            case 2: p = chunk_index( 15 - i.x, i.z, i.y); break;
            case 3: p = chunk_index( i.x, i.z, 15 - i.y); break;
            case 4: p = chunk_index( i.x, i.y, i.z); break;
            case 5: p = chunk_index( i.x, 15 - i.y, 15 - i.z); break;
            default: assert(false);
        }

        return dirs[dir][p.z][map_index(p)];
    }

    typedef slice<element> layer;
    //typedef boost::container::flat_map<uint8_t, layer> layers;
    typedef flat_map<uint8_t, layer> layers;
    std::array<layers, 6> dirs;
};

/** A render surface with similiar faces merged into larger rectangles.
 *  If the client supports 3D texture arrays, we can make good use of this
 *  by merging faces with the same texture and light levels into larger
 *  rectangles. */
class optimized_render_surface
{
public:
    struct element : public render_surface::element
    {
        element () : size (1, 1) { }

        element (const render_surface::element& init)
            : render_surface::element (init)
            , size (1, 1)
        { }

        bool operator== (const render_surface::element& comp) const
            { return texture == comp.texture && light_sun == comp.light_sun && light_amb == comp.light_amb && light_art == comp.light_art; }

        bool operator!= (const render_surface::element& comp) const
            { return !operator==(comp); }

        vector2<uint8_t> size;
    };

    typedef std::unordered_map<map_index, element> layer;
    //typedef boost::container::flat_map<uint8_t, layer> layers;
    typedef flat_map<uint8_t, layer> layers;

    std::array<layers, 6> dirs;
};

/** Merge similar faces of a render_surface into rectangles using a greedy
 ** algorithm. */
optimized_render_surface
optimize_greedy (const render_surface& in);

} // namespace hexa


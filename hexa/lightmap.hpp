//---------------------------------------------------------------------------
/// \file   lightmap.hpp
/// \brief  The lightmap for a chunk of terrain.
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

#include <memory>
#include "basic_types.hpp"
#include "chunk_base.hpp"
#include "pos_dir.hpp"
#include "serialize.hpp"

namespace hexa {

#pragma pack(push, 1)

/** The light values of a single block face.
 *  There are three channels: sunlight, ambient light, and artificial
 *  light.  Every channel has an intensity value of 0..15.  The final
 *  color of every block face is the sum of all light channels.  The
 *  actual color that is blended in is determined at runtime; artificial
 *  light usually stays white(-ish), but sunlight fades between yellow,
 *  red, and black during a day-night cycle.  Ambient, the light that
 *  is emitted by the skydome, fades between bright blue at day, and a
 *  very dark blue at night. */
struct light
{
    uint8_t     sunlight    : 4; /**< Sunlight intensity. */
    uint8_t     ambient     : 4; /**< Ambient (or skylight) intensity. */
    uint8_t     artificial  : 4; /**< Artificial light sources. */
    uint8_t     padding     : 4; /**< Pad struct to 2 bytes. */

    light() : sunlight(0), ambient(0), artificial(0), padding(0) { }

    light(uint8_t art)
        : sunlight(0), ambient(0), artificial(art), padding(0) { }

    light(uint8_t sun, uint8_t amb, uint8_t art)
        : sunlight(sun), ambient(amb), artificial(art), padding(0) { }

    bool    operator== (const light& comp) const
        { return   sunlight   == comp.sunlight
                && ambient    == comp.ambient
                && artificial == comp.artificial
                && padding    == comp.padding;
        }

    bool    operator!= (const light& comp) const
        { return !operator==(comp); }

    template <class archive>
    archive& serialize(archive& ar)
    {
        uint16_t& tmp (*(uint16_t*)this);
        return ar(tmp);
    }
};

#pragma pack(pop)

/** The light map of a chunk.
 *  This is a simple array of light values.  The position and direction of
 *  each element is determined by the chunk's \ref hexa::surface "surface";
 *  every face in the surface has a corresponding element in the lightmap
 *  array. */
class lightmap
{
    typedef std::vector<light>  data_t;

public:
    typedef data_t::value_type      value_type;
    typedef data_t::iterator        iterator;
    typedef data_t::const_iterator  const_iterator;

    data_t  data;

    iterator       begin()        { return data.begin(); }
    const_iterator begin() const  { return data.begin(); }
    iterator       end()          { return data.end(); }
    const_iterator end() const    { return data.end(); }

    size_t  size() const                 { return data.size();   }
    bool    empty() const                { return data.empty();   }
    void    emplace_back(value_type&& v) { data.emplace_back(v); }
    void    push_back(value_type v)      { data.push_back(v);    }
    void    resize(size_t s)             { data.resize(s);       }

    bool    operator== (const lightmap& comp) const
        { return data == comp.data; }

    bool    operator!= (const lightmap& comp) const
        { return !operator==(comp); }

    template <class archive>
    archive& serialize(archive& ar)
    {
        ///\todo Serialize lightmaps properly
        return ar(data);
    }
};


/** The combined light maps of the opaque and transparent surfaces. */
class light_data
{
public:
    lightmap    opaque;      /**< Opaque surfaces. */
    lightmap    transparent; /**< Transparent surfaces. */

    /** The quality of the light map.
     *  Calculating a light map is expensive.  To make sure the game keeps
     *  running smoothly, this is done in phases.  How many phases depends
     *  on the used light map generators, but this is usually three.  The
     *  first phase is very coarse, with only a few short rays for sunlight
     *  and ambient.  As time passes, the lightmap gets refined gradually
     *  in the next phases.  There's a chance the player will see the light
     *  map change, but this is usually not a problem (and pretty much
     *  unavoidable). */
    uint16_t    phase;

public:
    light_data() : phase(0) { }
    light_data(lightmap o, lightmap t) : opaque(o), transparent(t), phase(0) { }

    bool empty() const { return opaque.empty() && transparent.empty(); }

    template <class archive>
    archive& serialize(archive& ar)
        { return ar(opaque)(transparent)(phase); }
};

/** Reference counted pointer for light data. */
typedef std::shared_ptr<light_data> lightmap_ptr;

} // namespace hexa


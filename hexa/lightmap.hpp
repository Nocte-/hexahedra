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

namespace hexa
{

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
    uint8_t sunlight : 4;   /**< Sunlight intensity. */
    uint8_t ambient : 4;    /**< Ambient (or skylight) intensity. */
    uint8_t artificial : 4; /**< Artificial light sources. */
    uint8_t secondary : 4;  /**< Secondary light source. */

    light()
        : sunlight{0}
        , ambient{0}
        , artificial{0}
        , secondary{0}
    {
    }

    light(uint8_t overall)
        : sunlight{overall}
        , ambient{overall}
        , artificial{overall}
        , secondary{overall}
    {
    }

    light(uint8_t sun, uint8_t amb, uint8_t art, uint8_t sec = 0)
        : sunlight{sun}
        , ambient{amb}
        , artificial{art}
        , secondary{sec}
    {
    }

    bool operator==(const light& comp) const
    {
        return sunlight == comp.sunlight && ambient == comp.ambient
               && artificial == comp.artificial
               && secondary == comp.secondary;
    }

    bool operator!=(const light& comp) const { return !operator==(comp); }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        uint16_t& tmp(*(uint16_t*)this);
        return ar(tmp);
    }
};

/** High-resolution light intensities of a block face.
 *  This is the 8-bit version of \a light.  It is only used server-side.
 *  It holds a more detailed version of the client-side lightmap, as well
 *  as separate radiosity values for all four channels. */
struct light_hr
{
    uint8_t sunlight;   /**< Sunlight intensity. */
    uint8_t ambient;    /**< Ambient (or skylight) intensity. */
    uint8_t artificial; /**< Artificial light sources. */
    uint8_t secondary ; /**< Secondary light sources. */

    uint8_t r_sunlight;   /**< Sunlight radiosity. */
    uint8_t r_ambient;    /**< Ambient (or skylight)radiosity . */
    uint8_t r_artificial; /**< Artificial light source radiosity. */
    uint8_t r_secondary ; /**< Secondary light source radiosity. */

    light_hr()
        : sunlight{0}
        , ambient{0}
        , artificial{0}
        , secondary{0}
        , r_sunlight{0}
        , r_ambient{0}
        , r_artificial{0}
        , r_secondary{0}
    {
    }

    light_hr(uint8_t sun, uint8_t amb, uint8_t art, uint8_t sec = 0)
        : sunlight{sun}
        , ambient{amb}
        , artificial{art}
        , secondary{sec}
        , r_sunlight{0}
        , r_ambient{0}
        , r_artificial{0}
        , r_secondary{0}
    {
    }

    bool operator==(const light& comp) const
    {
        return sunlight == comp.sunlight && ambient == comp.ambient
               && artificial == comp.artificial
               && secondary == comp.secondary;
    }

    bool operator!=(const light& comp) const { return !operator==(comp); }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar(sunlight)(ambient)(artificial)(secondary)(
                 r_sunlight)(r_ambient)(r_artificial)(r_secondary);
    }
};

#pragma pack(pop)

/** The light map of a chunk.
 *  This is a simple array of light values.  The position and direction of
 *  each element is determined by the chunk's \ref hexa::surface "surface";
 *  every face in the surface has a corresponding element in the lightmap
 *  array. */
template <typename T>
class lightmap_base
{
    typedef std::vector<T> data_t;

public:
    typedef T value_type;
    typedef typename data_t::iterator iterator;
    typedef typename data_t::const_iterator const_iterator;

public:
    lightmap_base() {}
    lightmap_base(const lightmap_base&) = default;

    lightmap_base(lightmap_base&& m)
        : data{std::move(m.data)}
    {
    }

    lightmap_base& operator=(lightmap_base&& m)
    {
        if (&m != this)
            data = std::move(m.data);

        return *this;
    }

    iterator begin() { return data.begin(); }
    const_iterator begin() const { return data.begin(); }
    iterator end() { return data.end(); }
    const_iterator end() const { return data.end(); }

    size_t size() const { return data.size(); }
    bool empty() const { return data.empty(); }
    void emplace_back(value_type&& v) { data.emplace_back(v); }
    void push_back(value_type v) { data.push_back(v); }
    void resize(size_t s) { data.resize(s); }

    bool operator==(const lightmap_base& comp) const { return data == comp.data; }

    bool operator!=(const lightmap_base& comp) const { return !operator==(comp); }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar(data);
    }

public:
    data_t data;
};

using lightmap = lightmap_base<light>;
using lightmap_hr = lightmap_base<light_hr>;

/** The combined light maps of the opaque and transparent surfaces. */
template <typename T>
class light_data_base
{
public:
    T opaque;      /**< Opaque surfaces. */
    T transparent; /**< Transparent surfaces. */

    /** The quality of the light map.
     *  Calculating a light map is expensive.  To make sure the game keeps
     *  running smoothly, this is done in phases.  How many phases depends
     *  on the used light map generators, but this is usually three.  The
     *  first phase is very coarse, with only a few short rays for sunlight
     *  and ambient.  As time passes, the lightmap gets refined gradually
     *  in the next phases.  There's a chance the player will see the light
     *  map change, but this is usually not a problem (and pretty much
     *  unavoidable). */
    uint16_t phase;

public:
    light_data_base()
        : phase{0}
    {
    }

    light_data_base(const light_data_base&) = default;

    light_data_base(light_data_base&& m)
        : opaque{std::move(m.opaque)}
        , transparent{std::move(m.transparent)}
        , phase{m.phase}
    {
    }

    light_data_base(T&& o, T&& t)
        : opaque{std::move(o)}
        , transparent{std::move(t)}
        , phase{0}
    {
    }

    light_data_base& operator=(light_data_base&& m)
    {
        if (&m != this) {
            opaque = std::move(m.opaque);
            transparent = std::move(m.transparent);
        }
        return *this;
    }

    light_data_base& operator=(const light_data_base&) = default;

    bool empty() const { return opaque.empty() && transparent.empty(); }

    template <typename Archive>
    Archive& serialize(Archive& ar)
    {
        return ar(opaque)(transparent)(phase);
    }
};

using light_data = light_data_base<lightmap>;
using light_data_hr = light_data_base<lightmap_hr>;

typedef std::shared_ptr<light_data> lightmap_ptr;
typedef std::shared_ptr<light_data_hr> lightmap_hr_ptr;

} // namespace hexa

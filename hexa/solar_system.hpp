//---------------------------------------------------------------------------
/// \file   solar_system.hpp
/// \brief  Simplified solar system for drawing the day and night sky.
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

#include <vector>
#include "basic_types.hpp"
#include "matrix.hpp"

namespace hexa
{

/** A circular celestial orbit. */
class orbit
{
public:
    orbit() {}

    /** Constructor.
     * @param inclination  Inclination in radians.
     * @param lon_asc      Longitude of the ascending node, in radians.
     * @param radius       Radius in astronomical units.
     * @param angular_velocity Orbital velocity in radians per time unit.
     * @param angular_offset   Offset in radians. */
    orbit(double inclination, double lon_asc, double radius,
          double angular_velocity, double angular_offset);

    /** Get the position of the body at a given point in time. */
    vector position(double time) const;

private:
    double radius_;
    double angular_velocity_;
    double angular_offset_;
    matrix4<float> rot_;
};

/** Definition of a solar system, with a sun, a home planet, and any
 *  number of other moons and planets.
 *  Note that this is not meant to be an exact simulation. But hey, if
 *  it's good enough for Copernicus, it's good enough for us. */
class solar_system
{
public:
    /** A celesial body, either a planet or a moon. */
    struct body
    {
        double size;      /**< Radius in million blocks. */
        orbit trajectory; /**< Its orbit around the star or planet. */
    };

    struct observation
    {
        yaw_pitch position; /**< Azimuth and declination. */
        float app_size;     /**< Apparent size. */
        float phase;        /**< 0: new, 0.5: waxing, 1: full, 1.5: waning. */
    };

    /** Construct a solar system.
     * @param seconds_per_rotation  Rotation speed of the home planet.
     * @param gravitational_constant  Guassian gravitational constant. */
    solar_system(double seconds_per_rotation,
                 double gravitational_constant = 0.017202);

    /** Returns a prefab solar system a bit like our own. */
    static solar_system real_system();

    /** Add a planet.
     * @param planet_radius  Radius in millions of blocks.
     * @param orbit_radius   Orbit radius in astronomical units.
     * @param inclin         Inclination
     * @param lon_asc        Longitudal ascension
     * @param offset         Time offset */
    void add_planet(double planet_radius, double orbit_radius, double inclin,
                    double lon_asc, double offset = 0.0);

    /** Add a moon.
     * @param moon_radius    Radius in millions of blocks.
     * @param orbit_radius   Orbit radius in astronomical units.
     * @param inclin         Inclination
     * @param lon_asc        Longitudal ascension
     * @param offset         Time offset */
    void add_moon(double moon_radius, double orbit_radius, double inclin,
                  double lon_asc, double offset = 0.0);

    yaw_pitch observe_sun(double time) const;

    std::vector<observation> observe_planets(double time) const;

private:
    double sec_per_rotation_;
    orbit home_planet_orbit_;
    double k_;
    matrix4<double> latrot_;

    std::vector<body> planets_;
    std::vector<body> moons_;
};

} // namespace hexa

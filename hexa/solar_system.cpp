//---------------------------------------------------------------------------
// solar_system.cpp
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

#include "solar_system.hpp"

#include <cmath>

namespace hexa {

orbit::orbit (double inclination, double lon_asc, double radius,
              double angular_velocity, double angular_offset)
    : radius_   (radius)
    , angular_velocity_ (angular_velocity)
    , angular_offset_   (angular_offset)
    , rot_ (rotate_x<float>(inclination) * rotate_z<float>(lon_asc))
{ }

vector orbit::position (double time) const
{
    double angle (angular_offset_ + time * angular_velocity_);
    vector flat (std::sin(angle) * radius_,  std::cos(angle) * radius_, 0);
    return rot_ * flat;
}


solar_system::solar_system(double seconds_per_rotation,
                           double gravitational_constant)
    : home_planet_orbit_ (0, 0, 1.0, gravitational_constant, 0.0)
    , k_(gravitational_constant)
{ }


void solar_system::add_planet (double planet_radius, double orbit_radius,
                               double inclin, double lon_asc, double offset)
{
    double n (k_ * std::sqrt(1.0 / std::pow(orbit_radius, 3)));
    body planet;
    planet.size = planet_radius;
    planet.trajectory = orbit(inclin, lon_asc, orbit_radius, n, offset);

    planets_.push_back(planet);
}

void solar_system::add_moon (double moon_radius, double orbit_radius,
                             double inclin, double lon_asc, double offset)
{
    double n (k_ * std::sqrt(1.0 / std::pow(orbit_radius, 3)));
    body moon;
    moon.size = moon_radius;
    moon.trajectory = orbit(inclin, lon_asc, orbit_radius, n, offset);

    moons_.push_back(moon);
}

yaw_pitch solar_system::observe_sun (double time) const
{
    auto pos (-home_planet_orbit_.position(time));
    return to_spherical(pos);
}

std::vector<solar_system::observation>
solar_system::observe_planets (double time) const
{
    std::vector<observation> result;
    result.reserve(planets_.size());

    auto rel (home_planet_orbit_.position(time));
    for (const body& p : planets_)
    {
        auto pos (p.trajectory.position(time));
        auto sph (to_spherical(pos - rel));

        observation obs;
        obs.position = sph;
        obs.app_size = std::atan2(sph.z, p.size * 0.00001);
        obs.phase = std::acos(dot_prod(normalize(pos), normalize(rel)));

        result.push_back(obs);
    }
    return result;
}

solar_system solar_system::real_system()
{
    solar_system sol (86164.0905);

    sol.add_planet(2.4, 0.4, 0.12217, 0.83775); // Mercury
    sol.add_planet(6.0, 0.7, 0.05934, 1.32645); // Venus
    sol.add_planet(3.4, 1.5, 0.03228, 0.86393); // Mars
    sol.add_planet(70., 5.2, 0.02268, 1.74532); // Jupiter
    sol.add_planet(60., 9.5, 0.04328, 1.98339); // Saturn

    sol.add_moon(1.7, 0.0025, 0.089709, 0); // Luna

    return sol;
}

} // namespace hexa


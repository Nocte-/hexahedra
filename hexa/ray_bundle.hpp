//---------------------------------------------------------------------------
/// \file   ray_bundle.hpp
/// \brief  Aggregation of several rays
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
#include "ray.hpp"

namespace hexa {

/** A bundle of rays through voxels.
 *  This is used often in light calculations, where several rays have a
 *  common starting point.  A lot of rays will pass through the same voxel
 *  at first, before diverging later on.  This class bundles such groups of
 *  rays in a tree structure, so that every voxel has to be traversed only
 *  once, and large parts of the ray-tree can be skipped if a voxel is
 *  blocking them. */
class ray_bundle
{
public:
    typedef std::vector<world_vector>   value_type;

    /** Every part of the tree has a weight, usually it relates linearly
     *  to the number of rays in this branch. */
    float                   weight;
    /** The voxels that all rays traverse in this section of the tree. */
    value_type              trunk;
    /** After going through all voxels in \a trunk, the rays diverge into
     ** different branches. */
    std::vector<ray_bundle> branches;

public:
    ray_bundle() : weight(0) {}

    ray_bundle(value_type ray, float w)
        : weight (w)
        , trunk  (ray)
    { }

    /** Add a ray with a given weight to the bundle. */
    void add (value_type ray, float weight);

    /** Normalize the weights across the tree, so the root has a weight of
     ** 1. */
    void normalize_weight();

    /** Multiply all weights in the tree by a given value. */
    void multiply_weight(float factor);

    bool operator==(world_vector comp) const
        { return trunk.front() == comp; }
};

} // namespace hexa


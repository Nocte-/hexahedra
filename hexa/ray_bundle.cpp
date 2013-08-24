//---------------------------------------------------------------------------
// lib/ray_bundle.cpp
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

#include "ray_bundle.hpp"
#include <boost/range/algorithm.hpp>

using namespace std;
using namespace boost::range;

namespace hexa {

void ray_bundle::add(ray_bundle::value_type ray, float w)
{
    // If this is the first ray in the bundle, just add it.
    if (trunk.empty() && branches.empty())
    {
        trunk = move(ray);
        weight = w;
        return;
    }

    // If not, merge it.
    auto cursor (begin(ray));
    auto j      (begin(trunk));

    // Follow the ray along the trunk, and stop when we reach the end
    // of either the trunk or the ray, or when the ray diverges from
    // the trunk.
    while (j != end(trunk) && cursor != end(ray) && *j == *cursor)
    {
        ++j;
        ++cursor;
    }

    ray.erase(begin(ray), cursor);

    if (j == end(trunk))
    {
        // The end of the trunk was reached.  The ray's weight also
        // counts for the total weight.
        weight += w;

        if (!ray.empty())
        {
            // There's still a bit of ray left.  See if it continues
            // along one of the branches.
            auto k (find(branches, ray[0]));
            if (k == end(branches))
            {
                // Nope.  The remaining ray becomes a new branch.
                branches.emplace_back(ray_bundle(move(ray), w));
            }
            else
            {
                // It does.  Recurse down this branch.
                k->add(move(ray), w);
            }
        }
    }
    else
    {
        // The ray diverged from the trunk halfway.  Fork the trunk at
        // this point.  One branch is the rest of the trunk, the other
        // is the remaining part of the ray.
        ray_bundle split (value_type(j, end(trunk)), weight);
        std::swap(branches, split.branches);
        trunk.erase(j, end(trunk));
        branches.emplace_back(move(split));
        weight += w;

        if (!ray.empty())
            branches.emplace_back(ray_bundle(move(ray), w));
    }
}

void ray_bundle::normalize_weight()
{
    if (trunk.empty() || weight == 0)
        weight = 0;
    else
        multiply_weight(1.0f / weight);
}

void ray_bundle::multiply_weight(float factor)
{
    weight *= factor;
    for (auto& branch : branches) 
        branch.multiply_weight(factor);
}

} // namespace hexa


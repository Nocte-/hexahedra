//---------------------------------------------------------------------------
// server/terrain/transmute.cpp
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
// Copyright 2014, nocte@hippie.nu
//---------------------------------------------------------------------------

#include "transmute.hpp"

#include <boost/tokenizer.hpp>
#include <hexanoise/generator_opencl.hpp>
#include <hexanoise/generator_slowinterpreter.hpp>

#include <hexa/basic_types.hpp>
#include "../hndl.hpp"
#include "../random.hpp"

namespace hexa
{

transmute_generator::transmute_generator(
    world& w, const boost::property_tree::ptree& conf)
    : terrain_generator_i{w}
    , func_{compile_hndl(conf.get<std::string>("hndl"))}
{
    lu_table_.resize(65536);
    for (int unsigned i = 0; i < lu_table_.size(); ++i)
        lu_table_[i] = i;

    // Example of the JSON we're interpreting here:
    // 'replace': ['!air':'rock', 'water,lava':'snot',
    // 'wood.birch':'wood.jungle']
    //
    auto& list = conf.get_child("replace");
    for (auto& elem : list) {
        const std::string& source = elem.first.data();
        const std::string& target = elem.second.data();

        std::cout << "Replace " << source << ": " << target << "." << std::endl;
        
        if (source.empty())
            throw std::runtime_error(
                "transmute: 'replace' source material is empty");

        if (target.empty())
            throw std::runtime_error(
                "transmute: 'replace' target material is empty");

        auto target_material = find_material(elem.second.data());

        if (source[0] == '!') {
            auto exclude = find_material(source.substr(1));
            for (unsigned int i = 0; i < lu_table_.size(); ++i) {
                if (i != exclude)
                    lu_table_[i] = target_material;
            }
        } else if (source == "*") {
            std::fill(lu_table_.begin(), lu_table_.end(), target_material);
        } else {
            for (auto& m : boost::tokenizer<>{source})
                lu_table_[find_material(m)] = target_material;
        }
    }
}

void transmute_generator::generate(world_terraingen_access& data,
                                   const chunk_coordinates& pos, chunk& cnk)
{
    auto distrib = hndl_chunk(*func_, pos);

    for (int i = 0; i < chunk_volume; ++i) {
        if (distrib[i] > 0.0)
            cnk[i] = lu_table_[cnk[i]];
    }
}

} // namespace hexa

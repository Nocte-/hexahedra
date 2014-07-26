//---------------------------------------------------------------------------
/// \file   client/iqm_loader.hpp
/// \brief  Load an IQM (3D model and animations) file
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
// Copyright 2013-2014, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

#include <list>
#include <vector>
#include <boost/filesystem/path.hpp>
#include "model.hpp"

namespace hexa
{
namespace iqm
{

/** Load an IQM model from a memory buffer. */
model load_meshes(const std::vector<char>& buf);

/** Load IQM animations from a memory buffer. */
std::list<animation> load_anims(const std::vector<char>& buf);

/** Load a model and animations from an IQM file. */
std::pair<model, std::list<animation>>
load(const boost::filesystem::path& file);

} // namespace iqm
} // namespace hexa

//---------------------------------------------------------------------------
// persistent_storage_i.cpp
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

#include "persistent_storage_i.hpp"

namespace hexa
{

persistent_storage_i::raii_transaction::raii_transaction(
    persistent_storage_i& ref)
    : ref_(ref)
{
    ref_.begin_transaction();
}

persistent_storage_i::raii_transaction::~raii_transaction()
{
    ref_.end_transaction();
}

} // namespace hexa

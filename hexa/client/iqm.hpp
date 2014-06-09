//---------------------------------------------------------------------------
/// \file   client/iqm.hpp
/// \brief  Data structures for the IQM file format
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
// Copyright 2013, nocte@hippie.nu
//---------------------------------------------------------------------------
#pragma once

namespace hexa
{
namespace iqm
{

struct header
{
    char magic[16];
    uint32_t version;
    uint32_t filesize;
    uint32_t flags;
    uint32_t num_text, ofs_text;
    uint32_t num_meshes, ofs_meshes;
    uint32_t num_vertexarrays, num_vertices, ofs_vertexarrays;
    uint32_t num_triangles, ofs_triangles, ofs_adjacency;
    uint32_t num_joints, ofs_joints;
    uint32_t num_poses, ofs_poses;
    uint32_t num_anims, ofs_anims;
    uint32_t num_frames, num_framechannels, ofs_frames, ofs_bounds;
    uint32_t num_comment, ofs_comment;
    uint32_t num_extensions, ofs_extensions;
};

struct mesh
{
    uint32_t name;
    uint32_t material;
    uint32_t first_vertex, num_vertices;
    uint32_t first_triangle, num_triangles;
};

enum field_type {
    position = 0,
    texcoord = 1,
    normal = 2,
    tangent = 3,
    blendindices = 4,
    blendweights = 5,
    color = 6,
    custom = 0x10
};

enum data_type {
    byte_t = 0,
    ubyte_t = 1,
    short_t = 2,
    ushort_t = 3,
    int_t = 4,
    uint_t = 5,
    half_t = 6,
    float_t = 7,
    double_t = 8
};

typedef std::array<uint32_t, 3> triangle;

struct joint
{
    uint32_t name;
    int parent;
    float translate[3], rotate[4], scale[3];
};

struct pose
{
    int parent;
    uint32_t mask;
    float channeloffset[10];
    float channelscale[10];
};

struct anim
{
    uint32_t name;
    uint32_t first_frame, num_frames;
    float framerate;
    uint32_t flags;
};

enum flag_type { loop = 1 << 0 };

struct vertexarray
{
    uint32_t type;
    uint32_t flags;
    uint32_t format;
    uint32_t size;
    uint32_t offset;
};

struct bounds
{
    float bbmin[3], bbmax[3];
    float xyradius, radius;
};

struct extension
{
    uint32_t name;
    uint32_t num_data, ofs_data;
    uint32_t ofs_extensions;
};

} // namespace iqm
} // namespace hexa

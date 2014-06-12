//---------------------------------------------------------------------------
// client/iqm_loader.cpp
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

#include "iqm_loader.hpp"

#include <boost/filesystem/operations.hpp>
#include "iqm.hpp"

namespace hexa
{
namespace iqm
{

model load_meshes(const std::vector<char>& buf)
{
    const header& hdr = *(header*)&buf[0];

    const float* position = nullptr;
    const float* normal = nullptr;
    const float* tex_coord = nullptr;
    const uint8_t* blend_indices = nullptr;
    const uint8_t* blend_weights = nullptr;
    const uint8_t* colors = nullptr;

    const vertexarray* vas = (vertexarray*)&buf[hdr.ofs_vertexarrays];
    for (size_t i = 0; i < hdr.num_vertexarrays; ++i) {
        const vertexarray& va = vas[i];

        switch (va.type) {
        case field_type::position:
            if (va.format != float_t || va.size != 3)
                throw std::runtime_error(
                    "iqm::read: positions must be 3 floats");

            position = (float*)&buf[va.offset];
            break;

        case field_type::normal:
            if (va.format != float_t || va.size != 3)
                throw std::runtime_error(
                    "iqm::read: normals must be 3 floats");

            normal = (float*)&buf[va.offset];
            break;

        case field_type::texcoord:
            if (va.format != float_t || va.size != 2)
                throw std::runtime_error(
                    "iqm::read: texture coordinates must be 2 floats");

            tex_coord = (float*)&buf[va.offset];
            break;

        case field_type::blendindices:
            if (va.format != ubyte_t || va.size != 4)
                throw std::runtime_error(
                    "iqm::read: blend indices must be 4 bytes");

            blend_indices = (uint8_t*)&buf[va.offset];
            break;

        case field_type::blendweights:
            if (va.format != ubyte_t || va.size != 4)
                throw std::runtime_error(
                    "iqm::read: blend weights must be 4 bytes");

            blend_weights = (uint8_t*)&buf[va.offset];
            break;

        case field_type::color:
            if (va.format != ubyte_t || va.size != 3)
                throw std::runtime_error("iqm::read: colors must be 3 bytes");

            colors = (uint8_t*)&buf[va.offset];
            break;
        }
    }

    model result;

    const mesh* meshes = (mesh*)&buf[hdr.ofs_meshes];

    static const float defaultnormal[3] = {0.0f, 0.0f, 0.0f};
    static const uint8_t defaultblend[4] = {0, 0, 0, 0};
    static const uint8_t defaultcolor[3] = {255, 0, 0};

    if (tex_coord != nullptr) {
        result.vertex_buffer.resize(hdr.num_vertices);
        for (size_t i = 0; i < hdr.num_vertices; ++i) {
            result.vertex_buffer[i] = model::vertex(
                position + i * 3, tex_coord + i * 2,
                normal ? normal + i * 3 : defaultnormal,
                blend_indices ? blend_indices + i * 4 : defaultblend,
                blend_weights ? blend_weights + i * 4 : defaultblend);
        }
    } else {
        result.vertex_buffer_col.resize(hdr.num_vertices);
        for (size_t i = 0; i < hdr.num_vertices; ++i) {
            result.vertex_buffer_col[i] = model::vertex_col(
                position + i * 3, colors ? colors + i * 3 : defaultcolor,
                normal ? normal + i * 3 : defaultnormal,
                blend_indices ? blend_indices + i * 4 : defaultblend,
                blend_weights ? blend_weights + i * 4 : defaultblend);
        }
    }

    const triangle* tris{(triangle*)&buf[hdr.ofs_triangles]};
    result.edge_buffer.resize(hdr.num_triangles);
    for (size_t i = 0; i < hdr.num_triangles; ++i) {
        for (uint16_t j{0}; j < 3; ++j)
            result.edge_buffer[i][j] = static_cast<uint16_t>(tris[i][2 - j]);
    }

    const char* str{(char*)&buf[hdr.ofs_text]};
    for (size_t i = 0; i < hdr.num_meshes; ++i) {
        auto& m(meshes[i]);
        result.meshes.emplace_back(m.first_triangle, m.num_triangles,
                                   &str[m.material]);
    }

    const joint* joints{(joint*)&buf[hdr.ofs_joints]};
    for (size_t i = 0; i < hdr.num_joints; ++i) {
        const joint& j(joints[i]);

        result.bones.emplace_back(
            vector3<float>(j.translate), quaternion<float>(j.rotate),
            vector3<float>(j.scale), j.parent, std::string(&str[j.name]));
    }

    return result;
}

std::list<animation> load_anims(const std::vector<char>& buf)
{
    std::list<animation> result;
    const header& hdr(*(header*)&buf[0]);

    const char* str{(char*)&buf[hdr.ofs_text]};
    const anim* anims{(anim*)&buf[hdr.ofs_anims]};
    const pose* poses{(pose*)&buf[hdr.ofs_poses]};

    // std::vector<matrix3x4<float>> frames (hdr.num_frames * hdr.num_poses);
    uint16_t* framedata((uint16_t*)&buf[hdr.ofs_frames]);

    for (size_t i{0}; i < hdr.num_anims; ++i) {
        const anim& a(anims[i]);
        animation new_anim;

        new_anim.name = &str[a.name];
    }

    for (size_t i{0}; i < hdr.num_frames; ++i) {
        for (size_t j{0}; j < hdr.num_poses; ++j) {
            const pose& p(poses[j]);

            vector3<float> translate(p.channeloffset);
            quaternion<float> rotate(p.channeloffset + 3);
            vector3<float> scale(p.channeloffset + 7);

            if (p.mask & 0x01)
                translate.x += *framedata++ * p.channelscale[0];
            if (p.mask & 0x02)
                translate.y += *framedata++ * p.channelscale[1];
            if (p.mask & 0x04)
                translate.z += *framedata++ * p.channelscale[2];

            if (p.mask & 0x08)
                rotate.x += *framedata++ * p.channelscale[3];
            if (p.mask & 0x10)
                rotate.y += *framedata++ * p.channelscale[4];
            if (p.mask & 0x20)
                rotate.z += *framedata++ * p.channelscale[5];
            if (p.mask & 0x40)
                rotate.w += *framedata++ * p.channelscale[6];

            if (p.mask & 0x80)
                scale.x += *framedata++ * p.channelscale[7];
            if (p.mask & 0x100)
                scale.y += *framedata++ * p.channelscale[8];
            if (p.mask & 0x200)
                scale.z += *framedata++ * p.channelscale[9];

            // Concatenate each pose with the inverse base pose to avoid doing
            // this at animation time.
            // If the joint has a parent, then it needs to be pre-concatenated
            // with its parent's base pose.
            // Thus it all negates at animation time like so:
            //   (parentPose * parentInverseBasePose) * (parentBasePose *
            //   childPose * childInverseBasePose) =>
            //   parentPose * (parentInverseBasePose * parentBasePose) *
            //   childPose * childInverseBasePose =>
            //   parentPose * childPose * childInverseBasePose

            matrix3x4<float> m(rotation_matrix(normalize(rotate))
                               * matrix3<float>::diagonal(scale),
                               translate);
            //            m *= inversebaseframe[j];

            //            if(p.parent >= 0)
            //                frames[i*hdr.num_poses + j] = baseframe[p.parent]
            //                * m ;
            //            else
            //                frames[i*hdr.num_poses + j] = m;
        }
    }

    return result;
}

std::pair<model, std::list<animation>>
load(const boost::filesystem::path& file)
{
    auto file_size(boost::filesystem::file_size(file));
    if (file_size < sizeof(header))
        throw std::runtime_error("iqm::read: file too small");

    if (file_size > 16000000)
        throw std::runtime_error("iqm::read: file too large");

    std::ifstream str(file.string(), std::ios::binary);
    if (!str)
        throw std::runtime_error("iqm::read: cannot open file");

    std::vector<char> buf;
    buf.resize(file_size + 1);
    str.read(&buf[0], sizeof(header));

    const header& hdr(*(header*)&buf[0]);
    if (!std::equal(std::begin(hdr.magic), std::end(hdr.magic),
                    "INTERQUAKEMODEL"))
        throw std::runtime_error("iqm::read: not an IQM file");

    if (hdr.filesize > file_size)
        throw std::runtime_error("iqm::read: file is incomplete");

    if (hdr.ofs_vertexarrays + hdr.num_vertexarrays * sizeof(vertexarray)
        > hdr.filesize)
        throw std::runtime_error("iqm::read: vertex array out of bounds");

    if (hdr.ofs_meshes + hdr.num_meshes * sizeof(mesh) > hdr.filesize)
        throw std::runtime_error("iqm::read: mesh array out of bounds");

    if (hdr.ofs_triangles + hdr.num_triangles * sizeof(triangle)
        > hdr.filesize)
        throw std::runtime_error("iqm::read: triangle array out of bounds");

    auto remaining(hdr.filesize - sizeof(hdr));
    str.read(&buf[sizeof(header)], remaining);
    buf[hdr.filesize] = 0;

    return std::make_pair(load_meshes(buf), load_anims(buf));
}
}
} // namespace hexa::iqm

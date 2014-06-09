//---------------------------------------------------------------------------
/// \file   server/voxel_shapes.hpp
/// \brief  All kinds of geometric solids used for terrain generation
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
#pragma once

#include <cmath>
#include <initializer_list>
#include <memory>
#include <set>
#include <hexa/aabb.hpp>
#include <hexa/algorithm.hpp>
#include <hexa/basic_types.hpp>
#include <hexa/plane3d.hpp>
#include <hexa/quaternion.hpp>
#include <hexa/voxel_range.hpp>

namespace hexa
{
namespace csg
{

class shape
{
public:
    virtual ~shape() {}

    virtual aabb<vec3f> bounding_box() const = 0;

    virtual bool is_inside(const vec3f& p) const = 0;

    virtual std::set<world_vector> chunks() const;
};

class sphere : public shape
{
public:
public:
    sphere(vec3f center, float radius);

    aabb<vec3f> bounding_box() const override;

    virtual std::set<world_vector> chunks() const override;

    virtual bool is_inside(const vec3f& p) const override;

protected:
    bool intersects(const aabb<vec3f>& box) const;

protected:
    vec3f center_;
    float radius_;
    float sq_radius_;
};

class ellipsoid : public sphere
{
public:
    ellipsoid(const vec3f& center, const vec3f& radii,
              const yaw_pitch& direction);
};

class axis_aligned_box : public shape
{
public:
    axis_aligned_box(const aabb<vec3f>& box);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

protected:
    aabb<vec3f> box_;
};

class cuboid : public shape
{
public:
    cuboid(const vec3f& center, const vec3f& sizes,
           const quaternion<float>& rotation);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

protected:
    vec3f center_;
    vec3f sizes_;
    quaternion<float> rot_;
    aabb<vec3f> bbox_;
};

class cylinder : public shape
{
public:
    cylinder(const vec3f& pt1, const vec3f& pt2, float radius);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

protected:
    vec3f pt1_;
    vec3f pt2_;
    float sqlength_;
    float radius_;
    float sqradius_;
    aabb<vec3f> bbox_;
};

class truncated_cone : public shape
{
public:
    truncated_cone(const vec3f& pt1, float radius1, const vec3f& pt2,
                   float radius2);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

protected:
    vec3f pt1_;
    vec3f pt2_;
    vec3f norm_axis_;
    float length_;
    float radius1_;
    float radius_delta_;
    aabb<vec3f> bbox_;
};

class plane : public shape
{
public:
    plane(const vec3f& normal, const vec3f& pt);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& pt) const override;

protected:
    plane3d<float> p_;
};

//---------------------------------------------------------------------------

#ifndef _MSC_VER

class union_shape : public shape
{
public:
    union_shape(std::initializer_list<std::unique_ptr<shape>> shapes);

    virtual std::set<world_vector> chunks() const override;

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

private:
    std::vector<std::unique_ptr<shape>> shapes_;
    aabb<vec3f> bbox_;
};

class difference_shape : public shape
{
public:
    difference_shape(std::unique_ptr<shape>&& a, std::unique_ptr<shape>&& b);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

private:
    std::unique_ptr<shape> a_;
    std::unique_ptr<shape> b_;
};

class intersection_shape : public shape
{
public:
    intersection_shape(std::unique_ptr<shape>&& a, std::unique_ptr<shape>&& b);

    virtual aabb<vec3f> bounding_box() const override;

    virtual bool is_inside(const vec3f& p) const override;

private:
    std::unique_ptr<shape> a_;
    std::unique_ptr<shape> b_;
    aabb<vec3f> bbox_;
};

#endif
}
} // namespace hexa::csg

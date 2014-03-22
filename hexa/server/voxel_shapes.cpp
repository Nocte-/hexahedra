//---------------------------------------------------------------------------
// server/voxel_shapes.cpp
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

#include "voxel_shapes.hpp"

namespace hexa {
namespace csg {

std::set<world_vector>
shape::chunks() const
{
    std::set<world_vector> result;
    aabb<vec3i> span (cast_to<vec3i>(bounding_box()) >> cnkshift);
    for (auto p : range<vec3i>(span))
        result.insert(world_vector(p));

    return result;
}



sphere::sphere(vec3f center, float radius)
    : center_(center)
    , radius_(std::abs(radius))
    , sq_radius_(radius * radius)
{
}

aabb<vec3f>
sphere::bounding_box() const
{
    vec3f r (radius_);
    return { center_ - r, center_ + r };
}

bool
sphere::intersects (const aabb<vec3f>& box) const
{
    float dmin (0.0f);
    for (int i = 0; i < 3; ++i)
    {
        if (center_[i] < box.first[i])
            dmin += square(center_[i] - box.first[i]);
        else if (center_[i] > box.second[i])
            dmin += square(center_[i] - box.second[i]);
    }
    return dmin <= sq_radius_;
}

std::set<world_vector>
sphere::chunks() const
{
    std::set<world_vector> result;
    sphere tiny (center_ / (float)chunk_size, radius_ / (float)chunk_size);
    aabb<vec3i> span (cast_to<vec3i>(tiny.bounding_box()));

    for (auto p : range<vec3i>(span))
    {
        if (tiny.intersects(vec3f(p)))
            result.insert(p);
    }
    return result;
}

bool
sphere::is_inside (const vec3f& p) const
{
    return squared_distance(center_, p) < sq_radius_;
}




ellipsoid::ellipsoid (const vec3f& center,
                      const vec3f& radii,
                      const yaw_pitch& direction)
    : sphere (center, 1.0f)
{
}



axis_aligned_box::axis_aligned_box (const aabb<vec3f>& box)
    : box_(box)
{ }

aabb<vec3f> axis_aligned_box::bounding_box() const
{
    return box_;
}


bool axis_aligned_box::is_inside (const vec3f& p) const
{
    return hexa::is_inside(p, box_);
}




cuboid::cuboid (const vec3f& center, const vec3f& sizes,
                const quaternion<float>& rotation)
    : center_(center)
    , sizes_(sizes / 2.0f)
    , rot_(rotation)
    , bbox_(rot_ * sizes_ + center_, rot_ * -sizes_ + center)
{
}

aabb<vec3f> cuboid::bounding_box() const
{
    return bbox_;
}

bool cuboid::is_inside (const vec3f& p) const
{
    auto a (absolute(rot_ * (p - center_)));
    return a.x < sizes_.x && a.y < sizes_.y && a.z < sizes_.z;
}




cylinder::cylinder (const vec3f& pt1, const vec3f& pt2, float radius)
    : pt1_(pt1)
    , pt2_(pt2)
    , sqlength_(squared_distance(pt1, pt2))
    , radius_(std::abs(radius))
    , sqradius_(square(radius))
    , bbox_(aabb<vec3f>(pt1, radius) + aabb<vec3f>(pt2, radius))
{ }

aabb<vec3f> cylinder::bounding_box() const
{
    return bbox_;
}

bool cylinder::is_inside (const vec3f& p) const
{
    auto d (pt2_ - pt1_);
    auto pd (p - pt1_);

    auto dot (dot_prod(pd, d));
    if (dot < 0.0f || dot > sqlength_)
        return false;

    return (squared_length(pd) - square(dot) / sqlength_) < sqradius_;
}




truncated_cone::truncated_cone (const vec3f& pt1, float radius1, const vec3f& pt2, float radius2)
    : pt1_(pt1)
    , pt2_(pt2)
    , norm_axis_(normalize(pt2 - pt1))
    , length_(distance(pt1, pt2))
    , radius1_(std::abs(radius1))
    , radius_delta_((std::abs(radius2) - radius1_) / length_)
    , bbox_(aabb<vec3f>(pt1, radius1) + aabb<vec3f>(pt2, radius2))
{ }

aabb<vec3f> truncated_cone::bounding_box() const
{
    return bbox_;
}

bool truncated_cone::is_inside (const vec3f& p) const
{
    auto pd (p - pt1_);

    auto dist (dot_prod(pd, norm_axis_));
    if (dist < 0.0f || dist > length_)
        return false;

    auto cone_radius (radius_delta_ * dist + radius1_);
    return length(pd - norm_axis_ * dist) < cone_radius;
}



plane::plane (const vec3f& normal, const vec3f& pt)
    : p_(normal, pt)
{ }

aabb<vec3f> plane::bounding_box() const
{
    return aabb<vec3f>::initial();
}

bool plane::is_inside (const vec3f& pt) const
{
    return distance(p_, pt) < 0;
}



//---------------------------------------------------------------------------

#ifndef _MSC_VER

union_shape::union_shape(std::initializer_list<std::unique_ptr<shape>> shapes)
{
    for (auto& ptr : shapes_)
    {
        auto b (ptr->bounding_box());
        if (b.is_correct())
            bbox_ = bbox_ + b;
    }
}

std::set<world_vector>
union_shape::chunks() const
{
    std::set<world_vector> result;
    for (auto& p : shapes_)
    {
        auto m (p->chunks());
        std::merge(result.begin(), result.end(), m.begin(), m.end(),
                   std::inserter(result, result.begin()));
    }
    return result;
}

aabb<vec3f>
union_shape::bounding_box() const
{
    return bbox_;
}

bool
union_shape::is_inside (const vec3f& p) const
{
    return    hexa::is_inside(p, bounding_box())
           && any_of(shapes_, [&](const std::unique_ptr<shape>& s)
                                        { return s->is_inside(p); });
}



difference_shape::difference_shape(std::unique_ptr<shape>&& a,
                 std::unique_ptr<shape>&& b)
    : a_(std::move(a))
    , b_(std::move(b))
{
}

aabb<vec3f>
difference_shape::bounding_box() const
{
    return a_->bounding_box();
}

bool
difference_shape::is_inside (const vec3f& p) const
{
    return    hexa::is_inside(p, bounding_box())
           && a_->is_inside(p) && !b_->is_inside(p);
}



intersection_shape::intersection_shape(std::unique_ptr<shape>&& a,
                   std::unique_ptr<shape>&& b)
    : a_(std::move(a))
    , b_(std::move(b))
    , bbox_(intersection(a_->bounding_box(), b_->bounding_box()))
{
}

aabb<vec3f>
intersection_shape::bounding_box() const
{
    return bbox_;
}

bool
intersection_shape::is_inside (const vec3f& p) const
{
    return    hexa::is_inside(p, bounding_box())
           && a_->is_inside(p) && b_->is_inside(p);
}

#endif

}} // namespace hexa::csg


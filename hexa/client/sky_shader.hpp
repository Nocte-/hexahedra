//---------------------------------------------------------------------------
/// \file   client/sky_shader.hpp
/// \brief
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

// This implements Preetham's sky model as described in "A Practical Analytic
// Model for Daylight" (2003).  Special thanks to Ilya Zaytsev for the
// article "Beneath a Preetham Sky" (http://r2vb.com/index.php?id=9)

#include <hexa/color.hpp>

namespace hexa {

typedef std::array<unsigned int, 3> triangle;

class skydome
{
public:
    skydome (float size = 1, unsigned int subdiv = 2)
    {
        vertices_.push_back(vector(0, 0, size));
        vertices_.push_back(vector(size , 0   , 0));
        vertices_.push_back(vector(0    , size, 0));
        vertices_.push_back(vector(-size, 0   , 0));
        vertices_.push_back(vector(0    ,-size, 0));
        vertices_.push_back(vector(0, 0, -size));

        triangles_.push_back(make_triangle(0, 1, 2));
        triangles_.push_back(make_triangle(0, 2, 3));
        triangles_.push_back(make_triangle(0, 3, 4));
        triangles_.push_back(make_triangle(0, 4, 1));

        triangles_.push_back(make_triangle(5, 1, 2));
        triangles_.push_back(make_triangle(5, 2, 3));
        triangles_.push_back(make_triangle(5, 3, 4));
        triangles_.push_back(make_triangle(5, 4, 1));

        for (unsigned int i (0); i < subdiv; ++i)
            subdivide(vertices_, triangles_);
    }

    const std::vector<vector>&   vertices() const  { return vertices_; }
    const std::vector<triangle>& triangles() const { return triangles_; }

protected:
inline triangle make_triangle (unsigned int a, unsigned int b, unsigned int c)
{
    triangle result;
    result[0] = a;
    result[1] = b;
    result[2] = c;
    return result;
}

void subdivide (std::vector<vector>& vertices,
                std::vector<triangle>& triangles)
{
    std::vector<triangle> new_tris;

    for (const triangle& tri : triangles)
    {
        unsigned int a(tri[0]), b(tri[1]), c(tri[2]);
        unsigned int d, e, f;

        vector n1 (normalize(halfway(vertices[a], vertices[b])));
        auto f1 (std::find (vertices.begin(), vertices.end(), n1));
        if (f1 == vertices.end())
        {
            d = vertices.size();
            vertices.push_back(n1);
        }
        else
        {
            d = std::distance(vertices.begin(), f1);
        }

        vector n2 (normalize(halfway(vertices[b], vertices[c])));
        auto f2 (std::find (vertices.begin(), vertices.end(), n2));
        if (f2 == vertices.end())
        {
            e = vertices.size();
            vertices.push_back(n2);
        }
        else
        {
            e = std::distance(vertices.begin(), f2);
        }

        vector n3 (normalize(halfway(vertices[c], vertices[a])));
        auto f3 (std::find (vertices.begin(), vertices.end(), n3));
        if (f3 == vertices.end())
        {
            f = vertices.size();
            vertices.push_back(n3);
        }
        else
        {
            f = std::distance(vertices.begin(), f3);
        }

        new_tris.push_back(make_triangle(a, d, f));
        new_tris.push_back(make_triangle(d, b, e));
        new_tris.push_back(make_triangle(f, e, c));
        new_tris.push_back(make_triangle(d, e, f));
    }

    triangles.swap(new_tris);
}
private:
    std::vector<vector>   vertices_;
    std::vector<triangle> triangles_;
};

class skylight
{
public:
    skylight(yaw_pitch sun_position, float turbidity = 6.0f)
        : sun_pos_    (from_spherical(sun_position))
        , turbidity_  (turbidity)
    {
        thetas_ = std::acos(dot_prod(sun_pos_, vector(0, 0, 1)));
        compute_zenith_color();
        compute_distribution_coefficients();
        compute_term();
    }

    color zenith_color() const { return zenith_color_; }

    color operator() (vector pos) const
    {
        pos.z = std::abs(pos.z);

        float cos_dist_sun (dot_prod(pos, sun_pos_));
        float dist_sun (std::acos(cos_dist_sun));
        float cos_dist_sun_sq (cos_dist_sun * cos_dist_sun);
        float one_over_cos_zenith_angle (pos.z < 0.00001 ? 1e6 : 1.f / pos.z);

        color result;
        result.x() = term_.x() * (1.f + A.x() * std::exp(B.x() * one_over_cos_zenith_angle))
                     * (1.f + C.x() * std::exp(D.x() * dist_sun) + E.x() * cos_dist_sun_sq);

        result.y() = term_.y() * (1.f + A.y() * std::exp(B.y() * one_over_cos_zenith_angle))
                     * (1.f + C.y() * std::exp(D.y() * dist_sun) + E.y() * cos_dist_sun_sq);

        result.Y() = term_.Y() * (1.f + A.Y() * std::exp(B.Y() * one_over_cos_zenith_angle))
                     * (1.f + C.Y() * std::exp(D.Y() * dist_sun) + E.Y() * cos_dist_sun_sq);

        if (result.x() < 0 || result.y() < 0)
        {
            result.x() = 0.33f;
            result.y() = 0.33f;
            result.Y() = 0;
        }
        return result;
    }

private:
    void compute_zenith_color()
    {
        float chi ((4.f/9.f - turbidity_/120.f) * (3.1415927f - 2.f * thetas_));

        // Zenith luminance in Kcd/m2
        zenith_color_.Y() = (4.0453f * turbidity_ - 4.9710f) * std::tan(chi) - 0.2155f * turbidity_ + 2.4192f;

        if (zenith_color_.Y() < 0.001f)
            zenith_color_.Y() = 0.001f;

        float t2(turbidity_ * turbidity_);
        //vector4<float> th (thetas_*thetas_*thetas_, thetas_*thetas_, thetas_, 1.f);
        vector4<float> th (0,0,0, 1.f);

        static const vector4<float> xcoef1 ( 0.00616f, -0.00375f,  0.00209f,  0.0f    ),
                                    xcoef2 (-0.02903f,  0.06377f, -0.03202f,  0.00394f),
                                    xcoef3 ( 0.11693f, -0.21196f,  0.06052f,  0.25886f),

                                    ycoef1 ( 0.00275f, -0.00610f,  0.00317f,  0.0f    ),
                                    ycoef2 (-0.04214f,  0.08970f, -0.04153f,  0.00516f),
                                    ycoef3 ( 0.15346f, -0.26756f,  0.06670f,  0.26688f);

        zenith_color_.x() = t2 * dot_prod(xcoef1, th) + turbidity_ * dot_prod(xcoef2, th) + dot_prod(xcoef3, th);
        zenith_color_.y() = t2 * dot_prod(ycoef1, th) + turbidity_ * dot_prod(ycoef2, th) + dot_prod(ycoef3, th);
    }

    void compute_distribution_coefficients()
    {
        A.Y() = 0.1787f*turbidity_ - 1.4630f;
        B.Y() =-0.3554f*turbidity_ + 0.4275f;
        C.Y() =-0.0227f*turbidity_ + 5.3251f;
        D.Y() = 0.1206f*turbidity_ - 2.5771f;
        E.Y() =-0.0670f*turbidity_ + 0.3703f;

        A.x() =-0.0193f*turbidity_ - 0.2592f;
        B.x() =-0.0665f*turbidity_ + 0.0008f;
        C.x() =-0.0004f*turbidity_ + 0.2125f;
        D.x() =-0.0641f*turbidity_ - 0.8989f;
        E.x() =-0.0033f*turbidity_ + 0.0452f;

        A.y() =-0.0167f*turbidity_ - 0.2608f;
        B.y() =-0.0950f*turbidity_ + 0.0092f;
        C.y() =-0.0079f*turbidity_ + 0.2102f;
        D.y() =-0.0441f*turbidity_ - 1.6537f;
        E.y() =-0.0109f*turbidity_ + 0.0529f;
    }

    void compute_term()
    {
        float cos_thetas (std::cos(thetas_));
        float cos_thetas_sq (cos_thetas * cos_thetas);
        term_.x() = zenith_color_.x() / ((1.f + A.x() * std::exp(B.x())) * (1.f + C.x() * std::exp(D.x()*thetas_) + E.x() * cos_thetas_sq));
        term_.y() = zenith_color_.y() / ((1.f + A.y() * std::exp(B.y())) * (1.f + C.y() * std::exp(D.y()*thetas_) + E.y() * cos_thetas_sq));
        term_.Y() = zenith_color_.Y() / ((1.f + A.Y() * std::exp(B.Y())) * (1.f + C.Y() * std::exp(D.Y()*thetas_) + E.Y() * cos_thetas_sq));
    }


private:
    /// Sun position
    vector sun_pos_;
    /// Turbidity is a measure of sky clarity. 1 = pure air, 25 = haze
    float turbidity_;
    /// Angular distance between the sun and the zenith
    float thetas_;
    /// CIE color at the zenith
    color zenith_color_;
    /// Distribution coefficients
    color A, B, C, D, E;
    /// Precomputed term
    color term_;
};

/*

class atmosphere
{
public:
    atmosphere(vector sd = vector(0, 1, 0), float re = 6360e3, float ra = 6420e3,
               float hr = 7994, float hm = 1200)
        : betaR (5.5e-6, 13.0e-6, 22.4e-6)
        , betaM (21e-6, 21e-6, 21e-6)
        , Hr(hr), Hm(hm), radiusEarth(re), radiusAtmosphere(ra), sunDirection(sd)
        , sunIntensity(20), g(0.76)
    {}

    vector betaR; /// Rayleigh scattering coefficients at sea level
    vector betaM; /// Mie scattering coefficients at sea level
    float Hr; /// Rayleigh scale height
    float Hm; /// Mie scale height
    float radiusEarth; /// Earth radius
    float radiusAtmosphere; /// Atmosphere radius
    vector sunDirection; /// Sun direction
    float sunIntensity; /// Sun intensity
    float g; /// Mean cosine

    vector computeIncidentLight(const Ray<T>& r, unsigned i, unsigned j) const;
};

vector atmosphere::computeIncidentLight(const Ray<T>& r) const
{
    float t0, t1;

    if (!intersect(r, radiusAtmosphere, t0, t1) || t1 < 0)
        return Vec3<T>(0);

    if (t0 > r.tmin && t0 > 0)
        r.tmin = t0;

    if (t1 < r.tmax)
        r.tmax = t1;

    unsigned numSamples = 16;
    unsigned numSamplesLight = 8;
    float segmentLength = (r.tmax - r.tmin) / numSamples;
    float tCurrent = r.tmin;
    vector sumR(0), sumM(0);
    float opticalDepthR = 0, opticalDepthM = 0;
    float mu = r.direction.dot(sunDirection);
    float phaseR = 3 / (16 * M_PI) * (1 + mu * mu);
    float g = 0.76;
    float phaseM = 3 / (8 * M_PI) * ((1 - g * g) * (1 + mu * mu))/((2 + g * g) * pow(1 + g * g - 2 * g * mu, 1.5));
    for (unsigned i = 0; i < numSamples; ++i)
    {
        vector samplePosition = r(tCurrent + T(0.5) * segmentLength);
        float height = samplePosition.magnitude() - radiusEarth; // compute optical depth for light
        float hr = exp(-height / Hr) * segmentLength; T hm = exp(-height / Hm) * segmentLength; opticalDepthR += hr; opticalDepthM += hm; // light optical depth
        Ray<T> lightRay(samplePosition, sunDirection);
        intersect(lightRay, radiusAtmosphere, lightRay.tmin, lightRay.tmax);
        float segmentLengthLight = lightRay.tmax / numSamplesLight, tCurrentLight = 0;
        float opticalDepthLightR = 0, opticalDepthLightM = 0;
        unsigned j = 0;
        for (j = 0; j < numSamplesLight; ++j)
        {
            vector samplePositionLight = lightRay(tCurrentLight + T(0.5) * segmentLengthLight);
            float heightLight = samplePositionLight.magnitude() - radiusEarth;
            if (heightLight < 0)
                break;

            opticalDepthLightR += exp(-heightLight / Hr) * segmentLengthLight;
            opticalDepthLightM += exp(-heightLight / Hm) * segmentLengthLight;
            tCurrentLight += segmentLengthLight;
        }
        if (j == numSamplesLight)
        {
            vector tau = betaR * (opticalDepthR + opticalDepthLightR) + betaM * 1.1 * (opticalDepthM + opticalDepthLightM);
            vector attenuation(exp(-tau.x), exp(-tau.y), exp(-tau.z));
            sumR += hr * attenuation;
            sumM += hm * attenuation;
        }
        tCurrent += segmentLength;
    }

    return 20 * (sumR * phaseR * betaR + sumM * phaseM * betaM);
}





typedef boost::numeric::ublas::vector<float, boost::numeric::ublas::bounded_array<float, 5>>
    distribution_terms;

float perez_function(const distribution_terms& d, float theta, float gamma)
{
    float cosg {std::cos(gamma)};
    float cost {std::max(0.0001, std::cos(theta))};

    return   (1.f + d[0] * std::exp(d[1] / cost))
           * (1 + d[2] * std::exp(d[3] * gamma) + d[4] * cosg * cosg);
}

float angle_between(float thetav, float phiv, float theta, float phi)
{
    float cospsi (  std::sin(thetav) * std::sin(theta) * std::cos(phi - phiv)
                  + std::cos(thetav) * std::cos(theta));

    if (cospsi > 1)
        return 0;

    if (cospsi < -1)
        return boost::math::constants::pi<float>();

    return std::acos(cospsi);
}

color compute(float x_zenith, float y_zenith, float luminence, float theta, float phi,
              float sun_theta, float sun_phi, float turbidity)
{
    boost::numeric::ublas::vector<float, boost::numeric::ublas::bounded_array<float, 2>>
                turbidity_vec;

    float gamma (angle_between(theta, phi, sun_theta, sun_phi));

    turbidity_vec[0] = turbidity;
    turbidity_vec[1] = 1;

    float skycolor_y (1.0 - std::exp(-luminance * distribution(prod(ydc, turbidity_vec)) / 25.));
    float skycolor_x (x_zenith * distribution(prod(xdc, turbidity_vec)));

}
*/


} // namespace hexa


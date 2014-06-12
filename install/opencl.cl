
#ifdef cl_khr_fp64
    #pragma OPENCL EXTENSION cl_khr_fp64 : enable
#elif defined(cl_amd_fp64)
    #pragma OPENCL EXTENSION cl_amd_fp64 : enable
#else
    #error "Double precision floating point not supported by OpenCL implementation."
#endif

#define ONE_F1                 (1.0f)
#define ZERO_F1                (0.0f)

__constant int P_MASK = 255;
__constant int P_SIZE = 256;
__constant int P[512] = {151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,
  151,160,137,91,90,15,
  131,13,201,95,96,53,194,233,7,225,140,36,103,30,69,142,8,99,37,240,21,10,23,
  190, 6,148,247,120,234,75,0,26,197,62,94,252,219,203,117,35,11,32,57,177,33,
  88,237,149,56,87,174,20,125,136,171,168, 68,175,74,165,71,134,139,48,27,166,
  77,146,158,231,83,111,229,122,60,211,133,230,220,105,92,41,55,46,245,40,244,
  102,143,54, 65,25,63,161, 1,216,80,73,209,76,132,187,208, 89,18,169,200,196,
  135,130,116,188,159,86,164,100,109,198,173,186, 3,64,52,217,226,250,124,123,
  5,202,38,147,118,126,255,82,85,212,207,206,59,227,47,16,58,17,182,189,28,42,
  223,183,170,213,119,248,152, 2,44,154,163, 70,221,153,101,155,167, 43,172,9,
  129,22,39,253, 19,98,108,110,79,113,224,232,178,185, 112,104,218,246,97,228,
  251,34,242,193,238,210,144,12,191,179,162,241, 81,51,145,235,249,14,239,107,
  49,192,214, 31,181,199,106,157,184, 84,204,176,115,121,50,45,127, 4,150,254,
  138,236,205,93,222,114,67,29,24,72,243,141,128,195,78,66,215,61,156,180,  
  };
  
//////////////////////////////////////////////////////////////////////////
 
__constant int G_MASK = 15;
__constant int G_SIZE = 16;
__constant int G_VECSIZE = 4;
__constant float G[16*4] = {
      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
      +ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1, 
      -ONE_F1,  -ONE_F1, +ZERO_F1, +ZERO_F1,
      +ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 
      -ONE_F1, +ZERO_F1,  +ONE_F1, +ZERO_F1, 
      +ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1, 
      -ONE_F1, +ZERO_F1,  -ONE_F1, +ZERO_F1,
     +ZERO_F1,  +ONE_F1,  +ONE_F1, +ZERO_F1, 
     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 
     +ZERO_F1,  +ONE_F1,  -ONE_F1, +ZERO_F1, 
     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1,
      +ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
      -ONE_F1,  +ONE_F1, +ZERO_F1, +ZERO_F1, 
     +ZERO_F1,  -ONE_F1,  +ONE_F1, +ZERO_F1, 
     +ZERO_F1,  -ONE_F1,  -ONE_F1, +ZERO_F1
};  

__constant uint OFFSET_BASIS = 2166136261;
__constant uint FNV_PRIME = 16777619;
 
inline double lerp (double x, double a, double b)
{
    return mad(x, b - a, a);
}

inline double blend3 (const double a)
{
    return a * a * (3.0 - 2.0 * a);
}

inline double blend5 (const double a)
{
    const double a3 = a * a * a;
    const double a4 = a3 * a;
    const double a5 = a4 * a;

    return 10.0 * a3 - 15.0 * a4 + 6.0 * a5;
}

inline double2 lerp2d (const double x, const double2 a, const double2 b)
{
    return mad(x, b - a, a);
}

/* FNV hash: http://isthe.com/chongo/tech/comp/fnv/#FNV-source */
inline uint hash (int x, int y)
{
  return (uint)((((OFFSET_BASIS ^ (uint)x) * FNV_PRIME) ^ (uint)y) * FNV_PRIME);
}

inline uint rng (uint last)
{
    return (1103515245 * last + 12345) & 0x7FFFFFFF;
}

inline double gradient_noise2d (double2 xy, int2 ixy, uint seed)
{
    ixy.x += seed * 1013;
    ixy.y += seed * 1619;
    ixy &= P_MASK;

    int index = (P[ixy.x+P[ixy.y]] & G_MASK) * G_VECSIZE;
    double2 g = (double2)(G[index], G[index+1]);

    return dot(xy, g);
}

double p_perlin (double2 xy, uint seed)
{
    double2 t = floor(xy);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = xy - t;

    const int2 I01 = (int2)(0, 1);
    const int2 I10 = (int2)(1, 0);
    const int2 I11 = (int2)(1, 1);

    const double2 F01 = (double2)(0.0, 1.0);
    const double2 F10 = (double2)(1.0, 0.0);
    const double2 F11 = (double2)(1.0, 1.0);

    const double n00 = gradient_noise2d(xyf      , xy0, seed);
    const double n10 = gradient_noise2d(xyf - F10, xy0 + I10, seed);
    const double n01 = gradient_noise2d(xyf - F01, xy0 + I01, seed);
    const double n11 = gradient_noise2d(xyf - F11, xy0 + I11, seed);

    const double2 n0001 = (double2)(n00, n01);
    const double2 n1011 = (double2)(n10, n11);
    const double2 n2 = lerp2d(blend5(xyf.x), n0001, n1011);

    return lerp(blend5(xyf.y), n2.x, n2.y) * 1.2;
}

__constant double F2 = 0.366025404; // 0.5 * (sqrt(3.0) - 1.0)
__constant double G2 = 0.211324865; // (3.0 - sqrt(3.0)) / 6.0

double p_simplex (double2 xy, uint seed)
{
    double n0, n1, n2;

    // Skew the input space to determine which simplex cell we're in
    double s = (xy.x + xy.y) * F2;
    int i = floor(xy.x + s);
    int j = floor(xy.y + s);

    // Unskew the cell origin back to (x,y) space
    double t = (i + j) * G2;
    double2 o = (double2)(i - t, j - t);

    // The x,y distances from the cell origin
    double2 d0 = xy - o;

    // For the 2D case, the simplex shape is an equilateral triangle.
    // Determine which simplex we are in.
    int i1, j1; // Offsets for second (middle) corner of simplex in (i,j) coords
    if (d0.x > d0.y)
    {
        i1=1; // lower triangle, XY order: (0,0)->(1,0)->(1,1)
        j1=0;
    }
    else
    {
        i1=0; // upper triangle, YX order: (0,0)->(0,1)->(1,1)
        j1=1;
    }

    double2 d1 = (double2)(d0.x - i1 + G2, d0.y - j1 + G2);
    double2 d2 = (double2)(d0.x - 1.0 + 2.0 * G2, d0.y - 1.0 + 2.0 * G2);

    int ii = (i + seed * 1063) & 0xFF;
    int jj = j & 0xFF;
    int gi0 = (P[ii+P[jj]] & G_MASK) * G_VECSIZE;
    int gi1 = (P[ii+i1+P[jj+j1]] & G_MASK) * G_VECSIZE;
    int gi2 = (P[ii+1+P[jj+1]] & G_MASK) * G_VECSIZE;

    double t0 = 0.5 - dot(d0,d0);
    if (t0 < 0)
    {
        n0 = 0.0;
    }
    else
    {
        t0 *= t0;
        n0 = t0 * t0 * dot((double2)(G[gi0],G[gi0+1]), d0);
    }

    double t1 = 0.5 - dot(d1,d1);
    if(t1 < 0)
    {
        n1 = 0.0;
    }
    else
    {
        t1 *= t1;
        n1 = t1 * t1 * dot((double2)(G[gi1],G[gi1+1]), d1);
    }

    double t2 = 0.5 - dot(d2,d2);
    if(t2 < 0)
    {
        n2 = 0.0;
    }
    else
    {
        t2 *= t2;
        n2 = t2 * t2 * dot((double2)(G[gi2],G[gi2+1]), d2);
    }

    return 70.0 * (n0 + n1 + n2);
}

double2 p_worley (const double2 p, uint seed)
{
    double2 t = floor(p);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = p - t;

    double f0 = 9999.9;
    double f1 = 9999.9;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            int2 square = xy0 + (int2)(i,j);
            uint rnglast = rng(hash(square.x + seed, square.y));

            double2 rnd_pt;
            rnd_pt.x = (double)i + (double)rnglast / (double)0x7FFFFFFF;
            rnglast = rng(rnglast);
            rnd_pt.y = (double)j + (double)rnglast / (double)0x7FFFFFFF;

            double dist = distance(xyf, rnd_pt);
            if (dist < f0)
            {
                f1 = f0;
                f0 = dist;
            }
            else if (dist < f1)
            {
                f1 = dist;
            }
        }
    }
    return (double2)(f0, f1);
}

double2 p_voronoi (const double2 p, uint seed)
{
    double2 t = floor(p);
    int2 xy0 = (int2)((int)t.x, (int)t.y);
    double2 xyf = p - t;

    double f0 = 9999.9;
    double2 nearest;

    for (int i = -1; i < 2; ++i)
    {
        for (int j = -1; j < 2; ++j)
        {
            int2 square = xy0 + (int2)(i,j);
            uint rnglast = rng(hash(square.x + seed, square.y));

            double2 rnd_pt;
            rnd_pt.x = (double)i + (double)rnglast / (double)0x7FFFFFFF;
            rnglast = rng(rnglast);
            rnd_pt.y = (double)j + (double)rnglast / (double)0x7FFFFFFF;

            double dist = distance(xyf, rnd_pt);
            if (dist < f0)
            {
                nearest = rnd_pt;
                f0 = dist;
            }
        }
    }
    return t + nearest;
}

inline double2 p_rotate (double2 p, double a)
{
    double t = a * M_PI;
    return (double2)(p.x * cos(t) - p.y * sin(t), p.x * sin(t) + p.y * cos(t));
}

inline double2 p_swap (double2 p)
{
    return (double2)(p.y, p.x);
}

inline double p_angle (double2 p)
{
    return atan2pi(p.y, p.x);
}

inline double p_chebyshev (double2 p)
{
    return fmax(fabs(p.x), fabs(p.y));
}

inline double p_saw (double n)
{
    return n - floor(n);
}

inline double p_checkerboard (double2 p)
{
    double2 sp = p - floor(p);
    return (sp.x < 0.5 && sp.y < 0.5) || (sp.x >= 0.5 && sp.y >= 0.5)
           ? 1.0 : -1.0;
}

inline double p_manhattan (double2 p)
{
    return fabs(p.x) + fabs(p.y);
}

inline double p_blend (double x, double a, double b)
{
    x = (clamp(x, -1.0, 1.0) + 1.0) / 2.0;
    return lerp(x, a, b);
}

inline double p_range (double x, double a, double b)
{
    return clamp((a * 0.5 + x + 1.0) * ((b - a) * 0.5), a, b);
}

inline bool p_is_in_circle (double2 p, double r)
{
    return length(p) <= r;
}

inline bool p_is_in_rectangle (double2 p, double x1, double y1, double x2, double y2)
{
    return p.x >= x1 && p.y >= y1 && p.x <= x2 && p.y <= y2;
}


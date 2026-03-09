#pragma once
#include "geometry.h"
#include <cmath>
#include <algorithm>

struct SpaceTime4
{
    double t{}, r{}, theta{}, phi{};
};

inline SpaceTime4 operator+(const SpaceTime4 &a, const SpaceTime4 &b)
{
    return {a.t + b.t, a.r + b.r, a.theta + b.theta, a.phi + b.phi};
}
inline SpaceTime4 operator-(const SpaceTime4 &a, const SpaceTime4 &b)
{
    return {a.t - b.t, a.r - b.r, a.theta - b.theta, a.phi - b.phi};
}
inline SpaceTime4 operator*(double s, const SpaceTime4 &v)
{
    return {s * v.t, s * v.r, s * v.theta, s * v.phi};
}
inline SpaceTime4 operator*(const SpaceTime4 &v, double s) { return s * v; }

inline Point3D ToCartesianPoint(const SpaceTime4 &p)
{
    return {
        p.r * std::sin(p.theta) * std::cos(p.phi),
        p.r * std::sin(p.theta) * std::sin(p.phi),
        p.r * std::cos(p.theta),
    };
}

inline Vector3D ToCartesianVector(const SpaceTime4 &v)
{
    return {
        v.r * std::sin(v.theta) * std::cos(v.phi),
        v.r * std::sin(v.theta) * std::sin(v.phi),
        v.r * std::cos(v.theta),
    };
}

inline SpaceTime4 ToSchwPosition(Point3D p, double t0 = 0.0)
{
    const double r = std::sqrt(p.x * p.x + p.y * p.y + p.z * p.z);
    const double theta = std::acos(std::clamp(p.z / std::max(1e-15, r), -1.0, 1.0));
    const double phi = std::atan2(p.y, p.x);
    return {t0, r, theta, phi};
}

inline SpaceTime4 ToSchwRates(Point3D p, Vector3D dir, double dt_dlambda = 1.0)
{
    const Vector3D pVec = p;
    const double x = p.x, y = p.y, z = p.z;
    const double dx = dir.x, dy = dir.y, dz = dir.z;

    const double r2 = pVec.lengthSquared();
    const double r = pVec.length();

    const double dr = pVec * dir / r;

    const double cosTh = z / r;
    const double th = std::acos(std::clamp(cosTh, -1.0, 1.0));
    const double sinTh = std::sin(th);
    const double dz_over_r_dot = (dz * r - z * dr) / (r2);

    double dth = 0.0;
    if (std::abs(sinTh) > 1e-12)
    {
        dth = -(1.0 / sinTh) * dz_over_r_dot;
    }
    else
    {
        dth = 0.0;
    }

    const double xy2 = x * x + y * y;
    double dph = 0.0;
    if (xy2 > 1e-15)
    {
        dph = (x * dy - y * dx) / xy2;
    }
    else
    {
        dph = 0.0; // on axis
    }

    return {dt_dlambda, dr, dth, dph};
}

struct Photon
{
    SpaceTime4 x{}; // position
    SpaceTime4 k{}; // wavevector
};

struct PhotonDeriv
{
    SpaceTime4 dx{};
    SpaceTime4 dk{};
};

inline Photon operator+(const Photon &a, const PhotonDeriv &b)
{
    return {a.x + b.dx, a.k + b.dk};
}
inline Photon operator-(const Photon &a, const Photon &b) { return {a.x - b.x, a.k - b.k}; }

inline PhotonDeriv operator+(const PhotonDeriv &a, const PhotonDeriv &b)
{
    return {a.dx + b.dx, a.dk + b.dk};
}
inline PhotonDeriv operator*(double s, const PhotonDeriv &y)
{
    return {s * y.dx, s * y.dk};
}

inline double maxVal(const SpaceTime4 &v)
{
    return std::max({std::abs(v.r), std::abs(v.theta), std::abs(v.phi)});
}

inline double maxVal(const Photon &y)
{
    return std::max(maxVal(y.x), maxVal(y.k));
}
#pragma once
#include <cmath>
#include <stdexcept>

struct Vector2D;
struct Vector3D;

struct Point2D
{
    double x{}, y{};
    constexpr Point2D() = default;
    constexpr Point2D(double x, double y) : x(x), y(y) {}
};

struct Point3D
{
    double x{}, y{}, z{};
    constexpr Point3D() = default;
    constexpr Point3D(double x, double y, double z) : x(x), y(y), z(z) {}
};

struct Vector2D
{
    double x{}, y{};
    constexpr Vector2D() = default;
    constexpr Vector2D(double x, double y) : x(x), y(y) {}

    [[nodiscard]] double length() const noexcept
    {
        return std::sqrt(x * x + y * y);
    }
    [[nodiscard]] double lengthSquared() const noexcept
    {
        return x * x + y * y;
    }

    [[nodiscard]] bool isNearlyZero(double eps = 1e-12) const noexcept
    {
        return lengthSquared() <= eps * eps;
    }
    [[nodiscard]] Vector2D normalized(double eps = 1e-12) const
    {
        const double ls = lengthSquared();
        if (ls <= eps * eps)
        {
            throw std::runtime_error("Vector2D::normalized(): near-zero vector");
        }
        const double invLen = 1.0 / std::sqrt(ls);
        return {x * invLen, y * invLen};
    }
    Vector2D &normalize(double eps = 1e-12)
    {
        const double ls = lengthSquared();
        if (ls <= eps * eps)
        {
            throw std::runtime_error("Vector2D::normalize(): near-zero vector");
        }
        const double invLen = 1.0 / std::sqrt(ls);
        x *= invLen;
        y *= invLen;
        return *this;
    }
    bool tryNormalize(double eps = 1e-12) noexcept
    {
        try
        {
            normalize(eps);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
};

struct Vector3D
{
    double x{}, y{}, z{};
    constexpr Vector3D() = default;
    constexpr Vector3D(double x, double y, double z) : x(x), y(y), z(z) {};
    constexpr Vector3D(Point3D p) : x(p.x), y(p.y), z(p.z) {};

    [[nodiscard]] double length() const noexcept
    {
        return std::sqrt(x * x + y * y + z * z);
    }
    [[nodiscard]] double lengthSquared() const noexcept
    {
        return x * x + y * y + z * z;
    }

    [[nodiscard]] bool isNearlyZero(double eps = 1e-12) const noexcept
    {
        return lengthSquared() <= eps * eps;
    }
    [[nodiscard]] Vector3D normalized(double eps = 1e-12) const
    {
        const double ls = lengthSquared();
        if (ls <= eps * eps)
        {
            throw std::runtime_error("Vector2D::normalized(): near-zero vector");
        }
        const double invLen = 1.0 / std::sqrt(ls);
        return {x * invLen, y * invLen, z * invLen};
    }
    Vector3D &normalize(double eps = 1e-12)
    {
        const double ls = lengthSquared();
        if (ls <= eps * eps)
        {
            throw std::runtime_error("Vector2D::normalize(): near-zero vector");
        }
        const double invLen = 1.0 / std::sqrt(ls);
        x *= invLen;
        y *= invLen;
        z *= invLen;
        return *this;
    }
    bool tryNormalize(double eps = 1e-12) noexcept
    {
        try
        {
            normalize(eps);
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
};

// -------- Vector2D arithmetic --------
// plus and minus
constexpr Vector2D operator+(Vector2D a, Vector2D b) noexcept
{
    return {a.x + b.x, a.y + b.y};
}
constexpr Vector2D operator-(Vector2D a, Vector2D b) noexcept
{
    return {a.x - b.x, a.y - b.y};
}
constexpr Vector2D operator-(Vector2D v) noexcept
{
    return {-v.x, -v.y};
}

// inplace
constexpr Vector2D &operator+=(Vector2D &a, Vector2D b) noexcept
{
    a = a + b;
    return a;
}
constexpr Vector2D &operator-=(Vector2D &a, Vector2D b) noexcept
{
    a = a - b;
    return a;
}

// multiply and divide
constexpr Vector2D operator*(Vector2D v, double s) noexcept
{
    return {v.x * s, v.y * s};
}
constexpr Vector2D operator*(double s, Vector2D v) noexcept
{
    return v * s;
}
constexpr Vector2D operator/(Vector2D v, double s)
{
    return {v.x / s, v.y / s};
}
// inplace
constexpr Vector2D &operator*=(Vector2D &v, double s) noexcept
{
    v = v * s;
    return v;
}
constexpr Vector2D &operator/=(Vector2D &v, double s)
{
    v = v / s;
    return v;
}

// Vector ops
constexpr double dot(Vector2D a, Vector2D b) noexcept
{
    return a.x * b.x + a.y * b.y;
}
constexpr double operator*(Vector2D a, Vector2D b) noexcept
{
    return dot(a, b);
}
constexpr double cross(Vector2D a, Vector2D b) noexcept
{
    return a.x * b.y - a.y * b.x;
}

// -------- Vector3D arithmetic --------
// plus and minus
constexpr Vector3D operator+(Vector3D a, Vector3D b) noexcept
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
constexpr Vector3D operator-(Vector3D a, Vector3D b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
constexpr Vector3D operator-(Vector3D v) noexcept
{
    return {-v.x, -v.y, -v.z};
}

// inplace
constexpr Vector3D &operator+=(Vector3D &a, Vector3D b) noexcept
{
    a = a + b;
    return a;
}
constexpr Vector3D &operator-=(Vector3D &a, Vector3D b) noexcept
{
    a = a - b;
    return a;
}

// multiply and divide
constexpr Vector3D operator*(Vector3D v, double s) noexcept
{
    return {v.x * s, v.y * s, v.z * s};
}
constexpr Vector3D operator*(double s, Vector3D v) noexcept
{
    return v * s;
}
constexpr Vector3D operator/(Vector3D v, double s)
{
    return {v.x / s, v.y / s, v.z / s};
}
// inplace
constexpr Vector3D &operator*=(Vector3D &v, double s) noexcept
{
    v = v * s;
    return v;
}
constexpr Vector3D &operator/=(Vector3D &v, double s)
{
    v = v / s;
    return v;
}

// Vector ops
constexpr double dot(Vector3D a, Vector3D b) noexcept
{
    return a.x * b.x + a.y * b.y + a.z * b.z;
}
constexpr double operator*(Vector3D a, Vector3D b) noexcept
{
    return dot(a, b);
}
constexpr Vector3D cross(Vector3D a, Vector3D b) noexcept
{
    return {
        a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x};
}

// -------- Vector/Point geometry --------

// Point - Point = Vector
constexpr Vector2D operator-(Point2D a, Point2D b) noexcept
{
    return {a.x - b.x, a.y - b.y};
}
constexpr Vector3D operator-(Point3D a, Point3D b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

// Point +- Vector = Point
constexpr Point2D operator+(Point2D a, Vector2D b) noexcept
{
    return {a.x + b.x, a.y + b.y};
}
constexpr Point2D operator-(Point2D a, Vector2D b) noexcept
{
    return {a.x - b.x, a.y - b.y};
}
constexpr Point3D operator+(Point3D a, Vector3D b) noexcept
{
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
constexpr Point3D operator-(Point3D a, Vector3D b) noexcept
{
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}

// inplace
constexpr Point2D &operator+=(Point2D &a, Vector2D b) noexcept
{
    a = a + b;
    return a;
}
constexpr Point2D &operator-=(Point2D &a, Vector2D b) noexcept
{
    a = a - b;
    return a;
}
constexpr Point3D &operator+=(Point3D &a, Vector3D b) noexcept
{
    a = a + b;
    return a;
}
constexpr Point3D &operator-=(Point3D &a, Vector3D b) noexcept
{
    a = a - b;
    return a;
}

// -------- Point distances --------
[[nodiscard]] inline double distanceSquared(Point2D a, Point2D b) noexcept
{
    return (a - b).lengthSquared();
}
[[nodiscard]] inline double distance(Point2D a, Point2D b) noexcept
{
    return std::sqrt(distanceSquared(a, b));
}
[[nodiscard]] inline double distanceSquared(Point3D a, Point3D b) noexcept
{
    return (a - b).lengthSquared();
}
[[nodiscard]] inline double distance(Point3D a, Point3D b) noexcept
{
    return std::sqrt(distanceSquared(a, b));
}
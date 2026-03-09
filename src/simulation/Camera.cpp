#include "Camera.h"
#include <random>
#include <iostream>

void Camera::setPose(Point3D position, Point3D lookAt, Vector3D upHint)
{
    position_ = position;
    Vector3D diff = lookAt - position;
    if (not diff.tryNormalize())
    {
        throw std::runtime_error("Camera::setPose(): near-identical points for position and lookAt.");
    }
    direction_ = diff;
    // Build an orthonormal basis (right, up, forward)
    Vector3D right = cross(direction_, upHint);
    if (not right.tryNormalize())
    {
        throw std::runtime_error("Camera::setPose(): Trying to look along upHint vector.");
    }
    right_ = right;
    up_ = cross(right_, direction_).normalize();
}

void Camera::generateRays(double t0, double dt_dlambda)
{

    const double aspect = double(w_) / double(h_);
    const double tanHalfV = std::tan(0.5 * fovRad_);
    const double tanHalfH = tanHalfV * aspect;
    std::random_device device;
    std::default_random_engine engine(device());
    std::uniform_real_distribution dist{0.0, 1.0};

    for (std::size_t y = 0; y < h_; ++y)
    {
        for (std::size_t x = 0; x < w_; ++x)
        {
            // anti aliasing removed for now.
            for (std::size_t aa = 0; aa < antiAliasNum_; ++aa)
            {
                double jitterX = 0;
                double jitterY = 0;
                if (aa != 0)
                {
                    jitterX = (dist(engine) - 0.5);
                    jitterY = (dist(engine) - 0.5);
                }

                double u = ((double(x) + 0.5 + jitterX) / double(w_)) * 2.0 - 1.0;
                double v = 1.0 - ((double(y) + 0.5 + jitterY) / double(h_)) * 2.0;
                // Camera-space direction
                Vector3D dir = (direction_ + (u * tanHalfH) * right_ + (v * tanHalfV) * up_).normalize();

                Photon pr{};
                pr.x = ToSchwPosition(position_, t0);
                pr.k = ToSchwRates(position_, dir, dt_dlambda);

                photons_[antiAliasNum_ * y * w_ + x + aa] = pr;
            }
        }
    }
}
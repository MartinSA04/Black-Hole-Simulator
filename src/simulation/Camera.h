#pragma once
#include "geometry.h"
#include "Photon.h"
#include <vector>

class Camera
{
    std::size_t w_, h_;
    double fovRad_;
    Point3D position_{};
    Vector3D direction_{1, 0, 0};
    Vector3D up_{0, 0, 1};
    Vector3D right_{0, 1, 0};
    std::vector<Photon> photons_;
    int antiAliasNum_{};

public:
    Camera(std::size_t w, std::size_t h, double fov, int AANum = 5) : w_(w), h_(h), fovRad_(fov * (M_PI / 180.0)), photons_{AANum * w * h}, antiAliasNum_{AANum} {};

    std::size_t width() const noexcept { return w_; }
    std::size_t height() const noexcept { return h_; }
    std::size_t aaNum() const noexcept { return antiAliasNum_; }

    Photon &photon(std::size_t px, std::size_t py, std::size_t aaNum = 0)
    {
        return photons_[(w_ * antiAliasNum_) * py + px + aaNum];
    }

    void setPose(Point3D position, Point3D lookAt, Vector3D upHint = {0, 0, 1});
    void setFov(double fov)
    {
        fovRad_ = fov * (M_PI / 180.0);
    }
    void generateRays(double t0 = 0.0, double dt_dlambda = 1.0);
    void setAANum(int AANum)
    {
        antiAliasNum_ = AANum;
        photons_.resize(AANum * w_ * h_);
    }
    void setSize(std::size_t w, std::size_t h)
    {
        w_ = w;
        h_ = h;
        photons_.resize(antiAliasNum_ * w_ * h_);
    }
};
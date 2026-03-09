#pragma once
#include "scene.h"
#include "Camera.h"
#include <chrono>
#include <iostream>
#include <functional>

static TDT4102::Color avgColor(std::vector<TDT4102::Color> colors);
static RayStatus getStatus(std::vector<RayStatus> statuses);

struct Sample
{
    TDT4102::Color color{};
    RayStatus status{};
};

class RayCache
{
    const Scene &scene_;
    Camera &cam_;
    int x0_{}, y0_{}, w_{}, h_{};

    std::vector<Sample> samples_;
    std::vector<unsigned char> valid_;

    // Profiling
    static inline long long traceTime_ = 0;
    static inline long long cacheTime_ = 0;
    static inline int traceCount_ = 0;

    Sample tracePixel(std::size_t px, std::size_t py)
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<TDT4102::Color> colors;
        std::vector<RayStatus> statuses;
        for (int aaNum = 0; aaNum < cam_.aaNum(); ++aaNum)
        {
            Photon photon = cam_.photon(px, py, aaNum);
            double h = 0.2;
            double tol = 1e-7;
            TDT4102::Color color{};
            RayStatus status = scene_.trace(photon, h, tol, color);
            colors.push_back(color);
            statuses.push_back(status);
        }

        auto end = std::chrono::high_resolution_clock::now();
        traceTime_ += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
        traceCount_++;

        return {avgColor(colors), getStatus(statuses)};
    }

public:
    RayCache(const Scene &scene, Camera &cam, int x0, int y0, int x1, int y1)
        : scene_(scene), cam_(cam),
          x0_(x0), y0_(y0), w_(x1 - x0), h_(y1 - y0),
          samples_((std::size_t)w_ * (std::size_t)h_),
          valid_((std::size_t)w_ * (std::size_t)h_, 0)
    {
    }

    const Sample &get(int px, int py)
    {
        auto cacheStart = std::chrono::high_resolution_clock::now();

        const int lx = px - x0_;
        const int ly = py - y0_;
        const std::size_t idx = (std::size_t)ly * (std::size_t)w_ + (std::size_t)lx;

        if (!valid_[idx])
        {
            samples_[idx] = tracePixel(px, py);
            valid_[idx] = 1;
        }

        auto cacheEnd = std::chrono::high_resolution_clock::now();
        cacheTime_ += std::chrono::duration_cast<std::chrono::microseconds>(cacheEnd - cacheStart).count();

        return samples_[idx];
    }

    int camW() const { return cam_.width(); }

    static void printStats()
    {
        std::cout << "\n=== Profile Stats ===\n";
        std::cout << "Ray trace calls: " << traceCount_ << "\n";
        std::cout << "Total trace time: " << traceTime_ / 1000.0 << " ms\n";
        std::cout << "Total cache time: " << cacheTime_ / 1000.0 << " ms\n";
        if (traceCount_ > 0)
            std::cout << "Avg per trace: " << (double)traceTime_ / traceCount_ << " μs\n";
        std::cout << "=====================\n\n";
    }
};

void render(const Scene &scene, Camera &cam, std::vector<TDT4102::Color> &frame, std::function<void(double)> *loadFunction = nullptr);
#pragma once
#include "Color.h"
#include "BlackHole.h"
#include <vector>
#include <memory>

enum class RayStatus
{
    Running,
    HitObject,
    FellIntoHorizon,
    Escaped
};

struct HitInfo
{
    double u = 0.0;
    TDT4102::Color color{};
};

class SceneObject
{
public:
    virtual ~SceneObject() = default;

    virtual bool hitSegment(const BlackHole &blackHole, const Photon &prev, const Photon &curr, HitInfo &hit) const = 0;
};

class AccretionDisk final : public SceneObject
{
    double innerRadius2_, outerRadius2_, innerRadius_, outerRadius_, ringWidth_, fullRingWidth_, width_;
    int numRings_;
    std::vector<TDT4102::Color> colors;

public:
    AccretionDisk(double innerRadius, double width, int numRings, float ringFill, TDT4102::Color startColor, TDT4102::Color endColor);

    bool hitSegment(const BlackHole &blackHole, const Photon &prev, const Photon &curr, HitInfo &hit) const override;
};

class Scene
{
    BlackHole blackHole_;
    std::vector<std::unique_ptr<SceneObject>> objects_;

    double escapeRadius_ = 20.0;
    double horizonEps_ = 1e-4;
    int maxSteps_ = 200000;

    bool hitObjects(const Photon &prev, const Photon &curr, HitInfo &hit) const;

public:
    Scene(BlackHole blackHole) : blackHole_(blackHole) {}
    void addObject(std::unique_ptr<SceneObject> obj)
    {
        objects_.push_back(std::move(obj));
    }
    void setEscapeRadius(double r) { escapeRadius_ = r; }
    void setHorizonEps(double eps) { horizonEps_ = eps; }
    void setMaxAcceptedSteps(int n) { maxSteps_ = n; }
    void createAccretionDisk(double distanceToBH, double width, int numRings, float ringFill, TDT4102::Color startColor, TDT4102::Color endColor);

    RayStatus trace(Photon &photon, double &stepSize, double tol, TDT4102::Color &outColor) const;
};
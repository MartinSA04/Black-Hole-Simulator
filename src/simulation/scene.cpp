#include "scene.h"
#include "RK45.h"

static TDT4102::Color interpolateColor(TDT4102::Color a, TDT4102::Color b, float u)
{
    return TDT4102::Color{
        a.redChannel * u + b.redChannel * (1 - u),
        a.greenChannel * u + b.greenChannel * (1 - u),
        a.blueChannel * u + b.blueChannel * (1 - u),
        a.alphaChannel * u + b.alphaChannel * (1 - u)};
}

AccretionDisk::AccretionDisk(double innerRadius, double width, int numRings, float ringFill, TDT4102::Color startColor, TDT4102::Color endColor)
    : innerRadius_(innerRadius),
      innerRadius2_(innerRadius * innerRadius),
      width_(width),
      outerRadius_(innerRadius + width),
      fullRingWidth_(width / numRings),
      numRings_(numRings)
{
    ringWidth_ = fullRingWidth_ * ringFill;
    outerRadius2_ = outerRadius_ * outerRadius_;
    for (int i = 0; i < numRings; ++i)
    {
        float u = (float)i / (numRings - 1);
        colors.push_back(interpolateColor(startColor, endColor, u));
    }
}

bool AccretionDisk::hitSegment(const BlackHole &blackHole, const Photon &prev, const Photon &curr, HitInfo &hit) const
{
    // early exit most of the time :)
    if ((prev.x.theta > M_PI_2) == (curr.x.theta > M_PI_2))
        return false;

    const Vector3D p0 = ToCartesianVector(prev.x);
    const Vector3D p1 = ToCartesianVector(curr.x);

    const double u = (0.0 - p0.z) / (p1.z - p0.z);

    // Intersection point
    const Point3D p{
        p0.x + u * (p1.x - p0.x),
        p0.y + u * (p1.y - p0.y),
        0.0};

    const double rho2 = p.x * p.x + p.y * p.y;
    if (rho2 < innerRadius2_ or rho2 > outerRadius2_)
        return false;

    const double rhoDiff = std::sqrt(rho2) - innerRadius_;
    if (rhoDiff < 0.0 or rhoDiff > width_)
        return false;

    const double ringPos = rhoDiff / fullRingWidth_;
    const int ringIdx = static_cast<int>(ringPos);
    if (ringIdx < 0 || ringIdx >= numRings_)
        return false;

    const double withinRing = rhoDiff - ringIdx * fullRingWidth_;
    if (withinRing > ringWidth_)
        return false;

    hit.u = u;
    hit.color = colors[ringIdx];
    return true;
}

bool Scene::hitObjects(const Photon &prev, const Photon &curr, HitInfo &hit) const
{
    bool anyHit = false;
    double bestU = 1e100;

    for (const auto &obj : objects_)
    {
        HitInfo hit_{};
        if (obj->hitSegment(blackHole_, prev, curr, hit_))
        {
            if (hit_.u < bestU)
            {
                bestU = hit_.u;
                hit = hit_;
                anyHit = true;
            }
        }
    }
    return anyHit;
}

RayStatus Scene::trace(Photon &photon, double &stepSize, double tol, TDT4102::Color &outColor) const
{
    int accepted = 0;
    while (accepted < maxSteps_)
    {
        if (photon.x.r <= blackHole_.swchRadius() * (1.0 + horizonEps_))
        {
            outColor = TDT4102::Color::black;
            return RayStatus::FellIntoHorizon;
        }
        if (photon.x.r >= escapeRadius_)
        {
            outColor = TDT4102::Color(0x16161d);
            return RayStatus::Escaped;
        }

        Photon prev = photon;
        if (not RK45Step(blackHole_, photon, stepSize, tol))
            continue;

        ++accepted;
        if (photon.x.theta < 0)
        {
            photon.x.theta = -photon.x.theta;
            photon.x.phi += M_PI;
        }
        else if (photon.x.theta > M_PI)
        {
            photon.x.theta = 2.0 * M_PI - photon.x.theta;
            photon.x.phi += M_PI;
        }

        HitInfo hit{};
        if (hitObjects(prev, photon, hit))
        {
            outColor = hit.color;
            return RayStatus::HitObject;
        }
    }
    outColor = TDT4102::Color(0x16161d);
    return RayStatus::Escaped;
}

void Scene::createAccretionDisk(double distanceToBH, double width, int numRings, float ringFill, TDT4102::Color startColor, TDT4102::Color endColor)
{
    objects_.resize(0);
    addObject(std::make_unique<AccretionDisk>(blackHole_.swchRadius() + distanceToBH, width, numRings, ringFill, startColor, endColor));
}

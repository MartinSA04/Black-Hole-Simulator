#pragma once
#include "Photon.h"

class BlackHole
{
    double mass_{};

public:
    explicit BlackHole(double mass) : mass_(mass) {}

    double mass() const noexcept { return mass_; }
    double swchRadius() const noexcept { return 2.0 * mass_; } // 2*G*M/c^2
    double swchFactor(double r) const noexcept { return 1.0 - (mass_) / r; }

    PhotonDeriv deriv(const Photon &photon) const;
};

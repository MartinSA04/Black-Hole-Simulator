#include "BlackHole.h"

PhotonDeriv BlackHole::deriv(const Photon &photon) const
{
    const double r = photon.x.r;
    const double theta = photon.x.theta;

    const double invR = 1.0 / r;
    const double invR2 = invR * invR;

    const double f = swchFactor(r);
    const double fp = (2.0 * mass_) * invR2;
    const double halfLogPrime = fp / (2.0 * f);

    const double sinTheta = std::sin(theta);
    const double cosTheta = std::cos(theta);
    const double sin2Theta = sinTheta * sinTheta;

    // Wavevector components
    const double k_t = photon.k.t;
    const double k_r = photon.k.r;
    const double k_th = photon.k.theta;
    const double k_ph = photon.k.phi;

    // Reused squares/products
    const double k_t2 = k_t * k_t;
    const double k_r2 = k_r * k_r;
    const double k_th2 = k_th * k_th;
    const double k_ph2 = k_ph * k_ph;

    PhotonDeriv deriv{};
    deriv.dx = photon.k;

    deriv.dk.t = -2.0 * halfLogPrime * k_t * k_r;

    deriv.dk.r = -0.5 * f * fp * k_t2 + halfLogPrime * k_r2 + r * f * (k_th2 + sin2Theta * k_ph2);

    deriv.dk.theta = -(2.0 * invR) * k_r * k_th + (sinTheta * cosTheta) * k_ph2;

    constexpr double eps = 1e-15;
    const double cotTheta = cosTheta / std::max(eps, sinTheta);
    deriv.dk.phi = -(2.0 * invR) * k_r * k_ph - 2.0 * cotTheta * k_th * k_ph;

    return deriv;
}
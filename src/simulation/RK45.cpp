#include "RK45.h"
#include <cassert>

bool RK45Step(const BlackHole &blackHole, Photon &photon, double &h, double tol)
{
    // Uses the Dormand-Prince method
    assert(h > 0.0);
    assert(tol > 0.0);

    // Dormand–Prince 5(4) coefficients
    const double a21 = 1.0 / 5.0;

    const double a31 = 3.0 / 40.0, a32 = 9.0 / 40.0;

    const double a41 = 44.0 / 45.0, a42 = -56.0 / 15.0, a43 = 32.0 / 9.0;

    const double a51 = 19372.0 / 6561.0, a52 = -25360.0 / 2187.0, a53 = 64448.0 / 6561.0, a54 = -212.0 / 729.0;

    const double a61 = 9017.0 / 3168.0, a62 = -355.0 / 33.0, a63 = 46732.0 / 5247.0, a64 = 49.0 / 176.0, a65 = -5103.0 / 18656.0;

    // 5th order weights
    const double b1 = 35.0 / 384.0, b3 = 500.0 / 1113.0, b4 = 125.0 / 192.0, b5 = -2187.0 / 6784.0, b6 = 11.0 / 84.0;

    // 4th order weights (b*)
    const double bs1 = 5179.0 / 57600.0, bs3 = 7571.0 / 16695.0, bs4 = 393.0 / 640.0,
                 bs5 = -92097.0 / 339200.0, bs6 = 187.0 / 2100.0, bs7 = 1.0 / 40.0;

    const PhotonDeriv k1 = blackHole.deriv(photon);

    const Photon y2 = photon + h * (a21 * k1);
    const PhotonDeriv k2 = blackHole.deriv(y2);

    const Photon y3 = photon + h * ((a31 * k1) + (a32 * k2));
    const PhotonDeriv k3 = blackHole.deriv(y3);

    const Photon y4 = photon + h * ((a41 * k1) + (a42 * k2) + (a43 * k3));
    const PhotonDeriv k4 = blackHole.deriv(y4);

    const Photon y5 = photon + h * ((a51 * k1) + (a52 * k2) + (a53 * k3) + (a54 * k4));
    const PhotonDeriv k5 = blackHole.deriv(y5);

    const Photon y6 = photon + h * ((a61 * k1) + (a62 * k2) + (a63 * k3) + (a64 * k4) + (a65 * k5));
    const PhotonDeriv k6 = blackHole.deriv(y6);

    // 5th order solution
    const Photon ray5 = photon + h * ((b1 * k1) + (b3 * k3) + (b4 * k4) + (b5 * k5) + (b6 * k6));

    // 4th order solution uses k7 = f(ray5)
    const PhotonDeriv k7 = blackHole.deriv(ray5);
    const Photon ray4 = photon + h * ((bs1 * k1) + (bs3 * k3) + (bs4 * k4) + (bs5 * k5) + (bs6 * k6) + (bs7 * k7));

    const Photon err = ray5 - ray4;

    // Relative-ish error norm
    const double scale = std::max(1.0, maxVal(ray5));
    const double errN = maxVal(err) / scale;

    // Step controller
    const double safety = 0.8;
    const double minGrow = 0.2;
    const double maxGrow = 5.0;

    if (errN <= tol)
    {
        photon = ray5;
        double factor = (errN == 0.0) ? maxGrow : safety * std::pow(tol / errN, 0.2);
        factor = std::clamp(factor, safety, maxGrow);
        h *= factor;
        return true;
    }
    else
    {
        double factor = safety * std::pow(tol / errN, 0.2);
        factor = std::clamp(factor, minGrow, 1.0);
        h *= factor;
        return false;
    }
}

#pragma once
#include "BlackHole.h"

bool RK45Step(const BlackHole &blackHole, Photon &photon, double &h, double tol);
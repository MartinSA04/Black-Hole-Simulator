#include "SimulationWindow.h"

int main()
{

    int w = 1920;
    int h = 1080;

    SimulationWindow window{50,50,w,h};
    window.run();
    return 0;
}

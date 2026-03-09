#pragma once
#include "AnimationWindow.h"
#include "widgets/Button.h"
#include "widgets/Slider.h"
#include "widgets/CheckBox.h"
#include "BlackHole.h"
#include "renderer.h"

class SimulationWindow : public TDT4102::AnimationWindow
{
    // Layout constants
    static constexpr int panelH = 165;

    // Row 1: Button + checkboxes
    TDT4102::Button renderButton{{20, 12}, 120, 35, "Render"};
    TDT4102::CheckBox aaCheck{{155, 12}, 180, 35, "Anti-Aliasing"};
    TDT4102::CheckBox lowResCheck{{345, 12}, 180, 35, "Low Res Mode"};

    // Row 2: Disk + Infill sliders (labels drawn in draw_panel)
    TDT4102::Slider diskSlider{{160, 55}, 220, 25, 0, 50, 10, 1};
    TDT4102::Slider infillSlider{{530, 55}, 220, 25, 0, 100, 50, 1};

    // Row 3: Camera angle + Roll sliders
    TDT4102::Slider thetaSlider{{160, 87}, 220, 25, 10, 170, 80, 1};
    TDT4102::Slider rollSlider{{530, 87}, 220, 25, -180, 180, 0, 1};
    bool renderMode = true;
    bool frameRendered = false;
    bool isRendering = false;
    double renderProgress = 0.0;

    BlackHole blackHole{1};
    Scene scene{blackHole};
    Camera camera;
    std::vector<TDT4102::Color> frame;
    void render_();
    void draw_frame();
    void draw_panel();
    void draw_progress_bar();
    void writeFrameToFile();
    const std::string imagePath = "output/render.png";
    TDT4102::Image image;
    bool newRender = false;

    void draw_full_frame()
    {
        if (frameRendered)
        {
            draw_frame();
        }
        draw_panel();
        draw_progress_bar();
        next_frame();
    }

public:
    SimulationWindow(int x, int y, int w, int h, const std::string &title = "Simulation") : AnimationWindow(x, y, w, h, title), camera{w, h, 70}
    {
        setBackgroundColor(TDT4102::Color{0x16161d});
        std::function<void()> renderLambda = [&]()
        { newRender = true; };
        std::function<void()> renderLambdaLowRes = [&]()
        { if (lowResCheck.isSelected())
            newRender = true; };
        renderButton.setCallback(renderLambda);
        diskSlider.setCallback(renderLambdaLowRes);
        infillSlider.setCallback(renderLambdaLowRes);
        thetaSlider.setCallback(renderLambdaLowRes);
        rollSlider.setCallback(renderLambdaLowRes);
        lowResCheck.setCallback(renderLambdaLowRes);
        add(renderButton);
        add(aaCheck);
        add(lowResCheck);
        add(diskSlider);
        add(infillSlider);
        add(thetaSlider);
        add(rollSlider);
    };

    void run();
};
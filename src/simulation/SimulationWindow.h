#pragma once
#include "AnimationWindow.h"
#include "Camera.h"
#include "scene.h"
#include "widgets/Button.h"
#include "widgets/Slider.h"
#include "widgets/CheckBox.h"
#include "BlackHole.h"

class SimulationWindow : public TDT4102::AnimationWindow
{
    static constexpr int panelW = 360;
    static constexpr int panelPadding = 20;
    static constexpr int sliderW = panelW - 2 * panelPadding;
    static constexpr int valueX = 285;

    TDT4102::Button renderButton{{panelPadding, 20}, 140, 35, "Render"};
    TDT4102::CheckBox aaCheck{{180, 20}, 160, 35, "Anti-Aliasing"};
    TDT4102::CheckBox lowResCheck{{180, 75}, 160, 35, "Low Res Mode"};

    TDT4102::Slider diskSlider{{panelPadding, 215}, sliderW, 25, 0, 50, 10, 1};
    TDT4102::Slider infillSlider{{panelPadding, 310}, sliderW, 25, 0, 100, 50, 1};
    TDT4102::Slider thetaSlider{{panelPadding, 405}, sliderW, 25, 10, 170, 80, 1};
    TDT4102::Slider rollSlider{{panelPadding, 500}, sliderW, 25, -180, 180, 0, 1};
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
    SimulationWindow(int x, int y, int w, int h, const std::string &title = "Simulation")
        : AnimationWindow(x, y, w, h, title),
          camera{static_cast<std::size_t>(w), static_cast<std::size_t>(h), 70}
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

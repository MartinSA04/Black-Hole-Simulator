#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "SimulationWindow.h"
#include "stb_image_write.h"
#include <chrono>
#include <iostream>

void SimulationWindow::render_()
{
    if (!lowResCheck.isSelected())
        frameRendered = false;
    double cam_dist = 15.0;
    double theta = thetaSlider.getValue() * M_PI / 180.0;
    double rollAngle = -rollSlider.getValue() * M_PI / 180.0;
    Point3D camPos{std::sin(theta) * cam_dist, 0, std::cos(theta) * cam_dist};
    Vector3D upHint{0, std::sin(rollAngle), std::cos(rollAngle)};
    camera.setPose(camPos, {0, 0, 0}, upHint);
    camera.setFov(50);
    // TODO: These should use resolution input from gui.
    if (lowResCheck.isSelected())
    {
        camera.setSize(width() / 8, height() / 8);
    }
    else
    {
        camera.setSize(width(), height());
    }
    int AANum = 1;
    if (aaCheck.isSelected())
    {
        AANum = 5;
    }
    camera.setAANum(AANum);
    camera.generateRays();

    scene.createAccretionDisk(2, 4, diskSlider.getValue(), (float)infillSlider.getValue() / 100.0, TDT4102::Color::yellow, TDT4102::Color::red);

    isRendering = true;
    renderProgress = 0.0;

    auto start = std::chrono::high_resolution_clock::now();
    std::function<void(double)> progressCallback = [this](double progress)
    {
        this->renderProgress = progress;
        this->draw_full_frame();
    };
    render(scene, camera, frame, &progressCallback);

    isRendering = false;
    renderProgress = 1.0;
    writeFrameToFile();
    frameRendered = true;
    newRender = false;

    auto stop = std::chrono::high_resolution_clock::now();
    auto duration = duration_cast<std::chrono::milliseconds>(stop - start);

    std::cout << "Render time: " << duration.count() << " ms" << std::endl;
    RayCache::printStats();
}

void SimulationWindow::draw_frame()
{
    draw_image({0, 0}, image, width(), height());
}

void SimulationWindow::draw_panel()
{
    // Panel background
    draw_rectangle({0, 0}, 800, panelH, TDT4102::Color{25, 25, 35}, TDT4102::Color::transparent);
    draw_line({0, panelH}, {800, panelH}, TDT4102::Color{50, 50, 65});
    draw_line({800, 0}, {800, panelH}, TDT4102::Color{50, 50, 65});

    // Slider labels
    TDT4102::Color labelColor{170, 170, 185};
    TDT4102::Color valueColor{130, 195, 255};
    unsigned int labelSize = 20;

    // Row 2 labels + values
    draw_text({20, 60}, "Disk Particles", labelColor, labelSize);
    draw_text({385, 60}, std::to_string(diskSlider.getValue()) + "  ", valueColor, labelSize);
    draw_text({430, 60}, "Infill", labelColor, labelSize);
    draw_text({755, 60}, std::to_string(infillSlider.getValue()) + "%  ", valueColor, labelSize);

    // Row 3 labels + values
    draw_text({20, 92}, "Camera Angle", labelColor, labelSize);
    draw_text({385, 92}, std::to_string(thetaSlider.getValue()) + "  ", valueColor, labelSize);
    draw_text({430, 92}, "Roll", labelColor, labelSize);
    draw_text({755, 92}, std::to_string(rollSlider.getValue()) + "  ", valueColor, labelSize);
}

void SimulationWindow::draw_progress_bar()
{
    const int barX = 20;
    const int barY = 122;
    const int barW = 760;
    const int barH = 32;

    // Background
    draw_rectangle({barX, barY}, barW, barH, TDT4102::Color{35, 35, 48}, TDT4102::Color{55, 55, 70});

    // Filled portion
    if (renderProgress > 0.001)
    {
        int fillW = static_cast<int>(renderProgress * (barW - 2));
        if (fillW < 1)
            fillW = 1;
        TDT4102::Color fillColor = isRendering
                                       ? TDT4102::Color{65, 130, 230}
                                       : TDT4102::Color{75, 190, 115};
        draw_rectangle({barX + 1, barY + 1}, fillW, barH - 2, fillColor, TDT4102::Color::transparent);
    }

    // Text overlay
    int percent = static_cast<int>(renderProgress * 100);
    std::string text;
    if (isRendering)
        text = std::to_string(percent) + "%  Rendering...";
    else if (frameRendered)
        text = "Complete";
    else
        text = "Ready";
    draw_text({barX + 8, barY + 4}, text, TDT4102::Color{220, 220, 235}, 20);
}

void SimulationWindow::writeFrameToFile()
{
    std::vector<uint8_t> rgba(camera.width() * camera.height() * 4);
    int w = camera.width();
    int h = camera.height();
    for (int y = 0; y < h; ++y)
    {
        for (int x = 0; x < w; ++x)
        {
            int i = (y * w + x) * 4;
            TDT4102::Color color = frame.at(y * w + x);
            rgba[i + 0] = (uint8_t)color.redChannel;   // R
            rgba[i + 1] = (uint8_t)color.greenChannel; // G
            rgba[i + 2] = (uint8_t)color.blueChannel;  // B
            rgba[i + 3] = (uint8_t)color.alphaChannel; // A
        }
    }
    stbi_write_png(imagePath.data(), w, h, 4, rgba.data(), w * 4);
    image = TDT4102::Image{imagePath};
}

void SimulationWindow::run()
{
    while (!should_close())
    {
        if (newRender)
        {
            render_();
        }
        draw_full_frame();
    }
}
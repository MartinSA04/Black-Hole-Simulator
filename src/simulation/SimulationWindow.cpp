#include "renderer.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "SimulationWindow.h"
#include "stb_image_write.h"
#include <chrono>
#include <cctype>
#include <charconv>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string_view>

namespace
{
TDT4102::Color parseHexColor(const std::string &text, TDT4102::Color fallback)
{
    std::string_view value{text};
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.front())))
    {
        value.remove_prefix(1);
    }
    while (!value.empty() && std::isspace(static_cast<unsigned char>(value.back())))
    {
        value.remove_suffix(1);
    }

    if (value.starts_with('#'))
    {
        value.remove_prefix(1);
    }
    else if (value.starts_with("0x") || value.starts_with("0X"))
    {
        value.remove_prefix(2);
    }

    if (value.size() != 6 && value.size() != 8)
    {
        return fallback;
    }

    unsigned int hexValue = 0;
    const char *begin = value.data();
    const char *end = value.data() + value.size();
    const auto [ptr, error] = std::from_chars(begin, end, hexValue, 16);
    if (error != std::errc{} || ptr != end)
    {
        return fallback;
    }

    return TDT4102::Color{hexValue};
}
}

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
        camera.setSize(static_cast<std::size_t>(width() / 8), static_cast<std::size_t>(height() / 8));
    }
    else
    {
        camera.setSize(static_cast<std::size_t>(width()), static_cast<std::size_t>(height()));
    }
    std::size_t AANum = 1;
    if (aaCheck.isSelected())
    {
        AANum = 5;
    }
    camera.setAANum(AANum);
    camera.generateRays();

    scene.createAccretionDisk(
        2,
        4,
        diskSlider.getValue(),
        static_cast<float>(infillSlider.getValue()) / 100.0F,
        parseHexColor(startColorInput.getText(), TDT4102::Color::yellow),
        parseHexColor(endColorInput.getText(), TDT4102::Color::red));

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
    draw_rectangle({0, 0}, panelW, height(), TDT4102::Color{25, 25, 35, 190}, TDT4102::Color::transparent);
    draw_line({panelW, 0}, {panelW, height()}, TDT4102::Color{50, 50, 65});

    TDT4102::Color labelColor{170, 170, 185};
    TDT4102::Color valueColor{130, 195, 255};
    unsigned int labelSize = 20;

    draw_text({panelPadding, 160}, "Disk Particles", labelColor, labelSize);
    draw_text({valueX, 160}, std::to_string(diskSlider.getValue()), valueColor, labelSize);

    draw_text({panelPadding, 255}, "Infill", labelColor, labelSize);
    draw_text({valueX, 255}, std::to_string(infillSlider.getValue()) + "%", valueColor, labelSize);

    draw_text({panelPadding, 350}, "Camera Angle", labelColor, labelSize);
    draw_text({valueX, 350}, std::to_string(thetaSlider.getValue()), valueColor, labelSize);

    draw_text({panelPadding, 445}, "Roll", labelColor, labelSize);
    draw_text({valueX, 445}, std::to_string(rollSlider.getValue()), valueColor, labelSize);

    const TDT4102::Color startColor = parseHexColor(startColorInput.getText(), TDT4102::Color::yellow);
    const TDT4102::Color endColor = parseHexColor(endColorInput.getText(), TDT4102::Color::red);

    draw_text({startColorInputX, colorSectionLabelY}, "Start", labelColor, labelSize);
    draw_rectangle(
        {startColorPreviewX, colorInputY},
        colorPreviewSize,
        colorPreviewSize,
        startColor,
        TDT4102::Color{55, 55, 70});

    draw_text({endColorInputX, colorSectionLabelY}, "End", labelColor, labelSize);
    draw_rectangle(
        {endColorPreviewX, colorInputY},
        colorPreviewSize,
        colorPreviewSize,
        endColor,
        TDT4102::Color{55, 55, 70});
}

void SimulationWindow::draw_progress_bar()
{
    const int barX = panelPadding;
    const int barY = 570;
    const int barW = sliderW;
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
    const std::size_t w = camera.width();
    const std::size_t h = camera.height();
    for (std::size_t y = 0; y < h; ++y)
    {
        for (std::size_t x = 0; x < w; ++x)
        {
            const std::size_t i = (y * w + x) * 4;
            const TDT4102::Color color = frame.at(y * w + x);
            rgba[i] = static_cast<uint8_t>(color.redChannel);       // R
            rgba[i + 1] = static_cast<uint8_t>(color.greenChannel); // G
            rgba[i + 2] = static_cast<uint8_t>(color.blueChannel);  // B
            rgba[i + 3] = static_cast<uint8_t>(color.alphaChannel); // A
        }
    }
    stbi_write_png(
        imagePath.data(),
        static_cast<int>(w),
        static_cast<int>(h),
        4,
        rgba.data(),
        static_cast<int>(w * 4));
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

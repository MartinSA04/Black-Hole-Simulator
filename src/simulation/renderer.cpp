#include "renderer.h"
#include <atomic>
#include <thread>
#include <algorithm>
#include <unordered_map>
#include <functional>

static TDT4102::Color avgColor(std::vector<TDT4102::Color> colors)
{
    unsigned int rChannel = 0;
    unsigned int gChannel = 0;
    unsigned int bChannel = 0;
    unsigned int alphaChannel = 0;
    for (int i = 0; i < colors.size(); ++i)
    {
        TDT4102::Color color = colors.at(i);
        rChannel += color.redChannel;
        gChannel += color.greenChannel;
        bChannel += color.blueChannel;
        alphaChannel += color.alphaChannel;
    }
    rChannel /= colors.size();
    gChannel /= colors.size();
    bChannel /= colors.size();
    alphaChannel /= colors.size();
    return TDT4102::Color{rChannel, gChannel, bChannel, alphaChannel};
}

static RayStatus getStatus(std::vector<RayStatus> statuses)
{
    std::unordered_map<RayStatus, int> counts;
    for (RayStatus s : statuses)
    {
        counts[s]++;
    }
    int maxCount = 0;
    RayStatus mostFrequentElement = RayStatus::Escaped;

    for (const auto &pair : counts)
    {
        if (pair.second > maxCount)
        {
            maxCount = pair.second;
            mostFrequentElement = pair.first;
        }
    }

    return mostFrequentElement;
}

inline double colorDiff(const TDT4102::Color &a, const TDT4102::Color &b)
{
    return std::max({std::abs(a.redChannel - b.redChannel), std::abs(a.greenChannel - b.greenChannel), std::abs(a.blueChannel - b.blueChannel)});
}

static void fillRect(
    std::vector<TDT4102::Color> &frame,
    int W,
    int x0,
    int y0,
    int x1,
    int y1,
    const TDT4102::Color &color)
{
    for (int y = y0; y < y1; ++y)
    {
        for (int x = x0; x < x1; ++x)
        {
            frame[(std::size_t)y * (std::size_t)W + (std::size_t)x] = color;
        }
    }
}

static bool isUniform(std::vector<Sample> samples, const double uniformEps)
{

    const RayStatus st = samples[0].status;
    const TDT4102::Color c = samples[0].color;
    double err = 0.0;
    for (Sample &s : samples)
    {
        if (s.status != st)
            return false;
        const double cDiff = colorDiff(c, s.color);
        if (cDiff > err)
            err = cDiff;
    }
    return (err <= uniformEps);
}

static inline bool checkSample(const Sample &s, const Sample &ref, double eps)
{
    if (s.status != ref.status)
        return false;
    if (colorDiff(s.color, ref.color) > eps)
        return false;
    return true;
}

static bool sampleUniform(RayCache &cache, Sample &sampleOut, int x0, int y0, int x1, int y1, double uniformEps)
{

    const int w = x1 - x0;
    const int h = y1 - y0;
    // Sample corners + center
    const int xA = x0;
    const int xB = x1 - 1;
    const int yA = y0;
    const int yB = y1 - 1;

    const Sample ref = cache.get(xA, yA);

    if (!checkSample(cache.get(xB, yA), ref, uniformEps))
        return false;
    if (!checkSample(cache.get(xA, yB), ref, uniformEps))
        return false;
    if (!checkSample(cache.get(xB, yB), ref, uniformEps))
        return false;

    const int x14C = x0 + w / 4;
    const int y14C = y0 + h / 4;
    const int x34C = x0 + (3 * w) / 4;
    const int y34C = y0 + (3 * h) / 4;

    if (!checkSample(cache.get(x14C, y14C), ref, uniformEps))
        return false;
    if (!checkSample(cache.get(x14C, y34C), ref, uniformEps))
        return false;
    if (!checkSample(cache.get(x34C, y14C), ref, uniformEps))
        return false;
    if (!checkSample(cache.get(x34C, y34C), ref, uniformEps))
        return false;

    for (int dx = 0; dx < w; dx += 2)
    {
        if (!checkSample(cache.get(xA + dx, yA), ref, uniformEps))
            return false;
        if (!checkSample(cache.get(xA + dx, yB), ref, uniformEps))
            return false;
    }
    for (int dy = 0; dy < h; dy += 2)
    {
        if (!checkSample(cache.get(xA, yA + dy), ref, uniformEps))
            return false;
        if (!checkSample(cache.get(xB, yA + dy), ref, uniformEps))
            return false;
    }
    sampleOut = ref;
    return true;
}

static void renderQuad(
    RayCache &cache,
    std::vector<TDT4102::Color> &frame,
    int x0,
    int y0,
    int x1,
    int y1,
    int depth,
    int maxDepth,
    double uniformEps)
{

    const int w = x1 - x0;
    const int h = y1 - y0;
    if (w <= 0 || h <= 0)
        return;

    if (w == 1 and h == 1)
    {
        const Sample s = cache.get(x0, y0);
        frame[(std::size_t)y0 * (std::size_t)cache.camW() + (std::size_t)x0] = s.color;
        return;
    }

    Sample sample{};
    const bool uniform = sampleUniform(cache, sample, x0, y0, x1, y1, uniformEps);

    if (uniform or depth >= maxDepth)
    {
        if (uniform)
        {
            fillRect(frame, cache.camW(), x0, y0, x1, y1, sample.color);
        }
        else
        {
            // brute-force render when maxDepth hit
            for (int y = y0; y < y1; ++y)
            {
                for (int x = x0; x < x1; ++x)
                {
                    const Sample sample = cache.get(x, y);
                    frame[(std::size_t)y * (std::size_t)cache.camW() + (std::size_t)x] = sample.color;
                }
            }
        }
        return;
    }

    // Split into 4 sub-rectangles
    const int xm = x0 + w / 2;
    const int ym = y0 + h / 2;

    renderQuad(cache, frame, x0, y0, xm, ym, depth + 1, maxDepth, uniformEps);
    renderQuad(cache, frame, xm, y0, x1, ym, depth + 1, maxDepth, uniformEps);
    renderQuad(cache, frame, x0, ym, xm, y1, depth + 1, maxDepth, uniformEps);
    renderQuad(cache, frame, xm, ym, x1, y1, depth + 1, maxDepth, uniformEps);
}

void render(const Scene &scene, Camera &cam, std::vector<TDT4102::Color> &frame, std::function<void(double)> *loadFunction)
{
    const int W = cam.width();
    const int H = cam.height();
    frame.resize(W * H);

    // Renders 64X64 tiles
    const int tile = 64;
    const int tilesX = (W + tile - 1) / tile;
    const int tilesY = (H + tile - 1) / tile;
    const int numTiles = tilesX * tilesY;

    std::atomic<int> nextTile{0};
    std::atomic<long> renderedPixels{0};

    const unsigned nThreads = std::max(1u, std::thread::hardware_concurrency());
    std::vector<std::thread> threads;
    threads.reserve(nThreads);

    auto worker = [&]()
    {
        while (true)
        {
            int t = nextTile.fetch_add(1);
            if (t >= numTiles)
                break;

            int tx = t % tilesX;
            int ty = t / tilesX;

            int x0 = tx * tile;
            int y0 = ty * tile;
            int x1 = std::min(x0 + tile, W);
            int y1 = std::min(y0 + tile, H);
            RayCache cache{scene, cam, x0, y0, x1, y1};
            renderQuad(cache, frame, x0, y0, x1, y1, 0, 8, 1e-3);
            const int w = x1 - x0;
            const int h = y1 - y0;
            renderedPixels.fetch_add(w * h);
        }
    };
    for (unsigned i = 0; i < nThreads; ++i)
        threads.emplace_back(worker);

    const long totalPixels = static_cast<long>(W) * static_cast<long>(H);
    const int barWidth = 40;

    while (true)
    {
        const long done = renderedPixels.load();
        const double ratio = std::min(1.0, static_cast<double>(done) / static_cast<double>(totalPixels));
        const int filled = static_cast<int>(ratio * barWidth);
        const int percentDone = static_cast<int>(ratio * 100.0);

        std::cout << "\r[";
        for (int i = 0; i < barWidth; ++i)
        {
            std::cout << (i < filled ? '=' : ' ');
        }
        std::cout << "] " << percentDone << "%" << std::flush;

        if (loadFunction != nullptr)
            (*loadFunction)(ratio);

        if (done >= totalPixels)
            break;

        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
    std::cout << std::endl;

    for (auto &th : threads)
        th.join();
}
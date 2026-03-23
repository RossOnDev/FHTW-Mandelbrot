// mandelbrot_benchmark.cpp
// Compile: g++ -O2 -fopenmp mandelbrot_benchmark.cpp -o mandelbrot

#include <iostream>
#include <vector>
#include <cmath>
#include <algorithm>
#include <string>
#include <omp.h>
#include <fstream>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

const int WIDTH  = 2048;
const int HEIGHT = 2048;

const double MIN_X = -2.0;
const double MAX_X =  1.0;
const double MIN_Y = -1.0;
const double MAX_Y =  1.0;

// const double MIN_X = -0.743643887037151 - 0.0005;
// const double MAX_X = -0.743643887037151 + 0.0005;
// const double MIN_Y =  0.131825904205330 - 0.0005;
// const double MAX_Y =  0.131825904205330 + 0.0005;

const int MAX_ITER = 1000;

struct BenchmarkPoint {
    int threads;
    double time;
    double speedup;
};

inline int mandelbrot(double cx, double cy, int max_iter) {
    double zx = cx;
    double zy = cy;

    for (int i = 0; i < max_iter; ++i) {
        double x = zx * zx - zy * zy + cx;
        double y = 2.0 * zx * zy + cy;

        zx = x;
        zy = y;

        if (zx * zx + zy * zy > 4.0)
            return i;
    }
    return max_iter;
}

inline void getColor(int iter, unsigned char &r, unsigned char &g, unsigned char &b) {
    if (iter == MAX_ITER) {
        r = g = b = 0;
        return;
    }

    double t = (double)iter / MAX_ITER;

    r = (unsigned char)(9*(1-t)*t*t*t*255);
    g = (unsigned char)(15*(1-t)*(1-t)*t*t*255);
    b = (unsigned char)(8.5*(1-t)*(1-t)*(1-t)*t*255);
}

// AI generated
inline void set_pixel(std::vector<unsigned char>& img, int w, int h, int x, int y,
                      unsigned char r, unsigned char g, unsigned char b) {
    if (x < 0 || x >= w || y < 0 || y >= h) {
        return;
    }
    int idx = (y * w + x) * 3;
    img[idx + 0] = r;
    img[idx + 1] = g;
    img[idx + 2] = b;
}

// AI generated
void draw_line(std::vector<unsigned char>& img, int w, int h,
               int x0, int y0, int x1, int y1,
               unsigned char r, unsigned char g, unsigned char b) {
    int dx = std::abs(x1 - x0);
    int sx = (x0 < x1) ? 1 : -1;
    int dy = -std::abs(y1 - y0);
    int sy = (y0 < y1) ? 1 : -1;
    int err = dx + dy;

    while (true) {
        set_pixel(img, w, h, x0, y0, r, g, b);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        int e2 = 2 * err;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

// AI generated
void save_speedup_graph(const std::vector<BenchmarkPoint>& points, const std::string& filename) {
    if (points.empty()) {
        return;
    }

    const int graph_w = 1000;
    const int graph_h = 600;
    const int margin_l = 80;
    const int margin_r = 40;
    const int margin_t = 40;
    const int margin_b = 70;

    std::vector<unsigned char> img(graph_w * graph_h * 3, 255);

    int max_threads = 1;
    double max_speedup = 1.0;
    for (const auto& p : points) {
        max_threads = std::max(max_threads, p.threads);
        max_speedup = std::max(max_speedup, p.speedup);
    }

    auto map_x = [&](int threads) {
        if (max_threads == 1) {
            return margin_l;
        }
        double t = static_cast<double>(threads - 1) / static_cast<double>(max_threads - 1);
        return margin_l + static_cast<int>(t * (graph_w - margin_l - margin_r));
    };

    auto map_y = [&](double speedup) {
        if (max_speedup <= 1.0) {
            return graph_h - margin_b;
        }
        double t = (speedup - 1.0) / (max_speedup - 1.0);
        return (graph_h - margin_b) - static_cast<int>(t * (graph_h - margin_t - margin_b));
    };

    // Axes
    draw_line(img, graph_w, graph_h, margin_l, graph_h - margin_b, graph_w - margin_r, graph_h - margin_b, 0, 0, 0);
    draw_line(img, graph_w, graph_h, margin_l, margin_t, margin_l, graph_h - margin_b, 0, 0, 0);

    // Tick marks
    for (int i = 0; i <= 5; ++i) {
        int x = margin_l + i * (graph_w - margin_l - margin_r) / 5;
        int y = graph_h - margin_b;
        draw_line(img, graph_w, graph_h, x, y - 5, x, y + 5, 0, 0, 0);
    }
    for (int i = 0; i <= 5; ++i) {
        int x = margin_l;
        int y = margin_t + i * (graph_h - margin_t - margin_b) / 5;
        draw_line(img, graph_w, graph_h, x - 5, y, x + 5, y, 0, 0, 0);
    }

    // Measured speedup curve
    for (size_t i = 1; i < points.size(); ++i) {
        int x0 = map_x(points[i - 1].threads);
        int y0 = map_y(points[i - 1].speedup);
        int x1 = map_x(points[i].threads);
        int y1 = map_y(points[i].speedup);
        draw_line(img, graph_w, graph_h, x0, y0, x1, y1, 0, 90, 220);
    }

    // Point markers
    for (const auto& p : points) {
        int cx = map_x(p.threads);
        int cy = map_y(p.speedup);
        for (int dy = -3; dy <= 3; ++dy) {
            for (int dx = -3; dx <= 3; ++dx) {
                if (dx * dx + dy * dy <= 9) {
                    set_pixel(img, graph_w, graph_h, cx + dx, cy + dy, 220, 40, 40);
                }
            }
        }
    }

    stbi_write_png(filename.c_str(), graph_w, graph_h, 3, img.data(), graph_w * 3);
}

double run_serial(std::vector<int>& buffer) {
    double dx = (MAX_X - MIN_X) / WIDTH;
    double dy = (MAX_Y - MIN_Y) / HEIGHT;

    double start = omp_get_wtime();

    for (int py = 0; py < HEIGHT; ++py) {
        for (int px = 0; px < WIDTH; ++px) {
            double cx = MIN_X + px * dx;
            double cy = MIN_Y + py * dy;

            buffer[py * WIDTH + px] = mandelbrot(cx, cy, MAX_ITER);
        }
    }

    double end = omp_get_wtime();
    return end - start;
}

double run_parallel(std::vector<int>& buffer, int threads) {
    double dx = (MAX_X - MIN_X) / WIDTH;
    double dy = (MAX_Y - MIN_Y) / HEIGHT;

    omp_set_num_threads(threads);

    double start = omp_get_wtime();

    #pragma omp parallel for schedule(dynamic, 1)
    for (int py = 0; py < HEIGHT; ++py) {
        for (int px = 0; px < WIDTH; ++px) {
            double cx = MIN_X + px * dx;
            double cy = MIN_Y + py * dy;

            buffer[py * WIDTH + px] = mandelbrot(cx, cy, MAX_ITER);
        }
    }

    double end = omp_get_wtime();
    return end - start;
}

void save_image(const std::vector<int>& buffer) {
    std::vector<unsigned char> img(WIDTH * HEIGHT * 3);

    for (int py = 0; py < HEIGHT; ++py) {
        for (int px = 0; px < WIDTH; ++px) {
            int iter = buffer[py * WIDTH + px];

            unsigned char r,g,b;
            getColor(iter, r, g, b);

            int idx = (py * WIDTH + px) * 3;
            img[idx+0] = r;
            img[idx+1] = g;
            img[idx+2] = b;
        }
    }

    stbi_write_png("mandelbrot.png", WIDTH, HEIGHT, 3, img.data(), WIDTH*3);
}

int main() {
    std::vector<int> buffer(WIDTH * HEIGHT);
    std::vector<BenchmarkPoint> points;

    std::cout << "Benchmarking Mandelbrot (" << WIDTH << "x" << HEIGHT << ")\n";

    // SERIAL
    double Ts = run_serial(buffer);
    points.push_back({1, Ts, 1.0});
    std::cout << "Serial Time: " << Ts << " s\n\n";

    // PARALLEL TESTS
    int max_threads = omp_get_max_threads();

    for (int t = 2; t <= max_threads; t *= 2) {
        double Tn = run_parallel(buffer, t);
        double speedup = Ts / Tn;
        points.push_back({t, Tn, speedup});

        std::cout << "Threads: " << t
                  << " | Time: " << Tn
                  << " | Speedup: " << speedup << "\n";
    }

    save_speedup_graph(points, "speedup.png");
    std::cout << "Saved speedup.png\n";

    std::cout << "\nGenerating final image...\n";
    run_parallel(buffer, max_threads);
    save_image(buffer);

    std::cout << "Saved mandelbrot.png\n";

    return 0;
}
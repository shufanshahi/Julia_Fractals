#include <iostream>
#include <vector>
#include <cmath>
#include <string>
#include <filesystem>

#define _CRT_SECURE_NO_WARNINGS
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;


struct Vec2 {
    double x, y;
    Vec2(double x = 0.0, double y = 0.0) : x(x), y(y) {}
    Vec2 operator+(const Vec2& other) const {
        return Vec2(x + other.x, y + other.y);
    }
};

struct IVec2 {
    int x, y;
    IVec2(int x = 0, int y = 0) : x(x), y(y) {}
};

double mod2(Vec2 z) {
    return z.x * z.x + z.y * z.y;
}

Vec2 computeNext(Vec2 current, Vec2 constant) {
    double zr = current.x * current.x - current.y * current.y;
    double zi = 2.0 * current.x * current.y;
    return Vec2(zr, zi) + constant;
}

int computeIteration(Vec2 z0, Vec2 constant, int max_iteration) {
    Vec2 zn = z0;
    int iteration = 0;
    while (mod2(zn) < 4.0 && iteration < max_iteration) {
        zn = computeNext(zn, constant);
        iteration++;
    }
    return iteration;
}

struct Gradient {
    vector<uint8_t> getColor(int iteration, int max_iteration) const {
        if (iteration == max_iteration) {
            return { 0, 0, 0 }; // Black for points inside the set
        }
        double t = double(iteration) / max_iteration; // Original interpolation
        uint8_t r = static_cast<uint8_t>(9 * (1 - t) * t * t * t * 255);
        uint8_t g = static_cast<uint8_t>(15 * (1 - t) * (1 - t) * t * t * 255);
        uint8_t b = static_cast<uint8_t>(8.5 * (1 - t) * (1 - t) * (1 - t) * t * 255);
        return { r, g, b };
    }
};

void render(const IVec2& render_size, const Vec2& center, double scale, int max_iteration, const string& output_filename) {
    vector<uint8_t> image(render_size.x * render_size.y * 3, 0); // RGB image
    Gradient gradient;

    for (int y = 0; y < render_size.y; ++y) {
        for (int x = 0; x < render_size.x; ++x) {
            double px = center.x + (x - render_size.x / 2) * scale;
            double py = center.y + (y - render_size.y / 2) * scale;
            int iteration = computeIteration({ px, py }, center, max_iteration);
            auto color = gradient.getColor(iteration, max_iteration);
            int idx = (y * render_size.x + x) * 3;
            image[idx] = color[0];
            image[idx + 1] = color[1];
            image[idx + 2] = color[2];
        }
    }

    stbi_write_jpg(output_filename.c_str(), render_size.x, render_size.y, 3, image.data(), 100);
    cout << "Image saved as " << output_filename << endl;
}

int main() {
    IVec2 render_size(1920, 1080); // High resolution
    Vec2 center(-0.70176, -0.3842); // Julia fractal center
    double scale = 4.0 / render_size.y; // Initial scale
    int max_iteration = 500; // Starting iteration count

    string output_folder = "Output";

    // Generate zoom images
    for (int i = 0; i < 1000; ++i) {
        char filename[50];
        sprintf_s(filename, "%s/image_%04d.jpg", output_folder.c_str(), i + 1);

        render(render_size, center, scale, max_iteration, filename);

        // Adjust scale and refine parameters
        scale *= 0.98; // Maintain original zoom speed
        max_iteration = static_cast<int>(500 + log(1.0 / scale) * 50);
        // Center remains fixed for consistent view
    }

    return 0;
}

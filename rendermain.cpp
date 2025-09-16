#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

#include <vector>
#include <map>
#include <string>
#include <iostream>

struct Block {
    int x, y, z, w, h, d;
    std::string label;
};

// Map labels to RGB colors
struct RGB { unsigned char r, g, b; };

RGB getColor(const std::string& label) {
    static std::map<std::string, RGB> colors = {
        {"sea", {100, 149, 237}},  // Cornflower blue
        {"WA",  {46, 139, 87}},    // Sea green
        {"NT",  {218, 165, 32}},   // Goldenrod
        {"SA",  {205, 92, 92}},    // Indian red
        {"QLD", {255, 215, 0}},    // Gold
        {"NSW", {70, 130, 180}},   // Steel blue
        {"VIC", {123, 104, 238}},  // Slate blue
        {"TAS", {34, 139, 34}}     // Forest green
    };
    auto it = colors.find(label);
    return (it != colors.end()) ? it->second : RGB{255, 255, 255};
}

int main() {
    int cols = 16, rows = 6, scale = 40;
    int barWidth = 10;

    // Compressed blocks
    std::vector<Block> blocks = {
        {0,0,0,3,1,1,"NSW"},
        {3,0,0,1,1,1,"sea"},
        {4,0,0,4,1,2,"sea"},
        {8,0,0,4,1,2,"sea"},
        {12,0,0,4,1,2,"sea"},
        {1,1,0,3,2,1,"WA"},
        {4,1,0,4,2,1,"NT"},
        {8,1,0,3,2,1,"NSW"},
        {11,1,0,1,2,1,"QLD"},
        {12,1,0,3,2,1,"QLD"},
        {1,3,0,3,2,1,"SA"},
        {4,3,0,4,2,1,"VIC"},
        {8,3,0,4,2,1,"TAS"},
        {0,5,0,2,1,1,"SA"},
        {2,5,0,2,1,1,"sea"}
    };

    // Raw input grid (characters -> labels)
    std::vector<std::string> raw = {
        "eeeooooooooooooo",
        "owwwnnnneeeqqqqo",
        "owwwnnnneeeqqqqo",
        "osssvvvvttttoooo",
        "osssvvvvttttoooo",
        "ssoooooooooooooo"
    };

    std::map<char, std::string> legend = {
        {'o', "sea"},
        {'w', "WA"},
        {'n', "NT"},
        {'s', "SA"},
        {'q', "QLD"},
        {'e', "NSW"},
        {'v', "VIC"},
        {'t', "TAS"}
    };

    // Image dimensions
    int width = (2 * cols * scale) + barWidth;
    int height = rows * scale;
    int channels = 3;
    std::vector<unsigned char> pixels(width * height * channels, 255); // white background

    // ---- Draw compressed blocks on left ----
        // ---- Draw compressed blocks on left ----
    for (const auto& b : blocks) {
        RGB color = getColor(b.label);

        int startX = b.x * scale;
        int startY = b.y * scale;
        int endX = startX + b.w * scale;
        int endY = startY + b.h * scale;

        // Fill rectangle
        for (int py = startY; py < endY; py++) {
            for (int px = startX; px < endX; px++) {
                int idx = (py * width + px) * channels;
                pixels[idx + 0] = color.r;
                pixels[idx + 1] = color.g;
                pixels[idx + 2] = color.b;
            }
        }

        // Draw black border
        for (int px = startX; px < endX; px++) {
            // Top
            int idxTop = (startY * width + px) * channels;
            pixels[idxTop + 0] = pixels[idxTop + 1] = pixels[idxTop + 2] = 0;

            // Bottom
            int idxBottom = ((endY - 1) * width + px) * channels;
            pixels[idxBottom + 0] = pixels[idxBottom + 1] = pixels[idxBottom + 2] = 0;
        }
        for (int py = startY; py < endY; py++) {
            // Left
            int idxLeft = (py * width + startX) * channels;
            pixels[idxLeft + 0] = pixels[idxLeft + 1] = pixels[idxLeft + 2] = 0;

            // Right
            int idxRight = (py * width + (endX - 1)) * channels;
            pixels[idxRight + 0] = pixels[idxRight + 1] = pixels[idxRight + 2] = 0;
        }
    }


        // ---- Draw raw grid blocks on right ----
    for (int y = 0; y < rows; y++) {
        for (int x = 0; x < cols; x++) {
            char ch = raw[y][x];
            std::string label = legend[ch];
            RGB color = getColor(label);

            int startX = (cols * scale + barWidth) + (x * scale);
            int startY = y * scale;
            int endX = startX + scale;
            int endY = startY + scale;

            // Fill rectangle
            for (int py = startY; py < endY; py++) {
                for (int px = startX; px < endX; px++) {
                    int idx = (py * width + px) * channels;
                    pixels[idx + 0] = color.r;
                    pixels[idx + 1] = color.g;
                    pixels[idx + 2] = color.b;
                }
            }

            // Draw black border
            for (int px = startX; px < endX; px++) {
                // Top
                int idxTop = (startY * width + px) * channels;
                pixels[idxTop + 0] = pixels[idxTop + 1] = pixels[idxTop + 2] = 0;

                // Bottom
                int idxBottom = ((endY - 1) * width + px) * channels;
                pixels[idxBottom + 0] = pixels[idxBottom + 1] = pixels[idxBottom + 2] = 0;
            }
            for (int py = startY; py < endY; py++) {
                // Left
                int idxLeft = (py * width + startX) * channels;
                pixels[idxLeft + 0] = pixels[idxLeft + 1] = pixels[idxLeft + 2] = 0;

                // Right
                int idxRight = (py * width + (endX - 1)) * channels;
                pixels[idxRight + 0] = pixels[idxRight + 1] = pixels[idxRight + 2] = 0;
            }
        }
    }

    // ---- Draw separator bar ----
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < barWidth; x++) {
            int px = (cols * scale) + x;
            int py = y;
            int idx = (py * width + px) * channels;
            pixels[idx + 0] = 0;
            pixels[idx + 1] = 0;
            pixels[idx + 2] = 0;
        }
    }

    // Save PNG
    if (stbi_write_png("comparison.png", width, height, channels, pixels.data(), width * channels)) {
        std::cout << "✅ Saved comparison.png\n";
    } else {
        std::cerr << "❌ Failed to save PNG\n";
    }

    return 0;
}

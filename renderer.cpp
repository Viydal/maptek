#include <iostream>
#include <vector>
#include <string>

struct Block {
    int x_pos, y_pos, z_pos;
    int x_size, y_size, z_size;
    std::string ch;
};

void render(const std::vector<Block>& blocks, int width, int height, int depth) {
    // Create a 3D grid filled with dots (empty sea by default)
    std::vector<std::vector<std::vector<char>>> grid(
        depth, std::vector<std::vector<char>>(height, std::vector<char>(width, '.'))
    );

    // Paint blocks into the grid
    for (auto& b : blocks) {
        for (int z = b.z_pos; z < b.z_pos + b.z_size; ++z) {
            for (int y = b.y_pos; y < b.y_pos + b.y_size; ++y) {
                for (int x = b.x_pos; x < b.x_pos + b.x_size; ++x) {
                    grid[z][y][x] = b.ch[0]; // Use first letter of name
                }
            }
        }
    }

    // Print each layer
    for (int z = 0; z < depth; ++z) {
        std::cout << "Layer z=" << z << "\n";
        for (int y = 0; y < height; ++y) {
            for (int x = 0; x < width; ++x) {
                std::cout << grid[z][y][x];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<Block> blocks = {
        {0,0,0,8,3,1,"SEA"},
        {8,0,0,8,3,1,"SEA"},
        {0,1,0,1,4,1,"SEA"},
        {1,1,0,3,2,1,"WA"},
        {4,1,0,4,2,1,"NT"},
        {8,1,0,3,2,1,"NSW"},
        {11,1,0,4,2,1,"QLD"},
        {15,1,0,1,2,1,"SEA"},
        {1,3,0,3,2,1,"SA"},
        {4,3,0,4,2,1,"VIC"},
        {8,3,0,4,2,1,"TAS"},
        {12,3,0,4,2,1,"SEA"},
        {0,6,0,6,1,1,"SEA"},
        {6,6,0,2,1,1,"TAS"},
        {8,6,0,2,1,1,"TAS"},
        {10,6,0,6,1,1,"SEA"},
        // Add z=1 blocks if you want here…
    };

    render(blocks, 16, 7, 2); // width=16, height=7, depth=2
}

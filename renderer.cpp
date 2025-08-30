#include <iostream>
#include <vector>
#include <string>

struct Block {
    int XPos, YPos, ZPos;
    int XSize, YSize, ZSize;
    std::string Ch;
};

void Render(const std::vector<Block>& Blocks, int Width, int Height, int Depth) {
    // Create a 3D grid filled with dots (empty sea by default)
    std::vector<std::vector<std::vector<char>>> Grid(
        Depth, std::vector<std::vector<char>>(Height, std::vector<char>(Width, '.'))
    );

    // Paint blocks into the grid
    for (auto& B : Blocks) {
        for (int Z = B.ZPos; Z < B.ZPos + B.ZSize; ++Z) {
            for (int Y = B.YPos; Y < B.YPos + B.YSize; ++Y) {
                for (int X = B.XPos; X < B.XPos + B.XSize; ++X) {
                    Grid[Z][Y][X] = B.Ch[0]; // Use first letter of name
                }
            }
        }
    }

    // Print each layer
    for (int Z = 0; Z < Depth; ++Z) {
        std::cout << "Layer z=" << Z << "\n";
        for (int Y = 0; Y < Height; ++Y) {
            for (int X = 0; X < Width; ++X) {
                std::cout << Grid[Z][Y][X];
            }
            std::cout << "\n";
        }
        std::cout << "\n";
    }
}

int main() {
    std::vector<Block> Blocks = {
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

    Render(Blocks, 16, 7, 2); // Width=16, Height=7, Depth=2
}

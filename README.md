# Block Model Compression Algorithm
## Software Engineering Project – Semester 2, 2025

This repository contains our implementation for the Block Model Compression Algorithm, a gamified software engineering challenge involving lossless compression of large 3D block models with the goal of optimising compression ratio and execution speed.

The algorithm reads a block model from standard input in a specified format, compresses it by grouping uniform-colour regions into larger rectangular blocks (up to a given parent block size), and writes the compressed result to standard output.

Command to create .exe for titan (If download is required to run then download the second option):
x86_64-w64-mingw32-g++ -static-libgcc -static-libstdc++ -o chill.exe main.cpp Parse.cpp Compression.cpp MultiLineCompression.cpp

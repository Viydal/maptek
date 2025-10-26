# Block Model Compression Algorithm
> Software Engineering Project – Semester 2, 2025

## Table of Contents

- [About](#about)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)

## About

This repository contains our implementation for the Block Model Compression Algorithm, a gamified software engineering challenge involving lossless compression of large 3D block models with the goal of optimising compression ratio and execution speed.

The algorithm reads a block model from standard input in a specified format, compresses it by grouping uniform-colour regions into larger rectangular blocks (up to a given parent block size), and writes the compressed result to standard output.

## Features

This compression algorithm offers the following capabilities:

- **Lossless Compression**: Maintains the content of the input without compromising its integrity
- **Optimised Performance**: Achieves **80%+** compression from original size with 0.6s execution time
- **Customisable**: The input file can be customised to various block models following the [standard input format](#standard-input-format)
- **Abstraction**: Minimal setup required, no modification of code necessary

## Installation

Make sure you have the following installed:
- C++17 or higher
- Git

You can either **clone** the repository locally, or **fork** it if you intend to contribute:
- `git clone https://github.com/Viydal/maptek.git`
- `cd maptek`

## Usage

The **Block Model Compression ALgorithm** reads its input form `stdin` and writes the compressed result to `stdout`.

### Standard Input Format

The input stream follows a specific structure consisting of three parts:

#### 1. **Header Line**

The first line contains **six** comma-seperated natural numbers:<br><br>
`x_count, y_count, z_count, parent_x, parent_y, parent_z`

- `x_count`, `y_count`, `z_count`: dimensions of the model (total size).
- `parent_x`, `parent_y`, `parent_z`: parent block sizes used for compression (to compress irrespective of parent block size set parent block size to dimension of model).

#### 2. **Tag Table**

The next set of lines define key-value pairs in terms of tag and label, one per line:<br><br>
`tag, label`

- The tag table ends with a blank line.

#### 3. **Block Data**

The remainder of the input specifies the model itself as a 3D grid of tag characters:

### Standard Output Format

The compressed output will contain one line per compressed block, with seven comma-seperated values:<br><br>
`x_position, y_position, z_position, x_size, y_size, z_size, label`

- `x_position`, `y_position`, `z_position`: coordinate of the block.
- `x_size`, `y_size`, `z_size`: dimensions of the compressed block.
- `label`: corresponding label from the tag table.

### Compression 

## How It Works

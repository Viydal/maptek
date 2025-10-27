# Block Model Compression Algorithm
> Software Engineering Project – Semester 2, 2025

## Table of Contents

- [About](#about)
- [Features](#features)
- [Installation](#installation)
- [Usage](#usage)
- [How It Works](#how-it-works)
- [Extra Info](#extra-info)

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

The **Block Model Compression ALgorithm** reads its input form `stdin` and writes the compressed result to `stdout`. For information on how to format the input file, please visit the [standard input format](#standard-input-format) section.

Once the project is built with `make`, you can run the compression algorithm using either file input or standard input.

### Running The Compression Algorithm

You can provide input to the program in two ways:<br><br>

**Option 1: From a File**<br><br>
Run the program with the `-f` flag followed by a file name containing your block model:

`./main.exe -f TestCases/T1.txt`
<br><br>

**Option 2: From Standard Input**<br><br>
Run the program without arguments and enter your block model directly into the terminal.
When you’re done, press Ctrl + D (on Linux/macOS) or Ctrl + Z followed by Enter (on Windows) to indicate the end of input:

`./main.exe`

## How It Works

Within this section, a brief but thorough explanation for the functionality of each section will be provided.

### Main Function

The entry point of the program is within `main.cpp`, it is the basis for argument parsing, input setup, multithreaded compression, and output formatting.

Below is a breakdown of its major stages:

#### 1. Argument Parsing

Arguments are read in from standard input and stored within an Args struct for later use, this Args struct contains information like whether the input is of type file, whether the program should be executed in testing mode, and whether the program should be verbose in its execution.

#### 2. Input Handling

The main function reads the input and calls several helper functions to further the programs execution. There are two major components:

- **Parsing**
- **Compression**

The input is first passed to the [parse function](#parsing-input) to be broken into its individual components as outlined in the [standard input format](#standard-input-format). After this has occurred, the parsed input is passed to the [compression function](#compression-logic) where it can be systematically compressed into appropriate blocks.

### Parsing Input

The parsing phase is responsible for reading the raw input data and converting it into a structured format that can be efficiently handled by the compression functions. A more [detailed description](#extra-info) of the decomposition of the programs input can be understood within the extra information provided at the base of the README.

In terms of the actual functionality of the parsing function, several key design choices occur such that the program may execute efficiently:

- **Run-Length Encoding (RLE)**
- **Caching System**

These design choices were made to serve very distinct but complementary purposes. As the input often contained large amounts of consecutive identical characters, Run-Length Encoding could significantly reduce data redundancy prior to passing to higher-level merging operations. 

Furthermore, by caching identical slices and blocks the parser could minimise recomputation considerably. The caching system would store 2D slices, parent blocks, and RLE operations, such that any sort of repetition or uniformity within the input file would use minimal computational efforts.

### Compression Logic

### Printing Blocks

## Extra Info

### Standard Input Format

The input stream follows a specific structure consisting of three parts that needs to be strictly adhered to:
> Note: Example input files are provided within the `TestCases/` directory for reference.

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

The remainder of the input specifies the model itself as a 3D grid of tag characters.

### Standard Output Format

The compressed output will contain one line per compressed block, with seven comma-seperated values:<br><br>
`x_position, y_position, z_position, x_size, y_size, z_size, label`

- `x_position`, `y_position`, `z_position`: coordinate of the block.
- `x_size`, `y_size`, `z_size`: dimensions of the compressed block.
- `label`: corresponding label from the tag table.


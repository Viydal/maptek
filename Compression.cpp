#include "Compression.h"
#include <cmath>
Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream& output, int x_pos,
                               int row_num, int layer_num, int numX, int numY,
                               int numZ, char ch,
                               std::unordered_map<char, std::string> TagTable) {
    output << x_pos << "," << row_num << "," << layer_num << "," << numX << ","
           << numY << "," << numZ << "," << TagTable[ch] << "\n";
    return;
}

std::string Compression::SingleLineCompress(
    const std::string Row, std::unordered_map<char, std::string> TagTable,
    int ParentX, int ParentY, int ParentZ, int row_num, int layer_num) {
    size_t count = 0;
    std::ostringstream output;

    // parse through the string
    while (count < Row.size()) {
        // read number
        int num = 0;
        while (count < Row.size() && isdigit(Row[count])) {
            num = num * 10 + (Row[count] - '0');
            count++;
        }

        // read character (label)
        char ch = Row[count];
        count++;

        // break the run into ParentX-sized chunks
        while (num > 0) {
            int remaining =
                ParentX - (x_pos % ParentX);  // space left in this block
            int chunk = std::min(num, remaining);
            // append line in the format:
            // x_position, y_position, z_position, x_size, y_size, z_size, label
            FormatOutput(output, x_pos, row_num, layer_num, chunk, 1, 1, ch,
                         TagTable);
            // advance x position
            x_pos += chunk;
            num -= chunk;
        }
    }

    return output.str();
}

std::vector<Block> Compression::SingleLineBlocks(const std::string Row,
                                                 int ParentX, int ParentY,
                                                 int ParentZ, int row_num,
                                                 int layer_num) {
    size_t count = 0;
    int x_pos = 0;
    std::vector<Block> blocks;

    while (count < Row.size()) {
        // read number
        int num = 0;
        while (count < Row.size() && isdigit(Row[count])) {
            num = num * 10 + (Row[count] - '0');
            count++;
        }

        char ch = Row[count];
        count++;

        while (num > 0) {
            int remaining = ParentX - (x_pos % ParentX);
            int chunk = std::min(num, remaining);

            blocks.push_back({x_pos, row_num, layer_num, chunk, 1, 1, ch});

            x_pos += chunk;
            num -= chunk;
        }
    }
    return blocks;
}

std::vector<Block> Compression::MergeRows(const std::vector<Block>& prevRow,
                                          const std::vector<Block>& currRow,
                                          int ParentY) {
    std::vector<Block> merged = prevRow;

    for (auto& c : currRow) {
        bool mergedFlag = false;
        for (auto& p : merged) {
            // same x range, same label, same z, same ParentY block
            // and c is directly above p
            if (p.x_pos == c.x_pos &&
                p.x_size == c.x_size &&
                p.ch == c.ch &&
                p.z_pos == c.z_pos &&
                ((p.y_pos / ParentY) == (c.y_pos / ParentY)) &&
                (c.y_pos == p.y_pos + p.y_size)) {

                // std::cout << "Merging block at (" << c.x_pos << "," << c.y_pos
                //           << "," << c.z_pos << ") size (" << c.x_size << ","
                //           << c.y_size << "," << c.z_size << ") with block at ("
                //           << p.x_pos << "," << p.y_pos << "," << p.z_pos
                //           << ") size (" << p.x_size << "," << p.y_size << ","
                //           << p.z_size << ")\n";

                // extend vertically
                p.y_size += c.y_size;

                // always set y_pos to the *lowest row*
                p.y_pos = std::min(p.y_pos, c.y_pos);

                mergedFlag = true;
                break;
            }
        }
        if (!mergedFlag) {
            merged.push_back(c);
        }
    }

    return merged;
}

void Compression::WriteBlocks(
    const std::vector<Block>& blocks, std::ostringstream& output,
    const std::unordered_map<char, std::string>& TagTable) {
    for (const auto& b : blocks) {
        FormatOutput(output, b.x_pos, b.y_pos, b.z_pos, b.x_size, b.y_size,
                     b.z_size, b.ch, TagTable);
    }
}

void Compression::ProcessLayer(const std::vector<std::string>& rows,
                               int ParentX, int ParentY, int ParentZ,
                               int layer_num,
                               std::ostringstream& output,
                               const std::unordered_map<char, std::string>& TagTable) {
    std::vector<Block> accumulated;   // merged blocks for current ParentY group

    int height = (int)rows.size();

    // Iterate bottom -> top
    for (int row_num = 0; row_num < height; row_num++) {
        int y_pos = row_num; // bottom = 0

        auto currRow = SingleLineBlocks(rows[y_pos],
                                        ParentX, ParentY, ParentZ,
                                        y_pos, layer_num);

        accumulated = MergeRows(accumulated, currRow, ParentY);

        // If we've completed a ParentY block or hit the last row, flush
        if ((row_num + 1) % ParentY == 0 || row_num == height - 1) {
            WriteBlocks(accumulated, output, TagTable);
            accumulated.clear();
        }
    }
}


std::string Compression::FormatOutputStrings(
    std::ostringstream& output, int x_pos, int row_num, int layer_num, int numX,
    int numY, int numZ, char ch,
    std::unordered_map<char, std::string> TagTable) {
    output << x_pos << "," << row_num << "," << layer_num << "," << numX << ","
           << numY << "," << numZ << "," << TagTable[ch] << "\n";
    return output.str();
}

std::vector<std::string> Compression::WriteBlocksVectorStrings(
    const std::vector<Block>& blocks, std::ostringstream& output,
    const std::unordered_map<char, std::string>& TagTable) {
    std::vector<std::string> result;
    for (const auto& b : blocks) {
        result.push_back(FormatOutputStrings(output, b.x_pos, b.y_pos, b.z_pos,
                                             b.x_size, b.y_size, b.z_size, b.ch,
                                             TagTable));
    }
    return result;
}

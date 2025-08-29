#include "Compression.h"

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
    int x_pos = 0;  // track where each run starts
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

bool Compression::TryRelaxedMerge(Block& prev,
                     Block& curr,
                     int ParentY,
                     std::vector<Block>& currLeftovers,
                     std::vector<Block>& prevLeftovers) 
{
    // Rule 0: must be same "row group"
    if ((prev.y_pos / ParentY) != (curr.y_pos / ParentY)) return false;
    if (prev.z_pos != curr.z_pos) return false;
    if (prev.ch != curr.ch) return false;

    // --- compute overlap in x
    int start = std::max(prev.x_pos, curr.x_pos);
    int end   = std::min(prev.x_pos + prev.x_size,
                         curr.x_pos + curr.x_size);
    int overlap = end - start;
    if (overlap <= 0) return false; // no horizontal overlap

    // --- Rule 2: cannot shrink prev overlap too much
    if (overlap <= prev.x_size / 2) {
        return false;
    }

        // --- split prev into (left, overlap, right)
    int prev_left  = start - prev.x_pos;
    int prev_right = (prev.x_pos + prev.x_size) - end;
    // --- split curr into (left, overlap, right)
    int curr_left  = start - curr.x_pos;
    int curr_right = (curr.x_pos + curr.x_size) - end;

    // enforce Rule 1/3/4 by tracking "which side relaxed"
    // Right now we only allow one side trimming for prev and curr
    if (prev_left > 0 && prev_right > 0) return false;
    if (curr_left > 0 && curr_right > 0) return false;
    if ((prev_left > 0 && curr_right > 0) ||
        (prev_right > 0 && curr_left > 0)) return false;

        // --- leftovers from prev: must be flushed immediately
    if (prev_left > 0) {
        Block left = {prev.x_pos, prev.y_pos, prev.z_pos,
                      prev_left, prev.y_size, prev.z_size, prev.ch};
        prevLeftovers.push_back(left);
        // std::cout << "Prev left leftover: (" << left.x_pos << "," << left.y_pos << "," << left.z_pos << ") size (" << left.x_size << "," << left.y_size << "," << left.z_size << ")" << left.ch << "\n";
    }
    if (prev_right > 0) {
        Block right = {end, prev.y_pos, prev.z_pos,
                       prev_right, prev.y_size, prev.z_size, prev.ch};
        prevLeftovers.push_back(right);
        // std::cout << "Prev right leftover: (" << right.x_pos << "," << right.y_pos << "," << right.z_pos << ") size (" << right.x_size << "," << right.y_size << "," << right.z_size << ")" << right.ch << "\n";
    }

    // --- perform the merge using overlap region
    prev.x_pos  = start;
    prev.x_size = overlap;
    prev.y_size += curr.y_size;

    if (curr_left > 0) {
        Block left = {curr.x_pos, curr.y_pos, curr.z_pos,
                      curr_left, curr.y_size, curr.z_size, curr.ch};
        currLeftovers.push_back(left);
        // std::cout << "Curr left leftover: (" << left.x_pos << "," << left.y_pos << "," << left.z_pos << ") size (" << left.x_size << "," << left.y_size << "," << left.z_size << ")" << left.ch << "\n";
    }
    if (curr_right > 0) {
        Block right = {end, curr.y_pos, curr.z_pos,
                       curr_right, curr.y_size, curr.z_size, curr.ch};
        currLeftovers.push_back(right);
        // std::cout << "Curr right leftover: (" << right.x_pos << "," << right.y_pos << "," << right.z_pos << ") size (" << right.x_size << "," << right.y_size << "," << right.z_size << ")" << right.ch << "\n";
    }


    return true;
}

std::vector<Block> Compression::MergeRows(std::vector<Block>& prevRows,
                                          std::vector<Block>& currRow,
                                          int ParentY) {
    std::vector<Block> merged = prevRows;
    std::vector<Block> PrevLeftovers;  // store cut-offs from prev to be output

    for (auto& c : currRow) {
        bool mergedFlag = false;
        std::vector<Block> CurrLeftovers; // store cut-offs from curr to be kept and checked for future merges
        for (auto& p : merged) {

            // --- STRICT MERGE CONDITIONS ---
            // same x range, same label, same z, same ParentY block
            // and c is directly above p
            if (p.x_pos == c.x_pos && p.x_size == c.x_size && p.ch == c.ch &&
                p.z_pos == c.z_pos &&
                ((p.y_pos / ParentY) == (c.y_pos / ParentY)) &&
                (c.y_pos == p.y_pos + p.y_size)) {
                // std::cout << "Merging block at (" << c.x_pos << "," <<
                // c.y_pos
                //           << "," << c.z_pos << ") size (" << c.x_size << ","
                //           << c.y_size << "," << c.z_size << ") with block at
                //           ("
                //           << p.x_pos << "," << p.y_pos << "," << p.z_pos
                //           << ") size (" << p.x_size << "," << p.y_size << ","
                //           << p.z_size << ")\n";

                // extend vertically
                p.y_size += c.y_size;

                // always set y_pos to the *lowest row*
                p.y_pos = std::min(p.y_pos, c.y_pos);

                mergedFlag = true;
                break;

                // --- RELAXED MERGE CONDITIONS ---
            } 
            else if (TryRelaxedMerge(p, c, ParentY, CurrLeftovers, PrevLeftovers)) {
                // std::cout << "Relaxed merging block at (" << c.x_pos << "," << c.y_pos << "," << c.z_pos << ") size (" << c.x_size << "," << c.y_size << "," << c.z_size << ") with block at (" << p.x_pos << "," << p.y_pos << "," << p.z_pos << ") size (" << p.x_size << "," << p.y_size << "," << p.z_size << ")\n";
                mergedFlag = true;
                                    // append any leftover cut pieces
                for (auto& l : CurrLeftovers) {
                    merged.push_back(l); 
                }
                break;
            }
        }
        if (!mergedFlag) {
            merged.push_back(c);
        }
    }

    // append prev leftovers (these will later be printed in WriteBlocks)
    for (auto& l : PrevLeftovers) {
        merged.push_back(l);
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

void Compression::ProcessLayer(
    const std::vector<std::string>& rows, int ParentX, int ParentY, int ParentZ,
    int layer_num, std::ostringstream& output,
    const std::unordered_map<char, std::string>& TagTable) {
    std::vector<Block> accumulated;  // merged blocks for current ParentY group

    int height = (int)rows.size();

    // Iterate bottom -> top
    for (int row_num = 0; row_num < height; row_num++) {
        int y_pos = row_num;  // bottom = 0

        auto currRow = SingleLineBlocks(rows[y_pos], ParentX, ParentY, ParentZ,
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
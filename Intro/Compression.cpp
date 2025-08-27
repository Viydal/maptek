#include "Compression.h"

Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream& output, int x_pos, int row_num, int layer_num, int num, char ch, std::unordered_map<char, std::string> TagTable) {
  output << x_pos << "," << row_num << "," << layer_num << "," << num << "," << 1 << "," << 1 << "," << TagTable[ch] << "\n";
  return;
}

void Compression::BetterFormat(int x_pos, int y_pos, int layer_num, int size_x, int size_y, int size_z, char location, std::unordered_map<char, std::string> TagTable) {
    std::cout << x_pos << "," << y_pos << "," << layer_num << "," << size_x << "," << size_y << "," << size_z << "," << TagTable[location] << "\n";
}

std::string Compression::SingleLineCompress(const std::string Row, std::unordered_map<char, std::string> TagTable, int ParentX, int ParentY, int ParentZ, int x_pos, int row_num, int layer_num) {
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
            int remaining = ParentX - (x_pos % ParentX); // space left in this block
            int chunk = std::min(num, remaining);
            // append line in the format:
            // x_position, y_position, z_position, x_size, y_size, z_size, label
            FormatOutput(output, x_pos, row_num, layer_num, chunk, ch, TagTable);
            // advance x position
            x_pos += chunk;
            num -= chunk;
        }
    }

    return output.str();
}

void Compression::TwoDCompression(Parse Parser, std::vector<std::vector<std::string>> block, size_t index) {
    int XCoord = (index % Parser.NumXBlocks) * Parser.ParentX;
    int YCoord = (Parser.NumYBlocks - 1 - ((index % (Parser.NumXBlocks * Parser.NumYBlocks)) / Parser.NumXBlocks)) * Parser.ParentY;
    int ZCoord = index / (Parser.NumYBlocks * Parser.NumXBlocks) * Parser.ParentZ;

    // std::cout << "num y blocks: " << Parser.NumYBlocks << std::endl;

    std::vector<std::string> row = block[0];
    bool uniform = true;
    for (size_t i = 0; i < block.size(); i++) {
        if (block[i] != row) {
            uniform = false;
        }
    }

    if (uniform) {
        char symbol = row[0].back();
        BetterFormat(XCoord, YCoord, ZCoord, Parser.ParentX, Parser.ParentY, Parser.ParentZ, symbol, Parser.TagTable);
    } else {
        for (size_t row_num = 0; row_num < block.size(); row_num++) {
            std::vector<std::string> row = block[row_num];
            std::string RowElements;
            for (size_t element = 0; element < block[row_num].size(); element++) {
                RowElements += row[element];
            }
            std::cout << "string: " << RowElements << std::endl;
            std::string output = SingleLineCompress(RowElements, Parser.TagTable, Parser.ParentX, Parser.ParentY, Parser.ParentZ, XCoord, YCoord + (Parser.ParentY - 1 - row_num), ZCoord);
            std::cout << output;
        }
    }
}
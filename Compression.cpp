#include "Compression.h"

Compression::Compression() {}

void Compression::FormatOutput(std::ostringstream& output, int x_pos, int row_num, int layer_num, int num, char ch, std::unordered_map<char, std::string> TagTable) {
  output << x_pos << "," << row_num << "," << layer_num << ","
               << num << "," << 1 << "," << 1 << ","
               << TagTable[ch] << "\n";
  return;
}

std::string Compression::SingleLineCompress(const std::string Row, std::unordered_map<char, std::string> TagTable, int ParentX, int ParentY, int ParentZ, int row_num, int layer_num) {
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
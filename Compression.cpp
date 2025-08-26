#include "Compression.h"

Compression::Compression() {}

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

        // append line in the format:
        // x_position, y_position, z_position, x_size, y_size, z_size, label
        while (num > ParentX) {
            output << x_pos << "," << row_num << "," << layer_num << ","
               << ParentX << "," << 1 << "," << 1 << ","
               << TagTable[ch] << "\n";
            num -= ParentX;
            x_pos += ParentX;
        }
        output << x_pos << "," << row_num << "," << layer_num << ","
               << num << "," << 1 << "," << 1 << ","
               << TagTable[ch] << "\n";

        // advance x position
        x_pos += num;
    }

    return output.str();
}
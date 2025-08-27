#include "Parse.h"
#include "Compression.h"
#include <iostream>
#include <string>

int main (void){
Compression compressor = Compression();

std::vector<std::string> output;
output.push_back("0,0,0,8,1,1,sea");
output.push_back("0,1,0,8,1,1,sea");
output.push_back("0,2,0,8,1,1,sea");
output.push_back("0,3,0,8,1,1,sea");
output.push_back("0,4,0,8,1,1,sea");
output.push_back("0,5,0,8,1,1,sea");
output.push_back("0,6,0,8,1,1,sea");
output.push_back("0,7,0,8,1,1,sea");
output.push_back("0,8,0,8,1,1,sea");

compressor.Uncompress2d(output, {}, 8, 8);

std::cout<<std::endl<<std::endl<<"Test 2"<<std::endl<<std::endl;

output.clear();

output.push_back("0,0,0,8,1,1,TAS");
output.push_back("0,1,0,8,1,1,TAS");
output.push_back("0,2,0,8,1,1,TAS");
output.push_back("0,3,0,8,1,1,TAS");
output.push_back("0,4,0,8,1,1,TAS");
output.push_back("0,5,0,8,1,1,TAS");
output.push_back("0,6,0,8,1,1,TAS");
output.push_back("0,7,0,8,1,1,TAS");
output.push_back("0,8,0,8,1,1,TAS");

compressor.Uncompress2d(output, {}, 8, 8);

}
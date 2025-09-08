#ifndef PARENTBLOCK_H
#define PARENTBLOCK_H
#include "Block.h"
#include <vector>
class ParentBlock{
public:
std::vector<Block> Blocks;

ParentBlock(){};
ParentBlock(std::vector<Block> Blocks) { this->Blocks = Blocks;}

void WriteBlock();
};

#endif
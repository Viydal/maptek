class Compress:
    def CompressMethod(self, BlockModel, x_count, y_count, z_count, parent_x, parent_y, parent_z, TagTable):
        print("parent blocks", (parent_x, parent_y, parent_z))
        for z_Origin in range(0, z_count-parent_z+1, parent_z): #iterate over z dimension for blocks
            for y_Origin in range(0, y_count-parent_y+1, parent_y): #iterate over y dimension for blocks
                for x_Origin in range(0, x_count - parent_x+1, parent_x): #iterate over x dimension for blocks
                    for z in range(z_Origin, parent_z+z_Origin): #iterate over z dimension for inside a block
                        for y in range(y_Origin, parent_y+y_Origin): #iterate over y dimension for inside a block
                            for x in range(x_Origin, parent_x+x_Origin): #iterate over x dimension for inside a block
                                print(f"{x}, {y}, {z}, 1, 1, 1, {TagTable[BlockModel[z][y][x]]}") #print in the requested format of just single blocks
import sys
import os

class TextParser:
  
    def ParseText(self):
        start = input()
        start = start.split(sep=",")
        
        self.x_count, self.y_count, self.z_count, self.parent_x, self.parent_y, self.parent_z = int(start[0]), int(start[1]), int(start[2]), int(start[3]), int(start[4]), int(start[5])
        print(self.x_count, self.y_count, self.z_count, self.parent_x, self.parent_y, self.parent_z)
        self.TagTable = {}
        
        for line in sys.stdin:
            if line == "\n":
                break
            tag, state = line.split(sep=",")
            self.TagTable[tag] = state[:-1]
            
        print(self.TagTable)
        self.ParseBlockModel()
        
    def ReturnGlobalVars(self):
        return (self.x_count, self.y_count, self.z_count, self.parent_x, self.parent_y, self.parent_z, self.TagTable)
      
    def ParseBlockModel(self):
        self.model = []
        ZSlice = []
        
        for z in range(self.z_count):
            print(z)
            for line in sys.stdin:
                if line == "\n":
                    self.model.append(ZSlice)
                    ZSlice = []
                    break
                ZSlice.append(line[0:self.x_count])
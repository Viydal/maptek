import sys
import fileinput
from Parse import TextParser
from Compress import Compress
TextParserObj = TextParser()
TextParserObj.ParseText()
x_count, y_count, z_count, parent_x, parent_y, parent_z, TagTable = TextParserObj.ReturnGlobalVars()

CompressClass = Compress()
CompressClass.CompressMethod(TextParserObj.model, x_count, y_count, z_count, parent_x, parent_y, parent_z, TagTable)
import sys
import os
import subprocess

opcodeData = sys.argv[1]
program = sys.argv[2]
commands = sys.argv[3:]

print("Opcode data file:", opcodeData)
print("Program file:    ", program)
print("commands:        ", commands)


# command example for graphNode = "D:\python\graphNode.py [[1,2,'best_tool'],[(1,2),(0,1),(0,2)]]"

print("opened")
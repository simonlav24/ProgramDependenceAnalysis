import sys
import os
import subprocess

# command example for graphNode = "D:\python\graphNode.py [[1,2,'best_tool'],[(1,2),(0,1),(0,2)]]"

ENTRY = "'Entry'"
EXIT = "'Exit'"

class Instruction:
	def __init__(self, opcode, dst, p1, p2):
		self.opcode = opcode
		self.dst = dst
		self.param1 = p1
		self.param2 = p2
		self.cycles = 0
		
		self.dependencies = []
		self.pointed = False
	def addDependencie(self, num):
		self.dependencies.append(num)
		if len(self.dependencies) > 2:
			self.dependencies.pop(0)
	def calculate(self, data):
		if self.opcode in [ENTRY, EXIT]:
			self.pointed = True
			return
		self.cycles = data[int(self.opcode)]
	def __str__(self):
		if self.opcode in [ENTRY, EXIT]:
			return self.opcode
		string = "'(" + self.opcode
		if self.cycles != 0:
			string += ", " + str(self.cycles)
		string += ") " + self.dst + "_" + self.param1 + "_" + self.param2
		return string + "'"
	def __repr__(self):
		return str(self)


def copy2clip(string):
	cmd = 'echo | set /p=' + string.strip() + '|clip'
	return subprocess.check_call(cmd, shell=True)


def main():
	opcodeData = sys.argv[1]
	program = sys.argv[2]
	commands = sys.argv[3:]
	
	# print("#############################################")
	# print("# Opcode data file:", opcodeData)
	# print("# Program file:    ", program)
	# print("# commands:        ", commands)
	# print("#############################################")

	# create instructions array and add "Entry"
	programCounter = []
	programCounter.append(Instruction(ENTRY, "", "", ""))
	opcodes = []
	
	# read instructions file and add to array
	with open(program, 'r') as fileProgram:
		for line in fileProgram:
			if line[0] == '#':
				continue
			opcode, dst, param1, param2 = line.split()
			programCounter.append(Instruction(opcode, dst, param1, param2))
	# add "Exit" to array
	programCounter.append(Instruction(EXIT, "", "", ""))
	
	# read opcodes cycles and calculate to instructions
	with open(opcodeData) as fileOpcode:
		for line in fileOpcode:
			opcodes.append(int(line))
	for i in programCounter:
		i.calculate(opcodes)
	
	# calculate dependencies graph
	edges = []
	destinations = []
	for i, pc in enumerate(programCounter):
		# check if a parameter is in previous destineations
		dependent = False
		for j, dest in enumerate(destinations):
			if pc.param1 == dest or pc.param2 == dest:
				pc.addDependencie(j)
				programCounter[j].pointed = True
				dependent = True
		if pc.opcode in [ENTRY, EXIT]:
			destinations.append("-1")
			continue
		# if not dependent then point to "Entry"
		if not dependent:
			pc.addDependencie(0)
		destinations.append(pc.dst)
	
	# assign "Exit" to unpointed nodes
	for i, pc in enumerate(programCounter):
		if not pc.pointed:
			programCounter[-1].dependencies.append(i)
	
	# create edges of graph
	for i, pc in enumerate(programCounter):
		for d in pc.dependencies:
			edges.append((i, d))
	
	string = "D:\python\graphNode.py " + str([programCounter, edges]).replace(" ", "")
	print(string)
	copy2clip(string)

if __name__ == "__main__":
	main()












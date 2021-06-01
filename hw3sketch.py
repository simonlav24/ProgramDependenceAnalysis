import sys
import os
import subprocess

# command example for graphNode = "D:\python\graphNode.py [[1,2,'best_tool'],[(1,2),(0,1),(0,2)]]"

ENTRY = "'Entry'"
EXIT = "'Exit'"
INFINITY = 1000

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

class Node:
	def __init__(self, index):
		self.index = index
		self.edges = []
	def __str__(self):
		return "(" + str(self.index) + ")-> " + str(self.edges)
	def __repr__(self):
		return str(self)
		
class Edge:
	def __init__(self, source, dest, weight):
		self.source = source
		self.dest = dest
		self.weight = weight
	def __str__(self):
		string = ""
		return "(" + str(self.dest) + ", " + str(self.weight) + ")"
	def __repr__(self):
		return str(self)

def copy2clip(string):
	cmd = 'echo | set /p=' + string.strip() + '|clip'
	return subprocess.check_call(cmd, shell=True)



def analyzeProg(opsLatencyFile, progTrace, numOfInsts):
	
	


	# create instructions array and add "Entry"
	programCounter = []
	programCounter.append(Instruction(ENTRY, "", "", ""))
	opcodes = []
	
	# read instructions file and add to array
	with open(progTrace, 'r') as fileProgram:
		for line in fileProgram:
			if line[0] == '#':
				continue
			opcode, dst, param1, param2 = line.split()
			programCounter.append(Instruction(opcode, dst, param1, param2))
	# add "Exit" to array
	programCounter.append(Instruction(EXIT, "", "", ""))
	
	# read opcodes cycles and calculate to instructions
	with open(opsLatencyFile) as fileOpcode:
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
	
	# output to visualization via graphNode.py
	string = "D:\python\graphNode.py " + str([programCounter, edges]).replace(" ", "")
	# print(string)
	copy2clip(string)
	
	# create handle
	handle = [None] * len(programCounter)
	for i, instruction in enumerate(programCounter):
		node = Node(i - 1)
		for edge in instruction.dependencies:
			e = Edge(i - 1, edge - 1, programCounter[edge].cycles)
			node.edges.append(e)
		handle[i] = node
	
	# for i in handle:
		# print(i)
	
	return handle

def getInstDeps(ctx, theInst, src1DepInst, src2DepInst):
	for i in ctx:
		if int(i.index) == theInst:
			if len(i.edges) == 0:
				src1DepInst[0] = -1
				src2DepInst[0] = -1
			if len(i.edges) == 1:
				src1DepInst[0] = i.edges[0].dest
				src2DepInst[0] = -1
			if len(i.edges) == 2:
				src1DepInst[0] = i.edges[0].dest
				src2DepInst[0] = i.edges[1].dest
			break
	print("getInstDeps(" + str(theInst) + ")==" + "{" + str(src1DepInst[0]) + "," + str(src2DepInst[0]) + "}")


def findShortestPath(handle, source):
	d = [INFINITY] * len(handle)
	d[source] = 0

	for i in range(len(handle) - 1):
		# initialize current distance
		d_current = d.copy()
		# iterate edges:
		for i in handle:
			for edge in i.edges:
				# relaxation
				if d[edge.source + 1] + (-1) * edge.weight < d_current[edge.dest + 1]:
					d_current[edge.dest + 1] = d[edge.source + 1] + (-1) * edge.weight
		d = d_current
		# print(d_current)
		
	return (-1) * d_current[0]

def getProgDepth(handle):
	pathLength = findShortestPath(handle, -1)
	print("getProgDepth()==" + str(pathLength))

def getInstDepth(handle, inst):
	pathLength = findShortestPath(handle, inst + 1)
	print("getDepDepth(" + str(inst) + ")==" + str(pathLength))

if __name__ == "__main__":

	opsLatencyFile = sys.argv[1]
	progTrace = sys.argv[2]
	numOfInsts = sys.argv[3:]
	
	# print("#############################################")
	# print("# Opcode data file:", opcodeData)
	# print("# Program file:    ", program)
	# print("# commands:        ", commands)
	# print("#############################################")

	handle = analyzeProg(opsLatencyFile, progTrace, numOfInsts)
	
	# for i in handle:
		# print(i)
	
	
	###./dflow_calc opcode1.dat example2.in p0 p10 p14 d4 d14
	getProgDepth(handle)
	
	getInstDepth(handle, 0)
	getInstDepth(handle, 10)
	getInstDepth(handle, 14)

	getInstDeps(handle, 4, [90],[90])
	getInstDeps(handle, 14, [90],[90])
	
	
	# getProgDepth(handle)
	
	# getInstDepth(handle, 0)
	# getInstDepth(handle, 3)
	# getInstDepth(handle, 5)
	# getInstDepth(handle, 7)
	# getInstDepth(handle, 9)

	# getInstDeps(handle, 3, [90],[90])
	# getInstDeps(handle, 9, [90],[90])









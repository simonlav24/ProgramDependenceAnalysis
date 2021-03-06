/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
#include <vector>
#include <bits/stdc++.h>

using std::cout;
using std::endl;

#define DEPENDENCIES 2
#define INF INT_MAX
// arbitrary denoting empty, exit, entry
#define EMPTY -101
#define ENTRY -102
#define EXIT -103

#define DEBUG if(debug) cout << "[DEBUG] " 
bool debug = false;

// class for holding instruction data
class Instruction {
public:
    int opcode;
    int dst;
    int param1;
    int param2;
    int cycles;

    std::vector<int> dependencies;
    bool pointed;

    Instruction() {}
    ~Instruction() {}
    Instruction(const Instruction& inst) {
        this->opcode = inst.opcode;
        this->dst = inst.dst;
        this->param1 = inst.param1;
        this->param2 = inst.param2;
        this->cycles = inst.cycles;

        this->dependencies = inst.dependencies;
        this->pointed = inst.pointed;
    }

    // custom constructor
    void takeFrom(int opcode, int dst, int p1, int p2) {
        this->opcode = opcode;
        this->dst = dst;
        this->param1 = p1;
        this->param2 = p2;
        this->cycles = 0;

        pointed = false;
    }

    // add dependencie
    void addDependencie(int num, int p) {    
        while (dependencies.size() != 2)
            dependencies.push_back(EMPTY);
        dependencies[p - 1] = num;
    }

    // calculate cycles based on opcodes file
    void calculate(const unsigned int* data) {
        if (opcode == EXIT || opcode == ENTRY) {
            pointed = true;
            return;
        }
        cycles = data[opcode];
    }
};

// struct for edge in graph
struct Edge {
    int source; 
    int dest;
    int weight;
    // num of source's parameter
    int param;
};

// struct for node in graph
struct Node {
    int index;
    std::vector<Edge> edges;
    // length of node array
    int length;
};

// printing for debugging
void printHandle(Node* handle) {
    if (!debug) return;
    for (int i = 0; i < handle[0].length; i++) {
        cout << "node[" << i << "] index:" << handle[i].index << " edges:" << endl;
        for (unsigned int e = 0; e < handle[i].edges.size(); e++) {
            cout << "(" << handle[i].edges[e].source << ", " << handle[i].edges[e].dest << ", " << handle[i].edges[e].weight << ")";
        }
        cout << endl;
    }
    cout << endl;
}

// copy two arrays
void copyArr(int* arr1, int* arr2, int length) {
    for (int i = 0; i < length; i++) {
        arr1[i] = arr2[i];
    }
}

// printing for debugging
void printArr(int* arr, int length) {
    if (!debug) return;
    DEBUG;
    for (int i = 0; i < length; i++) {
        cout << arr[i] << " ";
    }
    cout << endl;
}

// bellman ford algorithm for shortest path to entry from source
int findShortestPath(Node* handle, int source) {
    int length = handle[0].length;

    int* d = new int[length];
    for (int i = 0; i < length; i++) {
        d[i] = INF;
    }
    d[source] = 0;

    int* d_current = new int[length];

    for (int j = 0; j < length - 1; j++) {
        // initialize current distance
        copyArr(d_current, d, length);
        // iterate edges
        for (int i = 0; i < length; i++) {
            for (unsigned int k = 0; k < handle[i].edges.size(); k++) {
                Edge& edge = handle[i].edges[k];
                // relaxation with negative weights (for finding heaviest path)
                if (d[edge.source + 1] + (-1) * edge.weight < d_current[edge.dest + 1])
                    d_current[edge.dest + 1] = d[edge.source + 1] + (-1) * edge.weight;
            }
        }
        copyArr(d, d_current, length);
    }

    int result = (-1) * d_current[0];
    delete[] d;
    delete[] d_current;

    return result;
}


ProgCtx analyzeProg(const unsigned int opsLatency[], const InstInfo progTrace[], unsigned int numOfInsts) {
    Instruction* programCounter;
    programCounter = new Instruction[numOfInsts + 2];

    // insert Entry
    programCounter[0].takeFrom(ENTRY, 0, 0, 0);
    // insert instructions
    for (unsigned int i = 0; i < numOfInsts; i++) {
        programCounter[i + 1].takeFrom(progTrace[i].opcode, progTrace[i].dstIdx, progTrace[i].src1Idx, progTrace[i].src2Idx);
    }
    // insert Exit
    programCounter[numOfInsts + 1].takeFrom(EXIT, 0, 0, 0);

    // calculate cycles to instructions
    for (unsigned int i = 0; i < numOfInsts; i++) {
        programCounter[i + 1].calculate(opsLatency);
    }

    // calculate dependencies graph
    int totalInst = numOfInsts + 2;
    int* destinations = new int[totalInst];
    for (int i = 0; i < totalInst; i++)
        destinations[i] = EMPTY;

    bool dependent;
    for (int i = 0; i < totalInst; i++) {
        // check if a parameter is in previous destineations
        dependent = false;
        for (int j = 0; j < totalInst; j++) {
            int dest = destinations[j];
            if (programCounter[i].param1 == dest) {
                programCounter[i].addDependencie(j, 1);
                programCounter[j].pointed = true;
                dependent = true;
            }
            if (programCounter[i].param2 == dest) {
                programCounter[i].addDependencie(j, 2);
                programCounter[j].pointed = true;
                dependent = true;
            }
        }

        if (programCounter[i].opcode == ENTRY || programCounter[i].opcode == EXIT) {
            destinations[i] = -1;
            continue;
        }

        // if not dependent then point to "Entry"
        if (!dependent)
            programCounter[i].addDependencie(0, 1);
        destinations[i] = programCounter[i].dst;
    }

    // assign "Exit" to unpointed nodes
    for (int i = 0; i < totalInst; i++) {
        if (!programCounter[i].pointed) {
            programCounter[totalInst - 1].dependencies.push_back(i);
        }
    }

    // create handle
    Node* handle = new Node[totalInst]; // deallocated in freeProgCtx
    for (int i = 0; i < totalInst; i++) {
        Node n;
        n.index = i - 1;
        for (unsigned int edge = 0; edge < programCounter[i].dependencies.size(); edge++) {
            if (programCounter[i].dependencies[edge] == EMPTY) continue;
            Edge e;
            // for convinience
            e.param = edge + 1;
            // first node is Entry (index = -1)
            e.source = i - 1;
            e.dest = programCounter[i].dependencies[edge] - 1;
            e.weight = programCounter[programCounter[i].dependencies[edge]].cycles;
            n.edges.push_back(e);
        }
        handle[i] = n;
    }
    // mark length of node array in first node
    handle[0].length = totalInst;

    delete[] destinations;
    delete[] programCounter;

    printHandle(handle);

    return (ProgCtx*)handle;
}

void freeProgCtx(ProgCtx ctx) {
    Node* handle = (Node*)ctx;
    delete[] handle;
}

int getInstDepth(ProgCtx ctx, unsigned int theInst) {
    Node* handle = (Node*)ctx;
    int totalInst = handle[0].length;
    if ((int)theInst >= totalInst - 2)
        return -1;
    // find shortest path alg
    int pathLength = findShortestPath(handle, theInst + 1);

    return pathLength;
    return 0;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    Node* handle = (Node*)ctx;
    int totalInst = handle[0].length;
    if ((int)theInst >= totalInst - 2)
        return -1;

    for (int i = 0; i < totalInst; i++) {
        if (handle[i].index == (int)theInst) {
            if (handle[i].edges.size() == 0) {
                *src1DepInst = -1;
                *src2DepInst = -1;
            }
            if (handle[i].edges.size() == 1) {
                if (handle[i].edges[0].param == 1) {
                    *src1DepInst = handle[i].edges[0].dest;
                    *src2DepInst = -1;
                }
                else {
                    *src1DepInst = -1;
                    *src2DepInst = handle[i].edges[0].dest;
                }
            }
            if (handle[i].edges.size() == 2) {
                *src1DepInst = handle[i].edges[0].dest;
                *src2DepInst = handle[i].edges[1].dest;
            }
        }
    }

    return 0;
}

int getProgDepth(ProgCtx ctx) {
    Node* handle = (Node*)ctx;
    int totalInst = handle[0].length;
    // find shortest path alg
    int pathLength = findShortestPath(handle, totalInst - 1);

    return pathLength;
}



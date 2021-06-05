/* 046267 Computer Architecture - Spring 21 - HW #3               */
/* Implementation (skeleton)  for the dataflow statistics calculator */

#include "dflow_calc.h"
#include <iostream>
#include <vector>

using std::cout;
using std::endl;

#define DEPENDENCIES 2
#define EMPTY -101

#define INFINITY 1000 
/////////// change this to INT_MAX

#define ENTRY -102
#define EXIT -103

#define DEBUG if(debug) cout << "[DEBUG] " 

bool debug = true;

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

    void takeFrom(int opcode, int dst, int p1, int p2) {
        this->opcode = opcode;
        this->dst = dst;
        this->param1 = p1;
        this->param2 = p2;
        this->cycles = 0;

        pointed = false;
    }

    void addDependencie(int num) {
        dependencies.push_back(num);
        if(dependencies.size() > 2)
            dependencies.erase(dependencies.begin());
    }

    void calculate(const unsigned int* data) {
        if (opcode == EXIT || opcode == ENTRY) {
            pointed = true;
            return;
        }
        cycles = data[opcode];
    }
};

struct Edge {
    int source;
    int dest;
    int weight;
};

struct Node {
    int index;
    std::vector<Edge> edges;
    int length;
};

void printHandle(Node* handle) {
    for (int i = 0; i < handle[0].length; i++) {
        cout << "node[" << i << "] index:" << handle[i].index << " edges:" << endl;
        for (int e = 0; e < handle[i].edges.size(); e++) {
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

//print arr
void printArr(int* arr, int length) {
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
        d[i] = INFINITY;
    }
    d[source] = 0;

    int* d_current = new int[length];

    for (int j = 0; j < length - 1; j++) {
        // initialize current distance
        copyArr(d_current, d, length);
        //printArr(d_current, length);
        // iterate edges
        for (int i = 0; i < length; i++) {
            for (int k = 0; k < handle[i].edges.size(); k++) {
                Edge& edge = handle[i].edges[k];
                DEBUG << edge.source << " " << edge.dest << " " << edge.weight << endl;
                // relaxation
                if (d[edge.source + 1] + (-1) * edge.weight < d_current[edge.dest + 1])
                    d_current[edge.dest + 1] = d[edge.source + 1] + (-1) * edge.weight;
            }
        }
        copyArr(d, d_current, length);
        //printArr(d_current, length);
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

    if (debug) {
        DEBUG << "opcodes ";
        for (unsigned int i = 0; i < numOfInsts + 2; i++) {
            cout << programCounter[i].opcode << " ";
        }
        cout << endl;
    }

    // calculate cycles to instructions
    for (unsigned int i = 0; i < numOfInsts; i++) {
        programCounter[i + 1].calculate(opsLatency);
    }

    if (debug) {
        DEBUG << "cycles ";
        for (unsigned int i = 0; i < numOfInsts + 2; i++) {
            cout << programCounter[i].cycles << " ";
        }
        cout << endl;
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
            if (programCounter[i].param1 == dest || programCounter[i].param2 == dest) {
                programCounter[i].addDependencie(j);
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
            programCounter[i].addDependencie(0);
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
        for (int edge = 0; edge < programCounter[i].dependencies.size(); edge++) {
            Edge e;
            e.source = i - 1;
            e.dest = programCounter[i].dependencies[edge] - 1;
            e.weight = programCounter[programCounter[i].dependencies[edge]].cycles;
            DEBUG << "weight " << e.weight << endl;
            n.edges.push_back(e);
        }
        handle[i] = n;
    }
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
    return -1;
}

int getInstDeps(ProgCtx ctx, unsigned int theInst, int *src1DepInst, int *src2DepInst) {
    Node* handle = (Node*)ctx;
    int totalInst = handle[0].length;

    for (int i = 0; i < totalInst; i++) {
        if (handle[i].index == theInst) {
            if (handle[i].edges.size() == 0) {
                *src1DepInst = -1;
                *src2DepInst = -1;
            }
            if (handle[i].edges.size() == 1) {
                *src1DepInst = handle[i].edges[0].dest;
                *src2DepInst = -1;
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
    DEBUG << "length: " << totalInst << endl;
    int pathLength = findShortestPath(handle, totalInst - 1);

    return pathLength;
}



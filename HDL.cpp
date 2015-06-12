static auto AUTHOR = "Coleman";

#include <iostream>
#include <fstream>
#include <stdlib.h>
#include "vector"

using namespace std;

class Bus;
class IO;
class Arch;
class Gate;
class Build;
class Test;



struct Bit {
    Bit* source;
    int value = 0;

    void update();
};

class Bus {
public:
    string name;
    vector<Bit*> bits;
    bool clocked;
    bool isOutput;
    int size;

    Bus(int size, string name, bool clocked, bool output) : size(size), name(name), clocked(clocked), isOutput(isOutput){}
    void update(bool tock);
    void connect(int start, int end, Bus* other, int otherStart, int otherEnd);
};

class IO {
public:
    vector<string> names;
    vector<int> sizes;
    vector<bool> clocks;
    vector<bool> outputs;

    vector<Bus*> construct();
    void addBus(string name, int size, bool clocked, bool isOutput);
};

class Arch {
public:
    vector<string> gates;

    vector<vector<string>> inputNames;
    vector<vector<int*>> inputRanges;
    vector<vector<string>> sourceNames;
    vector<vector<int*>> sourceRanges;

    vector<vector<string>> outputNames;
    vector<vector<int*>> outputRanges;
    vector<vector<string>> destinationNames;
    vector<vector<int*>> destinationRanges;

    vector<Build*> construct(vector<Bus*> busses);
    void addGate(string name);
    void addInput(string name, int start, int end, string source, int sourceStart, int sourceEnd);
    void addOutput(string name, int start, int end, string output, int destinationStart, int destinationEnd);
};

class Gate {
public:
    IO busses;
    Arch arch;
    string name;

    virtual Build* build(); //constructs a build, calls construct of IO and then Arch
};

class Build {
public:
    string name;
    vector<Bus*> busses; //comes from IO::construct(), but taken from the same instance of the method
    vector<Build*> gates; //comes from Arch::construct()

    virtual Build() {}
    Build(string name) : name(name) {}
    virtual void run();
};

class Test {
public:
    Build* chip;
    vector<vector<string>> inputs;
    vector<vector<int>> inputValues;
    vector<vector<string>> outputs;
    vector<vector<int>> outputValues;
    vector<int> bases;
    vector<int[2]> cycles;

    void addCycles(int start, int wait);
    void addBase(string base);
    void addInput(string name, int value);
    void addOutput(string name, int value);

    void tick(); //updates time in tick of the build
    void tock(); //updates time in tock of the build
};



class NANDBuild : public Build {
public:
    NANDBuild() {
        name = "Nand";

        Bus* a = new Bus(1, "a", false, false);
        Bus* b = new Bus(1, "b", false, false);
        Bus* out = new Bus(1, "out", false, false);
        busses.push_back(a);
        busses.push_back(b);
        busses.push_back(out);
    }

    void run() {
        if(busses[0]->bits[0]->value == busses[1]->bits[0]->value) {
            busses[2]->bits[0]->value = 0;
            if(busses[0]->bits[0]->value == 0) {
                busses[2]->bits[0]->value = 1;
            }
        }
    }
};

class NANDGate : public Gate{
public:
    Build* build() {
        return new NANDBuild();
    }
};



vector<Gate*> allGates;
vector<Test*> allTests;
vector<ifstream*> filesOpened;
ifstream file;
string line;
string token;
Bit zeroSource;
Bit oneSource;


void Bit::update() {
    value = source->value;
}


void Bus::update(bool tock) {
    if((tock && isOutput && clocked) || (!tock && !isOutput && clocked) || !clocked) {
        for (int i = 0; i < this->bits.size(); i++) {
            bits[i]->update();
        }
    }
}
void Bus::connect(int start, int end, Bus* other, int otherStart, int otherEnd) {
    if(end - start != otherEnd - otherStart) {
        cout << name << " does not match " << other->name << " in range, " << start << " " << end << " " << otherStart << " " << otherEnd << endl;
        system("PAUSE");
    }
    for(int i = 0; i < end - start; i++) {
        bits[i + start]->source = other->bits[i + otherStart];
    }
}


vector<Bus*> IO::construct() {
    vector<Bus*> busses;
    for(int i = 0; i < sizes.size(); i++) {
        busses.push_back(new Bus(sizes[i], names[i], clocks[i], outputs[i]));
    }
    return busses;
}
void IO::addBus(string name, int size, bool clocked, bool isOutput) {
    names.push_back(name);
    sizes.push_back(size);
    clocks.push_back(clocked);
    outputs.push_back(isOutput);
}


vector<Build*> Arch::construct(vector<Bus*> busses) {
    vector<Build*> builds;
    for(int i = 0; i < gates.size(); i++) {
        string name = gates[i];
        bool found = false;
        for(int j = 0; j < allGates.size(); j++) {
            if (allGates[j]->name == name) {
                builds.push_back(allGates[j]->build());
                found = true;
            }
        }
        if(!found) {
            cout << name << " : Gate Not Found" << endl;
            system("PAUSE");
        }
        for(int j = 0; j < inputNames[i].size(); j++) {
            for(int k = 0; k < builds.back()->busses.size(); k++) {
                if(builds.back()->busses[k]->name == inputNames[i][j]) {
                    for(int l = 0; l < busses.size(); l++) {
                        if(busses[l]->name == sourceNames[i][j]) {
                            busses[l]->connect(sourceRanges[i][j][0], sourceRanges[i][j][1], builds.back()->busses[k], inputRanges[i][j][0], inputRanges[i][j][1]);
                        }
                    }
                }
            }
        }
        for(int j = 0; j < outputNames[i].size(); j++) {
            for(int k = 0; k < builds.back()->busses.size(); k++) {
                if(builds.back()->busses[k]->name == outputNames[i][j]) {
                    for(int l = 0; l < busses.size(); l++) {
                        if(busses[l]->name == destinationNames[i][j]) {
                            busses[l]->connect(destinationRanges[i][j][0], destinationRanges[i][j][1], builds.back()->busses[k], outputRanges[i][j][0], outputRanges[i][j][1]);
                        }
                    }
                }
            }
        }
    }

    return builds;
}
void Arch::addGate(string name) {
    gates.push_back(name);
    inputNames.push_back(vector<string>());
    inputRanges.push_back(vector<int*>());
    sourceNames.push_back(vector<string>());
    sourceRanges.push_back(vector<int*>());
    outputNames.push_back(vector<string>());
    outputRanges.push_back(vector<int*>());
    destinationNames.push_back(vector<string>());
    destinationRanges.push_back(vector<int*>());
}
void Arch::addInput(string name, int start, int end, string source, int sourceStart, int sourceEnd) {
    int* arr = new int[2];
    arr[0] = start;
    arr[1] = end;
    inputNames.back().push_back(name);
    inputRanges.back().push_back(arr);

    arr = new int[2];
    arr[0] = sourceStart;
    arr[1] = sourceEnd;
    sourceNames.back().push_back(source);
    sourceRanges.back().push_back(arr);
}
void Arch::addOutput(string name, int start, int end, string destination, int destinationStart, int destinationEnd) {
    int* arr = new int[2];
    arr[0] = start;
    arr[1] = end;
    inputNames.back().push_back(name);
    inputRanges.back().push_back(arr);

    arr = new int[2];
    arr[0] = destinationStart;
    arr[1] = destinationEnd;
    sourceNames.back().push_back(destination);
    sourceRanges.back().push_back(arr);
}


Build* Gate::build() {
    Build* build = new Build(name);
    build->busses = busses.construct();
    build->gates = arch.construct(build->busses);

    return build;
}


void Build::run() {

}


void Test::addCycles(int start, int wait) {

}
void Test::addBase(string base) {

}
void Test::addInput(string name, int value) {

}
void Test::addOutput(string name, int value) {

}
void Test::tick() {

}
void Test::tock() {

}



//namespace BUILTIN {
//
//    Gate* Not = new Gate();
//    Gate* Nor = new Gate();
//    Gate* Nand = new Gate();
//    Gate* Or = new Gate();
//    Gate* And = new Gate();
//    Gate* Xor = new Gate();
//    Gate* Xnor = new Gate();
//
//    Gate* Not8 = new Gate();
//    Gate* Nor8 = new Gate();
//    Gate* Nand8 = new Gate();
//    Gate* Or8 = new Gate();
//    Gate* And8 = new Gate();
//    Gate* Xor8 = new Gate();
//    Gate* Xnor8 = new Gate();
//
//    Gate* Not16 = new Gate();
//    Gate* Nor16 = new Gate();
//    Gate* Nand16 = new Gate();
//    Gate* Or16 = new Gate();
//    Gate* And16 = new Gate();
//    Gate* Xor16 = new Gate();
//    Gate* Xnor16 = new Gate();
//
//
//    Gate* Mux = new Gate();
//    Gate* Demux = new Gate();
//
//    Gate* Mux8 = new Gate();
//    Gate* Demux8 = new Gate();
//
//    Gate* Mux16 = new Gate();
//    Gate* Demux16 = new Gate();
//
//
//    Gate* DFF = new Gate();
//    Gate* Reg = new Gate();
//
//    Gate* DFF8 = new Gate();
//    Gate* Reg8 = new Gate();
//
//    Gate* DFF16 = new Gate();
//    Gate* Reg16 = new Gate();
//
//    Gate* RAM128x8 = new Gate();
//    Gate* RAM16Kx16 = new Gate();
//
//    Gate* RAM256x8 = new Gate();
//    Gate* RAM32Kx16 = new Gate();
//
//} code in HDL as the standard libraries, up to 16 bit sizes



int main() {

    allGates.push_back(new NANDGate);

    vector<Bit*> bits;
    for(int i = 0; i < 10; i++) {
        bits.push_back(new Bit());
    }

    for(int i = 0; i < 10; i++) {
        cout << bits[i]->value << endl;
    }

    Arch arch;
    cout << "initialized" << endl;
    arch.addGate("AND");
    cout << "name added" << endl;
    arch.addInput("a", 0, 1, "a", 0, 1);
    cout << "other stuff" << endl;

    cout << arch.gates[0] << endl;
    cout << arch.inputNames[0][0] << " " << arch.inputRanges[0][0][0] << " " << arch.inputRanges[0][0][1] << " " << arch.sourceNames[0][0] << " " << arch.sourceRanges[0][0][0] << " " << arch.sourceRanges[0][0][1] << endl;

    cout << ("this one" == "that one") << endl;

    return 0;

}


string symbols[] = {
        "\"", "\\",
        "+", "-",
        ",", "..",
        "(", ")", "{", "}", "[", "]",
        ":", ";", "|", "="
};
string keywords[] = {
        "IMPORT", "CHIP", "TEST",
        "IN", "OUT", "PIN", "CLOCKED",
        "BUILTIN"
};

int currentBase;

void import() {
// imports all the files of named imports in file objects and adds them to the vector of files
}

void builtin() {
//	instantiates the builtin chips
}

void build() {
//	assembles the Gate and Test classes
}

void test() {
//	runs a test of a chip
}

void resetFile() {

}

void resetLine() {

}

string nextToken() {
//	token reader of the current file and current line
}

bool isSymbol() {

}

bool isSymbol(string symbol) {

}

bool isInt() {

}

bool isKeyword(string word) {

}





vector<int> toBinary(int decimal) {
    vector<int> returnVector;
    vector<int> reversedVector;

    while (decimal > 0) {

        reversedVector.push_back(decimal % 2);
        decimal /= 2;

    }

    for (int i = reversedVector.size(); i > 0; i--) {

        returnVector.push_back(reversedVector[i - 1]);

    }

    return returnVector;
}
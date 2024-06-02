#include "api.hpp"
#include <iostream>
#include <fstream>
#include <string>
#include <cstring>

extern std::string dfa2re(DFA &d);
extern DFA dfa_minim(DFA& d);
extern DFA re2dfa(const std::string& regexp);

int main(int argc, char** argv) { // should be passed with mode, input filename and output filename
    if (argc != 4) {
        std::cout << "Wrong number of arguments, use program with 3 arguments:\n";
        std::cout << "mode (re2dfa, dfa_minim or dfa2re), input absolute filepath and output absolute filepath\n";
        return 0;
    }
    std::ifstream infile(argv[2]); // currently the path to file and data format inside is not checked before using.
    std::ofstream outfile(argv[3]);
    if (!strcmp(argv[1], "re2dfa")) {
        std::string line;
        std::getline(infile, line);
        outfile << re2dfa(line).to_string();
    }
    else if (!strcmp(argv[1], "dfa_minim")) {
        auto d = DFA::from_string(std::string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>()));
        outfile << dfa_minim(d).to_string();
    }
    else if (!strcmp(argv[1], "dfa2re")) {
        auto d = DFA::from_string(std::string((std::istreambuf_iterator<char>(infile)), std::istreambuf_iterator<char>()));
        outfile << dfa2re(d);
    }
    else {
        std::cout << "Wrong type of mode, use program with 3 arguments:\n";
        std::cout << "mode (re2dfa, dfa_minim or dfa2re), input absolute filepath and output absolute filepath\n";
        return 0;
    }
    
    return 0;
}

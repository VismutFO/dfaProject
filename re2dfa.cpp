#include "api.hpp"
#include <vector>
#include <string>
#include <iostream>
#include <set>
#include <deque>

#define START 0
#define EPS '?'

typedef std::vector<std::vector<std::set<int>>> NFA;

int makeNFA(const std::string& regexp, NFA& nfa, std::vector<bool>& hasNoEPSEdges, const Alphabet& alpha, int begin, int end, int startState);

void deleteEPSEdges(const NFA& nfaEPS, NFA& nfa, std::vector<bool>& isFinal, const std::vector<bool>& hasNoEPSEdges, int last);

void printNFA(const NFA& nfa, const std::vector<bool>& isFinal) {
	for (int i = 0; i < nfa.size(); i++) {
		for (int j = 0; j < 256; j++) {
			for (const int& to : nfa[i][j]) {
				std::cerr << i << " " << (char)j << " " << to << "\n";
			}
		}
	}
	std::cerr << "finals: ";
	for (int i = 0; i < isFinal.size(); i++) {
		if (isFinal[i]) {
			std::cerr << i << " ";
		}
	}
	std::cerr << "\n";
}

DFA makeDFA(const NFA& nfa, const std::vector<bool> isFinal, const Alphabet& alpha);

DFA re2dfa(const std::string& regexp) {
	std::cerr << "---START---\n";
	bool haveNonSpecial = false;
	for (char c : regexp) {
		if ((c >= 'a' && c <= 'Z') || (c >= '0' || c <= '9')) {
			haveNonSpecial = true;
			break;
		}
	}
	if (!haveNonSpecial) {
		DFA dfa = DFA(Alphabet("a"));
		dfa.create_state("0");
		dfa.make_final("0");
		return dfa;
	}
	Alphabet alpha = Alphabet(regexp);

	NFA nfaEPS(1, std::vector<std::set<int>>(256));
	std::vector<bool> hasNoEPSEdges(1);
	int last = makeNFA(regexp, nfaEPS, hasNoEPSEdges, alpha, 0, regexp.size(), START);
	std::cerr << "---END---\n";

	NFA nfa;
	std::vector<bool> isFinal(nfaEPS.size());
	isFinal[last] = true;
	deleteEPSEdges(nfaEPS, nfa, isFinal, hasNoEPSEdges, last);
	std::cerr << "---NoEPS---\n";

	printNFA(nfa, isFinal);
	std::cerr << "---DFA---\n";

	return makeDFA(nfa, isFinal, alpha);
}

int genNext(NFA& nfa, std::vector<bool>& hasNoEPSEdges)
{
	nfa.push_back(std::vector<std::set<int>>(256));
	hasNoEPSEdges.push_back(false);
	return nfa.size() - 1;
}

int makeNFA(const std::string& regexp, NFA& nfa, std::vector<bool>& hasNoEPSEdges, const Alphabet& alpha, int begin, int end, int startState) { // [), state to begin with, return last state
	int lastState = startState, endState = genNext(nfa, hasNoEPSEdges);
	for (int i = begin; i < end; i++) {
		if (alpha.has_char(regexp[i])) {
			int j1 = genNext(nfa, hasNoEPSEdges);
			int j2 = genNext(nfa, hasNoEPSEdges);
			std::cerr << lastState << " " << EPS << " " << j1 << std::endl;
			std::cerr << j1 << " " << regexp[i] << " " << j2 << std::endl;
			nfa[lastState][EPS].insert(j1);
			nfa[j1][regexp[i]].insert(j2);
			hasNoEPSEdges[j1] = true;

			int j3 = genNext(nfa, hasNoEPSEdges);
			std::cerr << j2 << " " << EPS << " " << j3 << std::endl;
			nfa[j2][EPS].insert(j3);
			if (regexp[i + 1] == '*') {
				std::cerr << j2 << " " << EPS << " " << j1 << std::endl;
				nfa[j2][EPS].insert(j1);

				std::cerr << lastState << " " << EPS << " " << j3 << std::endl;
				nfa[lastState][EPS].insert(j3);
				i++;
			}
			lastState = j3;
		}
		else if (regexp[i] == '|') {
			std::cerr << lastState << " " << EPS << " " << endState << std::endl;
			nfa[lastState][EPS].insert(endState);
			lastState = startState;

		}
		else if (regexp[i] == '(') {
			int j = i + 1, sum = 1;
			for (; j < end; j++) {
				if (regexp[j] == '(') {
					sum++;
				}
				else if (regexp[j] == ')') {
					sum--;
				}
				if (!sum) { break; }
			}
			int R1State = genNext(nfa, hasNoEPSEdges);

			std::cerr << lastState << " " << EPS << " " << R1State << std::endl;
			nfa[lastState][EPS].insert(R1State);

			int R2State = makeNFA(regexp, nfa, hasNoEPSEdges, alpha, i + 1, j, R1State);

			int RendState = genNext(nfa, hasNoEPSEdges);

			std::cerr << R2State << " " << EPS << " " << RendState << std::endl;
			nfa[R2State][EPS].insert(RendState);

			i = j;
			if (i < end - 1 && regexp[i + 1] == '*') {
				std::cerr << lastState << " " << EPS << " " << RendState << std::endl;
				nfa[lastState][EPS].insert(RendState);

				std::cerr << R2State << " " << EPS << " " << R1State << std::endl;
				nfa[R2State][EPS].insert(R1State);
				i++;
			}
			lastState = RendState;
		}
		/*
		else if (regexp[i] == ')') {
			while (true) {

			}
		}
		else if (regexp[i] == '*') {
			std::cerr << "ERROR2\n";
			exit(2);
		}
		else {
			std::cerr << "ERROR3\n";
			exit(3);
		}
		*/
	}
	if (lastState != endState) {
		std::cerr << lastState << " " << EPS << " " << endState << std::endl;
		nfa[lastState][EPS].insert(endState);
	}
	return endState;
}

void dfsEPS(const NFA& nfaEPS, const std::vector<bool>& hasNoEPSEdges, std::vector<bool>& used, int v, std::set<int>& dest, int last) {
	used[v] = true;
	if (nfaEPS[v][EPS].empty()) {
		//std::cerr << "END " << v << "\n";
		if (v == last || hasNoEPSEdges[v]) {
			dest.insert(v);
		}
		return;
	}
	//std::cerr << "Vertice: " << v << "\n";
	for (const int& x : nfaEPS[v][EPS]) {
		if (!used[x]) {
			dfsEPS(nfaEPS, hasNoEPSEdges, used, x, dest, last);
		}
	}
	return;
}

void deleteEPSEdges(const NFA& nfaEPS, NFA& nfa, std::vector<bool>& isFinal, const std::vector<bool>& hasNoEPSEdges, int last) {
	nfa.resize(nfaEPS.size(), std::vector<std::set<int>>(256));
	for (int i = 0; i < nfa.size(); i++) {
		for (int j = 0; j < 256; j++) {
			if (j == EPS || nfaEPS[i][j].empty()) {
				continue;
			}
			nfa[i][j] = nfaEPS[i][j];
		}
		if (!nfaEPS[i][EPS].empty()) {
			std::vector<bool> used(nfa.size());
			dfsEPS(nfaEPS, hasNoEPSEdges, used, i, nfa[i][EPS], last);
		}
	}
	printNFA(nfa, isFinal);
	for (int i = 0; i < nfa.size(); i++) {
		if (nfa[i][EPS].find(last) != nfa[i][EPS].end()) {
			isFinal[i] = true;
			nfa[i][EPS].erase(last);
		}
		for (const int& x : nfa[i][EPS]) {
			for (int j = 0; j < 256; j++) {
				if (j == EPS) {
					continue;
				}
				for (const int& to : nfa[x][j]) {
					nfa[i][j].insert(to);
				}
			}
		}
		nfa[i][EPS].clear();
	}
}

DFA makeDFA(const NFA& nfa, const std::vector<bool> isFinal, const Alphabet& alpha) {
	DFA dfa(alpha);
	std::deque<std::string> q;
	std::string start(nfa.size(), '0');

	start[START] = '1';
	q.push_back(start);

	dfa.create_state(start);
	dfa.set_initial(start);
	if (isFinal[START]) {
		dfa.make_final(start);
	}
	while (!q.empty()) {
		std::string temp = q.front();
		q.pop_front();
		//std::cerr << temp << "\n";
		for (int j = 0; j < 256; j++) {
			std::string dest(nfa.size(), '0');
			bool fin = false, empty = true;
			for (int i = 0; i < temp.size(); i++) {
				if (temp[i] == '1') {
					for (const int& x : nfa[i][j]) {
						empty = false;
						//std::cerr << "qqq2\n";
						dest[x] = '1';
						if (isFinal[x]) {
							fin = true;
						}
					}
				}
			}
			if (empty) {
				continue;
			}
			std::cerr << temp << " " << (char)j << " " << dest << "\n";
			if (!dfa.has_state(dest)) {
				dfa.create_state(dest);
				if (fin) {
					dfa.make_final(dest);
				}
				q.push_back(dest);
			}
			dfa.set_trans(temp, j, dest);
		}
	}

	return dfa;
}


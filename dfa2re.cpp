#include "api.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <set>
#include <map>

void markUsedStatesWithDR(const DFA& d, std::map<std::string, std::vector<std::string>>& dR, std::set<std::string>& used, const std::string& alpha, const std::string& state) {
	used.insert(state);
	for (char c : alpha) {
		if (d.has_trans(state, c)) {
			std::string to = d.get_trans(state, c);
			dR[to].push_back(state);
			if (used.find(to) == used.end()) {
				markUsedStatesWithDR(d, dR, used, alpha, to);
			}
		}
	}
	return;
}

void markUsedStates(std::map<std::string, std::vector<std::string>>& d, std::set<std::string>& used, const std::string& state) {
	used.insert(state);
	for (const std::string& to : d[state]) {
		if (used.find(to) == used.end()) {
			markUsedStates(d, used, to);
		}
	}
	return;
}

void dfs(const DFA& d, DFA& dfa, std::map<std::string, std::string>& newState, std::set<std::string>& used, const std::string& alpha, const std::string& state) {
	used.insert(state);
	for (char c : alpha) {
		if (d.has_trans(state, c)) {
			std::string to = d.get_trans(state, c);
			dfa.set_trans(newState[state], c, newState[to]);
			if (used.find(to) == used.end()) {
				dfs(d, dfa, newState, used, alpha, to);
			}
		}
	}
	return;
}

DFA dfa_minim(DFA& d) {

	std::string alpha = d.get_alphabet().to_string();
	std::set<std::string> used;
	std::string startState = d.get_initial_state();
	std::map<std::string, std::vector<std::string>> dR;

	markUsedStatesWithDR(d, dR, used, alpha, startState);

	std::set<std::set<std::string>> P;
	std::set<std::string> F = d.get_final_states(), Q_F = d.get_states();

	for (auto it = Q_F.begin(); it != Q_F.end();) {
		std::string temp = *it;
		it++;
		if (used.find(temp) == used.end()) {
			F.erase(temp);
			Q_F.erase(temp);
		}
	}
	used.clear();

	for (auto it = F.begin(); it != F.end(); it++) {
		Q_F.erase(*it);
		markUsedStates(dR, used, *it);
	}
	
	for (auto it = Q_F.begin(); it != Q_F.end();) {
		std::string temp = *it;
		it++;
		if (used.find(temp) == used.end()) {
			Q_F.erase(temp);
		}
	}
	used.clear();

	P.insert(F);
	P.insert(Q_F);
	std::set<std::pair<std::set<std::string>, char>> S;
	for (char c : alpha) {
		S.insert({ F, c });
		S.insert({ Q_F, c });
	}
	while (!S.empty()) {
		std::pair<std::set<std::string>, char> temp = *S.begin();
		S.erase(temp);
		std::set<std::set<std::string>> needToDel;
		for (std::set<std::string> const& R : P) {
			std::set<std::string> R1, R2;
			for (std::string const& s : R) {
				if (!d.has_trans(s, temp.second) || temp.first.find(d.get_trans(s, temp.second)) == temp.first.end()) {
					R2.insert(s);
				}
				else {
					R1.insert(s);
				}
			}
			if (!R1.empty() && !R2.empty()) {
				P.insert(R1);
				P.insert(R2);
				needToDel.insert(R);
				bool haveOne = true;
				if (S.find({ R, temp.second }) != S.end()) {
					S.erase({ R, temp.second });
					haveOne = false;
				}
				if (haveOne) {
					if (R1.size() < R2.size()) {
						for (char c : alpha) {
							S.insert({ R1, c });
						}
					}
					else {
						for (char c : alpha) {
							S.insert({ R2, c });
						}
					}
				}
				else {
					for (char c : alpha) {
						S.insert({ R1, c });
						S.insert({ R2, c });
					}
				}
			}
		}
		for (auto it = needToDel.begin(); it != needToDel.end(); it++) {
			P.erase(*it);
		}
		needToDel.clear();
	}

	DFA dfa(d.get_alphabet());
	std::map<std::string, std::string> newState;

	for (const std::set<std::string>& R : P) {
		if (R.empty()) {
			continue;
		}
		std::string cur = *R.begin();
		dfa.create_state(cur);
		if (d.is_final(cur)) {
			dfa.make_final(cur);
		}
		for (const std::string& s : R) {
			newState[s] = cur;
		}
	}
	dfa.set_initial(newState[startState]);
	dfs(d, dfa, newState, used, alpha, newState[startState]);
	return dfa;
}

void printRENFA(const std::map<std::string, std::set<std::pair<std::string, std::string>>>& nfa) {
	std::cerr << "----------NFA----------\n";
	for (auto i = nfa.begin(); i != nfa.end(); i++) {
		for (auto j = i->second.begin(); j != i->second.end(); j++) {
			std::cerr << "[" << i->first << "] " << j->second << " [" << j->first << "]\n";
		}
	}
	std::cerr << "----------END----------\n";
}

std::string dfa2re(DFA &d) {
	
	DFA dfa = d;
	if (dfa.get_final_states().empty()) {
		return "";
	}
	std::map<std::string, std::set<std::pair<std::string, std::string>>> renfa, prrenfa; // renfa[state] = {nextState, regexp}, prrenfa[state] = {prevState, regexp}
	std::string q0 = dfa.get_initial_state(), qf = "UnicEnd123456";
	std::string alpha = dfa.get_alphabet().to_string();
	
	for (const std::string& state : dfa.get_states()) {
		for (char c : alpha) {
			if (dfa.has_trans(state, c)) {
				std::string next = dfa.get_trans(state, c);
				std::string temp = "";
				temp += c;
				renfa[state].insert({ next, temp });
				prrenfa[next].insert({ state, temp });
			}
		}
		if (dfa.is_final(state)) {
			renfa[state].insert({ qf, ""});
			prrenfa[qf].insert({ state, ""});
		}
	}
	//std::cerr << "s2\n";
	std::set<std::string> statesToDel = dfa.get_states();
	statesToDel.erase(q0);
	std::cerr << "q0: " << q0 << std::endl;
	printRENFA(renfa);
	while (!statesToDel.empty()) {
		std::string state = *(statesToDel.begin());
		statesToDel.erase(state);
		std::cerr << state << std::endl;
		std::string o = "";
		while (true) {
			auto itO = renfa[state].lower_bound({ state, "" });
			if (itO != renfa[state].end() && itO->first == state) {
				std::string temp = itO->second;
				if (o == "") {
					o += "(" + temp + ")";
				}
				else {
					o += "|(" + temp + ")";
				}
				renfa[state].erase({ state, temp });
				prrenfa[state].erase({ state, temp });
			}
			else {
				break;
			}
		}
		std::cerr << "o: " << o << std::endl;
		std::set<std::pair<std::string, std::string>> prevStates = prrenfa[state], nextStates = renfa[state];
		for (const std::pair<std::string, std::string>& prev : prevStates) {
			for (const std::pair<std::string, std::string>& next : nextStates) {
				
				std::cerr << prev.first << ": " << prev.second << "; " << next.first << ": " << next.second << ";\n";
				auto it = renfa[prev.first].lower_bound({ next.first, "" });
				
				if (it != renfa[prev.first].end() && it->first == next.first) {
					std::string prefix = it->second;
					std::string add = "|";
					if (prev.second != "") {
						add += "(";
						add += prev.second;
						add += ")";
					}
					if (o != "") {
						add += "(";
						add += o;
						add += ")*";
					}
					if (next.second != "") {
						add += "(";
						add += next.second;
						add += ")";
					}
					std::cerr << add << std::endl;
					renfa[prev.first].erase({ next.first, prefix });
					prrenfa[next.first].erase({ prev.first, prefix });

					if (prefix != "") {
						prefix = "(" + prefix + ")";
					}

					renfa[prev.first].insert({ next.first, prefix + add});
					prrenfa[next.first].insert({ prev.first,  prefix + add});
				}
				else {
					std::string add = "";
					if (prev.second != "") {
						add += "(";
						add += prev.second;
						add += ")";
					}
					if (o != "") {
						add += "(";
						add += o;
						add += ")*";
					}
					if (next.second != "") {
						add += "(";
						add += next.second;
						add += ")";
					}
					std::cerr << add << std::endl;
					renfa[prev.first].insert({ next.first, add });
					prrenfa[next.first].insert({ prev.first, add });
				}
			}
		}
		for (const std::pair<std::string, std::string>& prev : prrenfa[state]) {
			renfa[prev.first].erase({state, prev.second});
		}
		for (const std::pair<std::string, std::string>& next : renfa[state]) {
			prrenfa[next.first].erase({ state, next.second });
		}
		renfa.erase(state);
		prrenfa.erase(state);
	}
	std::cerr << "s3\n";
	{
		std::string q0_q0 = "", q0_qf = "";
		for (auto it = renfa[q0].begin(); it != renfa[q0].end(); it++) {
			if (it->first == q0) {
				if (q0_q0 == "") {
					q0_q0 += "(" + it->second + ")";
				}
				else {
					q0_q0 += "|(" + it->second + ")";
				}
			}
			else if (it->first == qf) {
				if (q0_qf == "") {
					q0_qf += "(" + it->second + ")";
				}
				else {
					q0_qf += "|(" + it->second + ")";
				}
			}
			else {
				exit(-1);
			}
		}
		renfa.erase(q0);
		renfa[q0].insert({ q0, q0_q0 });
		renfa[q0].insert({ qf, q0_qf });
	}
	printRENFA(renfa);
	
	std::string _n = "", _a = "", _b = "", _o = "";
	if (renfa.size() != 2) {
		if (renfa[q0].size() == 0) {
			std::cerr << "answer: \n";
			return "";
		}
		if (renfa[q0].size() == 1) {
			std::cerr << "answer: " << renfa[q0].begin()->second << std::endl;
			return renfa[q0].begin()->second;
		}
		else if (renfa[q0].size() == 2) {
			if (renfa[q0].begin()->first == q0) {
				_n = renfa[q0].begin()->second;
				_a = (++renfa[q0].begin())->second;
			}
			else {
				_a = renfa[q0].begin()->second;
				_n = (++renfa[q0].begin())->second;
			}
			std::string answer = "(" + _n + ")*(" + _a + ")";
			std::cerr << "answer: " << answer << std::endl;
			return answer;
		}
		else {
			int err = 0;
			for (auto it = renfa[q0].begin(); it != renfa[q0].end(); it++) {
				if (it->first == q0) {
					err++;
				}
			}
			std::cerr << "Failed assert on line 305\n";
			exit(dfa.size());
		}
	}
	if (renfa[q0].begin()->first == q0) {
		_n = renfa[q0].begin()->second;
		_a = (++renfa[q0].begin())->second;
	}
	else {
		_a = renfa[q0].begin()->second;
		if (renfa[q0].size() != 1) {
			_n = (++renfa[q0].begin())->second;
		}
	}

	if (renfa[qf].begin()->first == qf) {
		_o = renfa[qf].begin()->second;
		if (renfa[qf].size() != 1) {
			_b = (++renfa[qf].begin())->second;
		}
	}
	else {
		_b = renfa[qf].begin()->second;
		if (renfa[qf].size() != 1) {
			_o = (++renfa[qf].begin())->second;
		}
	}
	std::cerr << "s4\n";
	std::string answer = "(" + _n + ")*(" + _a + ")((" + _o + ")|(" + _b + ")(" + _n + ")*(" + _a + "))*";
	std::cerr << "answer: " << answer << std::endl;
	return answer;
}

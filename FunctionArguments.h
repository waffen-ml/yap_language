#pragma once

#include <vector>
#include <map>
#include <string>
#include <memory>
#include <utility>
#include "Variable.h"


using namespace std;


class FunctionArguments {
public:
	vector<Variable> positional;
	map<string, Variable> keyword;

	void add_positional(Variable& var) {
		positional.push_back(var);
	}

	void add_keyword(string name, Variable& var) {
		keyword.insert(pair<string, Variable>(name, var));
	}

	shared_ptr<Variable> get_keyword(string name) {
		auto it = keyword.find(name);
		if (it == keyword.end())
			return nullptr;
		return shared_ptr<Variable>(&it->second);
	}
};



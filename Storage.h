#pragma once

#include <map>
#include <string>
#include <memory>
#include <stdexcept>
#include <vector>
#include "Variable.h"

using namespace std;

class Storage {
private:
	vector<Variable> & variables;
	map<string, int> name_id;
	vector<int> local_id_id;
	Storage* parent = nullptr;

public:

	Storage(vector<Variable> & variables_, Storage* parent) : variables(variables_) {
		this->parent = parent;
	}

	Variable& get_variable(int global_id) {
		return variables[global_id];
	}

	shared_ptr<Variable> get_variable(string name) {
		auto var = get_local_variable(name);

		if (var == nullptr && parent)
			return parent->get_variable(name);

		return var;
	}

	Variable& get_local_variable(int local_id) {
		return variables[local_id_id[local_id]];
	}

	shared_ptr<Variable> get_local_variable(string name) {
		auto it = name_id.find(name);
		if (it == name_id.end())
			return nullptr;
		return shared_ptr<Variable>(&variables[it->second]);
	}

	int add_variable(VariableType type, shared_ptr<void> value_ptr, bool is_const) {
		int id = variables.size();
		int local_id = local_id_id.size();

		Variable var(id, type, value_ptr, is_const);

		variables.push_back(var);
		local_id_id.push_back(id);

		return local_id;
	}

	int add_variable(string name, VariableType type, shared_ptr<void> value_ptr, bool is_const) {
		if (get_local_variable(name) != nullptr)
			throw invalid_argument("Variable with this name already exists in this env.!");

		int id = variables.size();
		Variable var(id, type, value_ptr, is_const);

		name_id.insert(pair<string, int>(name, id));
		variables.push_back(var);

		return id;
	}

	Storage* forward() {
		return new Storage(variables, this);
	}

	Storage* backward() {
		if (parent == nullptr)
			throw runtime_error("Cannot move scope backward because parent is null");
		return parent;
	}

	int add_global_reference(int global_id) {
		local_id_id.push_back(global_id);
		return local_id_id.size() - 1;
	}

	int add_parent_reference(int parent_local_id) {
		if (parent == nullptr)
			throw runtime_error("Parent is null");
		local_id_id.push_back(parent->get_local_reference(parent_local_id));
		return local_id_id.size() - 1;
	}

	int get_local_reference(int local_id) {
		return local_id_id[local_id];
	}
};

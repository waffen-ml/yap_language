#pragma once
#include <vector>
#include "Variable.h"
#include "Storage.h"


class Runtime {
private:
	vector<Variable> memory;
	Storage* storage_;

public:
	Runtime() {
		storage_ = new Storage(memory, nullptr);
	}

	Storage* storage() {
		return storage_;
	}

	void scope_fwd() {
		storage_ = storage_->forward();
	}

	void scope_bcw() {
		storage_ = storage_->backward();
	}

	Variable& id_to_var(int id) {
		if (id >= 0)
			return this->storage()->get_local_variable(id);
		else
			return this->storage()->get_variable(-id - 1);
	}

	vector<shared_ptr<Variable>> map_var_ids(vector<int>& ids) {
		vector<shared_ptr<Variable>> mapped(ids.size());

		for (int i = 0; i < ids.size(); i++) {
			Variable var = id_to_var(ids[i]);
			mapped[i] = make_shared<Variable>(var);
		}

		return mapped;
	}

};
#pragma once

#include <memory>
#include <string>
#include <stdexcept>
#include <map>

using namespace std;


enum VariableType {
	NONE,
	INT,
	STRING,
	BUILT_IN_FUNCTION
};


/*
map<VariableType, size_t> data_sizes = {
	{NONE, 0},
	{INT, sizeof(int)},
	{STRING, sizeof(string)},
	{BUILT_IN_FUNCTION, 0}
};

*/



class Variable {

private:
	int id_ = -1;
	VariableType type_ = NONE;
	shared_ptr<void> value_ptr_;
	bool is_const_ = false;

public:

	Variable(int id, VariableType type, shared_ptr<void> value_ptr, bool is_const) {
		type_ = type;
		value_ptr_ = value_ptr;
		id_ = id;
		is_const_ = is_const;
	}

	Variable(int id, VariableType type, shared_ptr<void> value_ptr) : Variable(id, type, value_ptr, false) {}

	VariableType type() {
		return type_;
	}

	shared_ptr<void> value_ptr() {
		return value_ptr_;
	}

	bool is_const() {
		return is_const_;
	}

	int id() {
		return id_;
	}

	void set(VariableType type, shared_ptr<void> value_ptr) {
		if (is_const_)
			throw runtime_error("Cannot set value of the variable because it is const!");

		type_ = type;
		value_ptr_ = value_ptr;
	}

	template<typename T>
	T as() {
		return *as_ptr<T>();
	}

	template<typename T>
	shared_ptr<T> as_ptr() {
		return static_pointer_cast<T>(value_ptr_);
	}
};
//#pragma once

#include <iostream>
#include <map>
#include <vector>
#include <functional>
#include <stdexcept>
#include "Variable.h"
#include "Runtime.h"
#include <array>

#define BCARGS vector<int>& args, Runtime & rt
#define ARGVEC vector<int>

enum ByteCommandType {
	SC_FWD,
	SC_BCW,
	PRINT,
	CREATE_INT,
	INCREMENT,
	REF,
	REF_GLOBAL,
	ADD,
	SUB,
	MUL,
	DIV,
	CP,
	CP_REF
};


class ByteCommand {
private:
	ByteCommandType type_;
	vector<int> args_;

public:
	ByteCommand(ByteCommandType type, vector<int> args) {
		type_ = type;
		args_ = args;
	}

	ByteCommand(ByteCommandType type) : ByteCommand(type, vector<int>()) {};

	vector<int>& args() {
		return args_;
	}

	ByteCommandType type() {
		return type_;
	}
};



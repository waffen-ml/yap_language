#include <iostream>
#include <memory>
#include <vector>
#include <sstream>
#include <iostream>
#include "Interpreter.h"

using namespace std;


class Interpreter {
private:
	map<ByteCommandType, function<void(vector<int>&, Runtime&)>> commands;

public:

	void bind(ByteCommandType type, function<void(vector<int>&, Runtime&)> func) {
		commands.insert({ type, func });
	}

	void execute(ByteCommand& command, Runtime& runtime) {
		auto it = commands.find(command.type());
		if (it == commands.end())
			throw runtime_error("Byte command type is not supported");
		it->second(command.args(), runtime);
	}

	void execute(vector<ByteCommand>& commands, Runtime& runtime) {
		for (ByteCommand& command : commands)
			execute(command, runtime);
	}

};


void print(vector<shared_ptr<Variable>> & args) {
	stringstream stream;

	for (auto var : args) {

		switch (var->type()) {
		case INT:
			stream << var->as<int>();
			break;
		case STRING:
			stream << var->as<string>();
			break;
		case NONE:
			stream << "null";
			break;
		default:
			stream << "<Object>";
			break;
		}

		stream << " ";
	}

	std::cout << stream.str() << std::endl;
}

void math_oper(int type, vector<int>& args, Runtime & rt) {
	Variable& a = rt.id_to_var(args[0]);
	auto ptr = a.as_ptr<int>();

	for (int i = 1; i < args.size(); i++) {
		Variable& b = rt.id_to_var(args[1]);
		int v = b.as<int>();

		switch (type) {
		case 0:
			*ptr = *ptr + v;
			break;
		case 1:
			*ptr = *ptr - v;
			break;
		case 2:
			*ptr = *ptr * v;
			break;
		case 3:
			*ptr = *ptr / v;
			break;
		}
	}
}

Interpreter make_interpreter() {

	Interpreter ip;

	ip.bind(SC_FWD, [](BCARGS) {
		rt.scope_fwd();
	});

	ip.bind(SC_BCW, [](BCARGS) {
		rt.scope_bcw();
	});

	ip.bind(PRINT, [](BCARGS) {
		auto v_args = rt.map_var_ids(args);
		print(v_args);
	});

	ip.bind(CREATE_INT, [](BCARGS) {
		int value = args.size() == 0 ? 0 : args[0];
		rt.storage()->add_variable(INT, make_shared<int>(value), false);
	});

	ip.bind(INCREMENT, [](BCARGS) {
		auto ptr = rt.storage()->get_local_variable(args[0]).as_ptr<int>();
		*ptr = *ptr + 1;
	});

	ip.bind(REF, [](BCARGS) {
		rt.storage()->add_parent_reference(args[0]);
	});

	ip.bind(REF_GLOBAL, [](BCARGS) {
		rt.storage()->add_global_reference(args[0]);
	});

	ip.bind(ADD, [](BCARGS) {
		math_oper(0, args, rt);
	});

	ip.bind(SUB, [](BCARGS) {
		math_oper(1, args, rt);
	});

	ip.bind(MUL, [](BCARGS) {
		math_oper(2, args, rt);
	});

	ip.bind(DIV, [](BCARGS) {
		math_oper(3, args, rt);
	});

	ip.bind(CP_REF, [](BCARGS) {
		Variable& a = rt.id_to_var(args[0]);
		Variable& b = rt.id_to_var(args[1]);

		a.set(b.type(), b.value_ptr());
	});

	ip.bind(CP, [](BCARGS) {
		Variable& a = rt.id_to_var(args[0]);
		Variable& b = rt.id_to_var(args[1]);

		shared_ptr<void> ptr;

		switch (b.type()) {
		case INT:
			ptr = make_shared<int>(b.as<int>());
			break;
		default:
			throw runtime_error("Copying this type is not supported!");
		}

		a.set(b.type(), ptr);
	});

	return ip;
}


int main() {
	Interpreter ip = make_interpreter();
	Runtime runtime;
	
	vector<ByteCommand> commands{
		ByteCommand(CREATE_INT, vector<int> {5}),
		ByteCommand(CREATE_INT, vector<int> {10}),
		ByteCommand(CP_REF, vector<int> {0, 1}),
		ByteCommand(INCREMENT, vector<int> {0}),
		ByteCommand(PRINT, vector<int> {0, 1})
	};

	ip.execute(commands, runtime);

}
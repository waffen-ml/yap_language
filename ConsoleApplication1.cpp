#include <iostream>
#include <vector>
#include <map>
#include <sstream>
#include "consoleapplication.h"

#define NULL_TYPE 0
#define INT_TYPE 1
#define STRING_TYPE  2
#define BUILT_IN_FUNCTION_TYPE 3


using namespace std;


class Variable {

private:
	string name_;
	int type_ = NULL_TYPE;	
	void* value_ptr_ = NULL;

public:
	Variable() {

	}

	Variable(string name, int type, void* value_ptr) {
		name_ = name;
		type_ = type;
		value_ptr_ = value_ptr;
	}

	~Variable() {
		//if (type_ != BUILT_IN_FUNCTION_TYPE)
		//	delete value_ptr_;
	}

	int type() {
		return type_;
	}

	void* value_ptr() {
		return value_ptr_;
	}

	string name() {
		return name_;
	}

	int as_int() {
		return *static_cast<int*>(value_ptr_);
	}

	string as_string() {
		return *static_cast<string*>(value_ptr_);
	}

	BuiltInFunction* as_built_in_function() {
		return static_cast<BuiltInFunction*>(value_ptr_);
	}

};


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

	Variable* get_keyword(string name) {
		auto it = keyword.find(name);
		if (it == keyword.end())
			return NULL;
		return &(it->second);
	}
};


class Environment {
private:
	map<string, Variable> variables;
	Environment* parent = NULL;

public:

	Environment() {}

	Environment(Environment* parent) {
		this->parent = parent;
	}

	Variable & get_local_variable(string name) {
		auto it = variables.find(name);
		if (it == variables.end())
			throw runtime_error("Variable with name\"" + name + "\" has not been found.");
		return it->second;
	}

	Variable & get_variable(string name) {
		if (!parent || has_local_variable(name))
			return get_local_variable(name);
		return parent->get_variable(name);
	}

	bool has_variable(string name) {
		return has_local_variable(name) || (parent && parent->has_variable(name));
	}

	bool has_local_variable(string name) {
		return variables.find(name) != variables.end();
	}

	void add_variable(Variable var) {
		if (has_local_variable(var.name()))
			throw invalid_argument("Variable with this name already exists in this env.!");
		variables.insert(pair<string, Variable>(var.name(), var));
	}
};


class BuiltInFunction {
private:
	Variable(*func)(FunctionArguments, Environment);
public:
	BuiltInFunction(Variable(*func)(FunctionArguments, Environment)) {
		this->func = func;
	}

	Variable call(FunctionArguments args, Environment env) {
		return func(args, env);
	}
};


template <typename T>
class ParseResult {
private:
	T value_;
	int end_;

public:
	ParseResult() {
		end_ = -1;
	}
	ParseResult(T value, int end) {
		value_ = value;
		end_ = end;
	}
	int end() {
		return end_;
	}
	bool empty() {
		return end_ < 0;
	}
	T value() {
		return value_;
	}
	T* value_ptr() {
		return &value_;
	}
};


int skip_space(string expr, int i) {
	while (i < expr.length() && expr[i] == ' ')
		i++;

	return i;
}


bool is_naming_start(char ch) {
	return 'A' <= ch && ch <= 'Z' || 'a' <= ch && ch <= 'z' || ch == '$' || ch == '_';
}


bool is_naming_continue(char ch) {
	return '0' <= ch && ch <= '9' || is_naming_start(ch);
;}


char slash_transform(char ch) {
	switch (ch) {
	case 'n':
		return '\n';
	default:
		return ch;
	}
}


ParseResult<string> parse_name(string expr, int i) {
	if (!is_naming_start(expr[i]))
		return ParseResult<string>();
	
	int j = i + 1;

	while (j < expr.length() && is_naming_continue(expr[j]))
		j++;

	return ParseResult<string>(expr.substr(i, j - i), j);
}


ParseResult<string> parse_string(string expr, int i) {
	char openSymbol = expr[i];

	if (openSymbol != '"' && openSymbol != '\'')
		return ParseResult<string>();
	
	stringstream stream;
	int j = i + 1;

	while (j < expr.length()) {
		if (expr[j] == '\\' && j + 1 < expr.length()) {
			stream << slash_transform(expr[j + 1]);
			j += 2;
			continue;
		}
		else if (expr[j] == openSymbol)
			return ParseResult<string>(stream.str(), j + 1);
		else
			stream << expr[j];

		j++;
	}

	return ParseResult<string>();
}


ParseResult<int> parse_int(string expr, int i) {
	int coef = 1;
	int n = 0;

	if (expr[i] == '-') {
		coef = -1;
		i++;
	}

	int j = i;

	while (j < expr.length() && '0' <= expr[j] && '9' >= expr[j]) {
		n = n * 10 + (expr[j] - '0');
		j++;
	}

	if (i == j)
		return ParseResult<int>();

	return ParseResult<int>(coef * n, j);
}


ParseResult<Variable> eval(string expr, int i, Environment& env) {
	auto r_string = parse_string(expr, i);

	if (!r_string.empty()) {
		Variable var("", STRING_TYPE, new string(r_string.value()));
		return ParseResult<Variable>(var, r_string.end());
	}

	auto r_int = parse_int(expr, i);

	if (!r_int.empty()) {
		Variable var("", INT_TYPE, new int(r_int.value()));
		return ParseResult<Variable>(var, r_int.end());
	}

	auto r_name = parse_name(expr, i);
	
	if (r_name.empty() || !env.has_variable(r_name.value()))
		return ParseResult<Variable>();

	Variable var = env.get_variable(r_name.value());
	auto r_args = parse_arguments(expr, r_name.end(), env);


	if (!r_args.empty() && var.type() != BUILT_IN_FUNCTION_TYPE || r_args.empty() && var.type() == BUILT_IN_FUNCTION_TYPE)
		return ParseResult<Variable>();


	if (r_args.empty())
		return ParseResult<Variable>(var, r_name.end());
	else {
		Variable func_result = var.as_built_in_function()->call(r_args.value(), env);
		return ParseResult<Variable>(func_result, r_args.end());
	}

}


ParseResult<FunctionArguments> parse_arguments(string expr, int i, Environment & env) {
	FunctionArguments args;

	if (expr[i] != '(')
		return ParseResult<FunctionArguments>();

	i = skip_space(expr, i + 1);

	while (i < expr.length()) {
		if (expr[i] == ')')
			return ParseResult<FunctionArguments>(args, i + 1);
		
		auto result = eval(expr, i, env);

		if (result.empty())
			return ParseResult<FunctionArguments>();

		Variable var = result.value();
		args.add_positional(var);
		i = skip_space(expr, result.end());

		if (i >= expr.length() || expr[i] == ')')
			continue;
		else if (expr[i] == ',')
			i = skip_space(expr, i + 1);
		else
			return ParseResult<FunctionArguments>();
	}

	return ParseResult<FunctionArguments>();
}


void shoutout(void* p) {
	cout << "hey" << endl;
}

Variable yap_func(FunctionArguments args, Environment env) {
	stringstream stream;

	for (Variable var : args.positional) {

		switch (var.type()) {
		case INT_TYPE:
			stream << var.as_int();
			break;
		case STRING_TYPE:
			stream << var.as_string();
			break;
		case NULL_TYPE:
			stream << "null";
			break;
		default:
			stream << "<Object>";
			break;
		}

		stream << " ";
	}

	cout << stream.str() << endl;

	return env.get_variable("null");
}


int main() {

	Environment env;

	Variable null_var("null", NULL_TYPE, NULL);
	env.add_variable(null_var);

	Variable test_func("yap", BUILT_IN_FUNCTION_TYPE, new BuiltInFunction(yap_func));
	env.add_variable(test_func);
	
	eval("yap(yap(12, 'new language hits different'), 12, null)", 0, env);

;}
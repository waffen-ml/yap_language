#pragma once

using namespace std;


class Variable;
class FunctionArguments;
class Environment;
class BuiltInFunction;

template <typename T>
class ParseResult;

ParseResult<FunctionArguments> parse_arguments(string expr, int i, Environment& env);
ParseResult<Variable> eval(string expr, int i, Environment& env);




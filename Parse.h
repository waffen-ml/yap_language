#pragma once

#include <string>
#include <sstream>
#include <memory>
#include <stdexcept>
#include <algorithm>
#include <vector>
#include "Variable.h"

using namespace std;


template <typename T>
class ParseResult {
private:
	unique_ptr<T> value_ptr_;
	int end_;

public:
	ParseResult() {
		end_ = -1;
		value_ptr_ = nullptr;
	}
	ParseResult(T value, int end) {
		end_ = end;
		value_ptr_ = make_unique<T>(value);
	}
	int end() {
		return end_;
	}
	bool empty() {
		return end_ < 0;
	}
	T value() {
		if (!value_ptr_)
			throw runtime_error("The result is empty!");
		return *value_ptr_;
	}
};


enum TokenType {
	LITERAL,
	NAME,
	OPERATOR,
	KEYWORD,
	SPECIAL
};


class Token {
private:
	TokenType type_;
	string value_;
	int var_id_;

public:
	Token(TokenType type, string value) {
		type_ = type;
		value_ = value;
		var_id_ = -1;
	}

	Token(TokenType type, string value, int var_id) {
		type_ = type;
		value_ = value;
		var_id_ = var_id;
	}

	TokenType type() {
		return type_;
	}

	string value() {
		return value_;
	}

	bool is(string str) {
		return value_ == str;
	}

	bool has_var() {
		return var_id_ > -1;
	}

	int var_id() {
		return var_id_;
	}

};


extern vector<string> operators;
extern vector<string> keywords;
extern vector<string> specials;
extern vector<pair<string, TokenType>> operators_specials;

ParseResult<string> parse_name(string expr, int i);
ParseResult<string> parse_string(string expr, int i);
ParseResult<int> parse_int(string expr, int i);
extern ParseResult<pair<string, TokenType>> parse_operator_special(string expr, int i);
extern vector<Token> tokenize(string expr, int i);

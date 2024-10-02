#include <iostream>
#include <vector>
#include <utility>
#include "Parse.h"

using namespace std;

vector<string> keywords = {
	"if", "for", "function",
	"class"
};


vector<string> operators = {
	"+", "-", "*", "/", "=", "++",
	"--", "+=", "-=", "*=", "/=",
	"&&", "||", "!", ">", "<",
	">=", "<=", "!=", "=="
};

vector<string> specials = {
	".", ",", "(", ")", "[", "]",
	"{", "}", "=>", "\n"
};

vector<pair<string, TokenType>> operators_specials = []() {
	vector<pair<string, TokenType>> m(operators.size() + specials.size());

	for (int i = 0; i < operators.size(); i++)
		m[i] = pair<string, TokenType>(operators[i], OPERATOR);

	for (int i = 0; i < specials.size(); i++)
		m[i + operators.size()] = pair<string, TokenType>(specials[i], SPECIAL);

	return m;
}();


int skip_space(string expr, int i) {
	while (i < expr.length() && expr[i] == ' ')
		i++;

	return i;
}


bool is_name_start(char ch) {
	return 'A' <= ch && ch <= 'Z' || 'a' <= ch && ch <= 'z' || ch == '$' || ch == '_';
}


bool is_name_continue(char ch) {
	return '0' <= ch && ch <= '9' || is_name_start(ch);
}


char slash_transform(char ch) {
	switch (ch) {
	case 'n':
		return '\n';
	default:
		return ch;
	}
}


bool is_keyword(string str) {
	return find(keywords.begin(), keywords.end(), str) != keywords.end();
}


ParseResult<string> parse_name(string expr, int i) {
	if (!is_name_start(expr[i]))
		return ParseResult<string>();

	int j = i + 1;

	while (j < expr.length() && is_name_continue(expr[j]))
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
	int n = 0;
	int j = i;

	while (j < expr.length() && '0' <= expr[j] && '9' >= expr[j]) {
		n = n * 10 + (expr[j] - '0');
		j++;
	}

	if (i == j)
		return ParseResult<int>();

	return ParseResult<int>(n, j);
}


ParseResult<pair<string, TokenType>> parse_operator_special(string expr, int i) {
	vector<bool> m(operators_specials.size(), true);

	int j = 0;
	int best = -1;
	bool run = true;

	while (i + j < expr.length() && run) {
		run = false;
		//cout << j << endl;
		//for (int w = 0; w < m.size(); w++)
		//	if (m[w])
		//		cout << operators_specials[w].first << " ";
		//cout << endl;

		for (int k = 0; k < operators_specials.size(); k++) {
			if (!m[k])
				continue;
			if (operators_specials[k].first.length() <= j || operators_specials[k].first[j] != expr[i + j]) {
				m[k] = false;
				continue;
			}
			if (operators_specials[k].first.length() == j + 1)
				best = k;
			run = true;
		}

		j++;
	}

	if (best == -1)
		return ParseResult<pair<string, TokenType>>();
	else
		return ParseResult<pair<string, TokenType>>(
			operators_specials[best], i + operators_specials[best].first.length());
}


vector<Token> tokenize(string expr, int i) {
	vector<Token> r;

	auto setIdx = [&i, expr](int w) {
		i = skip_space(expr, w);
	};

	setIdx(i);

	while (i < expr.length()) {
		auto oper_spec_res = parse_operator_special(expr, i);

		if (!oper_spec_res.empty()) {
			r.push_back(Token(oper_spec_res.value().second, oper_spec_res.value().first));
			setIdx(oper_spec_res.end());
			continue;
		}

		auto name_like_res = parse_name(expr, i);

		if (!name_like_res.empty()) {
			TokenType t = is_keyword(name_like_res.value()) ? KEYWORD : NAME;
			r.push_back(Token(t, name_like_res.value()));
			setIdx(name_like_res.end());
			continue;
		}

		auto int_res = parse_int(expr, i);

		if (!int_res.empty()) {
			//r.push_back(
			//	Token(
			//		LITERAL, "", 
			//		env.add_variable("", INT, make_shared<int>(int_res.value()), true)
			//	)
			//);
			setIdx(int_res.end());
			continue;
		}

		auto str_res = parse_string(expr, i);

		if (!str_res.empty()) {
			//r.push_back(
			//	Token(
			//		LITERAL, "", 
			//		env.add_variable("", STRING, make_shared<string>(str_res.value()), true)
			//	)
			//);
			setIdx(str_res.end());
			continue;
		}
		
		throw runtime_error("Unable to tokenize the expression");
	}
	
	return r;
}


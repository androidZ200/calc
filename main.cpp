#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <cstdlib>

constexpr char* crutch = "-1";
enum class Type {
	Number, Operator, Open, Close, KeyWord, Parametr, Comma
};
struct Param {
	std::string name;
	double num;
};
const std::vector<std::string> keywords = {"exp", "sin", "cos", "ln", "tan", "asin", "acos", "atan", "atan2", "sqrt", "pow", "abs"};
std::vector<Param> parametrs;
struct Lexema {
	std::string word;
	Type type;
};
bool is_void(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}
bool is_digit(char c) {
	return c >= '0' && c <= '9';
}
bool is_letter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
bool is_keyword(std::string word) {
	std::transform(word.begin(), word.end(), word.begin(),
		[](unsigned char c){ return std::tolower(c); });
	for(auto& x: keywords)
		if(x == word) return true;
	return false;
}
Lexema get_num(const char*& line) {
	const char* t = line;
	if(*line == '-') ++line;
	while(is_digit(*line)) ++line;
	if(*line == '.') ++line;
	while(is_digit(*line)) ++line;
	if(*line == 'e' || *line == 'E')
		if(*(line+1) == '-' || is_digit(*(line + 1))) {
			++line;
			if(*line == '-') ++line;
			while(is_digit(*line)) ++line;
		}
	Lexema lex = {std::string(t, line - t), Type::Number};
	return lex;
}
std::vector<Lexema>* read_line(const char* line) {
	std::vector<Lexema>* magic_vector = new std::vector<Lexema>();

	while(line) {
		while(is_void(*line)) ++line;
		if(*line == '\0') break;

		if(*line == '(') {
			magic_vector->push_back({std::string(line, 1), Type::Open});
			++line;
			continue;
		}
		else if(*line == ')') {
			magic_vector->push_back({std::string(line, 1), Type::Close});
			++line;
			continue;
		}
		else if(*line == ',') {
			magic_vector->push_back({std::string(line, 1), Type::Comma});
			++line;
			continue;
		}
		else if(*line == '+' || *line == '^' || *line == '*' || *line == '/') {
			magic_vector->push_back({std::string(line, 1), Type::Operator});
			++line;
			continue;
		}
		else if(is_letter(*line)) {
			const char* t = line;
			while(is_letter(*line) || is_digit(*line)) ++line;
			Lexema lex = {std::string(t, line - t), Type::KeyWord};
			if(!is_keyword(lex.word))
				lex.type = Type::Parametr;
			magic_vector->push_back(lex);
			continue;
		}
		else if(is_digit(*line)) {
			magic_vector->push_back(get_num(line));
			continue;
		}
		else if(*line == '-') {
			if(magic_vector->size() != 0 && ((*magic_vector)[magic_vector->size() - 1].type == Type::Number ||
				(*magic_vector)[magic_vector->size() - 1].type == Type::Parametr)) {
				magic_vector->push_back({std::string(line, 1), Type::Operator});
				++line;
			}
			else {
				Lexema t = get_num(line);
				if(t.word.size() == 1)
					magic_vector->push_back({std::string(crutch, 2), Type::Number});
				else
					magic_vector->push_back(t);

			}
			continue;
		}
		else {
			throw "unknown character\n";
		}
	}
	return magic_vector;
}
double input_number(std::string name) {
	double nans;
	char *err;
	do {
		std::cout << name << " = ";
		std::string ans;
		std::getline(std::cin, ans);
		nans = std::strtod(ans.c_str(), &err);
	}while(*err != '\0' || errno != 0);
	return nans;
}
double get_param(std::string name) {
	for(auto &x : parametrs)
		if(x.name == name)
			return x.num;

	double nans = input_number(name);
	parametrs.push_back({name, nans});
	return nans;
}
void add_param(Param p) {
	for(auto &x : parametrs)
		if(x.name == p.name)
			throw "parametr " + p.name + " already exist\n";
	parametrs.push_back(p);
}
void del_param(std::string name) {
	for(auto iter = parametrs.begin(); iter != parametrs.end(); iter++)
		if((*iter).name == name) {
			parametrs.erase(iter);
			return;
		}
}

double operator_low(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end);
double op_exp(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return exp(a);
}
double op_sin(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return sin(a);
}
double op_cos(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return cos(a);
}
double op_ln(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(a <= 0) throw "argument of ln have to be more 0\n";
	return log(a);
}
double op_tan(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return tan(a);
}
double op_asin(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(abs(a) > 1) throw "argument of asin have to be from -1 to 1\n";
	return asin(a);
}
double op_acos(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(abs(a) > 1) throw "argument of acos have to be from -1 to 1\n";
	return acos(a);
}
double op_atan(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return atan(a);
}
double op_atan2(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(iter == end) throw "unexpected ending\n";
	if(iter->type != Type::Comma) throw "expected common\n";
	iter++;
	if(iter == end) throw "unexpected ending\n";
	double b = operator_low(iter, end);
	return atan2(a, b);
}
double op_sqrt(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(a < 0) throw "argument of sqrt have to be more 0\n";
	return sqrt(a);
}
double op_pow(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	if(iter == end) throw "unexpected ending\n";
	if(iter->type != Type::Comma) throw "expected common\n";
	iter++;
	if(iter == end) throw "unexpected ending\n";
	double b = operator_low(iter, end);
	return pow(a, b);
}
double op_abs(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_low(iter, end);
	return abs(a);
}

double operator_high(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end);
double operator_brakets(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	if(iter->type == Type::Open) {
		iter++;
		if(iter == end) throw "unexpected ending\n";
		double a = operator_low(iter, end);
		if(iter == end) throw "unexpected ending\n";
		if(iter->type != Type::Close) throw "expected right braket\n";
		iter++;
		return a;
	}
	auto t = iter;
	iter++;
	if(t->type == Type::Number) return atof(t->word.c_str());
	if(t->type == Type::Parametr) return get_param(t->word);
	throw "incorrect input\n";
}
double operator_keyword(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	if(iter->type == Type::KeyWord) {
		auto t = iter;
		iter++;
		if(iter == end) throw "unexpected ending\n";
		if(iter->type != Type::Open) throw "expected left braket\n";
		iter++;
		if(iter == end) throw "unexpected ending\n";
		double a = 0;

		if(t->word == keywords[0]) a = op_exp(iter, end);
		else if(t->word == keywords[1]) a = op_sin(iter, end);
		else if(t->word == keywords[2]) a = op_cos(iter, end);
		else if(t->word == keywords[3]) a = op_ln(iter, end);
		else if(t->word == keywords[4]) a = op_tan(iter, end);
		else if(t->word == keywords[5]) a = op_asin(iter, end);
		else if(t->word == keywords[6]) a = op_acos(iter, end);
		else if(t->word == keywords[7]) a = op_atan(iter, end);
		else if(t->word == keywords[8]) a = op_atan2(iter, end);
		else if(t->word == keywords[9]) a = op_sqrt(iter, end);
		else if(t->word == keywords[10]) a = op_pow(iter, end);
		else if(t->word == keywords[11]) a = op_abs(iter, end);

		if(iter == end) throw "unexpected ending\n";
		if(iter->type != Type::Close) throw "expected right braket\n";
		iter++;
		return a;
	}
	return operator_brakets(iter, end);
}
double operator_mult(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	if(iter->type == Type::Number) {
		auto t = iter;
		iter++;
		if(iter == end || iter->type == Type::Operator || iter->type == Type::Close || iter->type == Type::Comma)
			return atof(t->word.c_str());
		else {
			double b = operator_high(iter, end);
			return atof(t->word.c_str()) * b;
		}
	}
	return operator_keyword(iter, end);
}
double operator_pow(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double a = operator_mult(iter, end);
	if(iter != end && iter->type == Type::Operator && iter->word == "^") {
		iter++;
		if(iter == end) throw "unexpected ending\n";
		double f = operator_mult(iter, end);
		a = pow(a, f);
	}
	return a;
}
double operator_high(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double mul = operator_pow(iter, end);
	while(iter != end && iter->type == Type::Operator && (iter->word == "*" || iter->word == "/")) {
		bool mult = iter->word == "*";
		iter++;
		if(iter == end) throw "unexpected ending\n";
		double f = operator_pow(iter, end);
		if(mult) mul *= f;
		else {
			if(f == 0) throw "division by 0\n";
			mul /= f;
		}
	}
	return mul;
}
double operator_low(std::vector<Lexema>::iterator &iter, std::vector<Lexema>::iterator &end) {
	double sum = operator_high(iter, end);
	while(iter != end && iter->type == Type::Operator && (iter->word == "+" || iter->word == "-")) {
		bool plus = iter->word == "+";
		iter++;
		if(iter == end) throw "unexpected ending\n";
		double f = operator_high(iter, end);
		if(plus) sum += f;
		else sum -= f;
	}
	return sum;
}

void help_function(int argc, char* argv[]) {
	std::cout << "help info\n";
}
int main(int argc, char* argv[])
{
	if(argc == 1 || (argc > 1 && std::string(argv[1]) == "--help")) {
		help_function(argc, argv);
		return 0;
	}

	parametrs.push_back({"pi", M_PIf64});
	parametrs.push_back({"e", M_Ef64});
	parametrs.push_back({"inf", std::numeric_limits<double>::infinity()});
	std::stringstream ss;
	for(int i = 1; i < argc; i++)
		ss << argv[i] << " ";
	std::string line = ss.str();
	std::vector<Lexema>* magic_vector;
	try {
		magic_vector = read_line(line.c_str());
		auto iter = magic_vector->begin();
		auto end = magic_vector->end();
		double res = operator_low(iter, end);
		std::cout << std::setprecision(18) << res << "\n";
		delete magic_vector;
	}
	catch (const char* text) {
		std::cerr << "Error: " << text << "\n";
		delete magic_vector;
		return 1;
	}

	return 0;
}

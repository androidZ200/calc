#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <numeric>
#include <cstdlib>

enum class Type {
	Number, Operator, Open, Close, KeyWord, Parametr, Comma, Equal
};
struct Lexema {
	std::string word;
	Type type;
};
class SmartIterator {
	std::vector<Lexema>::iterator iter;
	std::vector<Lexema>::iterator end;

public:
	SmartIterator(const std::vector<Lexema>::iterator& iter, const std::vector<Lexema>::iterator& end) : iter(iter), end(end)
	{	}

	Lexema& operator*() {
		if(is_end()) throw "unexpected ending";
		return *iter;
	}
	Lexema* operator->() {
		if(is_end()) throw "unexpected ending";
		return &*iter;
	}
	bool is_end() {
		return iter == end;
	}
	SmartIterator operator++() {
		auto clone = *this;
		++iter;
		return clone;
	}
	SmartIterator& operator++(int) {
		++iter;
		return *this;
	}
	SmartIterator& go() {
		++iter;
		if(is_end()) throw "unexpected ending";
		return *this;
	}
};
double operator_low(SmartIterator &iter);
double GET_RESULT(const std::string& line);

bool is_void(char c) {
	return c == ' ' || c == '\t' || c == '\n';
}
bool is_digit(char c) {
	return c >= '0' && c <= '9';
}
bool is_letter(char c) {
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}
struct KeyWord {
	std::string name;
	double (*func)(SmartIterator& iter);
};
std::vector<KeyWord> keywords;
bool is_keyword(std::string word) {
	std::transform(word.begin(), word.end(), word.begin(),
		[](unsigned char c){ return std::tolower(c); });
	for(auto& x: keywords)
		if(x.name == word) return true;
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
		else if(*line == '=') {
			magic_vector->push_back({std::string(line, 1), Type::Equal});
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
					magic_vector->push_back({std::string("-1"), Type::Number});
				else
					magic_vector->push_back(t);

			}
			continue;
		}
		else {
			throw "unknown character";
		}
	}
	return magic_vector;
}
double input_number(std::string name) {
	while(true) {
		std::cout << name << " = ";
		std::string ans;
		std::getline(std::cin, ans);
		try {
			return GET_RESULT(ans);
		}  catch (const char* mes) {
			std::cerr << "Error: " << mes << "\n";
		}
	};
}

enum class TypeParam {
	Const, Blocked, Users
};
struct Param {
	std::string name;
	TypeParam type;
	double num;
};
std::vector<Param> parametrs;
void unblock_param(const std::string& name, double value) {
	for(auto &x : parametrs)
		if(x.name == name) {
			x.type = TypeParam::Users;
			x.num = value;
		}
}
double get_param(const std::string& name) {
	for(auto &x : parametrs)
		if(x.name == name) {
			if(x.type == TypeParam::Blocked) throw "you can't use this parameter";
			return x.num;
		}

	parametrs.push_back({name, TypeParam::Blocked, 0});
	double nans = input_number(name);
	unblock_param(name, nans);
	return nans;
}
void add_param(Param p) {
	for(auto &x : parametrs)
		if(x.name == p.name) {
			if(p.type != TypeParam::Blocked || x.type == TypeParam::Const)
				throw "parameter " + p.name + " already exist";
			else
				x.type = TypeParam::Blocked;
		}
	parametrs.push_back(p);
}
void del_param(const std::string& name) {
	for(auto iter = parametrs.begin(); iter != parametrs.end(); iter++)
		if((*iter).name == name) {
			parametrs.erase(iter);
			return;
		}
}
void change_param(const std::string& name, double value) {
	for(auto &x : parametrs)
		if(x.name == name) {
			if(x.type == TypeParam::Users)
				x.num = value;
			else
				throw "you can't change this parameter";
		}
}

void fill_default_parametrs() {
	add_param({"pi", TypeParam::Const, M_PIf64});
	add_param({"e", TypeParam::Const, M_Ef64});
	add_param({"inf", TypeParam::Const, std::numeric_limits<double>::infinity()});
}
void fill_keywords() {
	keywords.push_back({"exp", [](SmartIterator& iter){
							double a = operator_low(iter);
							return exp(a);
						}});
	keywords.push_back({"sin", [](SmartIterator& iter){
							double a = operator_low(iter);
							return sin(a);
						}});
	keywords.push_back({"cos", [](SmartIterator& iter){
							double a = operator_low(iter);
							return cos(a);
						}});
	keywords.push_back({"ln", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(a <= 0) throw "argument of ln must be greater than 0";
							return log(a);
						}});
	keywords.push_back({"tan", [](SmartIterator& iter){
							double a = operator_low(iter);
							return tan(a);
						}});
	keywords.push_back({"asin", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(std::abs(a) > 1) throw "argument of asin must be from -1 to 1";
							return asin(a);
						}});
	keywords.push_back({"acos", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(std::abs(a) > 1) throw "argument of acos must be from -1 to 1";
							return acos(a);
						}});
	keywords.push_back({"atan", [](SmartIterator& iter){
							double a = operator_low(iter);
							return atan(a);
						}});
	keywords.push_back({"atan2", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							double b = operator_low(iter);
							return atan2(a, b);
						}});
	keywords.push_back({"sqrt", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(a < 0) throw "argument of sqrt mast be greater than 0";
							return sqrt(a);
						}});
	keywords.push_back({"pow", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							double b = operator_low(iter);
							return pow(a, b);
						}});
	keywords.push_back({"abs", [](SmartIterator& iter){
							double a = operator_low(iter);
							return a < 0 ? -a : a;
						}});
	keywords.push_back({"max", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							double b = operator_low(iter);
							return a > b ? a : b;
						}});
	keywords.push_back({"min", [](SmartIterator& iter){
							double a = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							double b = operator_low(iter);
							return a < b ? a : b;
						}});
	keywords.push_back({"sum", [](SmartIterator& iter){
							if(iter->type != Type::Parametr) throw "a parameter is expected";
							std::string param_name = iter->word;
							add_param({param_name, TypeParam::Blocked, 0});
							iter.go();
							if(iter->type != Type::Equal) throw "a equal is expected";
							iter.go();
							double beg = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							double end = operator_low(iter);
							if(iter->type != Type::Comma) throw "a comma is expected";
							iter.go();
							auto fixed = iter;
							unblock_param(param_name, beg);
							double sum = 0;
							operator_low(iter);
							while (beg <= end) {
								iter = fixed;
								sum += operator_low(iter);
								beg += 1;
								change_param(param_name, beg);
							}
							del_param(param_name);
							return sum;
						}});
}

void help_function(int argc, char* argv[]) {
	(void)argc;
	(void)argv;
	std::cout << "calc: built 08.01.2022\n";
	std::cout << "Author: Soshnikov Daniil (c)\n\n";

	std::cout << "Usage: calc [parametrs, expression]\n\n";

	std::cout << "Simple console calculator, supports multi-line mode. The expression can\n"
				 "be passed immediately as an argument to the calc command, some expressions\n"
				 "are supported without double quotes, but for some you need to use double quotes,\n"
				 "especially for those where there are brackets () or exponentiation ^.\n"
				 "There are also built-in constants pi, exp and infiniti.\n\n";

	std::cout << "Parametrs:\n";
	std::cout << "  --help \t Displays information about this program.\n";
	std::cout << "  --multiline \t Runs the program in multiline mode.\n";

	std::cout << "\nConstants:\n";
	std::cout << "  pi \t 3.14159265358979...\n";
	std::cout << "  e \t 2.71828182845905...\n";
	std::cout << "  inf \t double infinity.\n";

	std::cout << "\nFunctions:\n";
	std::cout << "  sin( expression )\n";
	std::cout << "  cos( expression )\n";
	std::cout << "  tan( expression ) \t\t sin( expression ) / cos( expression )\n";
	std::cout << "  exp( expression ) \t\t e^expression\n";
	std::cout << "  ln( expression ) \t\t Natural logarithm. The expression must be strictly \n"
				 "\t\t\t\t greater than zero.\n";
	std::cout << "  asin( expression ) \t\t arcsin. The expression must be from -1 to 1.\n";
	std::cout << "  acos( expression ) \t\t arccos. The expression must be from -1 to 1.\n";
	std::cout << "  atan( expression )\n";
	std::cout << "  atan2( expression_1, expression_2 ) \t Similarly to atan( expression_1 / expression_2 ).\n";
	std::cout << "  sqrt( expression ) \t\t Square root. The expression must be greater than zero.\n";
	std::cout << "  pow( expression_1, expression_2 ) \t Raises the expression_1 to the power of the\n"
				 "\t\t\t\t expression_2.\n";
	std::cout << "  abs( expression ) \t\t The module of the expression.\n";
	std::cout << "  min( expression_1, expression_2 ) \t A minimum of two expressions.\n";
	std::cout << "  max( expression_1, expression_2 ) \t A maximum of two expressions.\n";
	std::cout << "  sum( parameter = begining, end, expression(parameter) ) \t Calculates the sum of the\n"
				 "\t\t\t\t expression depending on the parameter from begining to\n"
				 "\t\t\t\t end in increments of 1\n\n";

	std::cout << "In multiline mode, you can use the\n"
				 "\"parameter = expression\"\n"
				 "to construct it to update the parameter value.\n\n";

	std::cout << "Examples:\n";
	std::cout << "  calc pi\n";
	std::cout << "  calc 3e - 8.5\n";
	std::cout << "  calc \"sum(n = min(x, 0), max(x, 0), 1/2^n)\"\n";
	std::cout << "  x = 5\n";
}
void multiline_mode() {
	std::string line;
	while (true) {
		std::cout << ">> ";
		std::getline(std::cin, line);
		if (line == "exit") break;
		try {
			double res = GET_RESULT(line);
			std::cout << "res = " << std::setprecision(15) << res << "\n";
		}  catch (const char* mes) {
			std::cerr << "Error: " << mes << "\n";
		}
	}
}
int main(int argc, char* argv[])
{
	if(argc == 1 || (argc > 1 && std::string(argv[1]) == "--help")) {
		help_function(argc, argv);
		return 0;
	}

	fill_default_parametrs();
	fill_keywords();
	if(argc == 2 && std::string(argv[1]) == "--multiline") {
		multiline_mode();
		return 0;
	}
	std::stringstream ss;
	for(int i = 1; i < argc; i++)
		ss << argv[i] << " ";
	std::string line = ss.str();
	try {
		std::cout << std::setprecision(15) << GET_RESULT(line) << "\n";
	}  catch (const char* mes) {
		std::cerr << "Error: " << mes << "\n";
		return 1;
	}
		return 0;
}
double GET_RESULT(const std::string& line) {
	std::vector<Lexema>* magic_vector;
	try {
		magic_vector = read_line(line.c_str());
		auto beg = magic_vector->begin();

		bool ch_param_flag = false;
		if(magic_vector->size() > 2 && (*magic_vector)[0].type == Type::Parametr &&
		   (*magic_vector)[1].type == Type::Equal) {
			add_param({(*magic_vector)[0].word, TypeParam::Blocked, 0});
			beg += 2;
			ch_param_flag = true;
		}

		SmartIterator si(beg, magic_vector->end());
		double res = operator_low(si);

		if(ch_param_flag)
			unblock_param((*magic_vector)[0].word, res);

		delete magic_vector;
		return res;
	}
	catch (...) {
		delete magic_vector;
		throw;
	}
}

double operator_high(SmartIterator &iter);
double operator_brakets(SmartIterator &iter) {
	if(iter->type == Type::Open) {
		iter.go();
		double a = operator_low(iter);
		if(iter->type != Type::Close) throw "the right bracket is expected";
		iter++;
		return a;
	}
	auto t = iter;
	iter++;
	if(t->type == Type::Number) return atof(t->word.c_str());
	if(t->type == Type::Parametr) return get_param(t->word);
	throw "incorrect input\n";
}
double operator_keyword(SmartIterator &iter) {
	if(iter->type == Type::KeyWord) {
		auto t = iter;
		iter.go();
		if(iter->type != Type::Open) throw "the left bracket is expected";
		iter.go();
		double a = 0;

		for(auto &x : keywords)
			if(x.name == t->word) {
				a = x.func(iter);
				break;
			}

		if(iter->type != Type::Close) throw "the right bracket is expected";
		++iter;
		return a;
	}
	return operator_brakets(iter);
}
double operator_mult(SmartIterator &iter) {
	if(iter->type == Type::Number) {
		auto t = iter;
		++iter;
		if(iter.is_end() || iter->type == Type::Operator || iter->type == Type::Close || iter->type == Type::Comma
		   || iter->type == Type::Equal)
			return atof(t->word.c_str());
		else {
			double b = operator_high(iter);
			return atof(t->word.c_str()) * b;
		}
	}
	return operator_keyword(iter);
}
double operator_pow(SmartIterator &iter) {
	double a = operator_mult(iter);
	if(!iter.is_end() && iter->type == Type::Operator && iter->word == "^") {
		iter.go();
		double f = operator_mult(iter);
		a = pow(a, f);
	}
	return a;
}
double operator_high(SmartIterator &iter) {
	double mul = operator_pow(iter);
	while(!iter.is_end() && iter->type == Type::Operator && (iter->word == "*" || iter->word == "/")) {
		bool mult = iter->word == "*";
		iter.go();
		double f = operator_pow(iter);
		if(mult) mul *= f;
		else {
			if(f == 0) throw "division by 0";
			mul /= f;
		}
	}
	return mul;
}
double operator_low(SmartIterator &iter) {
	double sum = operator_high(iter);
	while(!iter.is_end() && iter->type == Type::Operator && (iter->word == "+" || iter->word == "-")) {
		bool plus = iter->word == "+";
		iter.go();
		double f = operator_high(iter);
		if(plus) sum += f;
		else sum -= f;
	}
	return sum;
}

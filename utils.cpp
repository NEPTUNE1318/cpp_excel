#pragma once
#include "utils.h"

namespace MyExcel {
	Vector::Vector(int n) : data(new string[n]), capacity(n), length(0) {}

	void Vector::push_back(string s) {
		if (capacity <= length) {
			string* temp = new string[capacity * 2];
			for (int i = 0; i < length; i++) {
				temp[i] = data[i];
			}
			delete[] data;
			data = temp;
			capacity *= 2;
		}

		data[length] = s;
		length++;
	}

	string Vector::operator[](int i) { return data[i]; }

	void Vector::remove(int x) {
		for (int i = x + 1; i < length; i++) {
			data[i - 1] = data[i];
		}
		length--;
	}

	int Vector::size() { return length; }

	Vector::~Vector() {
		if (data) {
			delete[] data;
		}
	}

	Stack::Stack() : start(NULL, "") { current = &start; }

	void Stack::push(string s) {
		Node* n = new Node(current, s);
		current = n;
	}

	string Stack::pop() {
		if (current == &start) return "";

		string s = current->s;
		Node* prev = current;
		current = current->prev;

		delete prev;
		return s;
	}

	string Stack::peek() { return current->s; }

	bool Stack::is_empty() {
		if (current == &start) return true;
		return false;
	}

	Stack::~Stack() {
		while (current != &start) {
			Node* prev = current;
			current = current->prev;
			delete prev;
		}
	}

	NumStack::NumStack() : start(NULL, 0) { current = &start; }

	void NumStack::push(double s) {
		Node* n = new Node(current, s);
		current = n;
	}

	double NumStack::pop() {
		if (current == &start) return 0;

		double s = current->s;
		Node* prev = current;
		current = current->prev;

		delete prev;
		return s;
	}

	double NumStack::peek() { return current->s; }

	bool NumStack::is_empty() {
		if (current == &start) return true;
		return false;
	}

	NumStack::~NumStack() {
		while (current != &start) {
			Node* prev = current;
			current = current->prev;
			delete prev;
		}
	}

	Cell::Cell(int x, int y, Table* table)
		: data(data), x(x), y(y), table(table) {}

	StringCell::StringCell(string data, int x, int y, Table* t)
		: data(data), Cell(x, y, t) {}

	string StringCell::stringify() { return data; }
	int StringCell::to_numeric() { return 0; }

	NumberCell::NumberCell(int data, int x, int y, Table* t)
		: data(data), Cell(x, y, t) {}

	string NumberCell::stringify() { return to_string(data); }
	int NumberCell::to_numeric() { return data; }

	string DateCell::stringify() {
		char buf[50];
		tm temp;
		localtime_s(&temp, &data);

		strftime(buf, 50, "%F", &temp);
		return string(buf);
	}

	int DateCell::to_numeric() { return static_cast<int>(data); }

	DateCell::DateCell(string s, int x, int y, Table* t) : Cell(x, y, t) {
		int year = atoi(s.c_str());
		int month = atoi(s.c_str() + 5);
		int day = atoi(s.c_str() + 8);

		tm timeinfo;

		timeinfo.tm_year = year - 1900;
		timeinfo.tm_mon = month - 1;
		timeinfo.tm_mday = day;
		timeinfo.tm_hour = 0;
		timeinfo.tm_min = 0;
		timeinfo.tm_sec = 0;

		data = mktime(&timeinfo);
	}

	ExprCell::ExprCell(string data, int x, int y, Table* t) : data(data), Cell(x, y, t) {}

	int ExprCell::precedence(char c) {
		switch (c) {
		case '(':
		case '[':
		case '{':
			return 0;
		case '+':
		case '-':
			return 1;
		case '*':
		case '/':
			return 2;
		}
		return 0;
	}

	void ExprCell::parse_expression() {
		Stack stack;

		data.insert(0, "(");
		data.push_back(')');

		for (int i = 0; i < data.length(); i++) {
			if (isalpha(data[i])) {
				exp_vec.push_back(data.substr(i, 2));
				i++;
			}
			else if (isdigit(data[i])) {
				exp_vec.push_back(data.substr(i, 1));
			}
			else if (data[i] == '(' || data[i] == '[' || data[i] == '{') {
				stack.push(data.substr(i, 1));
			}
			else if (data[i] == ')' || data[i] == ']' || data[i] == '}') {
				string t = stack.pop();
				while (t != "(" && t != "[" && t != "{") {
					exp_vec.push_back(t);
					t = stack.pop();
				}
			}
			else if (data[i] == '+' || data[i] == '-' || data[i] == '*' || data[i] == '/') {
				while (!stack.is_empty() && precedence(stack.peek()[0])>=precedence(data[i])) {
					exp_vec.push_back(stack.pop());
				}
				stack.push(data.substr(i, 1));
			}
		}
	}

	int ExprCell::to_numeric() {
		parse_expression();

		double result = 0;
		NumStack stack;

		for (int i = 0; i < exp_vec.size(); i++) {
			string s = exp_vec[i];

			if (isalpha(s[0])) {
				stack.push(table->to_numeric(s));
			}
			else if (isdigit(s[0])) {
				stack.push(atoi(s.c_str()));
			}
			else {
				double y = stack.pop();
				double x = stack.pop();
				switch (s[0]) {
				case '+':
					stack.push(x + y);
					break;
				case '-':
					stack.push(x - y);
					break;
				case '*':
					stack.push(x * y);
					break;
				case '/':
					stack.push(x / y);
					break;
				}
			}
		}
		return stack.pop();
	}

	string ExprCell::stringify() { return to_string(to_numeric()); }

	Table::Table(int max_row_size, int max_col_size)
		: max_row_size(max_row_size), max_col_size(max_col_size) {
		data_table = new Cell * *[max_row_size];
		for (int i = 0; i < max_row_size; i++) {
			data_table[i] = new Cell * [max_col_size];
			for (int j = 0; j < max_col_size; j++) {
				data_table[i][j] = NULL;
			}
		}
	}

	Table::~Table() {
		for (int i = 0; i < max_row_size; i++) {
			for (int j = 0; j < max_col_size; j++) {
				if (data_table[i][j]) delete data_table[i][j];
			}
		}
		for (int i = 0; i < max_row_size; i++) {
			delete[] data_table[i];
		}
		delete[] data_table;
	}

	void Table::reg_cell(Cell* c, int row, int col) {
		if (!(row < max_row_size && col < max_col_size)) return;

		if (data_table[row][col]) {
			delete data_table[row][col];
		}
		data_table[row][col] = c;
	}

	int Table::to_numeric(const string& s) {
		int col = s[0] - 'A';
		int row = atoi(s.c_str() + 1) - 1;

		if (row < max_row_size && col < max_col_size) {
			if (data_table[row][col]) {
				return data_table[row][col]->to_numeric();
			}
		}
		return 0;
	}
	int Table::to_numeric(int row, int col) {
		if (row < max_row_size && col < max_col_size && data_table[row][col]) {
			return data_table[row][col]->to_numeric();
		}
		return 0;
	}
	string Table::stringify(const string& s) {
		int col = s[0] - 'A';
		int row = atoi(s.c_str() + 1) - 1;

		if (row < max_row_size && col < max_col_size) {
			if (data_table[row][col]) {
				return data_table[row][col]->stringify();
			}
		}
		return 0;
	}
	string Table::stringify(int row, int col) {
		if (row < max_row_size && col < max_col_size && data_table[row][col]) {
			return data_table[row][col]->stringify();
		}
		return "";
	}
	std::ostream& operator<<(std::ostream& o, Table& table) {
		o << table.print_table();
		return o;
	}

	TxtTable::TxtTable(int row, int col) : Table(row, col) {}

	string TxtTable::print_table() {
		string total_table;

		int* col_max_wide = new int[max_col_size];
		for (int i = 0; i < max_col_size; i++) {
			unsigned int max_wide = 2;
			for (int j = 0; j < max_row_size; j++) {
				if (data_table[j][i] &&
					data_table[j][i]->stringify().length() > max_wide) {
					max_wide = data_table[j][i]->stringify().length();
				}
			}
			col_max_wide[i] = max_wide;
		}
		total_table += "    ";
		int total_wide = 4;
		for (int i = 0; i < max_col_size; i++) {
			if (col_max_wide[i]) {
				int max_len = max(2, col_max_wide[i]);
				total_table += " | " + col_num_to_str(i);
				total_table += repeat_char(max_len - col_num_to_str(i).length(), ' ');

				total_wide += (max_len + 3);
			}
		}

		total_table += "\n";
		for (int i = 0; i < max_row_size; i++) {
			total_table += repeat_char(total_wide, '-');
			total_table += "\n" + to_string(i + 1);
			total_table += repeat_char(4 - to_string(i + 1).length(), ' ');

			for (int j = 0; j < max_col_size; j++) {
				if (col_max_wide[j]) {
					int max_len = max(2, col_max_wide[j]);

					string s = "";
					if (data_table[i][j]) {
						s = data_table[i][j]->stringify();
					}
					total_table += " | " + s;
					total_table += repeat_char(max_len - s.length(), ' ');
				}
			}
			total_table += "\n";
		}

		return total_table;
	}

	string TxtTable::repeat_char(int n, int c) {
		string s = "";
		for (int i = 0; i < n; i++) s.push_back(c);

		return s;
	}
	string TxtTable::col_num_to_str(int n) {
		string s = "";
		if (n < 26) {
			s.push_back('A' + n);
		}
		else {
			char first = 'A' + n / 26 - 1;
			char second = 'A' + n % 26;

			s.push_back(first);
			s.push_back(second);
		}

		return s;
	}

	HtmlTable::HtmlTable(int row, int col) : Table(row, col) {}

	string HtmlTable::print_table() {
		string s = "<table border='1' cellpadding='10'>";
		std::cout << s << std::endl;
		for (int i = 0; i < max_row_size; i++) {
			s += "<tr>";
			std::cout << s << std::endl;
			for (int j = 0; j < max_col_size; j++) {
				s += "<td>";
				std::cout << s << std::endl;
				if (data_table[i][j]) s += data_table[i][j]->stringify();
				s += "</td>";
				std::cout << s << std::endl;
			}
			s += "</tr>";
			std::cout << s << std::endl;
		}
		s += "</table>";
		std::cout << s << std::endl;
		return s;
	}

	CSVTable::CSVTable(int row, int col) : Table(row, col) {};

	string CSVTable::print_table() {
		string s = "";
		for (int i = 0; i < max_row_size; i++) {
			for (int j = 0; j < max_col_size; j++) {
				if (j >= 1) s += ",";
				string temp;
				if (data_table[i][j]) temp = data_table[i][j]->stringify();

				for (int k = 0; k < temp.length(); k++) {
					if (temp[k] == '"') {
						temp.insert(k, 1, '"');
						k++;
					}
				}
				temp = '"' + temp + '"';
				s += temp;
			}
			s += '\n';
		}
		return s;
	}

	Excel::Excel(int max_row, int max_col, int choice = 0) {
		switch (choice) {
		case 0:
			current_table = new TxtTable(max_row, max_col);
			break;

		case 1:
			current_table = new CSVTable(max_row, max_col);
			break;

		default:
			current_table = new HtmlTable(max_row, max_col);
			break;
		}  
	}

	int Excel::parse_user_input(string s) {
		int next = 0;
		string command = "";
		for (int i = 0; i < s.length(); i++) {
			if (s[i] == ' ') {

				next = i + 1;
				break;
			}
			else if (i == s.length() - 1) {
				command = s.substr(0, i + 1);
				next = i + 1;
				break;
			}
		}
		string to = "";
		for (int i = next; i < s.length(); i++) {
			if (s[i] == ' ' || i == s.length() - 1) {
				to = s.substr(next, i - next);
				next = i + 1;
				break;
			}
			else if (i == s.length() - 1) {
				to = s.substr(0, i + 1);
				next = i + 1;
				break;
			}
		}

		int col = to[0] - 'A';
		int row = atoi(to.c_str() + 1) - 1;

		string rest = s.substr(next);

		if (command == "SETS") {
			current_table->reg_cell(new StringCell(rest, row, col, current_table), row, col);
		}
		else if (command == "SETN") {
			current_table->reg_cell(new NumberCell(atoi(rest.c_str()), row, col, current_table), row, col);
		}
		else if (command == "SETD") {
			current_table->reg_cell(new DateCell(rest, row, col, current_table), row, col);
		}
		else if (command == "SETE") {
			current_table->reg_cell(new ExprCell(rest, row, col, current_table), row, col);
		}
		else if (command == "OUT") {
			ofstream out(to);
			out << current_table;
			std::cout << to << " 에 내용이 저장되었습니다." << std::endl;
		}
		else if (command == "EXIT") {
			return 0;
		}
		return 1;
	}

	void Excel::command_line() {
		string s;
		std::getline(cin, s);

		while (parse_user_input(s)) {
			std::cout << *current_table << std::endl << ">> ";
			getline(cin, s);
		}
	}
}
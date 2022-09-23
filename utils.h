#pragma once
#ifndef UTILS_H
#define UTILS_H

#include <string>
#include <algorithm>
#include <iostream>
#include <fstream>

using std::string;
using std::to_string;
using std::max;
using std::cin;
using std::ofstream;
using std::ifstream;

namespace MyExcel {
	class Vector {
		string* data;
		int capacity;
		int length;

	public:
		Vector(int n=1);

		void push_back(string s);
		string operator[](int i);
		void remove(int x);
		int size();

		~Vector();
	};

	class Stack {
		struct Node {
			Node* prev;
			string s;

			Node(Node* prev, string s) : prev(prev), s(s) {}
		};

		Node* current;
		Node start;

	public:
		Stack();

		void push(string s);
		string pop();
		string peek();
		bool is_empty();

		~Stack();
	};

	class NumStack {
		struct Node {
			Node* prev;
			double s;

			Node(Node* prev, double s) : prev(prev), s(s) {}
		};

		Node* current;
		Node start;

	public:
		NumStack();

		void push(double s);
		double pop();
		double peek();
		bool is_empty();

		~NumStack();
	};

	class Table;

	class Cell {
	protected:
		int x, y;
		Table* table;

		string data;

	public:
		Cell(int x, int y, Table* table);

		virtual string stringify() = 0;
		virtual int to_numeric() = 0;
	};

	class StringCell : public Cell {
		string data;

	public:
		string stringify();
		int to_numeric();

		StringCell(string data, int x, int y, Table* t);
	};

	class NumberCell : public Cell {
		int data;

	public:
		string stringify();
		int to_numeric();

		NumberCell(int data, int x, int y, Table* t);
	};

	class DateCell : public Cell {
		time_t data;
		
	public:
		string stringify();
		int to_numeric();

		DateCell(string s, int x, int y, Table* t);
	};

	class ExprCell : public Cell {
		string data;
		string* parsed_expr;

		Vector exp_vec;

		int precedence(char c);
		void parse_expression();

	public:
		ExprCell(string data, int x, int y, Table* t);

		string stringify();
		int to_numeric();
	};

	class Table {
	protected:
		int max_row_size, max_col_size;
		Cell*** data_table;

	public:
		Table(int max_row_size, int max_col_size);
		~Table();

		void reg_cell(Cell* c, int row, int col);

		int to_numeric(const string& s);
		int to_numeric(int row, int col);

		string stringify(const string& s);
		string stringify(int row, int col);

		virtual string print_table() = 0;
	};

	class TxtTable : public Table {
		string repeat_char(int n, int c);
		string col_num_to_str(int n);

	public:
		TxtTable(int row, int col);
		string print_table();
	};

	class HtmlTable : public Table {
	public:
		HtmlTable(int row, int col);
		string print_table();
	};

	class CSVTable : public Table {
	public:
		CSVTable(int row, int col);
		string print_table();
	};

	class Excel {
		Table* current_table;


	public:
		Excel(int max_row, int max_col, int choice);

		int parse_user_input(string s);
		void command_line();
	};
}

#endif
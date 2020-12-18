#include <fstream>
#include <iostream>
#include <common/standard.h>
#include <boolean/unsigned_int.h>
#include <boolean/signed_int.h>
#include <interpret_boolean/export.h>
#include <interpret_boolean/import.h>
#include <parse_expression/expression.h>

using namespace boolean;

void print_xfactor(string name, unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression_xfactor(n.bits[i], vars);
		cout << name << "[" << i << "]:\t" << result.to_string() << endl;
	}
	cout << endl;
}

void print_hfactor(string name, unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression_hfactor(n.bits[i], vars);
		cout << name << "[" << i << "]:\t" << result.to_string() << endl;
	}
	cout << endl;
}

void print(string name, unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression(n.bits[i], vars);
		cout << name << "[" << i << "]:\t" << result.to_string() << endl;
	}
	cout << endl;
}

void print_help()
{
	printf("Usage: gated [options] file...\n");
	printf("A self-contained logic synthesis engine.\n");
	printf("\nGeneral Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf("\nConversion Options:\n");
	printf(" -i <inputs>      Comma separated list of variable definitions <name>:<type>. Type is 'u' for unsigned int, 'i' for signed int, or 'f' for float followed by the bitwidth. For example: \"a:u8,b:i4,x:u5\"\n");
	printf(" -o <outputs>     Comma separated list of expressions. For example: \"r0=a+b,r1=b<<x\"\n");
}

void print_version()
{
	printf("gated 1.0.0\n");
	printf("Copyright (C) 2020 Cornell University.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}


int main(int argc, char **argv)
{
	ucs::variable_set vars;
	map<string, boolean::unsigned_int> u;
	map<string, boolean::signed_int> i;
	string input, output;

	tokenizer parser(false);
	parse_expression::expression::register_syntax(parser);

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg == "--help" || arg == "-h")			// Help
		{
			print_help();
			return 0;
		}
		else if (arg == "--version")	// Version Information
		{
			print_version();
			return 0;
		}
		else if (arg == "-i")
		{
			i++;
			if (i < argc) {
				if (input.size() > 0)
					input += ",";
				input += argv[i];
			} else {
				error("", "expected input specification", __FILE__, __LINE__);
				return 1;
			}
		}
		else if (arg == "-o")
		{
			i++;
			if (i < argc) {
				if (output.size() > 0)
					output += ",";
				output += argv[i];
			} else {
				error("", "expected output specification", __FILE__, __LINE__);
				return 1;
			}
		}
		else if (arg == "-f")
		{
			i++;
			if (i < argc) {
				ifstream f;
				f.open(argv[i], std::ios::in);
				string line;
				while (getline(f, line)) {
					if (line.size() > 3 and strncmp(line.c_str(), ".i ", 3) == 0) {
						if (input.size() > 0) {
							input += ",";
						}
						input += line.substr(3);
					} else if (line.size() > 3 and strncmp(line.c_str(), ".o ", 3) == 0) {
						if (output.size() > 0) {
							output += ",";
						}
						output += line.substr(3);
					}
				}

				f.close();
			} else {
				error("", "expected specification file", __FILE__, __LINE__);
				return 1;
			}
		}
	}

	int start = 0, end = 0;
	do {
		end = input.find(":", start);
		string name = input.substr(start, end - start);
		start = end+1;
		end = input.find(",", start);
		string type = input.substr(start, end - start);
		int width = atoi(type.c_str()+1);
		int offset = vars.nodes.size();
		for (int j = 0; j < width; j++)
		{
			int index = vars.define(ucs::variable());
			vars.nodes[index].name.push_back(ucs::instance(name, {j}));
		}
		if (type[0] == 'u') {
			u.insert(pair<string, boolean::unsigned_int>(name, boolean::unsigned_int(width, offset)));
		} else if (type[0] == 'i') {
			i.insert(pair<string, boolean::signed_int>(name, boolean::signed_int(width, offset)));
		} else {
			error("", string("unsupported data type '") + type[0] + "'", __FILE__, __LINE__);
		}
		start = end+1;
	} while (start > 0);

	map<string, boolean::bitset> result;

	start = 0;
	end = 0;
	while (end != string::npos) {
		end = output.find("=", start);
		string name = output.substr(start, end - start);
		start = end+1;
		end = output.find(",", start);
		string exp_str = output.substr(start, end - start);
		start = end+1;
		parser.insert("", exp_str);
		parse_expression::expression exp(parser);
		if (parser.is_clean()) {
			result.insert(pair<string, boolean::bitset>(name, import_unsigned_int(exp, u, 0, &parser)));
		}
		parser.reset();
	}

	/*boolean::unsigned_int p =
		((b == 1) & (a + y)) |
		((b == 2) & ((a<<1) + y)) |
		((b == 3) & ((a<<2) + (~a) + y)) |
		((b == 4) & ((a<<2) + y)) |
		((b == 5) & ((a<<2) + a + y)) |
		((b == 6) & ((a<<3) + (~a<<1) + y)) |
		((b == 7) & ((a<<3) + (~a) + y)) | 
		((b == 8) & ((a<<3) + y));
	p.espresso();*/

	bitset total;
	
	for (map<string, boolean::bitset>::iterator i = result.begin(); i != result.end(); i++) {
		i->second.espresso();
		total.append(i->second);
		print_xfactor(i->first, i->second, vars);
	}

	map<cube, int> factors;
	vector<int> hide;// = {0, 1, 2, 3, 8, 9, 10, 11, 12};

	int sz = 0;
	do {
		sz = (int)factors.size();
		total = total.decompose_hfactor(factors, 2, vars.nodes.size(), hide);
	} while (factors.size() > sz);

	for (int j = 0; j < (int)factors.size(); j++)
	{
		int index = vars.define(ucs::variable());
		vars.nodes[index].name.push_back(ucs::instance("f", {j}));
	}
	
	for (map<cube, int>::iterator i = factors.begin(); i != factors.end(); i++)
	{
		parse_expression::expression result = export_expression(i->first, vars);
		cout << vars.nodes[i->second].to_string() << ":\t" << result.to_string() << endl;
	}
	cout << endl;

	int index = 0;
	for (map<string, boolean::bitset>::iterator i = result.begin(); i != result.end(); i++) {
		print_xfactor(i->first, total.subset(index, i->second.bits.size()), vars);
		index += i->second.bits.size();
	}
}


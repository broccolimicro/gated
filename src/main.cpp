#include <common/standard.h>
#include <boolean/number.h>
#include <interpret_boolean/export.h>
#include <synthesize/boolean.h>

using namespace boolean;

void print_xfactor(unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression_xfactor(n.bits[i], vars);
		cout << "p" << i << ":\t" << result.to_string() << endl;
	}
	cout << endl;
}


void print_hfactor(unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression_hfactor(n.bits[i], vars);
		cout << "p" << i << ":\t" << result.to_string() << endl;
	}
	cout << endl;
}

void print(unsigned_int n, ucs::variable_set vars)
{
	for (int i = 0; i < n.bits.size(); i++)
	{
		parse_expression::expression result = export_expression(n.bits[i], vars);
		cout << "p" << i << ":\t" << result.to_string() << endl;
	}
	cout << endl;
}

void print_help()
{
	printf("Usage: gated [options] file...\n");
	printf("A logic minimization engine.\n");
	printf("\nGeneral Options:\n");
	printf(" -h,--help      Display this information\n");
	printf("    --version   Display version information\n");
	printf(" -v,--verbose   Display verbose messages\n");
	printf(" -d,--debug     Display internal debugging messages\n");
	printf("\nConversion Options:\n");
	printf(" -g <file>      Convert this HSE to an hse-graph and save it to a file\n");
	printf(" -eg <file>     Convert this HSE to an elaborated hse-graph and save it to a file\n");
	printf(" -pn <file>     Convert this HSE to a petri-net and save it to a file\n");
	printf(" -sg <file>     Convert this HSE to a state-graph and save it to a file\n");
}

void print_version()
{
	printf("hsesim 1.0.0\n");
	printf("Copyright (C) 2013 Sol Union.\n");
	printf("There is NO warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n");
	printf("\n");
}


int main()
{
	ucs::variable_set vars;
	for (int i = 0; i < 4; i++)
	{
		int index = vars.define(ucs::variable());
		vars.nodes[index].name.push_back(ucs::instance("a", {i}));
	}
	for (int i = 0; i < 4; i++)
	{
		int index = vars.define(ucs::variable());
		vars.nodes[index].name.push_back(ucs::instance("b", {i}));
	}
	for (int i = 0; i < 5; i++)
	{
		int index = vars.define(ucs::variable());
		vars.nodes[index].name.push_back(ucs::instance("y", {i}));
	}

	boolean::unsigned_int a(4, 0);
	boolean::unsigned_int b(4, 4);
	boolean::unsigned_int y(5, 8);
	boolean::unsigned_int p =
		((b == 1) & (a + y))/* |
		((b == 2) & ((a<<1) + y)) |
		((b == 3) & ((a<<2) + (~a) + y)) |
		((b == 4) & ((a<<2) + y)) |
		((b == 5) & ((a<<2) + a + y)) |
		((b == 6) & ((a<<3) + (~a<<1) + y)) |
		((b == 7) & ((a<<3) + (~a) + y)) | 
		((b == 8) & ((a<<3) + y))*/;
	p.simplify();
	print_hfactor(p, vars);
	printf("\n\n");
	
	vector<int> hide;// = {0, 1, 2, 3, 8, 9, 10, 11, 12};
	map<cube, int> factors;
	int sz = 0;
	
	do {
		sz = (int)factors.size();
		p = decompose_xfactor(p, 2, factors, vars, hide);
	} while (factors.size() > sz);
	
	for (map<cube, int>::iterator i = factors.begin(); i != factors.end(); i++)
	{
		parse_expression::expression result = export_expression(i->first, vars);
		cout << vars.nodes[i->second].to_string() << ":\t" << result.to_string() << endl << endl;
	}

	print_xfactor(p, vars);
}


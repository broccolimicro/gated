#include <common/standard.h>
#include <boolean/number.h>
#include <interpret_boolean/export.h>

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


unsigned_int decompose_hfactor(unsigned_int c, int w, map<cube, int> &factors, ucs::variable_set &vars, vector<int> hide)
{
	fflush(stdout);
	if (c.max_width() >= w and c.depth() > 1) {
		boolean::cube common = c.supercube();
		common.hide(hide);
		if (common.width() < w)
		{
			boolean::unsigned_int c_left, c_right;
			boolean::unsigned_int left_result, right_result;
			unsigned long cubes = 0;
			for (int i = 0; i < (int)c.bits.size(); i++) {
				cubes += c.bits[i].cubes.size();
			}
			float c_weight = c.partition(c_left, c_right);

			left_result = decompose_hfactor(c_left, w, factors, vars, hide);
			right_result = decompose_hfactor(c_right, w, factors, vars, hide);
			return left_result | right_result;
		}
		else
		{
			c.cofactor(common);
			int index = -1;
			map<cube, int>::iterator loc = factors.find(common);
			if (loc == factors.end()) {
				index = vars.define(ucs::variable());
				vars.nodes[index].name.push_back(ucs::instance("f", {(int)factors.size()}));
				factors.insert(pair<cube, int>(common, index));
			} else {
				index = loc->second;
			}

			unsigned_int result = decompose_hfactor(c, w, factors, vars, hide);
			for (int i = 0; i < (int)result.bits.size(); i++) {
				result.bits[i] &= boolean::cube(index, 1);
			}
			return result;
		}
	}

	return c;
}

unsigned_int decompose_xfactor(unsigned_int c, int w, map<cube, int> &factors, ucs::variable_set &vars, vector<int> hide)
{
	if (c.max_width() >= w and c.depth() > 1) {
		unsigned_int nc = ~c;
		boolean::cube common = c.supercube();
		boolean::cube ncommon = nc.supercube();
		common.hide(hide);
		ncommon.hide(hide);
		int cw = common.width(), ncw = ncommon.width();

		if (cw < w and ncw < w) {
			unsigned_int c_left, c_right, nc_left, nc_right;
			unsigned_int result, left_result, right_result;
			float c_weight, nc_weight;
			
			unsigned long cubes = 0;
			for (int i = 0; i < (int)c.bits.size(); i++) {
				cubes += c.bits[i].cubes.size();
			}

			c_weight = c.partition(c_left, c_right);

			cubes = 0;
			for (int i = 0; i < (int)nc.bits.size(); i++) {
				cubes += nc.bits[i].cubes.size();
			}

			nc_weight = nc.partition(nc_left, nc_right);

			if (c_weight <= nc_weight)
			{
				left_result = decompose_xfactor(c_left, w, factors, vars, hide);
				right_result = decompose_xfactor(c_right, w, factors, vars, hide);
				result = left_result | right_result;
			}
			else if (nc_weight < c_weight)
			{
				left_result = decompose_xfactor(nc_left, w, factors, vars, hide);
				right_result = decompose_xfactor(nc_right, w, factors, vars, hide);
				result = left_result | right_result;
				//printf("here\n");
				// We're getting stuck here...
				result = ~result;
				//printf("done\n");
			}
			return result;
		} else if (cw >= ncw) {
			c.cofactor(common);

			map<cube, int>::iterator loc = factors.find(common);
			int index = -1;
			if (loc == factors.end()) {
				index = vars.define(ucs::variable());
				vars.nodes[index].name.push_back(ucs::instance("f", {(int)factors.size()}));
				factors.insert(pair<cube, int>(common, index));
			} else {
				index = loc->second;
			}
		
			unsigned_int result = decompose_xfactor(c, w, factors, vars, hide);
			for (int i = 0; i < (int)result.bits.size(); i++) {
				result.bits[i] &= boolean::cube(index, 1);
			}
			return result;
		} else if (ncw > cw) {
			nc.cofactor(ncommon);

			map<cube, int>::iterator loc = factors.find(ncommon);
			int index = -1;
			if (loc == factors.end()) {
				index = vars.define(ucs::variable());
				vars.nodes[index].name.push_back(ucs::instance("f", {(int)factors.size()}));
				factors.insert(pair<cube, int>(ncommon, index));
			} else {
				index = loc->second;
			}
			
			unsigned_int result = decompose_xfactor(nc, w, factors, vars, hide);
			for (int i = 0; i < (int)result.bits.size(); i++) {
				result.bits[i] &= boolean::cube(index, 1);
			}
			return ~result;
		}
	}

	return c;
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


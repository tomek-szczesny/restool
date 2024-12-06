#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "restool.h"

// "res" struct for manipulating resistances in standard series.
//
// "res" struct represents a resistor in one of predefined series.
// It's easier to iterate through series this way.
// Otherwise, resistances are handled as float variables.
//
// "res" struct can be created by one of the three functions:
// - res_round
// - res_ceil
// - res_floor
// Each will differently treat the arbitrary input float.
//
// "res" structs can be iterated through using the following:
// - res_inc
// - res_dec
//
// "res" can be retrieved as a numerical value or printed to the terminal:
// - res_f
// - res_print.
//
// Some functions are implemented with prefix "r", with float result instead.
//

struct res res_round(float in, struct vss * series) {
	if (in < 0) {
		printf("WTF, input resistance < 0!\n");
		exit(-1);
	}

	struct res ret;
	ret.series = series;
	if (in == 0) {
		ret.z = 1;
		return ret;
	}
	ret.e = (int)(log10(in));
	if (in < 1) ret.e--;
	in /= pow(10, ret.e);

	int i;
	for (i=0; i<=series->n; i=i+1) {
		if (series->vals[i] >= in) {
			ret.n = i;
			ret.n += (in/series->vals[i-1] > (series->vals[i]/in)) ? 0 : -1;
			if (ret.n == series->n) {
				ret.n = 0;
				ret.e += 1;
			}
			return ret;
		}
	}
	printf("Eh? Something went wrong in res_round.\n");
	exit(-1);
}

struct res res_ceil(float in, struct vss * series) {
	if (in < 0) {
		printf("WTF, input resistance < 0!\n");
		exit(-1);
	}

	struct res ret;
	ret.series = series;
	if (in == 0) {
		ret.z = 1;
		return ret;
	}
	ret.e = (int)(log10(in));
	if (in < 1) ret.e--;
	in /= pow(10, ret.e);

	int i;
	for (i=0; i<=series->n; i=i+1) {
		if (series->vals[i] >= in) {
			ret.n = i;
			if (ret.n == series->n) {
				ret.n = 0;
				ret.e += 1;
			}
			return ret;
		}
	}
	printf("Eh? Something went wrong in res_ceil.\n");
	exit(-1);
}

struct res res_floor(float in, struct vss * series) {
	if (in < 0) {
		printf("WTF, input resistance < 0!\n");
		exit(-1);
	}

	struct res ret;
	ret.series = series;
	if (in == 0) {
		ret.z = 1;
		return ret;
	}
	ret.e = (int)(log10(in));
	if (in < 1) ret.e--;
	in /= pow(10, ret.e);

	int i;
	for (i=0; i<=series->n; i=i+1) {
		if (series->vals[i] <  in) continue;
		if (series->vals[i] == in) ret.n = i;
		if (series->vals[i] >  in) ret.n = i-1;
		if (ret.n == series->n) {
			ret.n = 0;
			ret.e += 1;
		}
		return ret;
	}
	printf("Eh? Something went wrong in res_floor.\n");
	exit(-1);
}

void res_inc (struct res * r) {
	r->n++;
	if (r->n >= r->series->n)
	{
		r->n = 0;
		r->e++;
	}
}

void res_dec (struct res * r) {
	r->n--;
	if (r->n < 0)
	{
		r->n = r->series->n-1;
		r->e--;
	}
}

float res_f(struct res * r) {
	float ret;
	ret = (r->series)->vals[r->n];
	ret *= pow(10,r->e);
	return ret;
}

void r_print(float in) {
	if (in >= 1e9) {
		printf("%gG", in/1e9);
		return;
	}
	if (in >= 1e6) {
		printf("%gM", in/1e6);
		return;
	}
	if (in >= 1e3) {
		printf("%gk", in/1e3);
		return;
	}
	if (in >= 1) {
		printf("%g", in);
		return;
	}
	if (in >= 1e-3) {
		printf("%gm", in*1e3);
		return;
	}
	if (in >= 1e-6) {
		printf("%gu", in*1e6);
		return;
	}
	printf("%gn", in*1e9);
	return;
}

void res_print(struct res * r) {
	r_print(res_f(r));
}

// returns negative number if it found any problems
float r_get(char * in) {
	// Attempts to catch values such as:
	// 12.34, 12.34k, 12k34
	float f;	// 12(.34)
	char s;		// k
	float i;		// 34
	int ssf = sscanf(in, "%f%c%f", &f, &s, &i);
	if (ssf == 0) return -1;
	if (ssf == 1) return f;
	if (ssf == 3) {
		if (f != floor(f)) return -1; // 12.34k56 ??
		f += i / pow(10,(floor(log10(i))+1));
	}
	if (ssf >= 2) {
		switch (s) {
			case 'n': f*=1e-9; break;
			case 'u': f*=1e-6; break;
			case 'm': f*=1e-3; break;
			case 'k': f*=1e3; break;
			case 'M': f*=1e6; break;
			case 'G': f*=1e9; break;
			default: return -1;
		}
		return f;
	}
	return -1;

}

float e_get(char * in) {
	// Attempts to catch values such as:
	// 0.03, 0.1%, 1%
	float f;
	char s;
	int ssf = sscanf(in, "%f%c", &f, &s);
	if (ssf == 0) return -1;
	if (ssf == 1) return f;
	if (ssf == 2) {
		if (s == '%') f /= 100;
		return f;
	}
	return -1;

}

//----------------------------------------------------------------------------//

// Finds the best match of two parallel resistors
void find_par(float r, struct vss * ser, struct res * r1, struct res * r2, float * et)
{
	float e;
	struct res tr1, tr2;
	float be = 1;

	tr1 = res_ceil(r, ser);
	if (res_f(&tr1) == r) {
		res_inc(&tr1);	// In case the value is already exact, we can explore others
		printf(bold);
		printf("The input value is present in the selected series of values.\n");
		printf("Attempting to find combinations without 0R or open circuit.\n\n");
		printf(normal);
	}
	tr2 = res_round(res_f(&tr1)*r/(res_f(&tr1)-r), ser);
	*r1 = tr1; *r2 = tr2;
	while (res_f(&tr1) <= res_f(&tr2)) {
		e = res_f(&tr1)*res_f(&tr2)/(res_f(&tr1)+res_f(&tr2));
		e /= r;
		e -= 1;
		e = fabs(e);
		if (e <= *et) {
			res_print(&tr1); printf(" || "); res_print(&tr2);
			printf("\tError: %.2g%%\n", e*100);
		}
		if (e < be) {
			*r1 = tr1; *r2 = tr2; be = e;
		}
		res_inc(&tr1);
		tr2 = res_round(res_f(&tr1)*r/(res_f(&tr1)-r), ser);
	}
	printf(bold);
	printf("The best parallel resistor combination:\n");
	res_print(r1); printf(" || "); res_print(r2);
	printf("\tError: %.2g%%\n", be*100);
	printf(normal);
	return;
}

// Finds the best match of two series resistors
void find_ser(float r, struct vss * ser, struct res * r1, struct res * r2, float * et)
{
	float e;
	struct res tr1, tr2;
	float be = 1;

	tr1 = res_floor(r, ser);
	if (res_f(&tr1) == r) res_dec(&tr1);	// In case the value is already exact, we can explore others
	tr2 = res_round(r-res_f(&tr1), ser);
	*r1 = tr1; *r2 = tr2;
	while (res_f(&tr1) >= res_f(&tr2)) {
		e = res_f(&tr1)+res_f(&tr2);
		e /= r;
		e -= 1;
		e = fabs(e);
		if (e <= *et) {
			res_print(&tr1); printf(" + "); res_print(&tr2);
			printf("\tError: %.2g%%\n", e*100);
		}
		if (e < be) {
			*r1 = tr1; *r2 = tr2; be = e;
		}
		res_dec(&tr1);
		tr2 = res_round(r-res_f(&tr1), ser);
	}
	printf(bold);
	printf("The best series resistor combination:\n");
	res_print(r1); printf(" + "); res_print(r2);
	printf("\tError: %.2g%%\n", be*100);
	printf(normal);
	return;
}

// Finds the best match of two resistors with a specified ratio
void find_ratio(float r, struct vss * ser, struct res * r1, struct res * r2, float * et)
{
	float e;
	struct res tr1, tr2;
	float be = r;

	tr1 = res_round(1000*r, ser);
	tr2 = res_round(1000, ser);
	*r1 = tr1; *r2 = tr2;
	while (res_f(&tr2) <= 10000) {
		e = res_f(&tr1)/res_f(&tr2);
		e /= r;
		e -= 1;
		e = fabs(e);
		if (e <= *et) {
			res_print(&tr1); printf(" : "); res_print(&tr2);
			printf("\tError: %.2g%%\n", e*100);
		}
		if (e < be) {
			*r1 = tr1; *r2 = tr2; be = e;
		}
		res_inc(&tr2);
		tr1 = res_round(res_f(&tr2)*r, ser);
	}
	printf(bold);
	printf("The closest ratio found:\n");
	res_print(r1); printf(" : "); res_print(r2);
	printf("\tError: %.2g%%\n", be*100);
	printf(normal);
	return;
}

// Finds the best match of n resistors with specified n weights
void find_weights(int n, float * r, struct vss * ser, float * et)
{
	float e;		// Current error
	struct res * trs = malloc(n*sizeof(struct res));	// resistors
	struct res * ans = malloc(n*sizeof(struct res));	// resistors with the best solution
	float be = 1e6;		// best (lowest) error so far
	float minr, maxr;	// Minimum and maximum trs/r ratio
	int minp;		// The position of minr
	float lw = r[0];	// The lowest weight
	float lv = 1e3;		// The lowest value

	int i = 0; // for loops

	// Finding the lowest weight for normalization
	i = 0;	
	while (i < n) {
		if (r[i] < lw) lw = r[i];
		i++;
	}

	// Setting up the first guess with normalized weighs
	i = 0;
	while (i < n) {
		trs[i] = res_round(1e3*r[i]/lw, ser);
		i++;
	}
	memcpy(ans, trs, n*sizeof(struct res));
	
	// Iterating through other possibilities
	// by incrementing resistors with the lowest
	// value / weight ratio,
	// until the lowest value is >= 10k.
	while (lv < 10e3) {
		// Find minimum and maximum trs/r
		// and the lowest value
		i = 0;
		while (i < n) {
			float tx = res_f(&trs[i]);
			if (i == 0) {
				lv = tx;
				minr = tx/r[i];
				maxr = tx/r[i];
				minp = 0;
			}
			else {
				if (tx < lv) lv = tx;
				if (tx/r[i] < minr) {
					minr = tx/r[i];
					minp = i;
				}
				if (tx/r[i] > maxr) maxr = tx/r[i];
			}
			i++;
		}
		
		// Calculate error, print the result if relevant
		e = (maxr/minr) - 1;
		if (e <= *et) {
			i = 0;
			while (i < n) {
				res_print(&(trs[i])); if (i < (n-1)) printf(" : ");
				i++;
			}
			printf("\tError: %.2g%%\n", e*100);
		}

		// Store the result if it's the best so far
		if (e < be) {
			be = e;
			memcpy(ans, trs, n*sizeof(struct res));
		}

		// Advance the proper resistor
		res_inc(&trs[minp]);
	}

	// Print the best solution
	printf(bold);
	printf("The closest match found:\n");
	i = 0;
	while (i < n) {
		res_print(&(ans[i])); if (i < (n-1)) printf(" : ");
		i++;
	}
	printf("\tError: %.2g%%\n", be*100);
	printf(normal);

	free(trs);
	free(ans);
	return;
}

// Finds the best match of n resistors to form a divider for voltages given in "v".
// The first voltage is treated as input voltage.
void find_divider(int n, float * v, struct vss * ser, float * et)
{
	float e;		// Current error
	struct res * trs = malloc(n*sizeof(struct res));	// resistors
	struct res * ans = malloc(n*sizeof(struct res));	// resistors with the best solution
	float * r = malloc(n*sizeof(float));			// Resistor ratios
	float be = 1e6;		// best (lowest) error so far
	float minr, maxr;	// Minimum and maximum trs/r ratio
	int minp;		// The position of minr
	float lw = r[0];	// The lowest weight
	float lv = 1e3;		// The lowest value

	int i = 0; // for loops

	// Finding the lowest weight for normalization
	i = 0;	
	while (i < n) {
		if (r[i] < lw) lw = r[i];
		i++;
	}

	// Setting up the first guess with normalized weighs
	i = 0;
	while (i < n) {
		trs[i] = res_round(1e3*r[i]/lw, ser);
		i++;
	}
	memcpy(ans, trs, n*sizeof(struct res));
	
	// Iterating through other possibilities
	// by incrementing resistors with the lowest
	// value / weight ratio,
	// until the lowest value is >= 10k.
	while (lv < 10e3) {
		// Find minimum and maximum trs/r
		// and the lowest value
		i = 0;
		while (i < n) {
			float tx = res_f(&trs[i]);
			if (i == 0) {
				lv = tx;
				minr = tx/r[i];
				maxr = tx/r[i];
				minp = 0;
			}
			else {
				if (tx < lv) lv = tx;
				if (tx/r[i] < minr) {
					minr = tx/r[i];
					minp = i;
				}
				if (tx/r[i] > maxr) maxr = tx/r[i];
			}
			i++;
		}
		
		// Calculate error, print the result if relevant
		e = (maxr/minr) - 1;
		if (e <= *et) {
			i = 0;
			while (i < n) {
				res_print(&(trs[i])); if (i < (n-1)) printf(" : ");
				i++;
			}
			printf("\tError: %.2g%%\n", e*100);
		}

		// Store the result if it's the best so far
		if (e < be) {
			be = e;
			memcpy(ans, trs, n*sizeof(struct res));
		}

		// Advance the proper resistor
		res_inc(&trs[minp]);
	}

	// Print the best solution
	printf(bold);
	printf("The closest match found:\n");
	i = 0;
	while (i < n) {
		res_print(&(ans[i])); if (i < (n-1)) printf(" : ");
		i++;
	}
	printf("\tError: %.2g%%\n", be*100);
	printf(normal);

	free(trs);
	free(ans);
	free(r);
	return;
}

//----------------------------------------------------------------------------//

void usage()
{
	printf("Resistor Tool\n");
	printf("by Tomek Szczęsny 2024\n\n");
	printf("Usage:\n");
	printf("restool [options] [function] num0 [num1] [num2] ...\n");
	printf("\n");
	printf("Options:\n");
	printf("\t-e6, -e12,\tSeries of values to be used for elaboration.\n");
	printf("\t-e24, -e48,\tOnly one can be selected.\n");
	printf("\t-e96, -e192\tE24 is the default.\n");
	printf("\n");
	printf("\t-e [error]\tThe maximum relative error of the result. A value in range [0-0.1].\n");
	printf("\t\t\tAll solutions below the specified error will be printed.\n");
	printf("\t\t\tThe best solution found is always printed.\n");
	printf("\t\t\tThe default value is 0 (only the best answer is printed).\n");
	printf("\n");
	printf("Functions:\n");
	printf("\tOnly one function can be selected.");
	printf("\n");
	printf("\t-c\tFinds a combination of resistors approximating the given value.\n");
	printf("\t\tOnly one numerical argument is anticipated.\n");
	printf("\t\tThis is the default function.\n");
	printf("\n");
	printf("\t-r\tLooks for a set of resistors satisfying a given ratio between them.\n");
	printf("\t\tIf one numerical argument is given, the tool will look for a ratio num0:1.\n");
	printf("\t\tIf more values are provided, the tool will interpret them as weighs of\n");
	printf("\t\teach resistor in the set.\n");
	printf("\t\tThe error is defined as the product of the maximum and minimum resistance/weight ratio, minus one.\n");
	printf("\n");
/*	printf("\t-v\tSimilar to -r, but the arguments are treated as voltages of a resistive divider.\n");
	printf("\t\tAt least two numerical values must be provided.\n");
	printf("\t\tThe first voltage on the list is treated as the input voltage of the divider.\n");
	printf("\t\tThe other values are treated as one or many outputs of the divider.\n");
	printf("\t\tAll voltages must be given in decreasing order..\n");
*/	printf("\n");
	printf("Examples:\n");
	printf("\tFind the best approximation of 12.34k resistance in E24 series:\n");
	printf("\trestool -e24 12.34k\n");
	printf("\n");
	printf("\tThe same as above, but show all results with error <1%% :\n");
	printf("\trestool -e24 -e 0.01 12.34k\n");
	printf("\n");
	printf("\tFind the best set of resistors to build a 5-bit DAC resistor ladder:\n");
	printf("\trestool -r 1 2 4 8 16\n");
	exit(0);
}

int main(int argc, char **argv)
{
	// Default values, to be overwritten by the user provided options
	struct vss * ser_d = &e24;
	float e_d = 0;
	enum func func_d = F_COMB;

	if (argc == 1) usage();

	int i = 0;
	while (i < (argc-1)) {
		i++;
		if (!strcmp(argv[i], "-e6")) {ser_d = &e6; continue;}
		if (!strcmp(argv[i], "-e12")) {ser_d = &e12; continue;}
		if (!strcmp(argv[i], "-e24")) {ser_d = &e24; continue;}
		if (!strcmp(argv[i], "-e48")) {ser_d = &e48; continue;}
		if (!strcmp(argv[i], "-e96")) {ser_d = &e96; continue;}
		if (!strcmp(argv[i], "-e192")) {ser_d = &e192; continue;}
		
		// error
		if (!strcmp(argv[i], "-e")) {
			e_d = e_get(argv[++i]);
			if (e_d < 0 || e_d > 0.101) usage();
			continue;
		}
		// If no other option is detected, try parsing a function switch
		break;
	}

	// function
	if (!strcmp(argv[i], "-c")) {func_d = F_COMB; i++; goto args;}
	if (!strcmp(argv[i], "-r")) {func_d = F_RATIO; i++; goto args;}
	if (!strcmp(argv[i], "-v")) {func_d = F_DIV; i++; goto args;}

args:
	// the rest are the numerical values. To be fetched by function code.

	if (func_d == F_COMB) {
		struct res r1, r2;
		find_par(r_get(argv[i]), ser_d, &r1, &r2, &e_d);
		printf("\n");
		find_ser(r_get(argv[i]), ser_d, &r1, &r2, &e_d);
	}

	if (func_d == F_RATIO) {
		int n = argc-i;
		if (n == 0) usage();
		if (n == 1) n++;
		float * ws = malloc(n*sizeof(float));
		int j = 0;
		while (i < argc) {
			ws[j] = r_get(argv[i]);
			i++; j++;
		}
		if (j == 1) ws[j] = 1;
		find_weights(n, ws, ser_d, &e_d);
		free(ws);
	}
	
	if (func_d == F_DIV) {
		int n = argc-i;
		if (n < 2) usage();
		float * ws = malloc(n*sizeof(float));
		int j = 0;
		while (i < argc) {
			ws[j] = r_get(argv[i]);
			i++; j++;
		}
		if (j == 1) ws[j] = 1;
		find_weights(n, ws, ser_d, &e_d);
		free(ws);
	}
	return 0;
}



#include <stdio.h>
#include "wgrib2.h"

/* intpower.c    10/2024 Public Domain Wesley Ebisuzaki
 *
 *  return x**y
 *
 *
 *  input: double x
 *	   int y
 *
 * v1.1 2/2021 converted y to unsigned int p
 *      a future compiler may commplain about bit operations on ints
 */
double Int_Power(double x, int y) {

	double value;
	unsigned int p;

	if (y < 0) {
		p = -y;
		x = 1.0 / x;
	}
	else p = y;

	value = 1.0;

	while (p) {
		if (p & 1) value *= x;
		x = x * x;
		p >>= 1;
	}
	return value;
}

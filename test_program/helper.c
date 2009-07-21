#include "helper.h"

void int2str(char *str, int i) {
	int xdata a;
	byte j, n;

	if (i < 0) {
		*str++ = '-';
		i = -i;
	}

	a = i;
	for (n = 1; n < 7; n++)
		if ((a /= 10) == 0)
			break;

	a = i;
	for (j = 0; j < n; j++) {
		a /= 10;
		str[n - 1 - j] = (i - (a * 10)) + '0';
		i /= 10;
	}
	str[n] = '\0';
}
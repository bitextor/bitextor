#include <stdarg.h>		// variadic functions for arrncat
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clustercat.h"	// macros

// Returns 0 if all values in array are 0.0; returns 1 otherwise
int anyf(const float array[], unsigned int arr_len) {
	while (arr_len--) {
		if (array[arr_len])
			return 1;
	}
	return 0;
}

// Returns 0 if all values in array are 0.0; returns 1 otherwise
int any(const double array[], unsigned int arr_len) {
	while (arr_len--) {
		if (array[arr_len])
			return 1;
	}
	return 0;
}

// Returns 1 if all values in array are non-zero; returns 0 otherwise
int allf(const float array[], unsigned int arr_len) {
	while (arr_len--) {
		if (!array[arr_len])
			return 0;
	}
	return 1;
}

// Returns 1 if all values in array are non-zero; returns 0 otherwise
int all(const double array[], unsigned int arr_len) {
	while (arr_len--) {
		if (!array[arr_len])
			return 0;
	}
	return 1;
}

float sumf(const float array[], unsigned int arr_len) {
	float sum = 0.0;
	while (arr_len--) {
		sum += array[arr_len];
	}
	return sum;
}

double sum(const double array[], unsigned int arr_len) {
	double sum = 0.0;
	while (arr_len--) {
		sum += array[arr_len];
	}
	return sum;
}

float productf(const float array[], unsigned int arr_len) {
	float product = 1.0;
	while (arr_len--) {
		product *= array[arr_len];
	}
	return product;
}

double product(const double array[], unsigned int arr_len) {
	double product = 1.0;
	while (arr_len--) {
		product *= array[arr_len];
	}
	return product;
}

float minf(const float array[], unsigned int arr_len) {
	arr_len--;
	float min = array[arr_len];
	while (1) {
		//printf("min=%g, arr_len=%u, val=%g\n", min, arr_len, array[arr_len]); fflush(stdout);
		if (array[arr_len] < min)
			min = array[arr_len];
		if (arr_len == 0)
			break;
		arr_len--;
	}
	return min;
}

double min(const double array[], unsigned int arr_len) {
	arr_len--;
	double min = array[arr_len];
	while (1) {
		//printf("min=%g, arr_len=%u, val=%g\n", min, arr_len, array[arr_len]); fflush(stdout);
		if (array[arr_len] < min)
			min = array[arr_len];
		if (arr_len == 0)
			break;
		arr_len--;
	}
	return min;
}

float maxf(const float array[], unsigned int arr_len) {
	arr_len--;
	float max = array[arr_len];
	while (1) {
		if (array[arr_len] > max)
			max = array[arr_len];
		if (arr_len == 0)
			break;
		arr_len--;
	}
	return max;
}

double max(const double array[], unsigned int arr_len) {
	arr_len--;
	double max = array[arr_len];
	while (1) {
		if (array[arr_len] > max)
			max = array[arr_len];
		if (arr_len == 0)
			break;
		arr_len--;
	}
	return max;
}

unsigned int which_minf(const float array[], const unsigned int arr_len) {
	unsigned int which_min = 0;
	float min = array[0];

	unsigned int i = 1;
	for (; i < arr_len; i++) {
		if (array[i] < min) {
			which_min = i;
			min = array[i];
		}
	}
	return which_min;
}

unsigned int which_min(const double array[], const unsigned int arr_len) {
	unsigned int which_min = 0;
	double min = array[0];

	unsigned int i = 1;
	for (; i < arr_len; i++) {
		if (array[i] < min) {
			which_min = i;
			min = array[i];
		}
	}
	return which_min;
}

unsigned int which_maxf(const float array[], const unsigned int arr_len) {
	unsigned int which_max = 0;
	float max = array[0];

	unsigned int i = 1;
	for (; i < arr_len; i++) {
		if (array[i] > max) {
			which_max = i;
			max = array[i];
		}
	}
	return which_max;
}

unsigned int which_max(const double array[], const unsigned int arr_len) {
	unsigned int which_max = 0;
	double max = array[0];

	unsigned int i = 1;
	for (; i < arr_len; i++) {
		if (array[i] > max) {
			which_max = i;
			max = array[i];
		}
	}
	return which_max;
}

void fprint_array(FILE *stream, const double array[const], const unsigned int arr_len, char * restrict sep) {
	//fputs("{ ", stream);
	unsigned int i = 0;
	for (; i < arr_len-1; i++)
		fprintf(stream, "%g%s", array[i], sep);
	fprintf(stream, "%g\n", array[arr_len-1]);
}

void fprint_arrayf(FILE *stream, const float array[const], const unsigned int arr_len, char * restrict sep) {
	//fputs("{ ", stream);
	unsigned int i = 0;
	for (; i < arr_len-1; i++)
		fprintf(stream, "%g%s", array[i], sep);
	fprintf(stream, "%g\n", array[arr_len-1]);
}

unsigned int scan_array_of_doubles(FILE *stream, double array[], char * restrict sep) {
	char line[STDIN_SENT_MAX_CHARS];
    if (fgets(line, sizeof(line), stream) == NULL) // Get line
		return 0;
	int elems = 0;
	char * restrict token;
	if ((token = strtok(line, sep)) == NULL)
		return 0;
	while (token) {
		array[elems] = atof(token);
		elems++;
		token = strtok(NULL, sep);
	}

	return elems;
}


// Analogous to strncat(), but with variable number of arguments
void arrncat(double full_array[], const unsigned int full_array_len, ...) {
	va_list argptr;
	va_start(argptr, full_array_len);

	double * restrict offset = full_array;
	double * restrict full_array_last = full_array + full_array_len;
	//printf("30: full_array=%p, offset=%p, full_array_len=%u, sizeof(double)=%lu, len*size=%lu, full_array_last=%p, diff=%li\n", full_array, offset, full_array_len, sizeof(double), full_array_len*sizeof(double), full_array_last, full_array_last - full_array);

	while (offset < full_array_last) {
		double * restrict arr = va_arg(argptr, double*);
		//printf("31\n");
		unsigned int arr_len = va_arg(argptr, unsigned int);
		//printf("32: arr_len=%u\n", arr_len);
		unsigned int arr_len_bytes = arr_len * sizeof(double);
		//printf("33: full_array=%p, offset=%p, *<-=%g, *+1=%g, full_array_len=%u, arr_len=%u, arr_len_bytes=%u, arr[0]=%g, arr[1]=%g, arr_last=%g\n", full_array, offset, *offset, *(offset+1), full_array_len, arr_len, arr_len_bytes, arr[0], arr[1], arr[arr_len-1]); fflush(stdout);
		memcpy(offset, arr, arr_len_bytes);
		//printf("34: offset=%p, *<-=%g, *+1=%g, *-1=%g, full_array_last=%p arr_len_bytes=%u\n", offset, *offset, *(offset+1), *(offset-1), full_array_last, arr_len_bytes); fflush(stdout);
		offset += arr_len;
		//printf("35: full_array=%p, offset=%p, full_array_last=%p arr_len_bytes=%u\n", full_array, offset, full_array_last, arr_len_bytes); fflush(stdout);
		//printf("36: Full array: "); fprint_array(stdout, full_array, full_array_len, ", "); printf("\n");
	}
	va_end(argptr);
	//printf("37: Full array: "); fprint_array(stdout, full_array, full_array_len, ", "); printf("\n");
}

#include "clustercat.h"				// Model importing/exporting functions
#include "clustercat-math.h"

double dot_product(const double probs[const], const double weights[const], int length) {
	double sum = 0;
	double sum_weights = 0;
	length--;

	for (; length >= 0; --length) {
		sum_weights += weights[length];
		sum += probs[length] * weights[length];
		//printf("dot_product: sum=%g += probs[%i]=%g * weights[%i]=%g; length=%i;\n", sum, length, probs[length], length, weights[length], length);
	}
	//printf("dot_product: final sum = %g = prob_sum=%g/weight_sum=%g\n", sum/sum_weights, sum, sum_weights);
	return sum_weights ? (sum / sum_weights) : 0.0;
}

float dot_productf(const float probs[const], const float weights[const], int length) {
	float sum = 0;
	float sum_weights = 0;
	length--;

	for (; length >= 0; --length) {
		sum_weights += weights[length];
		sum += probs[length] * weights[length];
		//printf("dot_product: sum=%g += probs[%i]=%g * weights[%i]=%g; length=%i;\n", sum, length, probs[length], length, weights[length], length);
	}
	//printf("dot_product: final sum = %g = prob_sum=%g/weight_sum=%g\n", sum/sum_weights, sum, sum_weights);
	return sum_weights ? (sum / sum_weights) : 0.0;
}

long int powi(long int base, long int exp) { // Integer exponentiation
	long int result = 1;
	while (exp--)
		result *= base;
	return result;
}

double perplexity(const double log_probs, const unsigned long num_words_queried) {
	// Assumes log_probs used log2()
	return pow(2, -log_probs / (double)num_words_queried);
}


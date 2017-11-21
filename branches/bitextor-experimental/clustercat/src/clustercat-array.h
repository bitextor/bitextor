#ifndef INCLUDE_DKLM_ARRAY_HEADER
#define INCLUDE_DKLM_ARRAY_HEADER

int any(const double array[], unsigned int arr_len);
int anyf(const float array[], unsigned int arr_len);
int all(const double array[], unsigned int arr_len);
int allf(const float array[], unsigned int arr_len);

double sum(const double array[], unsigned int arr_len);
float sumf(const float array[], unsigned int arr_len);
double product(const double array[], unsigned int arr_len);
float productf(const float array[], unsigned int arr_len);

double min(const double array[], unsigned int arr_len);
float minf(const float array[], unsigned int arr_len);
double max(const double array[], unsigned int arr_len);
float maxf(const float array[], unsigned int arr_len);

unsigned int which_min(const double array[], const unsigned int arr_len);
unsigned int which_minf(const float array[], const unsigned int arr_len);
unsigned int which_max(const double array[], const unsigned int arr_len);
unsigned int which_maxf(const float array[], const unsigned int arr_len);

void fprint_array(FILE *stream, const double array[], const unsigned int arr_len, char * restrict sep);
void fprint_arrayf(FILE *stream, const float array[], const unsigned int arr_len, char * restrict sep);

unsigned int scan_array_of_doubles(FILE *stream, double array[], char * restrict sep);

void arrncat(double full_array[], const unsigned int full_array_len, ...);

#endif // INCLUDE_HEADER

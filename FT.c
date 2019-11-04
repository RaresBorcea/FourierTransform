#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <complex.h>

FILE *input, *output;
char input_file[502], output_file[502];
int N; // number of values
int P; // number of threads
double *v; // input values array
complex double *ft; // output values array


// Obtain CLI parameters
void getArgs(int argc, char **argv) {
	if(argc < 4) {
		printf("Not enough paramters: ./program input_file output_file numThreads\n");
		exit(1);
	}
	strcpy(input_file, argv[1]);
	strcpy(output_file, argv[2]);
	P = atoi(argv[3]);
}

// Read values from input file
void readFile() {
	input = fopen(input_file, "r");
	if(input == NULL) {
		printf("Input file opening error\n");
		exit(1);
	}

	// Read number of values - N
	int result = fscanf(input, "%d", &N);
	if(result != 1) {
		printf("Reading N error\n");
		exit(1);
	}

	// Allocate memory for input array
	v = (double*) malloc(sizeof(double) * N);
	if(v == NULL) {
		printf("Malloc failed\n");
		exit(1);
	}

	// Allocate memory for output array
	ft = (complex double*) calloc(N, sizeof(complex double));
	if(ft == NULL) {
		printf("Malloc failed\n");
		exit(1);
	}

	// Read Xn values from file
	for(int i = 0; i < N; i++) {
		result = fscanf(input, "%lf", &v[i]);
		if (result != 1) {
	  		printf("Reading array error\n");
			exit(1);
	  	}
	}

  	fclose(input);
}

// Print output values to output file
void printFile() {
	output = fopen(output_file, "w");
	if(output == NULL) {
		printf("Output file opening error\n");
		exit(1);
	}

	fprintf(output, "%d\n", N);
	for(int i = 0; i < N; i++) {
		fprintf(output, "%lf %lf\n", creal(ft[i]), cimag(ft[i]));
	}

	fclose(output);
}

// DFT function for parallelization
void* threadFunction(void *var)
{
	int thread_id = *(int*)var;

	// Obtain start and end indexes for each thread
	int start = thread_id * ceil((double)N/P);
	int end = fmin(N, (thread_id + 1) * ceil((double)N/P));

	// Apply DFT
    for(int k = start; k < end; k++) {
		for(int n = 0; n < N; n++) {
			ft[k] += v[n] * cexp(-I * 2 * M_PI * k * n / N);
		}
	}

	return 0;
}

int main(int argc, char *argv[])
{
	int i;
	getArgs(argc, argv);
	readFile();

	// Create P threads
	pthread_t tid[P];
	int thread_id[P];
	for(i = 0; i < P; i++)
		thread_id[i] = i;

	for(i = 0; i < P; i++) {
		pthread_create(&(tid[i]), NULL, threadFunction, &(thread_id[i]));
	}

	for(i = 0; i < P; i++) {
		pthread_join(tid[i], NULL);
	}

	printFile();

	return 0;
}
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <math.h>
#include <string.h>
#include <complex.h>

// Structure used to pass parameters to each thread
struct Params {
  complex double *buff;
  complex double *out;
  int step;
};

FILE *input, *output;
char input_file[502], output_file[502];
int N; // number of values
int P, numThreads; // number of threads
double *v; // input values array
complex double *ft, *exchange; // output & auxiliary values arrays
pthread_mutex_t mutex;

// Obtain CLI parameters
void getArgs(int argc, char **argv) {
	if(argc < 4) {
		printf("Not enough paramters: ./program input_file output_file numThreads\n");
		exit(1);
	}
	strcpy(input_file, argv[1]);
	strcpy(output_file, argv[2]);
	P = atoi(argv[3]);
	numThreads = P;
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

	// Allocate memory for auxiliary array
	exchange = (complex double*) calloc(N, sizeof(complex double));
	if(exchange == NULL) {
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
	  	ft[i] = v[i];
	  	exchange[i] = v[i];
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

// FFT function for parallelization
void* fft(void *var) {

	// Obtain parameters from struct
	struct Params *params = (struct Params*)var;
    int step = (*params).step;
    complex double *buff = (*params).buff;
  	complex double *out = (*params).out;

	if(step < N) {
		int ok1 = 0; // ok to create one more thread
		int ok2 = 0; // ok to create two more threads

		pthread_mutex_lock(&mutex);
		if(numThreads >= 2) {
			ok2 = 1;
			numThreads -= 2;
		} else if(numThreads == 1 && P != 1) {
			ok1 = 1;
			numThreads--;
		}
		pthread_mutex_unlock(&mutex);

		// Create structs for the new recursive calls
		struct Params *new_params1 = malloc(sizeof(struct Params));
		(*new_params1).step = 2 * step;
       	(*new_params1).buff = out;
       	(*new_params1).out = buff;

       	struct Params *new_params2 = malloc(sizeof(struct Params));
		(*new_params2).step = 2 * step;
       	(*new_params2).buff = out + step;
       	(*new_params2).out = buff + step;

       	pthread_t tid[2];
       	if(ok2) {
       		// Create two more threads
       		pthread_create(&tid[0], NULL, fft, (void*) new_params1);
       		pthread_create(&tid[1], NULL, fft, (void*) new_params2);

       		for(int i = 0; i < 2; i++) {
       			pthread_join(tid[i], NULL);
       		}
       	} else if(ok1) {
       		// Create one more thread
        	pthread_create(&tid[0], NULL, fft, (void*) new_params1);
      
        	fft((void*) new_params2);

        	pthread_join(tid[0], NULL);
		} else {
			// Else continue in current thread
			fft((void*) new_params1);
			fft((void*) new_params2);
		}
		
 		// Apply FFT 
		for(int k = 0; k < N; k += 2 * step) {
			complex double e = cexp(-I * M_PI * k / N) * out[k + step];
			buff[k / 2] = out[k] + e;
			buff[(k + N) / 2] = out[k] - e;
		}
	}

	return 0;
}

int main(int argc, char *argv[]) {
	pthread_mutex_init(&mutex, NULL);
	getArgs(argc, argv);
	readFile();

	struct Params *params = malloc(sizeof(struct Params));
	(*params).step = 1;
    (*params).buff = ft;
    (*params).out = exchange;

    fft((void*) params);
    
	printFile();
	pthread_mutex_destroy(&mutex);

	return 0;
}

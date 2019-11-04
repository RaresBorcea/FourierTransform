# FourierTransform

Parallel, scalable with the number of threads, versions of the Fourier Transform (FT)
and Fast Fourier Transform (FFT) algorithms in C, using the Pthread API.

To run the programs:

./FT inputValues.txt outputValues.txt numThreads

./FFT inputValues.txt outputValues.txt numThreads

Input file contains on the first line the number of values (n) and, on the following n
lines, the Xn data set, one value per line.

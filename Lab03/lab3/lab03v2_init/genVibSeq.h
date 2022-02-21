/*
Must call ais_init_sin_table( ) before calling any other function in this file.
stw is the desired sine table weight: make it a power of 2.  Recommend: 128 or 256
get_sample_vector( int mode_flag, int N, int* vals);
mode_flag = 1 for "normal vibration mode"
mode_flag = 2 for "abnormal vibration mode"
N is number of desired samples
vals is a pointer to an integer array of at least size N

get_sample_vector() is used to get a vector of samples from one of the two
vibration modes: normal or abnormal
*/
#ifndef genVibSeq_H
#define genVibSeq_H

// Function prototypes
void ais_init_sin_table(int stw);
int ais_sin(double x);
void get_sample_vector(int mode_flag, int N, int* vals);
#endif

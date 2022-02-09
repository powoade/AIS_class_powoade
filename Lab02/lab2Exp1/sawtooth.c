#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

// Function prototypes and defines
double sawtooth(double x);

// Global variables
double g_twopi = 2*M_PI;

// Main program
// int main(int argc, char *argv[]){
//   if( argc != 1 ){
//     fprintf(stderr," usage is: %s\n", argv[0]);
//     exit(1);
//   }

//   double f = 1.;
//   double T = .1;

//   for(int i = 0; i < 21; i++){
//     double t = i * T;
//     double x = sawtooth(2*M_PI*f*t);
//     printf("t = %6f  sawtooth(2*M_PI*t) = %6f\n", t, x);
//   }
// }

double sawtooth(double x){
  double rval;
  rval = fmod(x, g_twopi);
  if( rval <= M_PI ) rval /= M_PI;
  else rval = 2 - rval / M_PI;
  return(rval);
}

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

#define PTS_IN_SIN_TABLE 200
#define SIN_TABLE_WEIGHT 2048

// Function prototypes
void ais_init_sin_table();
int ais_sin(double x);

// Global variables
int    g_int_sin_table[PTS_IN_SIN_TABLE];
double g_twopi;
double g_ais_scalefac;

int main2(int argc, char *argv[]){
  if( argc != 1 ){
    fprintf(stderr," usage is: %s\n", argv[0]);
    exit(1);
  }
  else{
    // Initialize the globals
    g_twopi = 2 * M_PI;
    g_ais_scalefac = PTS_IN_SIN_TABLE / g_twopi;
    ais_init_sin_table();
  }

  double f = 1.;
  double T = .05;
  

  for(int i = 0; i < 21; i++){
    double t = i * T;
    int xint = ais_sin(2*M_PI*f*t);
    double xd = sin(2*M_PI*f*t);
    printf("xint = %4d xs = %10f x = %10f\n", xint, 
                (double)xint/SIN_TABLE_WEIGHT, xd);
  }
}

inline int ais_sin(double x){
  int ind = (int)floor( fmod(x, g_twopi) * g_ais_scalefac );
  return( g_int_sin_table[ind] );
}


void ais_init_sin_table(){
  double delta = g_twopi / PTS_IN_SIN_TABLE;
  for( int i = 0; i < PTS_IN_SIN_TABLE; i++){
    // round to nearest integer after applying the weight
    g_int_sin_table[i] = (int) floor( sin( i*delta ) * SIN_TABLE_WEIGHT + 0.5 );
  }
  return;
}

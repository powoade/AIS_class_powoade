// This is v2
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <math.h>

//NOTE: the size of the sin table is
//hardcoded to 360: 1 entry per degree

#define SIN_TABLE_WEIGHT 2048

// Function prototypes
void ais_init_sin_table();
int ais_sin(double x);

// Global variables
int    g_int_sin_table[360];
double g_rad2deg;
double g_twopi;

int main2(int argc, char *argv[]){
  if( argc != 1 ){
    fprintf(stderr," usage is: %s\n", argv[0]);
    exit(1);
  }
  else{
    // Initialize the globals
    g_rad2deg = 180/M_PI;
    g_twopi = 2*M_PI;
    ais_init_sin_table();
  }

  int f = 5000;
  double T = .0000625;

  for(int i = 0; i < 512; i++){
    double t = i * T;
    int xint = ais_sin(g_twopi*f*t);
    double xd = sin(g_twopi*f*t);
    printf("xint = %4d xs = %10f x = %10f\n", xint, 
                (double)xint/SIN_TABLE_WEIGHT, ais_sin(2*M_PI*f*t));
  }
}

// CONSTRAINT!  the argument to this function must
// satisfy 0 <= x < (2**32 - 1)*M_PI/180
inline int ais_sin(double x){
  // convert x from radians to degrees
  double x_deg = x * g_rad2deg;
  // take the integer part (maximum error is 1 degree),
  // then find the remainder modulo 360
  int ind = ( (uint32_t)x_deg ) % 360;
  return( g_int_sin_table[ind] );
}


void ais_init_sin_table(){
  double delta = g_twopi / 360;
  for( int i = 0; i < 360; i++){
    // round to nearest integer after applying the weight
    g_int_sin_table[i] = (int) floor( sin( i*delta ) * SIN_TABLE_WEIGHT + 0.5 );
  }
  return;
}

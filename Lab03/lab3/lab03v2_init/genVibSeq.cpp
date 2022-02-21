#include <stdint.h>
#include <math.h>
#include <time.h>
#include "genVibSeq.h"

// Globals visible in this file only
namespace{
  int SIN_TABLE_WEIGHT = 0;
  double g_twopi = 2*M_PI;
  double g_rad2deg = 180./M_PI;
  int    g_int_sin_table[360];
}

// Must call this function before any other!
void ais_init_sin_table(int stw){
  SIN_TABLE_WEIGHT = stw;
  double delta = g_twopi / 360;
  for( int i = 0; i < 360; i++){
    // round to nearest integer after applying the weight
    g_int_sin_table[i] = (int) floor( sin( i*delta ) * SIN_TABLE_WEIGHT + 0.5 );
  }
  return;
}

// CONSTRAINT!  the argument to this function must
// satisfy 0 <= x < (2**32 - 1)*M_PI/180
int ais_sin(double x){
  double x_deg = x * g_rad2deg;
  uint32_t ind = ((uint32_t)x_deg) % 360;
  return( g_int_sin_table[ind] );
}

void get_sample_vector(int mode_flag, int N, int* vals){
  static int P = 3;
  static int phi[3];
  static int prevout[3];

  static double T = 1. / 16000;
  static double f1 = 298;
  static double f2 = 517;
  static double f3 = 827;
  static int first_call = 1;

  if( first_call == 1 ){
    first_call = 0;
    srand( (unsigned int)clock() );
    // these must add to less than 128!
    phi[0] = -40;
    phi[1] =  20;
    phi[2] =  -5;
    for(int i = 0; i < P; i++) prevout[i] = 0;
  }

  int newval;
  double t = 0;
  double ranphase1 = (rand() % 360) * 3.14159265 / 180.;
  double ranphase2 = (rand() % 360) * 3.14159265 / 180.;
  double ranphase3 = (rand() % 360) * 3.14159265 / 180.;

  for(int j = 0; j < N; j++){
    newval = 0;
    // AR PART
    for(int i = 0; i < P; i++) newval += phi[i] * prevout[i];
    newval /= 128;  // takes integral part of the division

    // Compute next input value, including some noise
    if( mode_flag == 1 ){
      // "Normal vibration"
      newval += 10*ais_sin(g_twopi*f1*t + ranphase1) + 
         5*ais_sin(g_twopi*f2*t + ranphase2) - 2*ais_sin(g_twopi*f3*t + ranphase3) +
                   rand()%400 - 200;
    }
    else{
      // "Abnormal vibration"
      newval += 10*ais_sin(g_twopi*f1*t + ranphase1) + 
         8*ais_sin(g_twopi*f2*t + ranphase2) - 10*ais_sin(g_twopi*f3*t + ranphase3) +
                   rand()%600 - 300;
    }

    // shift right and put the newvalue in at the left
    for(int i = P; i > 0; i--) prevout[i] = prevout[i-1];
    vals[j] = prevout[0] = newval;
    t += T;
  }
}

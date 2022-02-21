// #include <Arduino_HTS221.h>      // Temperature / humidity
// #include <Arduino_LPS22HB.h>     // barometric pressure

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

#include "custom_serialcom.h"
#include <math.h>

#define SIN_TABLE_WEIGHT 2048

// Globals
namespace{
  byte databuf[128];
  int loop_cnt;
  int frequency = 500;
  double dot_product = 0.;   
  double time_sum = 0.;
  double T = 0.0000625;
  int vector2[512];
  int vector1[512];
  //int g_int_sin_table[360];
  int g_int_sawtooth_table[360];
  int    g_int_sin_table[360];
  double g_rad2deg;
  double g_twopi;
}
void setup(){
  //double g_twopi=2*M_PI
  loop_cnt = 0;
  g_twopi=2*M_PI;
  g_rad2deg = 180/M_PI;
  ais_init_sin_table();
  ais_init_sawtooth_table();
  for(int i = 0; i < 512; i++){
  double t = i * T;
  int xint_vec2 = ais_sin(2*M_PI*3000*t);
  vector2[i] = xint_vec2;
  }
}

double sawtooth(double x){
  double rval;
  rval = fmod(x, g_twopi);
  if( rval <= M_PI ) rval /= M_PI;
  else rval = 2 - rval / M_PI;
  return(rval);
}

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

inline int ais_sawtooth(double x){
  // convert x from radians to degrees
  double x_deg = x * g_rad2deg;
  // take the integer part (maximum error is 1 degree),
  // then find the remainder modulo 360
  int ind = ( (uint32_t)x_deg ) % 360;
  return( g_int_sawtooth_table[ind] );
}
void ais_init_sawtooth_table(){
  double delta = g_twopi / 360;
  for( int i = 0; i < 360; i++){
    // round to nearest integer after applying the weight
    g_int_sawtooth_table[i] = (int) floor( sawtooth( 2*M_PI*frequency*i*T ) * SIN_TABLE_WEIGHT + 0.5 );
  }
  return;
}

void loop(){
  // Should be able to do this in setup, and avoid this
  if (frequency <= 5000)
  {
    for(int i=0; i < 512; i++)
    {
      int xint_vec1 = ais_sawtooth(2*M_PI*frequency*T*i);
      vector1[i] = xint_vec1;
    }
    // double dot_product = 0.;
    unsigned long start_time = micros();
    for(int i = 0; i < 512; i++)
    {
      dot_product += (double)(vector2[i]*vector1[i])/(SIN_TABLE_WEIGHT*SIN_TABLE_WEIGHT);
      // sprintf((char *)databuf,
      //     "vector_sine = %d vector_sawtooth = %d dot product = %6f ", vector2[i], vector1[i], dot_product);
      // HandleOutput_log((const char *)databuf);
    }
    unsigned long stop_time = micros();
    unsigned long elapsed_time = stop_time - start_time;
    time_sum += elapsed_time;
    // sprintf((char *)databuf,
    //         "time_sum = %6f ", time_sum);
    //   HandleOutput_log((const char *)databuf);
  }
  else
  {
    double average_time = time_sum / loop_cnt;
    sprintf((char *)databuf,
            "average time = %6f ", average_time);
      HandleOutput_log((const char *)databuf);
    loop_cnt = 0;
    frequency = 500;
    time_sum = 0;
  }
 
  frequency +=50;
  loop_cnt++;
}


  

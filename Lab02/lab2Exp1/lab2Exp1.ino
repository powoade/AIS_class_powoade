#include <Arduino_HTS221.h>      // Temperature / humidity
#include <Arduino_LPS22HB.h>     // barometric pressure

#include "custom_serialcom.h"
#include <math.h>

// // For use with the LED
// #define RED_PIN    22
// #define GREEN_PIN  23
// #define BLUE_PIN   24
// #define YELLOW_PIN LED_BUILTIN
// #define NO_PIN     -1

// Globals
namespace{
  byte databuf[128];
  int loop_cnt;
  int frequency = 500;
  double dot_product = 0.;   
  double time_sum = 0.;
  double T = 0.0000625;
  double g_twopi=2*M_PI;
}
double sawtooth(double x){
  double rval;
  rval = fmod(x, g_twopi);
  if( rval <= M_PI ) rval /= M_PI;
  else rval = 2 - rval / M_PI;
  return(rval);
}

void setup(){
  loop_cnt = 0;
}

void loop(){
  // Should be able to do this in setup, and avoid this
  if (frequency <= 5000)
  {
    unsigned long start_time = micros();
    for(int i = 0; i < 512; i++)
    {
      double t = i * T;
      double vec1 = sawtooth(2*M_PI*frequency*t); //vector 1
      double vec2 = sin(2*M_PI*3000*t);
      dot_product = (vec2 * vec1) + dot_product;
      printf("t = %6f  sawtooth(2*M_PI*t) = %6f\n", t, vec1);
      sprintf((char *)databuf,
            "t = %6f vector1 = %6f vector2 = %6f dot product = %6f ", t, vec1, vec2, dot_product);
      HandleOutput_log((const char *)databuf);
    
    unsigned long stop_time = micros();
    unsigned long elapsed_time = stop_time - start_time;
    time_sum = elapsed_time + time_sum;
    }
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
  

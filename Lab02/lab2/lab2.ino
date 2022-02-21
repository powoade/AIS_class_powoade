#include <Arduino_HTS221.h>      // Temperature / humidity
#include <Arduino_LPS22HB.h>     // barometric pressure

#include "custom_serialcom.h"
#include <math.h>

// Globals
namespace{
  byte databuf[128];
  int loop_cnt;
  int frequency = 500;
  double dot_product = 0.;   
  double time_sum = 0.;
  double T = 0.0000625;
  double g_twopi=2*M_PI;
  double average_time;
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
      //computing dot product and incrementing 
      dot_product += (sin(2*M_PI*3000*T*i) * sawtooth(2*M_PI*frequency*T*i));
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
    average_time = time_sum / loop_cnt;
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
  

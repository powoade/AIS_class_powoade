#include <Arduino_HTS221.h>  // Temperature / humidity
#include <Arduino_LPS22HB.h> // barometric pressure

#include "custom_serialcom.h"

// For use with the LED
#define RED_PIN 22
#define GREEN_PIN 23
#define BLUE_PIN 24
#define YELLOW_PIN LED_BUILTIN
#define NO_PIN -1

// Globals
namespace
{
  byte databuf[128];
  int first_call = 1;
  uint loop_cnt = 0;
  // Initialize to values guaranteed to cause an update the
  // first time these values are checked
  float max_temp = -1000; // temperature
  float min_temp = 1000;
  float max_bp = 0; // barometric pressure
  float min_bp = 10000000000;
  float max_hum = 0; // humidity
  float min_hum = 110;
}

void setup()
{
  light_led(NO_PIN);
//   HandleOutput_log(
//       "Setup has been called");
}

void loop()
{
  // Should be able to do this in setup, and avoid this
  // "first_call" workaround, but experience shows it
  // must be done here.
  if (first_call == 1)
  {
    first_call = 0;
    HTS.begin();
    BARO.begin();
  //   HandleOutput_log(
  //       "Sensors are initialized");
  }

  // READ SENSORS
  // float temp = HTS.readTemperature(FAHRENHEIT);
  float temp = HTS.readTemperature(); // Celsius
  float hum = HTS.readHumidity();
  float bp = BARO.readPressure(MILLIBAR);

  //

  float array[10];
  array[loop_cnt % 10] = temp;
  // array[0] = temp;
  float total = 0;
  if (loop_cnt > 9)
  {
    for (int i = 0; i < 10; i++)
    {
      total = total + array[i];
    }
  }

  float average = total / 10;
  
  sprintf((char *)databuf,
          "temp = %f hum = %f bp = %f average_temp = %f ", temp, hum, bp, average);
  HandleOutput_log((const char *)databuf);

  // Adjust max and min values.  Light the appropriate LED
  // if a max value is increased
  if (temp > average)
  {
    average = temp;
    light_led(RED_PIN);
  }
  else
  {
    light_led(BLUE_PIN);
  }
  // if (bp > max_bp)
  // {
  //   max_bp = bp;
  //   light_led(GREEN_PIN);
  // }
  // if (hum > max_hum)
  // {
  //   max_hum = hum;
  //   light_led(BLUE_PIN);
  // }
  // if (temp < min_temp)
  //   min_temp = temp;
  // if (bp < min_bp)
  //   min_bp = bp;
  // if (hum < min_hum)
  //   min_hum = hum;
/*
  // Print out max/min values every 10'th iteration
  if (loop_cnt % 10 == 0)
  {
    sprintf((char *)databuf,
            "max_temp = %f min_temp = %f", max_temp, min_temp);
    HandleOutput_log((const char *)databuf);

    sprintf((char *)databuf,
            "max_hum = %f min_hum = %f", max_hum, min_hum);
    HandleOutput_log((const char *)databuf);

    sprintf((char *)databuf,
            "max_bp = %f min_bp = %f", max_bp, min_bp);
    HandleOutput_log((const char *)databuf);
  }
  */
  delay(500); // Delay for 5 seconds
  light_led(NO_PIN);
  loop_cnt++;
}

// Control the LEDs
void light_led(int color)
{
  if (color == NO_PIN)
  { // turn off all the LEDs
    // These pins are asserted low
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    // This pin is asserted high
    // (FYI so is the power pin which we don't use)
    digitalWrite(YELLOW_PIN, LOW);
  }
  else
  { // Turn on the pin you want
    if (color == YELLOW_PIN)
    {
      digitalWrite(color, HIGH);
    }
    else
    {
      digitalWrite(color, LOW);
    }
  }
}

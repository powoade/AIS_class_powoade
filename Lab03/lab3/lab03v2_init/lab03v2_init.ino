#include <TensorFlowLite.h>
#include <math.h>
#include <Arduino.h>             // Needed to use the LEDs
#include "avr/dtostrf.h"

#include "main_functions.h"
#include "tensorflow/lite/micro/all_ops_resolver.h"
#include "model.h"
#include "tensorflow/lite/micro/micro_error_reporter.h"
#include "tensorflow/lite/micro/micro_interpreter.h"
#include "tensorflow/lite/schema/schema_generated.h"
#include "tensorflow/lite/version.h"
#include "custom_serialcom.h"     // Custom com link to the RPi
#include "genVibSeq.h"

#define SIN_TABLE_WEIGHT 128
#define SAMP_LEN 250

// For use with the LED
#define RED_PIN    22
#define GREEN_PIN  23
#define BLUE_PIN   24
#define YELLOW_PIN LED_BUILTIN
#define NO_PIN     -1

// Globals
namespace{
  char varbuf[50];
  int samples[SAMP_LEN];
  int start_event = 1;
  int loop_cnt = 0;
  int vib_type;
  double time_sum = 0.;
  double average_time;
  int count_correct_pred;

  /** Globals needed for TensorFlow Lite Micro (TFLM) **/
  // The first line below pulls in all the TFLM ops; you can remove the line,
  // and only pull in the TFLM ops you need, if would like to reduce 
  // the compiled size of the sketch.
  tflite::AllOpsResolver tflOpsResolver;
  tflite::MicroErrorReporter tflErrorReporter;
  const tflite::Model* tflModel = nullptr;
  tflite::MicroInterpreter* tflInterpreter = nullptr;
  TfLiteTensor* tflInputTensor = nullptr;
  TfLiteTensor* tflOutputTensor = nullptr;

  // Create a static memory buffer for TFLM; the size needs to
  // be adjusted based on the model you are using
  const int tensorArenaSize = 9 * 32*94*sizeof(int);
  byte tensorArena[tensorArenaSize] __attribute__((aligned(16)));
}

void setup(){
  // Set up TFLM
  // Get the TFL representation of the model byte array.
  // Create an interpreter to run the model.
  // Allocate memory to hold the tensors.
  // Get pointers to the tensors.
  start_event = 1;
  ais_init_sin_table(SIN_TABLE_WEIGHT);

  tflModel = tflite::GetModel(g_model);
  if (tflModel->version() != TFLITE_SCHEMA_VERSION) {
    HandleOutput_error( -100 );
    while (1);
  }
  tflInterpreter = new tflite::MicroInterpreter(tflModel, tflOpsResolver, 
                      tensorArena, tensorArenaSize, &tflErrorReporter);
  
  tflInterpreter->AllocateTensors();
  tflInputTensor = tflInterpreter->input(0);
  tflOutputTensor = tflInterpreter->output(0);

  // These are values in structure TfLiteIntArray
  // Google it and you can find the .h file where it is defined
  HandleOutput_int( tflInputTensor->dims->size );
  HandleOutput_int( tflInputTensor->dims->data[0] );
  HandleOutput_int( tflInputTensor->dims->data[1] );

  // configure the LEDs as outputs and initialize for word recognition
  pinMode(RED_PIN, OUTPUT);
  pinMode(GREEN_PIN, OUTPUT);
  pinMode(BLUE_PIN, OUTPUT);
  pinMode(YELLOW_PIN, OUTPUT);
  // We define end of setup() as "status 1"
  HandleOutput_status( 1 );
}

void loop(){
  
  if( start_event == 1 ){
    // We are ready to kick off an event
    start_event = 0;
    // Use this read to allow the user to signal when he is ready
    int data_cnt = csc_read_data( (byte*)varbuf );
    float x = atof( (const char*)varbuf );

    // Turn off all LEDS to indicate new data is being acquired
    // then capture data
    light_led(NO_PIN);
  }  

 for(int i=0; i < 500; i++)
 {
    if( loop_cnt % 2 == 0){
      vib_type = 1;
      light_led(GREEN_PIN);
      get_sample_vector(1, SAMP_LEN, samples);
    }
    else{
      vib_type = 2;
      light_led(RED_PIN);
      get_sample_vector(2, SAMP_LEN, samples);
    }
    loop_cnt++;

    // Find the minimum and maximum values in the sample vector
    // and the maximum absolute value
    int maxval = -2000000000;
    int minval = 2000000000;
    for(int i = 0; i < SAMP_LEN; i++){
      if( samples[i] > maxval ) maxval = samples[i];
      if( samples[i] < minval ) minval = samples[i];
    }
    if( abs(maxval) > abs(minval) ) maxval = abs(maxval);
    else maxval = abs(minval);
    if (maxval == 0) maxval = 1; // Prevent division by zero

    // rescale the data to lie in range [-1, 1]
    // and load the input tensor with those values
    for(int i = 0; i < SAMP_LEN; i++){
      tflInputTensor->data.f[i] = (float)samples[i] / (float)maxval;
    }

    // Do the Inference
    unsigned long start_time = micros();
    TfLiteStatus invokeStatus = tflInterpreter->Invoke();
    unsigned long stop_time = micros();
    unsigned long elapsed_time = stop_time - start_time;
    time_sum += elapsed_time;


  // if (invokeStatus != kTfLiteOk){
  //   HandleOutput_error( -101 );
  //   while(1);
  // }

  // Look at the output tensor
    float prob_norm =   tflOutputTensor->data.f[0];
    float prob_abnorm = tflOutputTensor->data.f[1];

    if( (vib_type == 1 && prob_norm   < prob_abnorm) ||
        (vib_type == 2 && prob_abnorm < prob_norm) ){
      light_led(YELLOW_PIN);
      // We define a wrong inference to be status -500
      //HandleOutput_status( -500 );
    }
    else
    {
      count_correct_pred++;
    }

    // HandleOutput_double( prob_norm );
    // HandleOutput_double( prob_abnorm );
 }
  average_time = time_sum / 500;
  HandleOutput_int(count_correct_pred);
  HandleOutput_double(double((count_correct_pred)/500.) * 100);
  HandleOutput_double(average_time);
  exit(1);
}

/*
These functions send data to the RPi from the Arduino.
The data is tagged differently when sent to the RPi depending
on which of these functions is called
*/
void HandleOutput_int(int log_int){
  itoa(log_int, varbuf, 10);
  csc_write_data(CSC_CMD_WRITE_PI_LOG, (byte*)varbuf, strlen(varbuf) );
}
void HandleOutput_double(double log_double){
  dtostrf(log_double, 10, 3, varbuf);
  csc_write_data(CSC_CMD_WRITE_PI_LOG, (byte*)varbuf, strlen(varbuf) );
}

void HandleOutput_status(int status){
  itoa(status, varbuf, 10);
  csc_write_data(CSC_CMD_WRITE_PI_STATUS, (byte*)varbuf, strlen(varbuf) );
}
void HandleOutput_error(int error_code){
  itoa(error_code, varbuf, 10);
  csc_write_data(CSC_CMD_WRITE_PI_ERROR, (byte*)varbuf, strlen(varbuf) );
}

void light_led(int color){
  if( color == NO_PIN ){ // turn off all the LEDs
    // These pins are asserted low
    digitalWrite(RED_PIN, HIGH);
    digitalWrite(GREEN_PIN, HIGH);
    digitalWrite(BLUE_PIN, HIGH);
    // This pin is asserted high (FYI so is the power pin which we don't use)
    digitalWrite(YELLOW_PIN, LOW);
  }
  else{ // Turn on the pin you want
    if( color == YELLOW_PIN){
      digitalWrite(color, HIGH);
    }
    else{
      digitalWrite(color, LOW);
    }
  }
}

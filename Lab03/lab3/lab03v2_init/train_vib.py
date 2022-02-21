#!/usr/bin/python3
#pip install scikit-learn
import sys
import os
import matplotlib.pyplot as plt
import numpy as np
import pandas as pd
import tensorflow as tf
import time
#from sklearn.metrics import accuracy_score

print(f"TensorFlow version = {tf.__version__}\n")

# the list of training sequence .csv files for which data is available 
# for training.
VIBS = [
    "data_normal",
    "data_abnormal"
]
NUM_VIBS = len(VIBS)

# Create a one-hot encoded matrix that is used to build the output tensors.
# Each row has a single 1, while all other entries are zeroes.  In fact,
# it is just the identity matrix.  Each row corresponds to a file in
# the list VIB and is just the desired network output for all
# the instances in that file
ONE_HOT_ENCODED_VIBS = np.eye(NUM_VIBS)
sample_length = 40

# read each csv file and build the tensors for model training and
# evaluation.  The lists immediately below will hold the input and output
# tensors when done
inputs = np.array( [] )
outputs = np.array( [] )
rowmax = []
for vib_index in range(NUM_VIBS):
  vib = VIBS[vib_index]
  print(f"Processing index {vib_index} for vib '{vib}'.")
  df = pd.read_csv(vib + ".csv")

  # calculate the number of instances in the file
  num_instances = int(df.shape[0])
  print(f"\tThere are {num_instances} instances of the {vib} vib.")

  # convert pandas dataframe to a numpy array
  npd = df.to_numpy()

  output = ONE_HOT_ENCODED_VIBS[vib_index]
  for i in range(num_instances):
    inputs  = np.vstack([inputs, npd[i]]) if inputs.size else npd[i]
    outputs = np.vstack([outputs, output]) if outputs.size else output
    rowmax.append( int( np.amax(np.absolute(inputs[i])) ) )

print(f"inputs type prior to any conversion {type(inputs)}")
print(f"inputs[0,1] type = {type(inputs[0,1])}")

# EVERYTHING ABOVE IS INTEGERS!

# Now force inputs to float, then normalize instances to be between -1 and 1
inputs = inputs.astype(np.float32)
rowmax = np.asarray( rowmax, np.float32 )
for i in range( len(inputs) ): inputs[i] = inputs[i] / rowmax[i]

# Print some info
print("Data set parsing and preparation complete.")
print(f"num_inputs = {len(inputs)}")
print(f"inputs type = {type(inputs)}")
print(f"inputs[0,1] type = {type(inputs[0,1])}")
print(f"length of each input vector = {len(inputs[0])}")

# Randomize the inputs while keeping all instances of
# the same type grouped.  You can replace the use of time.time()
# to seed the random number generator with an integer of your
# choice if you want the result to be repeatable
np.random.seed( int( time.time() ) )

randomize = np.arange(500)
np.random.shuffle(randomize)
inputs[0:500] = inputs[randomize]
outputs[0:500] = outputs[randomize]

randomize = np.arange(500,1000)
np.random.shuffle(randomize)
inputs[500:1000] = inputs[randomize]
outputs[500:1000] = outputs[randomize]

# Splice up the inputs and outputs to form the sets we need
# for training and validation
inputs_train = \
  np.concatenate((inputs[0:300], inputs[500:800]))
outputs_train = \
  np.concatenate((outputs[0:300], outputs[500:800]))
inputs_validate = \
  np.concatenate((inputs[300:400], inputs[800:900]))
outputs_validate = \
  np.concatenate((outputs[300:400], outputs[800:900]))
inputs_test = \
  np.concatenate((inputs[400:500], inputs[900:1000]))
outputs_test = \
  np.concatenate((outputs[400:500], outputs[900:1000]))

print(f"val shape: {len(inputs_validate)}")
# Build the  model
model = tf.keras.Sequential()
model.add(tf.keras.Input(shape=(sample_length,) ) )     #cc
model.add(tf.keras.layers.Dense(19, activation='relu') )   #19
# model.add(tf.keras.layers.Dense(10, activation='relu') )
model.add(tf.keras.layers.Dense(NUM_VIBS, activation='softmax'))
model.summary()

# Compile the model
#model.compile(optimizer='adam', loss='binary_crossentropy', metrics=['accuracy'])
model.compile(optimizer='rmsprop', loss='mse', metrics=['mae'])

inputs_train = np.reshape(inputs_train, (-1,sample_length) )  #cc
inputs_validate = np.reshape(inputs_validate, (-1, sample_length) )  #cc
inputs_test = np.reshape(inputs_test, (-1, sample_length) )    #cc

# Fit the model
history = model.fit(inputs_train, outputs_train, epochs=50, \
                    validation_data=(inputs_validate, outputs_validate))

# use the model to predict the test inputs and print a subset
# of the results
predictions = np.round(model.predict(inputs_test))
# predictions = np.round(predictions)

scores = tf.keras.metrics.Accuracy()
scores.update_state(outputs_test, predictions)
yyy = scores.result().numpy() * 100
# scores = Accuracy.result(outputs_test, predictions).numpy()
print(f"The accuracy is: {yyy}%")
# scores = model.evaluate(outputs_test, predictions)
# print(scores)

print("rounded predictions for inputs_test[0:20] =\n", \
                      np.round(predictions[0:20], decimals=3))
print("rounded predictions for inputs_test[100:120] =\n", \
                      np.round(predictions[100:120], decimals=3))
                  
print()


# Convert the model to the TensorFlow Lite format without quantization
converter = tf.lite.TFLiteConverter.from_keras_model(model)
tflite_model = converter.convert()

# Save the model to disk, then get its size and print it
fd = open("perk_vib_model.tflite", "wb")
fd.write(tflite_model)
fd.close()
basic_model_size = os.path.getsize("perk_vib_model.tflite")
print(f"Model is {basic_model_size} bytes")

#----------
# Read in the tflite model just created and do some predictions
# with it here on the RPi.  Should see same performance on
# the Arduino
#----------
tfl_interpreter = tf.lite.Interpreter("perk_vib_model.tflite")
tfl_interpreter.allocate_tensors()
input_details = tfl_interpreter.get_input_details()
output_details = tfl_interpreter.get_output_details()

# Print the shape the model expects and the shape of our
# actual test vectors
input_shape = input_details[0]['shape']
print(f"shape model expects: input_details[0]['shape'] = {input_shape}")
print(f"shape of our inputs: inputs_test[0].shape = {inputs_test[0].shape}")

#Predict some specifically chosen values from both instance types
indx_to_test = [0, 1, 2, 3, 4, 100, 101, 102, 103, 104]
for i in indx_to_test:
  input_to_test = np.copy( inputs_test[i] )
  # Reshape our test vector to the shape expected by the model
  input_to_test = np.reshape(input_to_test, (1, sample_length))     #cc
  # Do the inference
  tfl_interpreter.set_tensor(input_details[0]['index'], 
                               input_to_test.astype(np.float32))
  tfl_interpreter.invoke()
  output_data = tfl_interpreter.get_tensor(output_details[0]['index'])
  print(f"tflite model output index = {i} is {output_data}")

#!/usr/bin/python3
import time
import sys
import csc_io as csc
import logging
import os.path

if len( sys.argv ) != 1 :
  print("usage is: rpicom")
  sys.exit(1)

csc.rpi_init('/dev/ttyACM0', 115200)

max_temp = 0.00
min_temp = -10.00
max_bp = 5
min_bp = 0
max_hum = 42
min_hum = -18
log = "mylogfile.log"

while(True):
  # Tell the Arduino we are ready to process a command
  csc.rpi_tell_ard_ready()

  # Read the command and process it
  ard_cmd = csc.rpi_get_ard_cmd()
  if ard_cmd == csc.CMD_READ_PI:
    # Arduino wants to read from RPi
    print("ERROR: this shouldn't happen");
    sys.exit(1)
  elif ard_cmd == csc.CMD_WRITE_PI_ERROR:
    # Arduino is sending RPi an error message
    print(f"*** ERROR ***: {csc.rpi_get_data()}")
  elif ard_cmd == csc.CMD_WRITE_PI_LOG:
    # Arduino is sending RPi a log message
    split_data = csc.rpi_get_data().decode("ascii").split()
    temp = float(split_data[2])
    hum = float(split_data[5])
    bp = float(split_data[8])
    if temp > max_temp:
      max_temp = temp
      print("New update on temperature logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the max temperature to {temp}")
    elif temp < min_temp:
      min_temp = temp
      print("New update on temperature logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the min temperature to {temp}")

    if hum > max_hum:
      max_hum = hum
      print("New update on humidity logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the max humidity to {hum}")
    elif hum < min_hum:
      min_hum = hum
      print("New update on humidity logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the min humidity to {hum}")

    if bp > max_bp:
      max_bp = bp
      print("New update on bp logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the max bp to {bp}")
    elif bp < min_bp:
      min_bp = bp
      print("New update on bp logged to mylogfile.log")
      logging.basicConfig(filename=log, format='%(asctime)s %(message)s', datefmt='%m/%d/%Y %I:%M:%S %p')
      logging.warning(f"- Updated the min bp to {bp}")
      # print(f"LOG: {csc.rpi_get_data()}")
  else:
    print("ERROR: this shouldn't happen")
    sys.exit(1)

  if os.path.exists('STOPME'):
    os.remove('STOPME')
    logging.shutdown()
    sys.exit(1)
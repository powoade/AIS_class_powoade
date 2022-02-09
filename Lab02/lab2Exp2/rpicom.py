#!/usr/bin/python3
import time
import sys
import csc_io as csc

if len( sys.argv ) != 1 :
  print("usage is: rpicom")
  sys.exit(1)

csc.rpi_init('/dev/ttyACM0', 115200)
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
    print(f"LOG: {csc.rpi_get_data()}")
  else:
    print("ERROR: this shouldn't happen")
    sys.exit(1)

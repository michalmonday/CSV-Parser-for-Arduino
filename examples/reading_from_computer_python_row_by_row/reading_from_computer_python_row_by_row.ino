/*  Usage example for: https://github.com/michalmonday/CSV-Parser-for-Arduino  */

#include <CSV_Parser.h>

/*
Python script used to send the file through serial (from PC to arduino) can be found at:
https://github.com/michalmonday/CSV-Parser-for-Arduino/blob/master/examples/reading_from_computer_python/arduino_serial.py

The csv file I used for this particular example can be found at:
https://github.com/michalmonday/CSV-Parser-for-Arduino/blob/master/examples/reading_from_computer_python/hurricanes.csv

This example shows how Arduino can receive and parse csv file from a computer (sent using a python script).
It first sends the file from PC to Arduino, Arduino parses it, for verification purposes it sends values
from 2 columns ("Month", "Average") back to PC (python scripts reads them and prints them in console window). 
In this particular example the csv file I used was containing information about hurricanes. The csv file contained:

Month,Average,2005,2006,2007,2008,2009,2010,2011,2012,2013,2014,2015
May,0.1,0,0,1,1,0,0,0,2,0,0,0
Jun,0.5,2,1,1,0,0,1,1,2,2,0,1
Jul,0.7,5,1,1,2,0,1,3,0,2,2,1
Aug,2.3,6,3,2,4,4,4,7,8,2,2,3
Sep,3.5,6,4,7,4,2,8,5,2,5,2,5
Oct,2.0,8,0,1,3,2,5,1,5,2,3,0
Nov,0.5,3,0,0,1,1,0,1,0,1,0,1
Dec,0.0,1,0,1,0,0,0,0,0,0,0,1

After uploading this code to arduino and running the python script (command: python --csv hurricanes.csv), 
the python script outputs the following:

All ports:
1. device=COM1 name=COM1 description=Communications Port (COM1) manufacturer=(Standard port types)
2. device=COM4 name=COM4 description=Arduino Leonardo (COM4) manufacturer=Arduino LLC (www.arduino.cc)

A single arduino port was found and will be used
COM4 - Arduino Leonardo (COM4)
Received:
0. month = May
0. average = 0.10
1. month = Jun
1. average = 0.50
2. month = Jul
2. average = 0.70
3. month = Aug
3. average = 2.30
4. month = Sep
4. average = 3.50
5. month = Oct
5. average = 2.00
6. month = Nov
6. average = 0.50
7. month = Dec
7. average = 0.00

Received:
end
Closing port


Note: I suggest to close Serial Monitor window (if it's open) before running the python script,
      otherwise the port may be busy.
*/


unsigned long last_start_time = 0;
bool all_file_received = false;
const int TIMEOUT = 1000; // 1 second
// This function is responsible for supplying characters to be parsed.
char feedRowParser() {
  // wait indefinitely but only if no characters were received yet
  // (if some characters were received, then wait for the TIMEOUT period)
  while (!Serial.available() && last_start_time == 0);

  while (!Serial.available() && last_start_time) {
    if (millis() - last_start_time > TIMEOUT) {
      all_file_received = true;
      return '\n';
    }
  }
  char c = Serial.read();
  return c;

}

// This function must return true when the whole CSV file was supplied 
// to feedRowParser() function. It will make sure that cp.parseRow() 
// returns false when the end of CSV was reached.
bool rowParserFinished() {
  return all_file_received;
}
 

void setup() {
  Serial.begin(115200);
  // CSV_Parser cp(/*format*/ "sfccccccccccc");
  CSV_Parser cp(/*format*/    "sf-----------");

  // get "Month" and "Average" columns (must be accessed by index because the header was not supplied to the cp object yet)
  char **months = (char**)cp[0];
  float *averages = (float*)cp[1];

  // Send "Month" and "Average" columns back to the PC 
  // (python script will read and print them in console window)
  int i = 0;
  while(cp.parseRow()) {
    Serial.print(i, DEC);
    Serial.print(". month = ");
    Serial.println(months[0]);
    Serial.print(i, DEC);
    Serial.print(". average = ");
    Serial.println(String(averages[0]));
  }
  Serial.println();

  // the python script is set to terminate after receiving "end" message
  delay(500);
  Serial.print("end");
}

void loop() {

}

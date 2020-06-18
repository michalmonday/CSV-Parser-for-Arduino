/*  Basic usage example for: https://github.com/michalmonday/CSV-Parser-for-Arduino  */

#include <CSV_Parser.h>

/*
This code prints:

Accessing values by column name:
0. String = hello
0. Number = 5
1. String = world
1. Number = 10

Accessing values by column number:
0. String = hello
0. Number = 5
1. String = world
1. Number = 10

 */
 
void setup() {
  Serial.begin(115200);
  delay(5000);

  char * csv_str = "my_strings,my_numbers\n"
                   "hello,5\n"
                   "world,10\n";
                 
  CSV_Parser cp(csv_str, /*format*/ "sL");


  Serial.println("Accessing values by column name:");
  char **strings = (char**)cp["my_strings"];
  int32_t *numbers = (int32_t*)cp["my_numbers"];
  
  for(int row = 0; row < cp.getRowsCount(); row++) {
    Serial.print(row, DEC);
    Serial.print(". String = ");
    Serial.println(strings[row]);
    Serial.print(row, DEC);
    Serial.print(". Number = ");
    Serial.println(numbers[row], DEC);
  }
  Serial.println();

  
  Serial.println("Accessing values by column number:");
  strings = (char**)cp[0];
  numbers = (int32_t*)cp[1];
  
  for(int row = 0; row < cp.getRowsCount(); row++) {
    Serial.print(row, DEC);
    Serial.print(". String = ");
    Serial.println(strings[row]);
    Serial.print(row, DEC);
    Serial.print(". Number = ");
    Serial.println(numbers[row], DEC);
  }

  /* 
    If we wanted to just check if the csv was parsed properly we could use:
    cp.print();

    (assumming that "Serial.begin(baud_rate)" was previously called. Because "cp.print()" is using "Serial" object)
  */
}

void loop() {

}

#include <CSV_Parser.h>

/*
This code prints:
0. String = hello
0. Number = 5
1. String = world
1. Number = 10

*/
 
void setup() {
  Serial.begin(115200);
  delay(5000);

  CSV_Parser cp(/*format*/ "sL");

  /*CSV file:
    my_strings,my_numbers\n
    hello,5\n
    world,10\n
  */

  /* File supplied in chunks: */
  cp << "my_st" << "rings" << ",my_n";
  cp << "umbers\nh" << "ello,5\nwor" << "ld,10\n";
                
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
}

void loop() {

}

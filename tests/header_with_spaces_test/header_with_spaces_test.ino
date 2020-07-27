/*  Test for: https://github.com/michalmonday/CSV-Parser-for-Arduino
*/

#include <CSV_Parser.h>

void setup() {
  Serial.begin(115200);
  delay(5000);
  
  static const char * csv_str = " a , b \n"
                                "1,2\n"
                                "3,4\n";
  CSV_Parser cp("ss");
  while (*csv_str)
    cp << *csv_str++;

  cp.print();

  char  **a = (char**)cp["a"];
  char  **b = (char**)cp["b"];
  for (int i = 0; i < cp.getRowsCount(); i++) {
    Serial.println(a[i]);
    Serial.println(b[i]);
  }

  Serial.println("Wrong keys result (should be 0)= ");
  Serial.println((uint32_t)cp[" a "],DEC);
  Serial.println((uint32_t)cp[" b "],DEC);
}

void loop() {

}

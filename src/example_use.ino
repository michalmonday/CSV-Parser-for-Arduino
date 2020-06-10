#include "csv_parser.h"


void setup() {
  Serial.begin(115200);
  delay(5000);

  /* CSV file content visualized:
   
        Column1,Column2,my_floats
        hello,1,3.33
        world,2,7.77 
   */
  
  String csv = "Column1,Column2,my_floats\nhello,1,3.33\nworld,2,7.77\n";
  CSV_Parser cp(csv, "sLf"); // where 2nd parameter specifies format (s = string, L = long, f = float), I prefer using long because in Arduino one "int" has only 16 bits
  char **col = (char**)cp["Column1"]; // cp.GetValues("Column1") also could be used
  long *col2 = (long*)cp["Column2"];
  float *my_floats = (float*)cp["my_floats"];
  
  if (col) 
    for (int i = 0; i < cp.GetRowsCount(); i++) 
      Serial.println(String(i) + ". Col 1 value = " + String(col[i]));
  
  if (col2)
    for (int i = 0; i < cp.GetRowsCount(); i++)
      Serial.println(String(i) + ". Col 2 value = " + String(col2[i]));  

  if (my_floats)
    for (int i = 0; i < cp.GetRowsCount(); i++)
      Serial.println(String(i) + ". Col 3 value = " + String(my_floats[i]));
}

void loop() {
  
}

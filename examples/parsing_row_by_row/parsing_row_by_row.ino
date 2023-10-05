/*  parsing_row_by_row example for: https://github.com/michalmonday/CSV-Parser-for-Arduino

The user must define the following 2 functions/callbacks:
- char feedRowParser()
- bool rowParserFinished()

That is because cp.parseRow() calls these functions continuously "under the hood".

*/
#include <CSV_Parser.h>
/*
This code prints:

Accessing values by column name:
0. String = hello
0. Number = 5

1. String = world
1. Number = 10


CSV_Parser content:
rows_count = 0, cols_count = 2
   Header:
      my_strings | my_numbers
   Types:
      char* | int32_t
   Values:
Memory occupied by values themselves = 22
sizeof(CSV_Parser) = 25


 */
 

char * csv_str = "my_strings,my_numbers\r\n"
                 "hello,5\r\n"
                 "world,10\r\n";
int csv_str_index = 0;

// This function is responsible for supplying characters to be parsed.
// csv_str may be replaced with SD card reading code or any other
// source of CSV text, like serial port or HTTP request
char feedRowParser() {
  return csv_str[csv_str_index++];
}

// This function must return true when the whole CSV file was supplied 
// to feedRowParser() function. It will make sure that cp.parseRow() 
// returns false when the end of CSV was reached.
bool rowParserFinished() {
  return csv_str[csv_str_index] == 0;
}


/*
Alternative way to write both of these functions:

bool csv_end_was_supplied = false;
char feedRowParser() {
  if (csv_str_index >= strlen(csv_str)) {
    csv_end_was_supplied = true;
    return 0;
  }
  return csv_str[csv_str_index++];
}

bool rowParserFinished() {
  return csv_end_was_supplied;
}
 */
 
void setup() {
  Serial.begin(115200);
  delay(5000);
                   
  CSV_Parser cp(/*format*/ "sL");

  Serial.println("Accessing values by column name:");
  
  // WARNING: String indexing can't be used here because the header was not supplied to the cp object yet.
  // char **strings = (char**)cp["my_strings"];
  // int32_t *numbers = (int32_t*)cp["my_numbers"];

  char **strings = (char**)cp[0];
  int32_t *numbers = (int32_t*)cp[1];

  // parseRow calls feedRowParser() continuously until it reads a 
  // full row or until the rowParserFinished() returns true
  int row_index = 0;
  while (cp.parseRow()) {
    // Here we could use string indexing but it would be much slower.
    // char *string = ((char**)cp["my_strings"])[0];
    // int32_t number = ((int32_t*)cp["my_numbers"])[0];
    
    char *string = strings[0];
    int32_t number = numbers[0];

    Serial.print(String(row_index) + ". String = ");
    Serial.println(string);
    Serial.print(String(row_index) + ". Number = ");
    Serial.println(number, DEC);
    Serial.println();
    row_index++;
  }

  Serial.println();
  cp.print();
}

void loop() {

}

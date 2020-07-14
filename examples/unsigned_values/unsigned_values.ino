/*  Example showing how to parse/store unsigned values for: https://github.com/michalmonday/CSV-Parser-for-Arduino      
    
    Signed value = value of which 1-bit determines whether value is positive or negative.
    Unsigned value = value that can't be negative

    How signed values are implemented in this library?
        Using the following format specifiers:
            L - 32-bit signed int
            d - 16-bit signed int
            c - 8-bit  signed int
            x - 32-bit signed int (using hex as input)

    How unsigned values are implemented in this library?
        By preceeding their signed equivalent format specifiers with letter "u".
        Let's assume the following csv file:
      
            column_1,column_2\n
            201,202\n
            203,204\n
  
        Using signed type we could parse it like:
        CSV_Parser cp(csv_str, "dd");
  
        Using unsigned type we could parse it like:
        CSV_Parser cp(csv_str, "ucuc"); // we don't need 16-bit type ("d") anymore, because values up to 255 can be stored in 8-bit unsigned type ("c" format specifier)
      
    
    Why store unsigned values?
    If we know that values we store are never going to be negative, we can sometimes save some space.

    Let's take a look again at the example csv file below:

        column_1, column_2\n
        200,201\n
        202,203\n

    These values could be stored in 8-bits but only when we use unsigned 8-bit type.
    If we used signed type, we would have to use 9-bits (because 8-bit signed integer can store values up to 127).
    There's no 9-bit type in C language, so we'd have to use 16-bit signed type to store values from example csv above.
    That would waste space, and could become problem if there's a lot of values to store.

    The ouptut of this example is:
    
        Output of cp.print():
        CSV_Parser content:
        rows_count = 2, cols_count = 4
           Header:
              bytes | words | dwords | dwords_from_hex
           Types:
              uint8_t | uint16_t | uint32_t | hex (uint32_t)
           Values:
              255 | 65535 | 4294967295 | FFFFFFFF
              254 | 65534 | 4294967294 | FFFFFFFE
        Memory occupied by values themselves = 57
        sizeof(CSV_Parser) = 25
        
        
        Parsed values:
        row = 0, byte = 255, word = 65535, dword = 4294967295, dword_from_hex = FFFFFFFF
        row = 1, byte = 254, word = 65534, dword = 4294967294, dword_from_hex = FFFFFFFE
    */

#include <CSV_Parser.h>

void setup() {
  Serial.begin(9600);
  delay(5000);
  
  /*  byte = 8-bit unsigned int (uint8_t) 
      word = 16-bit unsigned int (uint16_t)
      dword (double word) = 32-bit unsigned int (uint32_t)  */
  const char * csv_str = "bytes,words,dwords,dwords_from_hex\n"
                         "255,65535,4294967295,FFFFFFFF\n"
                         "254,65534,4294967294,FFFFFFFE\n";

  CSV_Parser cp(/*format*/ "ucuduLux");
  
  cp << csv_str;
 
  // ensure that the last value of the file is parsed (even if the file doesn't end with '\n')
  cp.parseLeftover();

  // output parsed values (allows to check that the file was parsed correctly)
  Serial.println("Output of cp.print():");
  cp.print(); // assumes that "Serial.begin()" was called before (otherwise it won't work)

  // Getting all the values:
  uint8_t  *bytes =   (uint8_t*)cp["bytes"];
  uint16_t *words =  (uint16_t*)cp["words"];
  uint32_t *dwords = (uint32_t*)cp["dwords"];
  uint32_t *dwords_from_hex = (uint32_t*)cp["dwords_from_hex"];

  if (!bytes || !words || !dwords || !dwords_from_hex) {
    Serial.println("At least 1 of the columns was not found, something went wrong.");
    return;
  }

  Serial.println();
  Serial.println();
  Serial.println("Parsed values:");
  for(int row = 0; row < cp.getRowsCount(); row++) {
    Serial.print("row = ");
    Serial.print(row, DEC);
    Serial.print(", byte = ");
    Serial.print(bytes[row], DEC);
    Serial.print(", word = ");
    Serial.print(words[row], DEC);
    Serial.print(", dword = ");
    Serial.print(dwords[row], DEC);
    Serial.print(", dword_from_hex = ");

    /*  "HEX" is specified (unlike "DEC" in all the others), 
        but in this example "dwords_from_hex" stores the same values as "dwords",
        it just has different input format (hexadecimal) and it's printed as hexadecimal.  */
    Serial.println(dwords_from_hex[row], HEX); 
  }
}

void loop() {

}

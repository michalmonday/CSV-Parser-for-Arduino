/*  Functionality tests for: https://github.com/michalmonday/CSV-Parser-for-Arduino

    Sometimes fixing 1 bug, creates 2 other bugs. This file was created to find such bugs easily.
*/

#include <CSV_Parser.h>

// how to use assertions in Arduino: https://gist.github.com/jlesech/3089916
#define __ASSERT_USE_STDERR
#include <assert.h>

int tests_done = 0;

struct TestData {
  const char * csv_str;
  const char * fmt;
  int expected_rows_count;
  int expected_cols_count;
  bool has_header;
  char delimiter;
  char quote_char;
  int32_t * expected_values;
};

int32_t basic_values[] = {1,2,3,4};
int32_t expected_values_1[] = {1,2,3,4,5,6};
int32_t expected_values_6[] = {0,0,0,1,0,0,0,0,1,0,0,0,1,0,0,0,0,1};
int32_t expected_values_8[] = {-1,-2,-3,-4};
int32_t expected_values_9[] = {0xFF, 0xFFFF, 0xFFFFFF, 0xAABBCC};

TestData test_data[] = { 
  
  // 0
  // CSV with header
  "a,b\n"
  "1,2\n"   // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "3,4\n",     "LL",    2,       2,       true,          ',',          '"',           basic_values,
  
  // 1          
  // CSV without header
  // CSV where number of rows is different from number of columns 
  "1,2,3\n"    // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values 
  "4,5,6\n",      "LLL",   2,       3,       false,         ',',          '"',           expected_values_1,

  // 2
  // CSV where some lines are separated by \r\n instead of \n
  "a,b\r\n"
  "1,2\n"     // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "3,4\r\n",     "LL",    2,       2,       true,          ',',          '"',           basic_values,

  // 3
  // CSV with custom delimiter
  "a;b\n"
  "1;2\n"      // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "3;4\n",        "LL",    2,       2,       true,          ';',          '"',           basic_values,  

  // 4
  // CSV with some values enclosed in quote char
  "\"a\",b\n"
  "1,\"2\"\n"   // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "\"3\",4\n",     "LL",    2,       2,       true,          ',',          '"',           basic_values,

  // 5
  // CSV with some values enclosed in single quote char
  "a,'b'\n"
  "'1',2\n"     // fmt,     rows,    cols,    has_header,    delimiter,    quote char,     expected values
  "3,'4'\n",       "LL",    2,       2,       true,          ',',          '\'',           basic_values,

  // 6
  // CSV with some empty fields (some empty fields containing quotes)
  "a,b,c\n"
  ",'',''\n"    
  "1,,\n"          
  "'',,1\n"   
  ",'',''\r\n"    
  "1,,\r\n"       // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values    
  "'',,1\r\n",    "LLL",      6,       3,       true,          ',',          '\'',          expected_values_6,

  // 7
  // CSV with single column (including \r\n at one line and quoted value at the end)
  "a\n"
  "1\n"    
  "2\n"         
  "3\r\n"   // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "\"4\"\n",    "L",      4,       1,     true,          ',',          '"',           basic_values,

  // 8
  // CSV with negative values
  "a,b\n"
  "-1,-2\n"   // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "-3,-4\n",     "LL",    2,       2,       true,          ',',          '"',           expected_values_8,

  // 9
  // CSV with hex input (it always supposed to be parsed as int32_t by design of this parser)  
  "a,b\n"
  "'FF',FFFF\r\n"       // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "FFFFFF,'AABBCC'\n",     "xx",    2,       2,       true,          ',',          '\'',          expected_values_9
};


void rows_and_columns_count_test(const TestData &td) {
  CSV_Parser cp(td.fmt, td.has_header, td.delimiter, td.quote_char);
  for (char c : String(td.csv_str))
    cp << c;
  int rows = cp.getRowsCount(); // assert is likely a macro so I'd rather call any methods before supplying them to assert
  int cols = cp.getColumnsCount();
  if (rows != td.expected_rows_count) {
    Serial.println("rows = " + String(rows) + ", td.expected_rows_count = " + String(td.expected_rows_count));
    Serial.println("csv_str = " + String(td.csv_str));
    cp.print();
  }
  assert(rows == td.expected_rows_count);
    
  if (cols != td.expected_cols_count) {
    Serial.println("cols = " + String(cols) + ", td.expected_cols_count = " + String(td.expected_cols_count));
    Serial.println("csv_str = " + String(td.csv_str));
    cp.print();
  }
  assert(cols == td.expected_cols_count); 
}

void expected_numeric_values_test(const TestData &td) {
  CSV_Parser cp(td.fmt, td.has_header, td.delimiter, td.quote_char);
  for (char c : String(td.csv_str))
    cp << c;
  int rows = cp.getRowsCount();
  int cols = cp.getColumnsCount();
  for (int col = 0; col < cols; col++) {
    int32_t * column_values = (int32_t *)cp[col];
    for (int row = 0; row < rows; row++) {
      int32_t expected_value = td.expected_values[row * cols + col];
      int32_t value = column_values[row];
      if (value != expected_value) {
        Serial.println("row = " + String(row) + ", col = " + String(col) + ", value = " + String(value) + ", expected_value = " + String(expected_value));
        Serial.println("csv_str = " + String(td.csv_str));
        cp.print();
      }
      assert(value == expected_value);
    }
  }
}


void unsigned_values_test() {
  Serial.println(F("Testing unsigned values"));
  static const char * csv_str = "bytes,words,dwords\n"
                                "255,65535,4294967295\n"
                                "254,65534,4294967294\n";
  CSV_Parser cp(csv_str, "ucuduL");
  uint8_t *bytes = (uint8_t*)cp["bytes"];
  uint16_t *words = (uint16_t*)cp["words"];
  uint32_t *dwords = (uint32_t*)cp["dwords"];
  assert(bytes && words && dwords);
  assert(bytes[0] == 255 && bytes[1] == 254);
  assert(words[0] == 65535 && words[1] == 65534);
  assert(dwords[0] == 4294967295L && dwords[1] == 4294967294L);

  Serial.println(F("Testing unsigned values (with signed value between)"));
  static const char * csv_str_2 = "bytes,signed,dwords\n"
                                  "255,1,4294967295\n"
                                  "254,1,4294967294\n";                            
  CSV_Parser cp2(csv_str_2, "uccuL");
  bytes = (uint8_t*)cp2["bytes"];
  char *ones = (char*)cp2["signed"];
  dwords = (uint32_t*)cp2["dwords"];
  assert(bytes && ones && dwords);
  assert(bytes[0] == 255 && bytes[1] == 254);
  assert(ones[0] == 1 && ones[1] == 1);
  assert(dwords[0] == 4294967295L && dwords[1] == 4294967294L); 
}

void chunked_supply_test() {
  Serial.println(F("Chunked supply test"));
  CSV_Parser cp("ddd", /*has_header*/ false, /*delimiter*/ ',', /*quote_char*/ "'");
  
  // closing double quote and comma is supplied at once
  cp << "'101" << "',";

  // closing double quote and comma is supplied separately
  cp << "'102" << "'" << ",103\n";

  // opening quote is supplied alone
  cp << "201," << "'" << "202',203\n";

  // supplying different types
  cp << 301 << "," << 302L << "," << '3' << String("03") << "\n";
  
  int16_t * first = (int16_t*)cp[0];
  int16_t * second = (int16_t*)cp[1];
  int16_t * third = (int16_t*)cp[2];
  if (!first || !second || !third) {
    Serial.println(F("One of 'first', 'second', 'third' was not retrieved."));
  }

  if (first[0] != 101 ||
      first[1] != 201 ||
      first[2] != 301 ||
      second[0] != 102 ||
      second[1] != 202 ||
      second[2] != 302 ||
      third[0] != 103 ||
      third[1] != 203 ||
      third[2] != 303 
     ){
    Serial.println(F("Chunked supply test FAILED"));
    cp.print();
  }
}

void setup() {
  Serial.begin(115200);
  delay(5000);
  
  for (int i = 0; i < sizeof(test_data) / sizeof(TestData); i++) {
    Serial.println("Testing expected rows and columns counts using test_data[" + String(i) + "]");
    rows_and_columns_count_test(test_data[i]);
  }
  tests_done++;

  for (int i = 0; i < sizeof(test_data) / sizeof(TestData); i++) {
    Serial.println("Testing expected numeric values using test_data[" + String(i) + "]");
    expected_numeric_values_test(test_data[i]);
  }
  tests_done++;

  unsigned_values_test();
  tests_done++;

  chunked_supply_test();
  tests_done++;

  Serial.print(F("Tests done = ")); 
  Serial.println(tests_done, DEC);
}

void loop() {

}

// __assert function was copied from: https://gist.github.com/jlesech/3089916
// It is not executed when this code is ran with Esp8266, but gets executed when testing on Arduino Pro Micro 5V (using Leonardo board setting)
//
// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(F("\n\n\n[FAIL]"));
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.println(F("\n"));
    Serial.flush();
    // abort program execution.

    // Using abort when testing with Arduino Pro Micro 5V (using Leonardo board setting) glitched the serial monitor
    // It also made it impossible to upload new code to it (which was solved by connecting RST to GND shortly before upload started)
    //abort(); 
}

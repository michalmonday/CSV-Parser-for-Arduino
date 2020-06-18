#include "CSV_Parser.h"

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
int32_t expected_values_6[] = {0,0,0,1,0,0};
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
  ",'',''\n"    // fmt,     rows,    cols,    has_header,    delimiter,    quote char,    expected values
  "1,,\n",         "LLL",    2,       3,      true,          ',',          '\'',          expected_values_6,

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
  CSV_Parser cp(td.csv_str, td.fmt, td.has_header, td.delimiter, td.quote_char);
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
  CSV_Parser cp(td.csv_str, td.fmt, td.has_header, td.delimiter, td.quote_char);
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

  Serial.print("Tests done = "); 
  Serial.println(tests_done, DEC);
}

void loop() {

}

// __assert function was copied from: https://gist.github.com/jlesech/3089916
// It is not executed when this code is ran with Esp8266, maybe it would be executed with Arduino
//
// handle diagnostic informations given by assertion and abort program execution:
void __assert(const char *__func, const char *__file, int __lineno, const char *__sexp) {
    // transmit diagnostic informations through serial link. 
    Serial.println(__func);
    Serial.println(__file);
    Serial.println(__lineno, DEC);
    Serial.println(__sexp);
    Serial.flush();
    // abort program execution.
    abort();
}

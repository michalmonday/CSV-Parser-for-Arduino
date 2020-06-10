#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <Arduino.h>

struct Dict;

class CSV_Parser {
    Dict **dict;
    int dict_size;
    int rows_count, cols_count;
    
public:
  CSV_Parser(String s, const char * fmt);
    /* Acceptable format types are:
     L - long
     f - float
     s - string (C-like, not a "String" Arduino object, just char pointer)
  */

  ~CSV_Parser();

  void PrintKeys();
  int GetColumnsCount();
  int GetRowsCount(); // excluding header
  void * GetValues(const String & key);    
};

#endif

#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <Arduino.h>

struct Dict {
  char * key;
  void * values;
  int values_count;
  char type;
  
  int header_index; // in csv file 
  
  Dict(String k, int vc, int t, int single_value_size) 
    :
      key(strdup(k.c_str())), 
      values(t == '-' ? 0 : malloc((vc + 1) * single_value_size)),
      values_count(vc),
      type(t)
    {
    }
  
  ~Dict() {
    free(key);
    if (type == 's') 
      for (int i = 0; i < values_count; i++)
        free(((char**)values)[i]);
    if (type != '-')
      free(values);
  }
};

class CSV_Parser {
    Dict **dict;
    int dict_size;
    int rows_count, cols_count;

    static String GetTypeName(char c);
public:
  CSV_Parser(String s, const char * fmt, bool has_header=true);
    /* Acceptable format types are:
     L - long (32-bit signed value)
     f - float
     s - string (C-like, not a "String" Arduino object, just char pointer)
     d - int (16-bit signed value, can't be used for values over 65535)
     c - char (8-bit signed value, can't be used for values over 127)
     x - hex (stored as long)
     - - unused (this way memory won't be allocated for the values)
  */

  ~CSV_Parser();

  void PrintKeys();
  int GetColumnsCount();
  int GetRowsCount(); // excluding header
  
  void * GetValues(const String & key);    
  void * GetValues(int index);  
  void * operator [] (const char *key);
  void * operator [] (int index);
  
  operator String ();
};

#endif

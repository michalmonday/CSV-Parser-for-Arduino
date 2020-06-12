#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <Arduino.h>

struct Dict {
  char * key;
  void * values;
  int values_count;
  char type;
  
  Dict(const char * k, int vc, int t, int single_value_size) 
    :
      key(strdup(k)), 
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
    int rows_count, cols_count;

    static String GetTypeName(char c);
public:
  CSV_Parser(const char * s, const char * fmt, bool has_header=true, char delimiter=',');
    /* Acceptable format types are:
        s - string  (C-like string, not a "String" Arduino object, just a char pointer, terminated by 0)  
        f - float  
        L - int32_t (32-bit signed value, can't be used for values over 2147483647)  
        d - int16_t (16-bit signed value, can't be used for values over 32767)  
        c - char    (8-bit signed value, can't be used for values over 127)  
        x - hex     (stored as int32_t)  
        "-" (dash character) means that value is unused/not-parsed (this way memory won't be allocated for values from that column)
  */

  ~CSV_Parser();

  void PrintKeys();
  int GetColumnsCount();
  int GetRowsCount(); // excluding header
  
  void * GetValues(const char * key);    
  void * GetValues(int index);  
  void * operator [] (const char *key);
  void * operator [] (int index);
  
  operator String ();
  void Print();
};

#endif

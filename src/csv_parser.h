#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <Arduino.h>

class CSV_Parser {
    char ** keys;
    void ** values;
    char * types;
   
    int rows_count, cols_count;

    bool has_header;
    char delimiter;
    char quote_char;

    char * ParseStringValue(const char *, const char * delim_chars, int * chars_occupied);
    void RemoveEnclosingDoubleQuotes(char *);
    void SaveNewValue(const char * val, char type_specifier, int row, int col);

    int CountRows(const char *s);
    
    static int8_t GetTypeSize(char type_specifier);
    static const char * GetTypeName(char type_specifier); 
public:
  CSV_Parser(const char * s, const char * fmt, bool has_header=true, char delimiter=',', char quote_char='"');
  /* Acceptable format types are:
        s - string  (C-like string, not a "String" Arduino object, just a char pointer, terminated by 0)  
        f - float  
        L - int32_t (32-bit signed value, can't be used for values over 2147483647)  
        d - int16_t (16-bit signed value, can't be used for values over 32767)  
        c - char    (8-bit signed value, can't be used for values over 127)  
        x - hex     (stored as int32_t)  
        "-" (dash character) means that value is unused/not-parsed (this way memory won't be allocated for values from that column)
  */
  
  // Below, there's additional constructor to allow supplying quote char as a string. Why? 
  // Because supplied quote char is likely to be a single-quote, which would require escaping using backslash if it was supplied as char.
  // I bet some people would rather use "'" instead of '\'' 
  // And that's the point of having additional constructor below:
  CSV_Parser(const char * s, const char * fmt, bool hh, char d, const char * qc) : CSV_Parser(s, fmt, hh, d, qc[0]) {}

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

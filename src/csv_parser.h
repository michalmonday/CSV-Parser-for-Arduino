/*!
  @mainpage Arduino CSV Parser
 
  @section description Description
	It's a class to which you can supply:  
	- csv string (including new-line characters)  
	- [format string](#specifying-value-types) (where each letter specifies type of value for each column)  

	Class parses that string, in other words, it extracts values, stores them and provides you with:  
	- easily accessible set of arrays (their types are specified by the format string)  

	It adheres to the [RFC 4180 specification](https://tools.ietf.org/html/rfc4180).  

  @section author Author
	Michal Borowski
  
  @section license License
	MIT
*/
 
#ifndef CSV_PARSER_H
#define CSV_PARSER_H

#include <Arduino.h>

class CSV_Parser {
    char ** keys;
    void ** values;
    char * types; // Example type:  s = char*, f = float, L = uint32_t, d = uint16_t etc. (see github page for full list of types)
   
    int rows_count, cols_count;

    bool has_header;
    char delimiter;
    char quote_char;

    char delim_chars[4]; // useful for parsing
    static Stream * debug_serial;

    void ParseHeader(const char *, int * chars_occupied);
    char * ParseStringValue(const char *, int * chars_occupied);
    void RemoveEnclosingDoubleQuotes(char *);
    void SaveNewValue(const char * val, char type_specifier, int row, int col);

    int CountRows(const char *s);
    
    static int8_t GetTypeSize(char type_specifier);
    static const char * GetTypeName(char type_specifier); 
public:
  /**  	
	@param s - string containing csv   
	@param fmt - string containing format specifiers for each column  
		 Acceptable format types are:  
			s - string  (C-like string, not a "String" Arduino object, just a char pointer, terminated by 0)   
			f - float    
			L - int32_t (32-bit signed value, can't be used for values over 2147483647)   
			d - int16_t (16-bit signed value, can't be used for values over 32767)    
			c - char    (8-bit signed value, can't be used for values over 127)   
			x - hex     (stored as int32_t)   
			"-" (dash character) means that value is unused/not-parsed (this way memory won't be allocated for values from that column)  
	@param has_header (optional) - If the supplied csv string does not have header line then "false" may be supplied  
	@param delimiter (optional) - It's a character that separates values. By default it's a comma. If the delimiter is not a comma (e.g. if it's ";" or "\t" instead) then it may be supplied.  
	@param quote_char (optional) - Quote char allows to include delimiter or new line characters to be part of the string value itself. By default it's double quote character.    
  */
  CSV_Parser(const char * s, const char * fmt, bool has_header=true, char delimiter=',', char quote_char='"');

  
  /** @brief Additional constructor to allow supplying quote char as a string. Why?   
	  Because supplied quote char is likely to be a single-quote, which would require escaping using backslash if it was supplied as char.
	  I bet some people would rather use "'" instead of '\''   */
  CSV_Parser(const char * s, const char * fmt, bool hh, char d, const char * qc) : CSV_Parser(s, fmt, hh, d, qc[0]) {}

  /** @brief Releases all dynamically allocated memory.  
	  Making values unusable once the CSV_Parser goes out of scope.  */
  ~CSV_Parser();
  
  int GetColumnsCount();
  
  /**  @brief Excluding header (if it was part of supplied CSV).  */
  int GetRowsCount();
  
  /**  @brief Gets values given the column key name.  
         @param key - column name  
         @return pointer to the first value (it must be cast by the user)   */
  void * GetValues(const char * key);    
  
  /**  @brief Gets values given the column index.  
	     @param col_index - column number (starting with 0)  
         @return pointer to the first value (it must be cast by the user)  */  
  void * GetValues(int col_index);  
  
  /**  @brief It's the same as GetValues(key) but allows to use operator instead of method call, like:  
       int32_t * my_values = (int32_t*)cp["my_key"];  */
  void * operator [] (const char *key);
 
  /**  @brief It's the same as GetValues(col_index) but allows to use operator instead of method call, like:  
       int32_t * my_values = (int32_t*)cp[0];  */ 
  void * operator [] (int col_index);
  
  //operator String ();
  
  void PrintKeys(Stream &ser = Serial);
  
  /**  @brief Prints whole parsed content including:  
		- column names  
		- column types  
		- all parsed values  
		
		@param ser (optional) - Stream object like "Serial" (by default) or "Serial1". 
								For example, it allows to supply "Serial1" or an object of
								"SoftwareSerial.h" library.  */
  void Print(Stream &ser = Serial);

  /**  @brief If invalid parameters are supplied to this class, then debug serial is used to output error information.   
	   This function is static, which means that it supposed to be called like:  
	   CSV_Parser::SetDebugSerial(stream_object);  
	   @param ser - Stream object like "Serial" (by default), "Serial1" or an object of
					"SoftwareSerial.h" library. */
  static void SetDebugSerial(Stream &ser) { debug_serial = &ser; }
};

#endif

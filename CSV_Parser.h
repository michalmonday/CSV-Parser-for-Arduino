/*  https://github.com/michalmonday/CSV-Parser-for-Arduino  */

/**
  @mainpage CSV Parser for Arduino
 
  @section description Description
	It's a class to which you can supply:  
	- csv string (including new-line characters)  
	- format string (where each letter specifies type of value for each column)  

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
  char * fmt; // Example type:  s = char*, f = float, L = uint32_t, d = uint16_t etc. (see github page for full list of types)
              // https://github.com/michalmonday/CSV-Parser-for-Arduino#specifying-value-types
  char * is_fmt_unsigned;

  /* What is stored at fmt and is_fmt_unsigned?
     
     When supplied format is "dd", then:
         fmt = "dd"
         is_fmt_unsigned = {false, false}

     When supplied format is "udud", then:
         fmt = "dd"
         is_fmt_unsigned = {true, true}
  */
 
  int rows_count, cols_count;

  bool has_header;
  char delimiter;
  char quote_char;

  char delim_chars[4]; // useful for parsing
  static Stream * debug_serial;

  bool whole_csv_supplied; // if the whole csv was supplied at once, then the last value does not have to end with "\n"
                           // if csv was supplied by chunks then the last value must end with "\n"
                           // this member will allow to check it throughout the class

  /*  Members responsible for keeping track of chunked supply of csv string.  */
  char * leftover;        // string that wasn't parsed yet because it doesn't end with delimiter or new line
  uint16_t current_col;
  bool header_parsed;

  /*  Private methods  */
  char * parseStringValue(const char *, int * chars_occupied);
  void saveNewValue(const char * val, char type_specifier, int row, int col, bool is_unsigned);
  
  static int8_t getTypeSize(char type_specifier);
  static const char * getTypeName(char type_specifier, bool is_unsigned); 

  void AssignIsFmtUnsignedArray(const char * fmt_);

  /*  Helper functions useful for handling unsigned format specifiers.  */
  static char * strdup_ignoring_u(const char *s);
  static size_t strlen_ignoring_u(const char *s);
  static char * strdup_trimmed(const char * s);

  /*  Passes part of csv string to be parsed.  
      Passing the string by chunks will allow the program using CSV_Parser to occupy much less memory (because it won't have to store the whole string). 
      This function should be called repetitively until the whole csv string is supplied.  
      
      chunk - part of the csv string (it can be incomplete value, does not have to end with delimiter, could be a single char string)  */
  void supplyChunk(const char *s);
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

  /** @brief Constructor for supplying csv string by chunks.  */
  CSV_Parser(const char * fmt_, bool hh=true, char d=',', char qc='"') : CSV_Parser(0, fmt_, hh, d, qc) {}
  CSV_Parser(const char * fmt_, bool hh, char d, const char * qc)      : CSV_Parser(0, fmt_, hh, d, qc[0]) {}

  /** @brief Releases all dynamically allocated memory.  
	  Making values unusable once the CSV_Parser goes out of scope.  */
  ~CSV_Parser();

  /** @brief Reads file from SD card. 
      @param f_name - file name (provided file must have format that was supplied in CSV_Parser constructor)
      @return True if file could be read, false if not.
    It requires previously calling "SD.begin()".  */

#ifndef CSV_PARSER_DONT_IMPORT_SD
  bool readSDfile(const char *f_name);
#endif
  
  int getColumnsCount();
  
  /**  @brief Excluding header (if it was part of supplied CSV).  */
  int getRowsCount();
  
  /**  @brief Gets values given the column key name.  
       @param key - column name  
       @return pointer to the first value (it must be cast by the user)   */
  void * getValues(const char * key);    
  
  /**  @brief Gets values given the column index.  
	     @param col_index - column number (starting with 0)  
         @return pointer to the first value (it must be cast by the user)  */  
  void * getValues(int col_index);  
  
  /**  @brief It's the same as GetValues(key) but allows to use operator instead of method call, like:  
              int32_t * my_values = (int32_t*)cp["my_key"];                 
       @param key - column name   */
  void * operator [] (const char *key);
 
  /**  @brief It's the same as GetValues(col_index) but allows to use operator instead of method call, like:  
              int32_t * my_values = (int32_t*)cp[0];  
       @param col_index - column index (0 being the first column)   */ 
  void * operator [] (int col_index);
  
  void printKeys(Stream &ser = Serial);
  
  /**  @brief Prints whole parsed content including:  
          		- column names  
          		- column types  
          		- all parsed values  
		
		   @param ser (optional) - Stream object like "Serial" (by default) or "Serial1". 
							For example, it allows to supply "Serial1" or an object of
							"SoftwareSerial.h" library.  */
  void print(Stream &ser = Serial);
  
  /**  @brief It's the same as supplyChunk(s) but allows to use operator instead of method call, like:  
              cp << "my_strings,my_ints\n" << "hello,1\n" << "world,2\n";  */ 
  CSV_Parser & operator << (const char *s);

  /**  @brief Example:   
              cp << String(5) + "," + String(6) + "\n";   */ 
  CSV_Parser & operator << (String s);
  
  /**  @brief Example:   
              cp << 'a' << ',' << 'b';   */ 
  CSV_Parser & operator << (char c);

  /**  @brief This handler converts all types (not covered in other "<<" operator handlers) to String. For example it will handle:   
              cp << 5;
              cp << 5.5f;
              cp << 5L;
              cp << 0b11111111     */ 
  template<typename T>
  CSV_Parser & operator << (T t){
      supplyChunk(String(t).c_str());
      return *this;
  };

  /** @brief Forces the previously supplied (but not parsed) chunks to be parsed despite not ending with "\n" or "\r\n" or delimiter.  
             This function should be called after full csv string is supplied with repetitive supplyChunk method calls.  
             If the csv string ended with "\n" or "\r\n" then endChunks() call is not necessary.  
             If the csv string did not end with "\n" or "\r\n" then endChunks() must be called, otherwise the last row won't be returned when using "GetValues".  */
  void parseLeftover();

  /**  @brief If invalid parameters are supplied to this class, then debug serial is used to output error information.   
	   This function is static, which means that it supposed to be called like:  
	   CSV_Parser::SetDebugSerial(stream_object);  
	   @param ser - Stream object like "Serial" (by default), "Serial1" or an object of
					"SoftwareSerial.h" library. */
  //static void setDebugSerial(Stream &ser) { debug_serial = &ser; }
};





#endif

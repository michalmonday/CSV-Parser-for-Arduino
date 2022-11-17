/*  https://github.com/michalmonday/CSV-Parser-for-Arduino  */

#include "CSV_Parser.h"

#ifndef CSV_PARSER_DONT_IMPORT_SD
    #include <SD.h>
#endif

//#include "mem_check.h" // COMMENT-OUT BEFORE UPLOAD

//Stream * CSV_Parser::debug_serial = &Serial;

/*  Helper function useful for handling unsigned format specifiers.   */
char * CSV_Parser::strdup_ignoring_u(const char *s) {
  char * new_s = (char *)malloc(strlen_ignoring_u(s) + 1);
  char * ret = new_s;
  
  while (*s) {
    *new_s = *s++;
    if (*new_s != 'u')
      new_s++;
  }
  *new_s = 0;

  return ret;
}

/*  Helper function useful for handling unsigned format specifiers.   */
size_t CSV_Parser::strlen_ignoring_u(const char *s) {
  size_t sz = 0;
  while (*s)
    sz += (*s++ != 'u');
  return sz;
}

/*  Creates dynamically allocated copy of a string with leading and trailing space removed. 
	Example input = "   test string    "
	Example output = "test string"
	
	(it is used for saving header names)
	*/
char * CSV_Parser::strdup_trimmed(const char * s) {
  s += strspn(s, " ");
  int len = strlen(s);
  while(isspace(s[len - 1])) 
    --len;
  //return strndup(s, len); // unfortunately strndup is not available in Arduino
  char * new_s = strdup(s);
  new_s[len] = 0;
  return new_s;
}

/*  It populates "is_fmt_unsigned" array. To clarify:
        fmt_ = format supplied in constructor (including "u", if there are values to be stored as unsigned)
        fmt  = member, format without "u" if any was there  */
void CSV_Parser::AssignIsFmtUnsignedArray(const char * fmt_) {
  is_fmt_unsigned = (char*)malloc(strlen(fmt));

  int sz = strlen(fmt);
  for (int i = 0; i < sz; i++) {
    if (fmt_[i] == 'u') {
      fmt_++;
      is_fmt_unsigned[i] = true;
    } else {
      is_fmt_unsigned[i] = false;
    }
  }
}

CSV_Parser::CSV_Parser(const char * s, const char * fmt_, bool has_header_, char delimiter_, char quote_char_) :
  fmt( strdup_ignoring_u(fmt_) ),
  rows_count(NULL), 
  cols_count( strlen_ignoring_u(fmt_) ),
  has_header(has_header_),
  delimiter(delimiter_),
  quote_char(quote_char_),
  delim_chars({'\r', '\n', delimiter_, 0}),
  whole_csv_supplied(false),
  //whole_csv_supplied((bool)s ? true : false), // in constructor where whole csv is not supplied at once it should be set to false
  leftover(NULL),
  current_col(NULL),
  header_parsed(!has_header_)
{  
  AssignIsFmtUnsignedArray(fmt_);
  
  keys =   (char**)calloc(cols_count, sizeof(char*));         // calloc fills memory with 0's so then I can simply use "if(keys[i]) { do something with key[i] }"
  values = (void**)calloc(cols_count, sizeof(void*));

  /*mem.check("constructor");
  mem.check("after calloc 1, should be = " + String(cols_count * sizeof(char*)));
  mem.check("after calloc 2, should be = " + String(cols_count * sizeof(void*)));*/ 
  if (s)
    supplyChunk(s);
}

CSV_Parser::~CSV_Parser() {
  for (int col = 0; col < cols_count; col++) {
    if (fmt[col] == 's')
      for (int row = 0; row < rows_count; row++)
        free(((char**)values[col])[row]);
    
    free(keys[col]);
    free(values[col]);
  }
  free(keys);
  free(values);
  free(fmt);
  free(leftover);
  free(is_fmt_unsigned);
}

#ifndef CSV_PARSER_DONT_IMPORT_SD
bool CSV_Parser::readSDfile(const char *f_name) {
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.

  File csv_file = SD.open(f_name);
  if (!csv_file) 
    return false;
    
  // read file and supply it to csv parser
  while (csv_file.available())
    *this << (char)csv_file.read();
  
  csv_file.close();
  
  // ensure that the last value of the file is parsed (even if the file doesn't end with '\n')
  parseLeftover();
  return true;
}
#endif

/*  It ensures that '\r\n' characters, delimiter and quote characters that are enclosed within string 
    value itself are properly parsed. It dynamically allocates memory, creates copy of parsed string 
    value and returns a pointer to it. Memory is supposed to be released outside of this function.  
*/
char * CSV_Parser::parseStringValue(const char * s, int * chars_occupied) {
  if (!s) {
	  *chars_occupied = 0;
	  return 0;
  }
  
  /*  If value is not enclosed in double quotes  */
  if(*s != quote_char) {
    char * first_delim = strpbrk(s, delim_chars);
    int val_len = 0;
    if (first_delim || whole_csv_supplied) {
      if (first_delim) {
        val_len = first_delim - s;
        *chars_occupied = val_len + (*first_delim == delimiter) + strspn(first_delim, "\r\n");
      }
      else {
        val_len = strlen(s);
        *chars_occupied = val_len;
      }
        
    	//return strndup(s, *chars_occupied); // available for Esp8266 but not for Arduino :(
      char *str = (char*)malloc(val_len + 1);
    	memcpy(str, s, val_len);
    	str[val_len] = 0;
      return str;
    }
    // delim_chars not found in string
    *chars_occupied = 0;
    return 0;
  }

  /*  If value is enclosed in double quotes. Being enclosed in double quotes automatically 
      means that the total number of occupied characters is 2 more than usual.  */
  *chars_occupied = 2;
  s++;
  const char * base = s;

  int len = 0; 
  bool ending_quote_found = false;
  while (char *next_quote = strchr(s, quote_char)) {
    if (*(next_quote+1) == quote_char) {
  	  s = next_quote+2;
  	  len--;
  	  continue;
  	}
    ending_quote_found = true;
  	
  	*chars_occupied += next_quote - base;
  	len += next_quote - base;
  	
  	if ((*(next_quote+1) == delimiter))
  		*chars_occupied += 1;
  	else
  		*chars_occupied += strspn(next_quote + 1, "\r\n");
  	
  	break;
  }

  if (!ending_quote_found) {
    *chars_occupied = 0;
    return 0;
  }

  /*  Copy string and turn 2's of adjacent double quotes into 1's.  */
  char * new_s = (char*)malloc(len + 1);
  new_s[len] = 0;
  char * base_new_s = new_s;
  
  s = base;
  for (int i = 0; i < len; i++) {
    *new_s++ = *s++;
    if (*(s-1) == quote_char && *s == quote_char)
      s += 1;
  }
  return base_new_s;
}


int8_t CSV_Parser::getTypeSize(char type_specifier) {
  switch (type_specifier) {
    case 's': return sizeof(char*);   // c-like string
    case 'f': return sizeof(float); 
    case 'L': return sizeof(int32_t); // 32-bit signed number (not higher than 2147483647)       
    case 'd': return sizeof(int16_t); // 16-bit signed number (not higher than 32767)
    case 'c': return sizeof(char);    // 8-bit signed number  (not higher than 127)
    case 'x': return sizeof(int32_t); // hex input is stored as long (32-bit signed number)
    case '-': return 0;   
    case   0: return 0;
    default : return 0; //debug_serial->println("CSV_Parser, wrong fmt specifier = " + String(type_specifier));
  }
  return 0;
}

const char * CSV_Parser::getTypeName(char type_specifier, bool is_unsigned) {
  if (is_unsigned) {
    switch(type_specifier){
        case 'L': return "uint32_t";
        case 'd': return "uint16_t";
        case 'c': return "uint8_t";
        case 'x': return "hex (uint32_t)"; // hex input, but it's stored as int32_t
    }
  }
  
  switch(type_specifier){
      case 's': return "char*";          
      case 'f': return "float";
      case 'L': return "int32_t";
      case 'd': return "int16_t";
      case 'c': return "char";
      case 'x': return "hex (int32_t)"; // hex input, but it's stored as int32_t
      case '-': return "-";
      case   0: return "-";
      default : return "unknown";
  }
}

void CSV_Parser::saveNewValue(const char * val, char type_specifier, int row, int col, bool is_unsigned) {
  if (is_unsigned) {
    switch (type_specifier) {
      /*  If at this point type_specifier is 's' or 'f', then format was probably invalid. 
          Only 'L', 'd', 'c', 'x' values could be preceded with 'u' (and be stored as unsigned types).  */
      case 'L': { ((uint32_t*)values[col])[row] = (uint32_t)strtoul(val, 0, 10); break; } // 32-bit unsigned number (not higher than 4294967295, not lower than 0) 
      case 'd': { ((uint16_t*)values[col])[row] = (uint16_t)strtoul(val, 0, 10); break; } // 16-bit unsigned number (not higher than 65535, not lower than 0)
      case 'c': { ((uint8_t*) values[col])[row] = (uint8_t)strtoul(val, 0, 10); break; } // 8-bit  unsigned number (not higher than 255, not lower than 0)
      case 'x': { ((uint32_t*)values[col])[row] = (uint32_t)strtoul(val, 0, 16); break; } // 32-bit unsigned number (not higher than 4294967295, not lower than 0)
      case '-': break;
    }
    return;
  }
  
  switch (type_specifier) {
    case 's': { ((char**)  values[col])[row] = strdup(val);                 break; } // c-like string
    case 'f': { ((float*)  values[col])[row] = (float)atof(val);            break; }
    case 'L': { ((int32_t*)values[col])[row] = (int32_t)atol(val);          break; } // 32-bit signed number (not higher than 2147483647) 
    case 'd': { ((int16_t*)values[col])[row] = (int16_t)atoi(val);          break; } // 16-bit signed number (not higher than 32767)
    case 'c': { ((char*)   values[col])[row] = (char)atoi(val);             break; } // 8-bit signed number  (not higher than 127)
    case 'x': { ((int32_t*)values[col])[row] = (int32_t)strtol(val, 0, 16); break; } // hex input is stored as long (32-bit signed number)
    case '-': break;
  }
}


void CSV_Parser::printKeys(Stream &ser) {
  ser.println("Keys:");
  for (int col = 0; col < cols_count; col++) 
    ser.println(String(col) + ". Key = " + String(keys[col] ? keys[col] : "unused"));
}

int CSV_Parser::getColumnsCount() { return cols_count; }
int CSV_Parser::getRowsCount() { return rows_count; } // excluding header

/*  Get values pointer given column name (key in other words)  */
void * CSV_Parser::operator [] (const char *key) { 
    for (int col = 0; col < cols_count; col++) 
    if (keys[col] && !strcmp(keys[col], key))
      return values[col];
  return (void*)0;
}

/*  Get values pointer given column index (0 being the first column)  */
void * CSV_Parser::operator [] (int index) { return index < cols_count ? values[index] : (void*)0; }


/*  Prints column names, their types and all stored values.  */
void CSV_Parser::print(Stream &ser) {
  ser.println("CSV_Parser content:");
  ser.print("rows_count = ");
  ser.print(rows_count, DEC);
  ser.print(", cols_count = ");
  ser.println(cols_count, DEC);
  if (has_header) {
    ser.println("   Header:");
    ser.print("      ");
    for (int col = 0; col < cols_count; col++) { 
      ser.print(keys[col] ? keys[col] : " - "); if(col == cols_count - 1) { continue; }
      ser.print(" | ");
    }
    ser.println();
  } else {
    ser.println("(No header)");
  }

  ser.println("   Types:");
  ser.print("      ");
  for (int col = 0; col < cols_count; col++) { 
    ser.print(getTypeName(fmt[col], is_fmt_unsigned[col])); if(col == cols_count - 1) { continue; }
    ser.print(" | ");
  }
  ser.println();
  
  ser.println("   Values:");
  for (int i = 0; i < rows_count; i++) {
    ser.print("      ");
    for (int j = 0; j < cols_count; j++) {  
      if (is_fmt_unsigned[j]) {
        switch(fmt[j]){
            case 'L': ser.print( ((uint32_t*)values[j])[i]  , DEC); break;          
            case 'd': ser.print( ((uint16_t*)values[j])[i]  , DEC); break;
            case 'c': ser.print( ((uint8_t*) values[j])[i]  , DEC); break;
            case 'x': ser.print( ((uint32_t*)values[j])[i]  , HEX); break;
            default : ser.print("Invalid unsigned type"); break;
        }
      } else {
        switch(fmt[j]){
            case 's': ser.print( ((char**)  values[j])[i] );       break;
            case 'f': ser.print( ((float*)  values[j])[i] );       break;
            case 'L': ser.print( ((int32_t*)values[j])[i]  , DEC); break;          
            case 'd': ser.print( ((int16_t*)values[j])[i]  , DEC); break;
            case 'c': ser.print( ((char*)   values[j])[i]  , DEC); break;
            case 'x': ser.print( ((int32_t*)values[j])[i]  , HEX); break;
            case '-': ser.print('-'); break;
            case   0: ser.print('-'); break;
        }
      }
      if(j == cols_count - 1) { continue; }
      ser.print(" | ");
    }
    ser.println();
  }
  uint32_t sum = 0;
  for (int col = 0; col < cols_count; col++)
    sum += getTypeSize(fmt[col]) * rows_count + (has_header && fmt[col] != '-' ? strlen(keys[col]) + 1 : 0);
  ser.print("Memory occupied by values themselves = "); 
  ser.println(sum, DEC);
  ser.print("sizeof(CSV_Parser) = ");
  ser.println(sizeof(CSV_Parser), DEC);
}



void CSV_Parser::supplyChunk(const char *s) {
  whole_csv_supplied = false;
  
  static bool ignore_next_delimchar = false;

  if (leftover) {
    int leftover_len = strlen(leftover);
	
	// If there's no leftover and first supplied char is '\n' then it could be the case that the last char was "\r",
	// so '\n' should be ignored.
	// The same applies to situation where " (quote char) was previously received and the supplied char is '\r'
	if (leftover_len == 0 && ignore_next_delimchar && (*s == '\n' || *s == '\r' || *s == delimiter)) {
		if(*s != '\r')
			ignore_next_delimchar = false;
		s++;
	}
	
    int s_len = strlen(s);
    //debug_serial->println("leftover_len = " + String(leftover_len) + ", s_len = " + String(s_len));
    leftover = (char*)realloc(leftover, leftover_len + s_len + 1);
	
	//if (!leftover) 
	//	debug_serial->println("leftover realloc failed");
    strcat(leftover, s);
    s = leftover;
    //debug_serial->println("merged leftover = " + String(leftover));
  }

  int chars_occupied = 0;
  char * val = 0;
  while (val = parseStringValue(s, &chars_occupied)) {
    //debug_serial->println("rows_count = " + String(rows_count) + ", current_col = " + String(current_col) + ", val = " + String(val));
    if (fmt[current_col] != '-') {
      if (!header_parsed) {
        keys[current_col] = strdup_trimmed(val);
      } else {
        //mem.check("values[" + String(current_col) + "]");
        values[current_col] = (char*)realloc(values[current_col], (rows_count+1) * getTypeSize(fmt[current_col])); 
        saveNewValue(val, fmt[current_col], rows_count, current_col, is_fmt_unsigned[current_col]);
      }
    }
    
    if (++current_col == cols_count) {
      current_col = 0;
      if (!header_parsed) header_parsed = true;
      else rows_count++;
    }
    free(val);
    s += chars_occupied;
	//debug_serial->println("chars_occupied = " + String(chars_occupied));
    chars_occupied = 0;
	
	if (!(*s) && (*(s-1) == quote_char || *(s-1) == '\r'))
		ignore_next_delimchar = true;
	else 
		ignore_next_delimchar = false;
  }

  if (s != leftover) {
	//Serial.println("if (*s && s != leftover) s = " + String(s));
	//Serial.println("leftover = " + String(leftover));
    int new_size = strlen(s);
	  if (!leftover)
		  leftover = (char*)malloc(new_size+1);
	    memmove(leftover, s, new_size+1);
      leftover = (char*)realloc(leftover, new_size + 1);   
	//debug_serial->println("new_size = " + String(new_size) + ", new leftover = " + String(leftover));
  }
}


CSV_Parser & CSV_Parser::operator << (const char *s) {
  supplyChunk(s);
  /*  Returns const reference to itself to allow chaining like:
  cp << s << another_s << and_another_s; */
  return *this;
}

CSV_Parser & CSV_Parser::operator << (String s) {
  supplyChunk(s.c_str());
  return *this;
  /* return *this << s.c_str(); */
}

CSV_Parser & CSV_Parser::operator << (char c) {
  char s[2] = {c, 0};
  supplyChunk(s);
  return *this;
}

void CSV_Parser::parseLeftover() {
  if (leftover) {
    whole_csv_supplied = true;
    int chars_occupied = 0;
    if (char * val = parseStringValue(leftover, &chars_occupied)) {
      if (fmt[current_col] != '-') {
        values[current_col] = realloc(values[current_col], (rows_count+1) * getTypeSize(fmt[current_col]));
        saveNewValue(val, fmt[current_col], rows_count, current_col, is_fmt_unsigned[current_col]);  
      }
      if (++current_col == cols_count) {
        current_col = 0;
        rows_count++;
      }
      free(val);
    }
    free(leftover);
    leftover = 0;
  }
}

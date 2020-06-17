#include "csv_parser.h"

//#include "mem_check.h" // DELETE BEFORE UPLOAD

Stream * CSV_Parser::debug_serial = &Serial;


CSV_Parser::CSV_Parser(const char * s, const char * fmt_, bool has_header_, char delimiter_, char quote_char_) :
  rows_count(NULL), 
  cols_count(strlen(fmt_)),
  fmt(strdup(fmt_)),
  has_header(has_header_),
  delimiter(delimiter_),
  quote_char(quote_char_),
  delim_chars({'\r', '\n', delimiter_, 0}),
  leftover(NULL),
  current_col(NULL),
  whole_csv_supplied(false),
  //whole_csv_supplied((bool)s ? true : false), // in constructor where whole csv is not supplied at once it should be set to false
  header_parsed(!has_header_)
{  
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
}

/*  This function is not meant to properly parse string values enclosed in quotes.
    This function is meant to be used for numeric values that happen to be enclosed in quotes.
    It assumes that the only possible double quotes are at the begining and end.
    Example:
      
      my_ints,my_floats\n
      "1","3.3"\n
      "2","6.6"
  
    So this function is used for any columns that are not of "s" type (e.g. floats, uint32_t, uint16_t etc.).
    Why is that?
    Because such values may be enclosed in quote chars but they: 
    - won't include delimeters within themselves
    - won't include the quote char themselves
    - won't include carriage-return/new-line within themselves
    Knowing this, we can parse them faster (by only checking if they start and end with quote character, and removing these characters if they're present).
*/
/*void CSV_Parser::removeEnclosingDoubleQuotes(char * s) {
  if (*s  == quote_char) {
    // shift whole string to left
    while(*++s) *(s-1) = *s;

    // remove last char if it's double quotes
    if (*(s-2) == quote_char) *(s-2) = 0;
  }
  // if the string didn't start with double quotes then assume it doesn't end with double quotes, so do nothing
}*/


/*  It ensures that '\r\n' characters, delimiter and quote characters that are enclosed within string 
    value itself are properly parsed. It dynamically allocates memory, creates copy of parsed string 
    value and returns a pointer to it. Memory is supposed to be released outside of this function.  
*/
char * CSV_Parser::parseStringValue(const char * s, int * chars_occupied) {
  if (!s) return 0;

  
  /*  If value is not enclosed in double quotes 
  */
  if(*s != quote_char) {
    char * first_delim = strpbrk(s, delim_chars);
    int val_len = 0;
    if (first_delim || whole_csv_supplied) {
      if (first_delim) {
        val_len = first_delim - s;
        *chars_occupied = val_len + 1;
      }
      else {
        val_len = strlen(s);
        *chars_occupied = val_len;
      }
        
    	//return strndup(s, *chars_occupied); // available for Esp8266 but not for Arduino :(
      char *str = (char*)malloc(val_len + 1);
    	memcpy(str, s, val_len);
    	str[val_len] = 0;
      
      *chars_occupied += strspn(s+val_len+1, "\r\n");
      return str;
    }
    // delim_chars not found in string
    *chars_occupied = 0;
    return 0;
  }

  /*  If value is enclosed in double quotes. Being enclosed in double quotes automatically 
      means that the total number of occupied characters is 2 more than usual.
  */
  *chars_occupied = 2;
  
  s += 1;
  const char * base = s;

  int len = 0; 
  bool ending_quote_found = false;
  while(*s) {
    if (*s == quote_char && *(s+1) != quote_char) {
      ending_quote_found = true;
      *chars_occupied += 1;
      break; // end of string was found 
    }
    
    byte to_add = (*s == quote_char && *(s+1) == quote_char) ? 2 : 1;
    s += to_add;
    *chars_occupied += to_add;
    len += 1;
  }

  if (!ending_quote_found) {
    *chars_occupied = 0;
    return 0;
  }

  *chars_occupied += strspn(s+1, "\r\n");

  /*  Copy string and turn 2's of adjacent double quotes into 1's.
  */
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

/*  The supplied string must be the whole file, including header (if it happens to have header).  
*/
/*
int CSV_Parser::countRows(const char *s) {
  int count = 0;
  while(*s) {
    for (int col = 0; col < cols_count; col++) {
      if (fmt[col] == 's' || fmt[col] == '-' || (has_header && count == 0)) {
        int occupied_chars = 0;
        free(parseStringValue(s, &occupied_chars));
        s += occupied_chars;
      } else {
        s += strcspn(s, delim_chars) + 1;
        s += strspn(s, "\r\n");
      }
    }   
    count += 1;
    
    if (*(s-1) == 0)
      break;
  }
  return count - has_header;
}*/


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
    default : debug_serial->println("CSV_Parser, wrong fmt specifier = " + String(type_specifier));
  }
  return 0;
}

const char * CSV_Parser::getTypeName(char type_specifier) {
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

void CSV_Parser::saveNewValue(const char * val, char type_specifier, int row, int col) {
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
    ser.print(getTypeName(fmt[col])); if(col == cols_count - 1) { continue; }
    ser.print(" | ");
  }
  ser.println();
  
  ser.println("   Values:");
  for (int i = 0; i < rows_count; i++) {
    ser.print("      ");
    for (int j = 0; j < cols_count; j++) {     
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

  //Serial.println("supplyChunk s = " + String(s));

  if (leftover) {
    int leftover_len = strlen(leftover);
    int s_len = strlen(s);
    //Serial.println("leftover_len = " + String(leftover_len) + ", s_len = " + String(s_len));
    leftover = (char*)realloc(leftover, leftover_len + s_len + 1);
    //Serial.println("chunk (post realloc) = " + String(chunk));
    strcat(leftover, s);
    //Serial.println("post strcat");
    s = leftover;
    //Serial.println("if (chunk) s = " + String(s));
  }

  int chars_occupied = 0;
  char * val = 0;
  while (val = parseStringValue(s, &chars_occupied)) {
    //Serial.println("rows_count = " + String(rows_count) + ", current_col = " + String(current_col) + ", val = " + String(val));

    if (fmt[current_col] != '-') {
      if (!header_parsed) {
        keys[current_col] = strdup(val);
      } else {
        //mem.check("values[" + String(current_col) + "]");
        values[current_col] = (char*)realloc(values[current_col], (rows_count+1) * getTypeSize(fmt[current_col]));
              
        saveNewValue(val, fmt[current_col], rows_count, current_col);
      }
    }
    
    if (++current_col == cols_count) {
      current_col = 0;
      if (!header_parsed) header_parsed = true;
      else rows_count++;
    }
    free(val);
    s += chars_occupied;
    chars_occupied = 0;
  }

  if (*s && s != leftover) {
    int new_size = strlen(s);
    leftover = (char*)realloc(leftover, new_size + 1);    
    memcpy(leftover, s, new_size);
    leftover[new_size] = 0;

    //Serial.println("if (*s && s != chunk) chunk = " + String(s));
  }
}


CSV_Parser & CSV_Parser::operator << (const char *s) {
  supplyChunk(s);
  /*  Returns const reference to itself to allow chaining like:
  cp << s << another_s << and_another_s; */
  return *this;
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
        saveNewValue(val, fmt[current_col], rows_count, current_col);  
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

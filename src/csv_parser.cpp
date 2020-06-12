#include "csv_parser.h"

int CountCharInStr(const char * s, char c, int size_limit=0) {
  int count = 0;
  for (int i = 0; i < strlen(s); i++) {
    if (s[i] == c)
      count++;
      
    if (size_limit && i == size_limit)
      break;
  }
  return count;
}
  

CSV_Parser::CSV_Parser(const char * s, const char * fmt, bool has_header, char delimiter) {
  const char delim_chars[3] = {'\n', delimiter, 0};
  
  cols_count = CountCharInStr(s, delimiter, strcspn(s, "\n")) + 1;
  rows_count = CountCharInStr(s, '\n') - (s[strlen(s)-1] == '\n') + 1 - has_header; // exclude header if it's present

  keys =   (char**)calloc(cols_count, sizeof(char*)); // calloc fills memory with 0's so then I can simply use "if(keys[i]) { do something with key[i] }"
  values = (void**)calloc(cols_count, sizeof(void*));
  types =   (char*)calloc(cols_count, 1);

  //Serial.println("Free heap (CSV_Parser constructor) = " + String(ESP.getFreeHeap())); 

  for (int col = 0; col < strlen(fmt); col++) {
      int key_len = strcspn(s, delim_chars);

      int single_value_size;
      switch (fmt[col]) {
        case 's': { single_value_size = sizeof(char*);   break; } // c-like string
        case 'f': { single_value_size = sizeof(float);   break; } 
        case 'L': { single_value_size = sizeof(int32_t); break; } // 32-bit signed number (not higher than 2147483647)       
        case 'd': { single_value_size = sizeof(int16_t); break; } // 16-bit signed number (not higher than 32767)
        case 'c': { single_value_size = sizeof(char);    break; } // 8-bit signed number  (not higher than 127)
        case 'x': { single_value_size = sizeof(int32_t); break; } // hex input is stored as long (32-bit signed number)
        case '-': { single_value_size = 0;               break; }
        
        default : Serial.println("CSV_Parser, wrong fmt specifier = " + String(fmt[col])); break;
      }

      //Serial.println("single_value_size =" + String(single_value_size) +  ", values_count = " + String(rows_count));
      
      if (fmt[col] != '-') {
        /*
          keys   = (char**)realloc(keys, (col+1) * sizeof(char*));
          values = (void**)realloc(values, (col+1) * sizeof(void*));
          types  =  (char*)realloc(types, col+1);
        */
        
          if (has_header) {
            keys[col] = strndup(s, key_len);
            s += key_len + 1; 
          } else {
            keys[col] = 0; //(char*)malloc(12);
            //sprintf(keys[col], "col_%d", col);
          }
          values[col] = malloc(single_value_size * rows_count);
          types[col] = fmt[col];
      }
      //Serial.println("Free heap (after col no " + String(col) + " was created) = " + String(ESP.getFreeHeap())); 
  }    

  for (int row = 0; row < rows_count; row++) {
    for (int col = 0; col < strlen(fmt); col++) {
      int val_len = strcspn(s, delim_chars);
      char * val = strndup(s, val_len);

      switch (fmt[col]) {
        case 's': { ((char**)  values[col])[row] = strdup(val);                 break; } // c-like string
        case 'f': { ((float*)  values[col])[row] = (float)atof(val);            break; }
        case 'L': { ((int32_t*)values[col])[row] = (int32_t)atol(val);          break; } // 32-bit signed number (not higher than 2147483647) 
        case 'd': { ((int16_t*)values[col])[row] = (int16_t)atoi(val);          break; } // 16-bit signed number (not higher than 32767)
        case 'c': { ((char*)   values[col])[row] = (char)atoi(val);             break; } // 8-bit signed number  (not higher than 127)
        case 'x': { ((int32_t*)values[col])[row] = (int32_t)strtol(val, 0, 16); break; } // hex input is stored as long (32-bit signed number)
        case '-': break;
      }
      s += val_len + 1;
      free(val);
    }
    //Serial.println("Free heap (after row no " + String(row) + " was parsed) = " + String(ESP.getFreeHeap())); 
  }

  //Serial.println("Free heap (CSV_Parser constructor end) = " + String(ESP.getFreeHeap()));
}

CSV_Parser::~CSV_Parser() {
  for (int col = 0; col < cols_count; col++) {
    if (types[col] == 's')
      for (int row = 0; row < rows_count; row++)
        free(((char**)values)[row]);
    
    if (keys[col])   free(keys[col]);
    if (values[col]) free(values[col]);
  }
  free(keys);
  free(values);
  free(types);
}

void CSV_Parser::PrintKeys() {
  Serial.println("Keys:");
  for (int col = 0; col < cols_count; col++) 
    Serial.println(String(col) + ". Key = " + String(keys[col] ? keys[col] : "unused"));
}

int CSV_Parser::GetColumnsCount() { return cols_count; }
int CSV_Parser::GetRowsCount() { return rows_count; } // excluding header

void * CSV_Parser::GetValues(const char * key) {
  for (int col = 0; col < cols_count; col++) 
    if (keys[col] && !strcmp(keys[col], key))
      return values[col];
  return (void*)0;
}

void * CSV_Parser::GetValues(int index)          { return index < cols_count ? values[index] : (void*)0; }
void * CSV_Parser::operator [] (const char *key) { return GetValues(key);   }
void * CSV_Parser::operator [] (int index)       { return GetValues(index); }

String CSV_Parser::GetTypeName(char c) {
  switch(c){
      case 's': return "char*";          
      case 'f': return "float";
      case 'L': return "int32_t";
      case 'd': return "int16_t";
      case 'c': return "char";
      case 'x': return "hex (int32_t)"; // hex input, but it's stored as int32_t
      case '-': return "unused";
      case   0: return "unused";
      default : return "unknown (" + String(c) + ")";
  }
}

CSV_Parser::operator String() {
  String ret = "CSV_Parser:\n";
  ret += "  header fields:\n";
  for (int col = 0; col < cols_count; col++) 
    ret += "    " + String(keys[col]) + " (" + GetTypeName(types[col]) + ")\n"; 

  ret += "  rows number = " + String(rows_count);
  return ret;
}

void CSV_Parser::Print() {
  Serial.println("CSV_Parser content:");
  Serial.println("   Header:");
  Serial.print("      ");
  for (int col = 0; col < cols_count; col++) { 
    Serial.print(keys[col] ? keys[col] : "unused"); if(col == cols_count - 1) { continue; }
    Serial.print(" | ");
  }
  Serial.println();

  Serial.println("   Types:");
  Serial.print("      ");
  for (int col = 0; col < cols_count; col++) { 
    Serial.print(GetTypeName(types[col])); if(col == cols_count - 1) { continue; }
    Serial.print(" | ");
  }
  Serial.println();
  
  Serial.println("   Values:");
  for (int i = 0; i < rows_count; i++) {
    Serial.print("      ");
    for (int j = 0; j < cols_count; j++) {     
      switch(types[j]){
          case 's': Serial.print( ((char**)  values[j])[i] );       break;
          case 'f': Serial.print( ((float*)  values[j])[i] );       break;
          case 'L': Serial.print( ((int32_t*)values[j])[i]  , DEC); break;          
          case 'd': Serial.print( ((int16_t*)values[j])[i]  , DEC); break;
          case 'c': Serial.print( ((char*)   values[j])[i]  , DEC); break;
          case 'x': Serial.print( ((int32_t*)values[j])[i]  , HEX); break;
          case '-': Serial.print('-'); break;
          case   0: Serial.print('-'); break;
      }
      if(j == cols_count - 1) { continue; }
      Serial.print(" | ");
    }
    Serial.println();
  }
}

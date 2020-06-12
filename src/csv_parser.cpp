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
  byte dict_size = 0;
  
  dict = (Dict**)malloc(cols_count * sizeof(Dict*));

  //Serial.println("Free heap (CSV_Parser constructor) = " + String(ESP.getFreeHeap())); 

  for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      char * key;
      int key_len = strcspn(s, delim_chars);
      
      if (has_header) {
        key = strndup(s, key_len);
      } else {
        key = (char*)malloc(12);
        sprintf(key, "col_%d", header_index);
      }
      s += key_len + 1; 

      int single_value_size;
      switch (fmt[header_index]) {
        case 's': { single_value_size = sizeof(char*);   break; } // c-like string
        case 'f': { single_value_size = sizeof(float);   break; } 
        case 'L': { single_value_size = sizeof(int32_t); break; } // 32-bit signed number (not higher than 2147483647)       
        case 'd': { single_value_size = sizeof(int16_t); break; } // 16-bit signed number (not higher than 32767)
        case 'c': { single_value_size = sizeof(char);    break; } // 8-bit signed number  (not higher than 127)
        case 'x': { single_value_size = sizeof(int32_t); break; } // hex input is stored as long (32-bit signed number)
        case '-': { single_value_size = 0;               break; }
        
        default : Serial.println("CSV_Parser, wrong fmt specifier = " + String(fmt[header_index])); break;
      }
           
      dict[dict_size] = new Dict(key, rows_count, fmt[header_index], single_value_size);
      dict_size++;
      free(key);
  }    

  for (int row = 0; row < rows_count; row++) {
    for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      int val_len = strcspn(s, delim_chars);
      char * val = strndup(s, val_len);

      switch (fmt[header_index]) {
        case 's': { ((char**)dict[header_index]->values)[row] =   strdup(val);                 break; } // c-like string
        case 'f': { ((float*)dict[header_index]->values)[row] =   (float)atof(val);            break; }
        case 'L': { ((int32_t*)dict[header_index]->values)[row] = (int32_t)atol(val);          break; } // 32-bit signed number (not higher than 2147483647) 
        case 'd': { ((int16_t*)dict[header_index]->values)[row] = (int16_t)atoi(val);          break; } // 16-bit signed number (not higher than 32767)
        case 'c': { ((char*)dict[header_index]->values)[row] =    (char)atoi(val);             break; } // 8-bit signed number  (not higher than 127)
        case 'x': { ((int32_t*)dict[header_index]->values)[row] = (int32_t)strtol(val, 0, 16); break; } // hex input is stored as long (32-bit signed number)
        case '-': break;
      }
      s += val_len + 1;
      free(val);
    }
  }
}

CSV_Parser::~CSV_Parser() {
  for (int i = 0; i < cols_count; i++)
    delete dict[i];
  free(dict);
}

void CSV_Parser::PrintKeys() {
  Serial.println("Keys:");
  for (int i = 0; i < cols_count; i++) 
    Serial.println(String(i) + ". Key = " + String(dict[i]->key));
}

int CSV_Parser::GetColumnsCount() { return cols_count; }
int CSV_Parser::GetRowsCount() { return rows_count; } // excluding header

void * CSV_Parser::GetValues(const char * key) {
  for (int i = 0; i < cols_count; i++) 
    if (!strcmp(dict[i]->key, key))
      return dict[i]->values;
  return (void*)0;
}

void * CSV_Parser::GetValues(int index)          { return index < cols_count ? dict[index]->values : (void*)0; }
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
      default : return "unknown (" + String(c) + ")";
  }
}

CSV_Parser::operator String() {
  String ret = "CSV_Parser:\n";
  ret += "  header fields:\n";
  for (int i = 0; i < cols_count; i++) 
    ret += "    " + String(dict[i]->key) + " (" + GetTypeName(dict[i]->type) + ")\n"; 

  ret += "  rows number = " + String(rows_count);
  return ret;
}

void CSV_Parser::Print() {
  Serial.println("CSV_Parser content:");
  Serial.println("   Header:");
  Serial.print("      ");
  for (int i = 0; i < cols_count; i++) { 
    Serial.print(dict[i]->key); if(i == cols_count - 1) { continue; }
    Serial.print(" | ");
  }
  Serial.println();

  Serial.println("   Types:");
  Serial.print("      ");
  for (int i = 0; i < cols_count; i++) {
    Serial.print(GetTypeName(dict[i]->type)); if(i == cols_count - 1) { continue; }
    Serial.print(" | ");
  }
  Serial.println();
  
  Serial.println("   Values:");
  for (int i = 0; i < rows_count; i++) {
    Serial.print("      ");
    for (int j = 0; j < cols_count; j++) {     
      switch(dict[j]->type){
          case 's': Serial.print( ((char**)  dict[j]->values)[i] );       break;
          case 'f': Serial.print( ((float*)  dict[j]->values)[i] );       break;
          case 'L': Serial.print( ((int32_t*)dict[j]->values)[i]  , DEC); break;          
          case 'd': Serial.print( ((int16_t*)dict[j]->values)[i]  , DEC); break;
          case 'c': Serial.print( ((char*)   dict[j]->values)[i]  , DEC); break;
          case 'x': Serial.print( ((int32_t*)dict[j]->values)[i]  , HEX); break;
          case '-': Serial.print('-'); break;
      }
      if(j == cols_count - 1) { continue; }
      Serial.print(" | ");
    }
    Serial.println();
  }
}

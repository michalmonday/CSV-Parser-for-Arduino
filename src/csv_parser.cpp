#include "csv_parser.h"

int CountCharInStr(const char * s, const char c, int size_limit=0) {
  int count = 0;
  for (int i = 0; i < strlen(s); i++) {
    if (s[i] == c)
      count++;
      
    if (size_limit && i == size_limit)
      break;
  }
  return count;
}
  

CSV_Parser::CSV_Parser(const char * s, const char * fmt, bool has_header) {
  cols_count = CountCharInStr(s, ',', strcspn(s, "\n")) + 1;
  rows_count = CountCharInStr(s, '\n') - (s[strlen(s)-1] == '\n') + 1 - has_header; // exclude header if it's present
  byte dict_size = 0;
  
  dict = (Dict**)malloc(cols_count * sizeof(Dict*));

  Serial.println("Free heap (CSV_Parser constructor) = " + String(ESP.getFreeHeap())); 

  for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      char * key;
      int key_len = strcspn(s, "\n,");
      
      if (has_header) {
        key = strndup(s, key_len);
      } else {
        key = (char*)malloc(12);
        sprintf(key, "col_%d", header_index);
      }
      s += key_len + 1; 

      int single_value_size;
      switch (fmt[header_index]) {
        case 'L': { single_value_size = sizeof(long);  break; }
        case 'f': { single_value_size = sizeof(float); break; }
        case 's': { single_value_size = sizeof(char*); break; }         
        case 'd': { single_value_size = sizeof(int);   break; }
        case 'c': { single_value_size = sizeof(char);  break; }
        case 'x': { single_value_size = sizeof(long);  break; } // hex input is stored as long
        case '-': { single_value_size = 0;             break; }
        
        default : Serial.println("CSV_Parser, wrong fmt specifier = " + String(fmt[header_index])); break;
      }
           
      dict[dict_size] = new Dict(key, rows_count, fmt[header_index], single_value_size);
      dict_size++;
      free(key);
  }    

  for (int row = 0; row < rows_count; row++) {
    for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      int val_len = strcspn(s, "\n,");
      char * val = strndup(s, val_len);

      switch (fmt[header_index]) {
        case 'L': {
          *((long*)dict[header_index]->values + row) = atol(val);
          break;
          }
        case 'f': {
          *((float*)dict[header_index]->values + row) = (float)atof(val);
          break;
        }
        case 's': {
          *((char**)dict[header_index]->values + row) = strdup(val);
          break;
        }
        case 'd': {
          *((int*)dict[header_index]->values + row) = atoi(val);
          break;
        }
        case 'c': {
          *((char*)dict[header_index]->values + row) = (char)atoi(val);
          break;
        }
        case 'x': {
          *((long*)dict[header_index]->values + row) = strtol(val, 0, 16); // hex input is stored as long
          break;
        }
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
      case 'L': return "long";
      case 'f': return "float";
      case 's': return "char*";          
      case 'd': return "int";
      case 'c': return "char";
      case 'x': return "hex (long)"; // hex input, but it's stored as long
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
          case 'L': Serial.print( ((long*) dict[j]->values)[i]  , DEC); break;
          case 'f': Serial.print( ((float*)dict[j]->values)[i] );       break;
          case 's': Serial.print( ((char**)dict[j]->values)[i] );       break;          
          case 'd': Serial.print( ((int*)  dict[j]->values)[i]  , DEC); break;
          case 'c': Serial.print( ((char*) dict[j]->values)[i]  , DEC); break;
          case 'x': Serial.print( ((long*) dict[j]->values)[i]  , HEX); break;
          case '-': Serial.print('-'); break;
      }
      if(j == cols_count - 1) { continue; }
      Serial.print(" | ");
    }
    Serial.println();
  }
}

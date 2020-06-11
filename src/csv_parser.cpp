#include "csv_parser.h"

int CountCharInStr(const String &s, const char c) {
  int count = 0;
  for (const char &c2 : s)
    if (c2 == c)
      count++;
  return count;
}



  

CSV_Parser::CSV_Parser(String s, const char * fmt, bool has_header) 
  : 
  dict(0),
  dict_size(0)
{

  cols_count = CountCharInStr(s.substring(0, s.indexOf('\n')), ',') + 1;
  rows_count = CountCharInStr(s, '\n') - s.endsWith("\n") + 1 - has_header; // exclude header if it's present
  
  dict = (Dict**)malloc(cols_count * sizeof(Dict*));

  for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      
      String key;
      if (has_header) {
        int key_len = strcspn(s.c_str(), "\n,");
        key = s.substring(0, key_len);
        s.remove(0, key_len + 1); 
      } else {
        key = "col_" + String(header_index);
      }

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
  }    

  for (int row = 0; row < rows_count; row++) {
    for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      int val_len = strcspn(s.c_str(), "\n,");
      String val = s.substring(0, val_len);

      switch (fmt[header_index]) {
        case 'L': {
          *((long*)dict[header_index]->values + row) = atol(val.c_str());
          break;
          }
        case 'f': {
          *((float*)dict[header_index]->values + row) = val.toFloat();
          break;
        }
        case 's': {
          *((char**)dict[header_index]->values + row) = strdup(val.c_str());
          break;
        }
        case 'd': {
          *((int*)dict[header_index]->values + row) = val.toInt();
          break;
        }
        case 'c': {
          *((char*)dict[header_index]->values + row) = (char)val.toInt();
          break;
        }
        case 'x': {
          *((long*)dict[header_index]->values + row) = strtol(val.c_str(), 0, 16); // hex input is stored as long
          break;
        }
        case '-': break;
      }
      
      s.remove(0, val_len + 1);
    }
  }
}

CSV_Parser::~CSV_Parser() {
  for (int i = 0; i < dict_size; i++)
    delete dict[i];
  free(dict);
}

void CSV_Parser::PrintKeys() {
  Serial.println("Keys:");
  for (int i = 0; i < dict_size; i++) 
    Serial.println(String(i) + ". Key = " + String(dict[i]->key));
}

int CSV_Parser::GetColumnsCount() { return cols_count; }
int CSV_Parser::GetRowsCount() { return rows_count; } // excluding header

void * CSV_Parser::GetValues(const String & key) {
  for (int i = 0; i < dict_size; i++) 
    if (!strcmp(dict[i]->key, key.c_str()))
      return dict[i]->values;
  return (void*)0;
}

void * CSV_Parser::GetValues(int index)          { return index < dict_size ? dict[index]->values : (void*)0; }
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
  for (int i = 0; i < dict_size; i++) 
    ret += "    " + String(dict[i]->key) + " (" + GetTypeName(dict[i]->type) + ")\n"; 

  ret += "  rows number = " + String(rows_count);
  return ret;
}

void CSV_Parser::Print() {
  Serial.println("CSV_Parser content:");
  Serial.println("   Header:");
  Serial.print("      ");
  for (int i = 0; i < dict_size; i++) { 
    Serial.print(dict[i]->key); if(i == dict_size - 1) { continue; }
    Serial.print(" - ");
  }
  Serial.println();

  Serial.println("   Types:");
  Serial.print("      ");
  for (int i = 0; i < dict_size; i++) {
    Serial.print(GetTypeName(dict[i]->type)); if(i == dict_size - 1) { continue; }
    Serial.print(" - ");
  }
  Serial.println();
  
  Serial.println("   Values:");
  for (int i = 0; i < rows_count; i++) {
    Serial.print("      ");
    for (int j = 0; j < dict_size; j++) {     
      switch(dict[j]->type){
          case 'L': Serial.print( ((long*) dict[j]->values)[i]  , DEC); break;
          case 'f': Serial.print( ((float*)dict[j]->values)[i] );       break;
          case 's': Serial.print( ((char**)dict[j]->values)[i] );       break;          
          case 'd': Serial.print( ((int*)  dict[j]->values)[i]  , DEC); break;
          case 'c': Serial.print( ((char*) dict[j]->values)[i]  , DEC); break;
          case 'x': Serial.print( ((long*) dict[j]->values)[i]  , HEX); break;
          case '-': Serial.print('-'); break;
      }
      if(j == dict_size - 1) { continue; }
      Serial.print(" - ");
    }
    Serial.println();
  }
}

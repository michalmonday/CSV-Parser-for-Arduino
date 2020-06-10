#include "csv_parser.h"

int CountCharInStr(const String &s, const char c) {
  int count = 0;
  for (const char &c2 : s)
    if (c2 == c)
      count++;
  return count;
}

struct Dict {
  char * key;
  void * values;
  int values_count;
  char type;
  
  int header_index; // in csv file 
  
  Dict(String k, int vc, int t, int single_value_size, int hi) 
    :
      key(strdup(k.c_str())), 
      values(malloc((vc + 1) * single_value_size)),
      values_count(vc),
      type(t),
      header_index(hi)
    {
    }
  
  ~Dict() {
    free(key);
    if (type == 's') 
      for (int i = 0; i < values_count; i++)
        free(((char**)values)[i]);
    free(values);
  }
};

  

CSV_Parser::CSV_Parser(String s, const char * fmt) 
  : 
  dict(0),
  dict_size(0)
{

  cols_count = CountCharInStr(s.substring(0, s.indexOf('\n')), ',') + 1;
  rows_count = CountCharInStr(s, '\n') - s.endsWith("\n"); // excluding header

  for (int header_index = 0; header_index < strlen(fmt); header_index++) {
      int key_len = strcspn(s.c_str(), "\n,");
      String key = s.substring(0, key_len);
      s.remove(0, key_len + 1);

      int single_value_size;
      switch (fmt[header_index]) {
        case 'L': single_value_size = sizeof(long);  break;
        case 'f': single_value_size = sizeof(float); break;
        case 's': single_value_size = sizeof(char*); break;          
        
        default : Serial.println("CSV_Parser, wrong fmt specifier = " + String(fmt[header_index])); break;
      }
      
      dict = (Dict**)realloc(dict, (dict_size + 1) * sizeof(Dict*));
      dict[dict_size] = new Dict(key, rows_count, fmt[header_index], single_value_size, header_index);
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

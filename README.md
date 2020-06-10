# Motivation
I wanted to parse [covid-19 csv](https://github.com/tomwhite/covid-19-uk-data) data and couldn't find any csv parser for Arduino. So instead of rushing with a quick/dirty solution, I decided to write something that could be reused in future (possibly by other people too).  

# Things to consider  
CSV must:  
* contain header line
* be separated by comma

Programmer must:  
* know and specify what kind of values (long = L, float = f, string = s) are stored in each of the CSV columns
* cast the return of GetValues(key) appropriately, example:
    * long * values = (long*)csv_parser_obj.GetValues("csv_column_name");
		 
It does not support integers for now (longs only).  		 
		  
# Example
Example file to be parsed:     
> Column1,Column2,my_floats  
> hello,1,3.33  
> world,2,7.77   

The part of the code to parse it:  

```cpp
  String csv = "Column1,Column2,my_floats\nhello,1,3.33\nworld,2,7.77\n";
  CSV_Parser cp(csv, "sLf"); // where 2nd parameter specifies format (s = string, L = long, f = float), I prefer using long because in Arduino one "int" has only 16 bits
  char **col = (char**)cp["Column1"]; // cp.GetValues("Column1") also could be used
  long *col2 = (long*)cp["Column2"];
  float *my_floats = (float*)cp["my_floats"];
  
  if (col) 
    for (int i = 0; i < cp.GetRowsCount(); i++) 
      Serial.println(String(i) + ". Col 1 value = " + String(col[i]));
  
  if (col2)
    for (int i = 0; i < cp.GetRowsCount(); i++)
      Serial.println(String(i) + ". Col 2 value = " + String(col2[i]));  

  if (my_floats)
    for (int i = 0; i < cp.GetRowsCount(); i++)
      Serial.println(String(i) + ". Col 3 value = " + String(my_floats[i]));
```

Output:  
> 0. Col 1 value = hello  
> 1. Col 1 value = world  
> 0. Col 2 value = 1  
> 1. Col 2 value = 2  
> 0. Col 3 value = 3.33  
> 1. Col 3 value = 7.77  


(see "example_use.ino" in "src" folder for full example)  

# Tested with 
- Esp8266  

# Motivation
I wanted to parse [covid-19 csv](https://github.com/tomwhite/covid-19-uk-data) data and couldn't find any csv parser for Arduino. So instead of rushing with a quick/dirty solution, I decided to write something that could be reused in future (possibly by other people too).  

# Things to consider  
CSV must:  
* be separated by comma  

Programmer must:  
* know and specify what type of values are stored in each of the CSV columns  
* cast returned values appropriately  
		  
# How to specify value types 
By supplying format parameter to constructor of CSV_Parser. Example:
```cpp
CSV_Parser cp("my_strings,my_floats\nhello,1.1\nworld,2.2", "sf");
```

Example above is specifying "s" (string) for the 1st column, and "f" (float) for the 2nd column. Possible specifiers are:  
L - long (32-bit signed value)  
f - float  
s - string (C-like string, not a "String" Arduino object, just a char pointer, terminated by 0)  
d - int (16-bit signed value, can't be used for values over 65535)  
c - char (8-bit signed value, can't be used for values over 127)  
x - hex (stored as long)  
- - unused (this way memory won't be allocated for the values)  

# How to cast returned values appropriately
By casting to the corresponding format specifier. Let's suppose that we parse the following:
```cpp
CSV_Parser cp("my_strings,my_floats\nhello,1.1\nworld,2.2", "sf");
```

To cast/retrieve the values we can use:  
```cpp
CSV_Parser cp("my_strings,my_floats\nhello,1.1\nworld,2.2", "sf");
char  **strings = (char**)cp["my_strings"];
float *floats =   (float*)cp["my_floats"];
```

"x" (hex input values), should be cast as "long*" (or unsigned long*), because that's how they're stored. Casting them to "int*" would result in wrong address being computed when using `ints[index]`.  


# Example
Example file to be parsed:     
> my_strings,my_longs,my_ints,my_chars,my_floats,my_hex,my_to_be_ignored  
> hello,70000,140,10,3.33,FF0000,this_value_wont_be_stored  
> world,80000,150,20,7.77,0000FF,this_value_wont_be_stored  
> noice,90000,160,30,9.99,FFFFFF,this_value_wont_be_stored   

The part of the code to parse it:  

```cpp
static const String csv = "my_strings,my_longs,my_ints,my_chars,my_floats,my_hex,my_to_be_ignored\nhello,70000,140,10,3.33,FF0000,this_value_wont_be_stored\nworld,80000,150,20,7.77,0000FF,this_value_wont_be_stored\nnoice,90000,160,30,9.99,FFFFFF,this_value_wont_be_stored";
CSV_Parser cp(/*csv*/ csv, /*format*/ "sLdcfx-");
Serial.println(cp);

char  **strings =       (char**)cp["my_strings"];
long  *longs =          (long*) cp["my_longs"];
int   *ints =           (int*)  cp["my_ints"];
char  *chars =          (char*) cp["my_chars"];
float *floats =         (float*)cp["my_floats"];
long  *longs_from_hex = (long*) cp["my_hex"]; // CSV_Parser stores hex as longs (casting to int* would point to wrong address when ints[ind] is used)

for (int i = 0; i < cp.GetRowsCount(); i++) {
Serial.print(strings[i]);             Serial.print(" - ");
Serial.print(longs[i], DEC);          Serial.print(" - ");
Serial.print(ints[i], DEC);           Serial.print(" - ");
Serial.print(chars[i], DEC);          Serial.print(" - ");
Serial.print(floats[i]);              Serial.print(" - ");
Serial.print(longs_from_hex[i], HEX); Serial.println();
}
```

Output:  
> hello - 70000 - 140 - 10 - 3.33 - FF0000  
> world - 80000 - 150 - 20 - 7.77 - FF  
> noice - 90000 - 160 - 20 - 9.99 - FFFFFF   
  
  
# CSV files without header
To parse CSV files without header we can specify 3rd optional argument to the constructor. Example:  
```cpp
CSV_Parser cp(/*csv*/ csv, /*format*/ "---L", /*has_header*/ false);
```

And then we can use the following to get the extracted values:  
```cpp
long * longs = (long*)cp[3]; // 3 becuase L is at index 3 of "---L" format string
```

  
# Tested with 
- Esp8266  

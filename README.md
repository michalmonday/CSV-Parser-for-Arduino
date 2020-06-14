## Table of contents
* [What is CSV format](#what-is-csv-format)  
* [What is this CSV parser](#what-is-this-csv-parser)  
* [Motivation](#motivation)  
* [Usage](#usage)  
* [Things to consider](#things-to-consider)  
* [Specifying value types](#specifying-value-types)  
* [Casting returned values](#casting-returned-values)  
* [Headerless files](#headerless-files)  
* [Custom delimiter](#custom-delimiter)  
* [Checking if the file was parsed correctly](#checking-if-the-file-was-parsed-correctly)  
* [Testing](#testing)  
* [To do](#to-do)  


## What is CSV format
CSV means comma separated values. It's like a normal "txt" file with commas at regular places to separate some values.  
Typically the first line of CSV file is a "header", containing names of columns (this way any reader knows which column means what).  
Example CSV file with header and 2 columns:  

> Date,Temperature  
> 2020/06/12,20  
> 2020/06/13,22  
> 2020/06/14,21  

Using CSV format is one way of organising data, which makes it easy for programs to read.  

## What is this CSV parser
It's a class to which you can supply:  
- csv string (including new-line characters)  
- [format string](#specifying-value-types) (where each letter specifies type of value for each column)  

Class parses that string, in other words, it extracts values, stores them and provides you with:  
- easily accessible set of arrays (their types are specified by the format string)  


## Motivation
I wanted to parse [covid-19 csv](https://github.com/tomwhite/covid-19-uk-data) data and couldn't find any csv parser for Arduino. So instead of rushing with a quick/dirty solution, I decided to write something that could be reused in the future (possibly by other people too).  


## Usage
```cpp
char * csv_str = "my_strings,my_longs,my_ints,my_chars,my_floats,my_hex,my_to_be_ignored\n"
		 "hello,70000,140,10,3.33,FF0000,this_value_wont_be_stored\n"
		 "world,80000,150,20,7.77,0000FF,this_value_wont_be_stored\n"
		 "noice,90000,160,30,9.99,FFFFFF,this_value_wont_be_stored";

CSV_Parser cp(csv_str, /*format*/ "sLdcfx-");

char    **strings =         (char**)cp["my_strings"];
int32_t *longs =          (int32_t*)cp["my_longs"];
int16_t *ints =           (int16_t*)cp["my_ints"];
char    *chars =             (char*)cp["my_chars"];
float   *floats =           (float*)cp["my_floats"];
int32_t *longs_from_hex = (int32_t*)cp["my_hex"];    // CSV_Parser stores hex as longs (casting to int* would point to wrong address when ints[ind] is used)

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

Notice how each character within `"sLdcfx-"` string specifies different type for each column. It is very important to set this format right. 
We could set each solumn to be strings like "sssssss", however this would use more memory than it's really needed. If we wanted to store a large array of small numerical values (e.g. under 128), then using "c" specifier would be appropriate. See "How to specify value types" section for full list of available specifiers and their descriptions.  


## Things to consider  
If CSV file doesn't contain header line, then it must be specified as 3rd argument of the constructor (see [this example](#headerless-files))  
If CSV file is separated by other character instead of comma, then it must be specified as 4th argument of the constructor (see [this example](#custom-delimiter))  

Programmer must:  
* know and specify what type of values are stored in each of the CSV columns (see [this example](#specifying-value-types))  
* cast returned values appropriately (see [this example](#casting-returned-values))  
		  
The parser does not handle quotes. What it means? Let's assume we  have the following CSV file:  

> "field1","fie,ld2"  

Typically, any proper CSV parser would extract these as 2 fields:  
* field1  
* fie,ld2 
 
Notice that double-quotes would not be a part of extracted values. But this parser is simple, it would treat these as 3 fields:  
* "field1"  
* "fie  
* ld2"  

Double-quotes being part of extracted strings (that's what this parser would do).  

		  
## Specifying value types 
```cpp
char * csv_str = "my_strings,my_floats\n"
		 "hello,1.1\n"
		 "world,2.2";
		 
CSV_Parser cp(csv_str, /*format*/ "sf"); // s = string, f = float
```

Example above is specifying "s" (string) for the 1st column, and "f" (float) for the 2nd column.   

Possible specifiers are:   
**s** - string   (C-like string, not a "String" Arduino object, just a char pointer, terminated by 0)  
**f** - float  
**L** - int32_t  (32-bit signed value, can't be used for values over 2147483647)  
**d** - int16_t  (16-bit signed value, can't be used for values over 32767)  
**c** - char     (8-bit signed value, can't be used for values over 127)  
**x** - hex      (stored as int32_t)  
**-** (dash character) means that value is unused/not-parsed (this way memory won't be allocated for values from that column)  


## Casting returned values
Let's suppose that we parse the following:  
```cpp
char * csv_str = "my_strings,my_floats\n"
		 "hello,1.1\n"
		 "world,2.2";
		 
CSV_Parser cp(csv_str, /*format*/ "sf"); // s = string, f = float
```

To cast/retrieve the values we can use:  
```cpp
char  **strings = (char**)cp["my_strings"];
float *floats =   (float*)cp["my_floats"];
```

"x" (hex input values), should be cast as "long*" (or unsigned long*), because that's how they're stored. Casting them to "int*" would result in wrong address being computed when using `ints[index]`.  
  
  
## Headerless files
To parse CSV files without header we can specify 3rd optional argument to the constructor. Example:  
```cpp
CSV_Parser cp(csv_str, /*format*/ "---L", /*has_header*/ false);
```

And then we can use the following to get the extracted values:  
```cpp
long * longs = (long*)cp[3]; // 3 becuase L is at index 3 of "---L" format string
```


## Custom delimiter
Delimiter is 4th parameter of the constructor. It's comma (,) by default. We can customize it like this:  
```cpp
char * csv_str = "my_strings;my_floats\n"
		 "hello;1.1\n"
		 "world;2.2";
		 
CSV_Parser cp(csv_str, /*format*/ "sf", /*has_header*/ true, /*delimiter*/ ';');
```  


## Checking if the file was parsed correctly
Use CSV_Parser.Print function and check serial monitor. Example:  
```cpp
CSV_Parser cp(csv_str, /*format*/ "---L");
cp.Print();
```

It will display parsed header fields, their types and all the parsed values. Like this:  
> CSV_Parser content:  
>   Header:  
>      my_strings | my_longs | my_ints | my_chars | my_floats | my_hex | my_to_be_ignored  
>   Types:  
>      char* | long | int | char | float | hex (long) | unused  
>   Values:  
>      hello | 70000 | 140 | 10 | 3.33 | FF0000 | -   
>      world | 80000 | 150 | 20 | 7.77 | FF | -  
>      noice | 90000 | 160 | 30 | 9.99 | FFFFFF | -  

  
## Testing
It was tested with Esp8266.  
`ESP.getFreeHeap()` function was used to check for possible memory leaks and to examine memory space efficiency, see [this wiki page](https://github.com/michalmonday/Arduino-CSV-Parser/wiki/Memory-space-efficiency-test) for test details and results.  


## To do
Check how much memory the code/sketch takes.   
Handle double quotes.  
Handle carriage return characters (now it assumes only "\n" is used instead of "\r\n").  


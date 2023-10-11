/*  performance tests for: https://github.com/michalmonday/CSV-Parser-for-Arduino
*/
#include <CSV_Parser.h>

/*
This code uses "people-10000.csv" file (renamed to "file.csv") from:
https://github.com/datablist/sample-csv-files

(with the last column removed using Excel due to the lack of memory on Arduino Pro Mini to store the whole row + header)
 */

#include <SPI.h>
#include <SD.h>

// /file4.csv is the "/customers-100.csv" file from: https://github.com/datablist/sample-csv-files
// it was renamed to /file4.csv because I "SD.begin" failed in my case (possibly damaged SD card? Or a bug in SD library?)
const char * f_name = "/file5.csv";
const int chipSelect = 10;
File file;
char feedRowParser() {
  return file.read();
}
bool rowParserFinished() {
  return ((file.available()>0)?false:true);
}
 
void setup() {
  Serial.begin(115200);
  delay(5000);

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("ERROR: Card failed, or not present");
    while (1);
  }
  Serial.println("Card initialized.");


  if (!SD.exists(f_name)) {
    Serial.println("ERROR: File \"" + String(f_name) + "\" does not exist.");  
    while (1);
  }
  file = SD.open(f_name, FILE_READ);
  if (!file) {
    Serial.println("ERROR: File open failed");
    while (1);
  }

  // Line below failed on Arduino Pro Mini because there wasn't enough memory to store the header, and the whole single row.
  // "customers-100.csv" has rather large rows and Pro Mini only has 1KB SRAM.
  //CSV_Parser cp(/*format*/ "Lsssssssssss");

  CSV_Parser cp(/*format*/ "Lsssssss");

  // parseRow calls feedRowParser() continuously until it reads a 
  // full row or until the rowParserFinished() returns true
  int row_index = 0;

  // WARNING: String indexing can't be used here because the header was not supplied to the cp object yet.
  // int32_t *ids = (int32_t*)cp["Index"];
  // char **customer_ids = (char**)cp["Customer Id"];
  // char **emails = (char**)cp["Email"];

  int32_t *ids = (int32_t*)cp[0];
  char **customer_ids = (char**)cp[1];
  char **emails = (char**)cp[5];

  while (cp.parseRow()) {
    // Here we could use string indexing but it would be much slower.
    // char *email = ((char**)cp["Email"])[0];

    int32_t id = ids[0];
    char *customer_id = customer_ids[0];
    char *email = emails[0];

    Serial.print(String(row_index) + ". customer_id=" + String(customer_id));
    Serial.print(", id=");
    Serial.print(id, DEC);
    Serial.print(", email=");
    Serial.println(email);
    row_index++;
  }

  // Serial.println();
  // cp.print();
}

void loop() {

}

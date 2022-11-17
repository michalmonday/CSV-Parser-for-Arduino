#include <CSV_Parser.h>

void setup() {
  Serial.begin(115200);
  delay(5000);

  // two quote characters next to each other include one quote character into the parsed string itself
  static char buff[] = "'Ajatempel (UTC)';'Kuupev (Eesti aeg)';'NPS Eesti'\n"
                          "'1590883200';'31.05.2020''''03:00';'1.96'\n"
                          "'1590966000';'01.06.2020 02:00';'0.16'";
  CSV_Parser cp(buff, "Lsf", true, ';', '\'');
  cp.parseLeftover();
  cp.print();
  long *timestamps = (long *)cp[0];
  char ** dates = (char**)cp[1];
  float * prices = (float*)cp[2];
  
  for (int i = 0; i < cp.getRowsCount(); i++) {
        Serial.print(timestamps[i]);          Serial.print(" - ");
        Serial.print(dates[i]);             Serial.print(" - ");
        Serial.print(prices[i]);           Serial.print(" - ");
        Serial.println("");
    }

}

void loop() {
}
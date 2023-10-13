/*  HTTP request example for: https://github.com/michalmonday/CSV-Parser-for-Arduino  
    It is designed for Esp8266, and is based on the example from the Esp for Arduino: BasicHTTPSClient.ino
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>

#include <CSV_Parser.h>

const char *wifi_ssid = "TP-LINK_51159C";
const char *wifi_password = "p13377331p";

// const uint8_t fingerprint[20] = {0x40, 0xaf, 0x00, 0x6b, 0xec, 0x90, 0x22, 0x41, 0x8e, 0xa3, 0xad, 0xfa, 0x1a, 0xe8, 0x25, 0x41, 0x1d, 0x1a, 0x54, 0xb3};

ESP8266WiFiMulti WiFiMulti;

// This function is responsible for supplying characters to be parsed.
// csv_str may be replaced with SD card reading code or any other
// source of CSV text, like serial port or HTTP request
char feedRowParser() {
  return csv_str[csv_str_index++];
}

// This function must return true when the whole CSV file was supplied 
// to feedRowParser() function. It will make sure that cp.parseRow() 
// returns false when the end of CSV was reached.
bool rowParserFinished() {
  return csv_str[csv_str_index] == 0;
}

 

String http_get(const char *url) {
  std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

  //client->setFingerprint(fingerprint);
  // Or, if you happy to ignore the SSL certificate, then use the following line instead:
  client->setInsecure();

  HTTPClient https;

  // if (!(https.begin(*client, "https://raw.githubusercontent.com/michalmonday/files/master/sample.csv"))) {  // HTTPS
  if (!(https.begin(*client, url))) {  // HTTPS
    Serial.printf("[HTTPS] Unable to connect\n");
    https.end();
    return "";
  }

  // start connection and send HTTP header
  int httpCode = https.GET();

  // httpCode will be negative on error
  if (httpCode <= 0) {
    Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    https.end();
    return "";
  }

  // HTTP header has been send and Server response header has been handled
  Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

  if (!(httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY)) {
    https.end();
    return "";
  }
  String payload = https.getString();
  https.end();

  return payload;
}

void setup() {
  Serial.begin(115200);
  // Serial.setDebugOutput(true);
  delay(5000);

  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(wifi_ssid, wifi_password);

  // wait for WiFi connection
  while ((WiFiMulti.run() != WL_CONNECTED)) {
    Serial.println("Connecting to WiFi...");
    delay(1000);
  }

  String csv_str = http_get("https://media.githubusercontent.com/media/datablist/sample-csv-files/main/files/people/people-100.csv");

  CSV_Parser cp("Lssssssss");

/* CSV file:
Index,User Id,First Name,Last Name,Sex,Email,Phone,Date of birth,Job Title
1,88F7B33d2bcf9f5,Shelby,Terrell,Male,elijah57@example.net,001-084-906-7849x73518,1945-10-26,Games developer
2,f90cD3E76f1A9b9,Phillip,Summers,Female,bethany14@example.com,214.112.6044x4913,1910-03-24,Phytotherapist
3,DbeAb8CcdfeFC2c,Kristine,Travis,Male,bthompson@example.com,277.609.7938,1992-07-02,Homeopath
*/
  int32_t *ids = (int32_t *)cp[0];
  char **user_ids = (char **)cp[1];
  char **first_names = (char **)cp[2];
  char **last_names = (char **)cp[3];
  char **sexes = (char **)cp[4];
  char **emails = (char **)cp[5];
  char **phones = (char **)cp[6];
  char **dates_of_birth = (char **)cp[7];
  char **job_titles = (char **)cp[8];

  while(cp.parseRow()) {
 
  }
}

void loop() {
  Serial.println("Wait 10s before next round...");
  delay(10000);
}

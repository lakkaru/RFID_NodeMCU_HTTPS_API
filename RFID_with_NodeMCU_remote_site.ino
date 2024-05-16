/* -----------------------------------------------------------------------------
  - Project: RFID vehical enabling system using NodeMCU and external server
  - Author:  lakkaru
  - Date:  15/05/2024
   -----------------------------------------------------------------------------

   ---------------------------------------------------------------------------*/
//NodeMCU                MFRC522 Module
//   D2  ---------------  SDA/SS
//   D5  ---------------  SCK
//   D7  ---------------  MOSI
//   D6  ---------------  MISO
//   NC  ---------------  IRQ
//   GND ---------------  GND
//   D1  ---------------  RST
//   3V3 ---------------  VCC


   
//*******************************libraries********************************

//RFID-----------------------------
#include <SPI.h>
#include <MFRC522.h>
//**********************************

//NodeMCU--------------------------
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecureBearSSL.h>
//************************************************************************

#define RST_PIN 5   // GPIO5 (D5) connected to the RST pin of the MFRC522 module
#define SS_PIN 4    // GPIO4 (D4) connected to the SDA/SS pin of the MFRC522 module


//************************************************************************

MFRC522 mfrc522(SS_PIN, RST_PIN); // Create MFRC522 instance.
//************************************************************************

/* Desired credentials. */
const char *ssid = "Lakkaru Pre";
const char *password = "4T4F0H88EYB";

String API = "https://cdn.radikadilanka.com:9001/nodemcu/?"; //the server domain

//************************************************************************

String getData, Link;
String OldCardID = "";
unsigned long previousMillis = 0;


//************************************************************************

void setup() {
  delay(1000);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);
  SPI.begin();  // Init SPI bus
  mfrc522.PCD_Init(); // Init MFRC522 card
  //************************
  connectToWiFi();
  //************************
}

//************************************************************************

void loop() {
  //check if there's a connection to Wi-Fi or not
  if(!WiFi.isConnected()){
    connectToWiFi();    //Retry to connect to Wi-Fi
  }
  //************************

  // removing double tap
  if (millis() - previousMillis >= 15000) {
    previousMillis = millis();
    OldCardID="";
  }
  delay(50);
  //****************************************
  
  //look for new card
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return;//got to start of loop if there is no card present
  }
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return;//if read card serial(0) returns 1, the uid struct contians the ID of the read card.
  }
  
  String CardID ="";
  for (byte i = 0; i < mfrc522.uid.size; i++) {
    CardID += mfrc522.uid.uidByte[i];
  }
  //---------------------------------------------
  if( CardID == OldCardID ){
    return;
  }
  else{
    OldCardID = CardID;
  }
  //---------------------------------------------

  SendCardID(CardID);

  delay(1000);
}
//************send the Card UID to the API*************

void SendCardID( String Card_uid ){
  Serial.println("Sending the Card ID");
   // Create a WiFiClientSecure object
  BearSSL::WiFiClientSecure *client = new BearSSL::WiFiClientSecure;

  // Ignore SSL certificate verification (not recommended for production use)
  client->setInsecure();

  // Set up the HTTPS client
  HTTPClient https;
  Serial.println("[HTTPS] begin...");
  Serial.println("Sending data - "+ API + "CardID=" +  Card_uid);
  // Begin HTTPS request to the specified URL
  if (https.begin(*client, API + "CardID=" +  Card_uid)) {
    Serial.println("[HTTPS] GET...");

    // Send the GET request and check the response code
    int httpCode = https.GET();

    // Check if the GET request was successful
    if (httpCode > 0) {
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

      // Read response data
      String payload = https.getString();
      Serial.println(payload);

      if(payload == "start"){
        digitalWrite(LED_BUILTIN, LOW);
        Serial.println("LED on");
        delay(1000);  //Post Data at every 1 seconds
      } else if(payload == "stop"){
        digitalWrite(LED_BUILTIN, HIGH);
        Serial.println("LED off");
        delay(1000);  //Post Data at every 1 seconds
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }

    // End the HTTPS request
    https.end();
  } else {
    Serial.println("[HTTPS] Unable to connect");
  }

  // Clean up resources
  delete client;

  // Delay before next iteration
  delay(1000);  // Adjust delay as needed

  
  }
 
//********************connect to the WiFi******************
void connectToWiFi(){
    WiFi.mode(WIFI_OFF);        //Prevents reconnection issue (taking too long to connect)
    delay(1000);
    WiFi.mode(WIFI_STA);
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("");
    Serial.println("Wifi Connected");
  
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //IP address assigned to your ESP
    
    delay(1000);
}

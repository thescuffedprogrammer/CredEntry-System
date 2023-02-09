#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <WiFiClientSecureBearSSL.h>
#include <ThingSpeak.h> 
#include <Servo.h>


// Replace with your network credentials
const char* ssid = "JG10N+";
const char* password = "mtfw9470";

#define BOTtoken "5897425831:AAEMdNecOrUni4RNRRrLVgRK6FWqvO_DWKk"  // your Bot Token (Get from Botfather)
#define CHAT_ID "966299841"

X509List cert(TELEGRAM_CERTIFICATE_ROOT);
WiFiClientSecure client2;
UniversalTelegramBot bot(BOTtoken, client2);

WiFiClient  client1;

unsigned long counterChannelNumber = 2006951;                // Channel ID
const char * myCounterReadAPIKey = "914KZLE08FPVTQLX";      // Read API Key
const int FieldNumber1 = 1;                                 // The field you wish to read
uint8_t LEDpin = D6;
Servo s1; 

String presentValue = " ";
String changeValue = " ";

// Fingerprint (might need to be updated)
const uint8_t fingerprint[20] = {0x97, 0x80, 0xC2, 0x50, 0x78, 0x53, 0x2F, 0xC0, 0xFD, 0x03, 0xDA, 0xE0, 0x1B, 0xFD, 0x8C, 0x92, 0x3F, 0xFF, 0x98, 0x78};

void setup() {

  pinMode(LEDpin, OUTPUT);
  digitalWrite(LEDpin , LOW);
  s1.attach(0);  // servo attach D3 pin of arduino  
  Serial.begin(115200);
  //Serial.setDebugOutput(true);
  client2.setInsecure();
  configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
  client2.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  Serial.println();

  Serial.println();
  Serial.println();
  Serial.println();

  Serial.print("Connecting Wifi: ");
  Serial.println(ssid);

  //Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print('.');
    delay(1000);
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "Bot started up", "");
}

void loop() {

    ThingSpeak.begin(client1);
    String A = ThingSpeak.readStringField(counterChannelNumber, FieldNumber1, myCounterReadAPIKey);
    presentValue = A;
  // wait for WiFi connection
  if ((WiFi.status() == WL_CONNECTED)) {

    std::unique_ptr<BearSSL::WiFiClientSecure>client(new BearSSL::WiFiClientSecure);

    client->setFingerprint(fingerprint);
    // Or, if you happy to ignore the SSL certificate, then use the following line instead:
    // client->setInsecure();

    HTTPClient https;

    Serial.print("[HTTPS] begin...\n");
    if (https.begin(*client, "https://thingspeak.com/")) {  // HTTPS

      Serial.print("[HTTPS] GET...\n");
      // start connection and send HTTP header
      int httpCode = https.GET();

      // httpCode will be negative on error
      if (httpCode > 0) {
        // HTTP header has been send and Server response header has been handled
        Serial.printf("[HTTPS] GET... code: %d\n", httpCode);

        // file found at server
        if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
          if (presentValue != changeValue)
          {
            Serial.println(A);

            if (A == "1000")
            {
              digitalWrite(LEDpin , HIGH);
              s1.write(180);
            }

            else
            {
              digitalWrite(LEDpin , LOW);
              s1.write(0); 
            }

          }
        }
      } else {
        Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
      }

      https.end();
    } else {
      Serial.printf("[HTTPS] Unable to connect\n");
    }
  }

  Serial.println();
  Serial.println("Waiting 2min before the next round...");

  if (A=="1000")
  {
    bot.sendMessage(CHAT_ID, "Door is Open!!", "");
    Serial.println("Door is Open");
  }
  else
  {
    bot.sendMessage(CHAT_ID, "Door is Closed!!", "");
    Serial.println("Door is Closed");
  }
}
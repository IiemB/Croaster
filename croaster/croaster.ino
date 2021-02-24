#include <Arduino.h>
#include <ESP8266WiFi.h>      //import for wifi functionality
#include <WebSocketsServer.h> //import for websocket
#include <WiFiManager.h>
#include <Thermocouple.h>
#include <MAX6675_Thermocouple.h>

//--- LCD setup //
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 32

#define OLED_RESET -1 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#define NUMFLAKES 10 // Number of snowflakes in the animation example

#define LOGO_HEIGHT 16
#define LOGO_WIDTH 16
static const unsigned char PROGMEM logo_bmp[] =
    {B00000000, B11000000,
     B00000001, B11000000,
     B00000001, B11000000,
     B00000011, B11100000,
     B11110011, B11100000,
     B11111110, B11111000,
     B01111110, B11111111,
     B00110011, B10011111,
     B00011111, B11111100,
     B00001101, B01110000,
     B00011011, B10100000,
     B00111111, B11100000,
     B00111111, B11110000,
     B01111100, B11110000,
     B01110000, B01110000,
     B00000000, B00110000};

// LCD setup ---//

String json; //variable for json

#define SCK_PIN D8
#define CS_PIN D6
#define SO_PIN D7

#define SCK_PIN2 D8
#define CS_PIN2 D5
#define SO_PIN2 D7

Thermocouple *thermocouple_bt;
Thermocouple *thermocouple_et;

const int ledPin = LED_BUILTIN;
int ledState = LOW;

unsigned long prevMillis = 0;
long interval = 2000;

String title;

String command = "showip";
bool etState = true;
bool btState = true;

char etStr[10];
char btStr[10];

WebSocketsServer webSocket = WebSocketsServer(81); //websocket init with port 81

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  //webscket event method
  String cmd = "";
  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.println("Websocket is disconnected");
    //case when Websocket is disconnected
    break;
  case WStype_CONNECTED:
  {
    //wcase when websocket is connected
    Serial.println("Websocket is connected");
    Serial.println(webSocket.remoteIP(num).toString());
    webSocket.sendTXT(num, "connected");
  }
  break;
  case WStype_TEXT:
    cmd = "";
    for (int i = 0; i < length; i++)
    {
      cmd = cmd + (char)payload[i];
    } //merging payload to single string
    Serial.println(cmd);

    if (cmd == "readdata")
    { //when command from app is "poweron"
      //recieved command from mobile app
      //we can do task according to command from mobile using if-else-else if
    }

    if (cmd == "hideip")
    {
      command = "hideip";
    }

    if (cmd == "showip")
    {
      command = "showip";
    }

    if (cmd == "satudetik")
    {
      interval = 1000;
    }

    if (cmd == "duadetik")
    {
      interval = 2000;
    }
    
    webSocket.sendTXT(num, cmd + ":success");
    //send response to mobile, if command is "poweron" then response will be "poweron:success"
    //this response can be used to track down the success of command in mobile app.
    break;
  case WStype_FRAGMENT_TEXT_START:
    break;
  case WStype_FRAGMENT_BIN_START:
    break;
  case WStype_BIN:
    hexdump(payload, length);
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(115200); //serial start

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ; // Don't proceed, loop forever
  }
  else
  {
    Serial.println(F("SSD1306 allocation success"));
  }

  delay(500);

  display.display();

  delay(500);

  display.clearDisplay();

  display.drawPixel(10, 10, SSD1306_WHITE);
  display.display();

  delay(500);

  pinMode(ledPin, OUTPUT);

  thermocouple_bt = new MAX6675_Thermocouple(SCK_PIN, CS_PIN, SO_PIN);
  thermocouple_et = new MAX6675_Thermocouple(SCK_PIN2, CS_PIN2, SO_PIN2);

  Serial.println("Connecting to wifi");

  WiFiManager wifiManager;
  wifiManager.autoConnect("Croaster_00");

  while (WiFi.status() != WL_CONNECTED)
  {
    wifiSetup();
  }

  Serial.println("Wifi Connected...");

  Serial.println("");
  Serial.print("IP Address : ");
  Serial.println(WiFi.localIP());

  wifiConnected();

  webSocket.begin();                 //websocket Begin
  webSocket.onEvent(webSocketEvent); //set Event for websocket
  Serial.println("Websocket is started");
}

void loop()
{
  webSocket.loop();

  unsigned long currMillis = millis();

  if (currMillis - prevMillis >= interval)
  {
    prevMillis = currMillis;

    float bt = thermocouple_bt->readCelsius();
    float et = thermocouple_et->readCelsius();

    if (ledState == LOW)
    {
      ledState = HIGH;   
    }
    else
    {
      ledState = LOW;
    }

    Serial.println("Temp ET : " + String(et) + " | " + "Temp BT : " + String(bt));

    if (isnan(bt))
    {
      Serial.println(F("Failed to read BT!"));
      btState = false;
      return;
    }
    else
    {
      btState = true;
    }

    if (isnan(et))
    {
      Serial.println(F("Failed to read ET!"));
      etState = false;
      return;
    }
    else
    {
      etState = true;
    }

    title = "Croaster";

    sensorDisplay(String(et), String(bt));

    json = "{\"et\":" + String(et) + ",\"bt\":" + String(bt) + "}";

    Serial.println("Data read Successful");
    webSocket.broadcastTXT(json);

    Serial.println(json);
  }

  digitalWrite(ledPin, ledState);
}

void wifiConnected(void)
{
  display.display();

  delay(500);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("Wifi Connected..."));
  display.println(F("yeeeeey :D"));
  display.println("");
  display.println(WiFi.localIP());

  display.display();
  delay(3000);
}

void wifiSetup(void)
{
  display.display();

  delay(500);

  display.clearDisplay();

  display.setTextSize(2);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(12, 12);
  display.println(F("Croaster"));

  display.display();
  delay(3000);

  display.clearDisplay();

  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(0, 0);
  display.println(F("WiFi Setup"));
  display.println(F(""));
  display.println(F("Connect to SSID "));
  display.println(F("\"Croaster_00\""));

  display.display();
  delay(2000);
}

void sensorDisplay(String et, String bt)
{
  if (command == "hideip")
  {
    display.display();

    display.clearDisplay();

    display.setTextSize(2);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);

    if (etState == true)
    {
      display.println("ET:" + et + "C");
    }
    if (btState == true)
    {
      display.println("BT:" + bt + "C");
    }

    display.display();
  }

  if (command == "showip")
  {
    display.display();

    display.clearDisplay();

    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println(title);
    display.print("IP : ");
    display.println(WiFi.localIP());

    if (etState == true)
    {
      display.println("ET : " + et + " C");
    }
    if (btState == true)
    {
      display.println("BT : " + bt + " C");
    }

    display.display();
  }
}

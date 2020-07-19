#include <ESP8266WiFi.h>
#include <arduino.h>
#include "DataToMaker.h"

// https://www.instructables.com/id/4-WiFi-Door-Alarm-Using-a-ESP8266-IoT/
// https://github.com/mylob/ESP-To-IFTTT

// Notify Me
// by Thomptronics
// https://www.thomptronics.com/about/notify-me
// open the alexa app, link and enable permissions for alert me - beware of non Thomptronics apps with the same name!
// then say "Alexa, launch the Notify Me skill" to get the email sent with the key
// amzn1.ask.account.AEHOJQVAFBDUV45BV7WZJX2QQEIXL6ZMSQBYK4ORFTEACKUBPPGKWWK5S3OXN4RDY4HCD6IZ53ISIWGZ5CAOOK52AIKESV4WCH5SJBNB2RXJBU4PDYKMBPP2O255UJ237E2JK5I36PPMP3PSIEGBGUU24JNWF6IOOTLQ3SSQOZZXBAHJ5CO53DNJGLKA5J7OGFBRSQWPZN3WOFA
// {"notification": "Hello World!", "accessCode":"amzn1.ask.account.AEHOJQVAFBDUV45BV7WZJX2QQEIXL6ZMSQBYK4ORFTEACKUBPPGKWWK5S3OXN4RDY4HCD6IZ53ISIWGZ5CAOOK52AIKESV4WCH5SJBNB2RXJBU4PDYKMBPP2O255UJ237E2JK5I36PPMP3PSIEGBGUU24JNWF6IOOTLQ3SSQOZZXBAHJ5CO53DNJGLKA5J7OGFBRSQWPZN3WOFA"}
#define SERIAL_Serial.print // Uncomment this to dissable serial Serial.printging


// pin for heatbeat LED
#define HEARTBEAT_PIN 16
#define LIGHT_SENSOR_PIN A0
#define ADC_BITS_LED_OFF 200
#define ADC_BITS_LED_ON 600
#define MIN_TIME_BETWEEN_SENDS_MS 600000  // wait a minimum of 5 minutes between sends so we don't blow up notifications
#define DISABLE_ACTION_SENDING false

// Define program constants

// key: ifttt.com, Webhooks, settings, last part of url after "use/" (https://maker.ifttt.com/use/cX5O7zS6TVqov06EHOUSgb)
const char* myKey = "cX5O7zS6TVqov06EHOUSgb"; // your maker key here
const char* ssid = "KR_Hosman"; // your router ssid here
const char* password = "restfest"; // your router password here

// declare new maker event with the name "ESP"
DataToMaker event(myKey, "dryer_complete");

bool led_state = false;
bool action_sent = false;
unsigned long action_send_time = 0;
bool connectedToWiFI = false;
int analog_val = 0;

void setup()
{
#ifdef SERIAL_Serial.print
  Serial.begin(115200);
  delay(200);
  Serial.println();
#endif

  delay(10); // short delay
  pinMode(HEARTBEAT_PIN, OUTPUT);
  pinMode(LIGHT_SENSOR_PIN, INPUT);
  
  WiFi.mode(WIFI_STA);
  ConnectWifi();
}

void loop() {
  if (wifiConnected)
  {
    GetSensorVal();
    
    if (led_state)
    {
      if (action_sent == false)
      {
        if (action_send_time == 0 or (millis() - action_send_time) >= MIN_TIME_BETWEEN_SENDS_MS)
        {
          if (DISABLE_ACTION_SENDING == false)
          {
             Serial.println("connecting to IFTTT...");

            if (event.connect())
            {
              Serial.println("Connected To IFTTT");
              event.post();
              Serial.println("Sent dryer_complete to IFTTT");
              action_sent = true;
              action_send_time = millis();
            }
            else 
            {
              Serial.println("Failed To Connect To IFTTT!");
            }
          }
          else
          {
            Serial.println("Not sending action, DISABLE_ACTION_SENDING is true");
          }
        }

      }
    }
    delay(500);
  }
  else
  {
    delay(60 * 1000); // 1 minute delay before trying to re connect
    ConnectWifi();
    delay(250);
  }
  
  digitalWrite(HEARTBEAT_PIN, !digitalRead(HEARTBEAT_PIN));

  Serial.print("analog_val: ");
  Serial.print(analog_val);
  Serial.print(" wifiConnected: ");
  bool wifi_val = wifiConnected;
  Serial.print(wifi_val);
  Serial.print(" led_state: ");
  Serial.print(led_state);
  Serial.print(" action_sent: ");
  Serial.print(action_sent);
  Serial.print(" action_send_time: ");
  Serial.print(action_send_time);
  Serial.print(" millis(): ");
  Serial.print(millis()); 
  Serial.println();
}

bool ConnectWifi()
{
  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  unsigned long startTime = millis();
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED && startTime + 30 * 1000 >= millis()) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED)
  {
    Serial.println("");
    Serial.println("WiFi connected");
  }
  else
  {
    WiFi.disconnect();
    Serial.println("");
    Serial.println("WiFi Timed Out!");
  }
}

bool wifiConnected()
{
  return WiFi.status() == WL_CONNECTED;
}

bool GetSensorVal()
{
  analog_val = analogRead(LIGHT_SENSOR_PIN); 

  if (analog_val > ADC_BITS_LED_ON)
  {
    led_state = true;
  }
  else if (analog_val < ADC_BITS_LED_OFF)
  {
    led_state = false;
    action_sent = false;
  }
  
  return led_state;
}

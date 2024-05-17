/*****************************************************************************************************************************
**********************************    Author  : Ehab Magdy Abdullah                      *************************************
**********************************    Linkedin: https://www.linkedin.com/in/ehabmagdyy/  *************************************
**********************************    Youtube : https://www.youtube.com/@EhabMagdyy      *************************************
******************************************************************************************************************************/

#if defined(ESP32)
#include <WiFi.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#endif

#include <Firebase_ESP_Client.h>
#include "DHT.h"

/* 1. Define the WiFi credentials */
#define WIFI_SSID "Your_Wifi_SSID"
#define WIFI_PASSWORD "Your_Wifi_Password"

/* 2. Define the API Key */
#define API_KEY "API_KEY"

/* 3. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "USER_EMAIL"
#define USER_PASSWORD "USER_PASSWORD"

/* 4. Define the RTDB URL */
#define DATABASE_URL "DATABASE_URL" 

#define FLAME_SENSOR_PIN        34
#define DHT_SENSOR_PIN          32

/* I'm using DHT22, if you use DHT11 you can change DHT22 to DHT11 */
#define DHT_SENSOR_TYPE      DHT22
DHT dht(DHT_SENSOR_PIN, DHT_SENSOR_TYPE);

uint16_t flameSensor;
uint8_t humidity;
uint8_t temperature;

// Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;

void setup()
{
    dht.begin();
    
    Serial.begin(115200);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    Serial.print("Connecting to Wi-Fi");
    while (WiFi.status() != WL_CONNECTED)
    {
      Serial.print(".");
      delay(300);
    }
    Serial.println();
    Serial.print("Connected with IP: ");
    Serial.println(WiFi.localIP());
    Serial.println();
    /* Assign the api key (required) */
    config.api_key = API_KEY;
    /* Assign the user sign in credentials */
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    /* Assign the RTDB URL (required) */
    config.database_url = DATABASE_URL;
    // Comment or pass false value when WiFi reconnection will control by your code or third party library e.g. WiFiManager
    Firebase.reconnectNetwork(true);
    // Since v4.4.x, BearSSL engine was used, the SSL buffer need to be set.
    // Large data transmission may require larger RX buffer, otherwise connection issue or data read time out can be occurred.
    fbdo.setBSSLBufferSize(4096 /* Rx buffer size in bytes from 512 - 16384 */, 1024 /* Tx buffer size in bytes from 512 - 16384 */);
    // Limit the size of response payload to be collected in FirebaseData
    fbdo.setResponseSize(2048);
    Firebase.begin(&config, &auth);
    Firebase.setDoubleDigits(5);
    config.timeout.serverResponse = 10 * 1000;
}

void loop()
{
    flameSensor = analogRead(FLAME_SENSOR_PIN);
    humidity = dht.readHumidity();
    temperature = dht.readTemperature();

    flameSensor = map(flameSensor, 0, 4095, 100, 0);

    // Firebase.ready() should be called repeatedly to handle authentication tasks.
    if (Firebase.ready() && (millis() - sendDataPrevMillis > 1000 || sendDataPrevMillis == 0))
    {
        sendDataPrevMillis = millis();

        if(Firebase.RTDB.setInt(&fbdo, "/Home/Temperature", temperature)){
            Serial.print("Temp: ");
            Serial.print(temperature);
        }
        else{
            Serial.println("FAILED: " + fbdo.errorReason());
        }

        if(Firebase.RTDB.setInt(&fbdo, "/Home/Humidity", humidity)){
            Serial.print("C - Humidity: ");
            Serial.print(humidity);
        }
        else{
            Serial.println("FAILED: " + fbdo.errorReason());
        }

        if(Firebase.RTDB.setInt(&fbdo, "/Home/Flame", flameSensor)){
            Serial.print("% - Flame: ");
            Serial.print(flameSensor);
            Serial.print("%\n");
        }
        else{
            Serial.println("FAILED: " + fbdo.errorReason());
        }
    }
    delay(2000);
}

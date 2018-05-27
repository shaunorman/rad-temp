/**
 * RadWeather
 *
 * Records temperature and humidity and uploads them to:
 * Local MQTT server
 * Thingspeak (for historical purposes)
 * - removed, the mqtt server also has a listener which stores all
 *   events in the database
 *
 * @author Shaun Norman
 * @email shaun@speedfreaks.com.au
 */

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <RadConfig.h>
#include <RadLED.h>
#include <RadWifi.h>
#include <RadDevice.h>
#include <RadHelpers.h>

#define LED_BUILTIN 2
#define FLASH_ERROR_ON 1000
#define FLASH_ERROR_OFF 250

#define FLASH_SUCCESS_ON 250
#define FLASH_SUCCESS_OFF 250

RadConfig config = RadConfig();

DHT dht(D2, DHT22);

String device_id = "";

WiFiClient wclient;
PubSubClient client(config.mqtt_server, config.mqtt_port, wclient);


void setup() {
    pinMode(LED_BUILTIN, OUTPUT);
    // Make sure it's turned off on init. (High is low on the internal LED)
    digitalWrite(LED_BUILTIN, HIGH);

    Serial.begin(115200);

    Serial.println();
    Serial.println("===[ Welcome to RadWeather ]===");
    Serial.println("");
    device_id = get_device_id();
    Serial.printf("[INFO] Initializing %s\n", device_id.c_str());

    setup_wifi(device_id, config);
}

void loop() {
    LED_flash(LED_BUILTIN, 1, FLASH_SUCCESS_ON, 1000);
    if (!client.connected()) {
        mqtt_reconnect(client, device_id, config);
    }
    client.loop();

    float f_temp = dht.readTemperature();
    float f_humidity = dht.readHumidity();
    String s_temp = float_to_string(f_temp, 2);
    String s_humidity = float_to_string(f_humidity, 2);

    char buffer[120];
    sprintf(buffer, "{\"device_id\": \"%s\", \"temp\": %s, \"humidity\": %s}", device_id.c_str(), s_temp.c_str(), s_humidity.c_str());
    Serial.println("[MQTT] publishing to /temp: " + String(buffer));
    bool result = client.publish("/temp", buffer);
    Serial.println("[MQTT] /temp response: " + String(result));

    if (result) {
        LED_flash(LED_BUILTIN, 3, FLASH_SUCCESS_ON, FLASH_SUCCESS_OFF);
    }
    else {
        LED_flash(LED_BUILTIN, 3, FLASH_ERROR_ON, FLASH_ERROR_OFF);
    }

    delay(300000);
}

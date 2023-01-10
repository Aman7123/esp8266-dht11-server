#include <ESP8266WiFi.h>
#include <Arduino_JSON.h>
#include "DHT.h"
#define DHTPIN 5
#define DHTTYPE DHT11
#define SSID "ssid here"
#define PASS "password here"
#define NEWLINE "\r\n"

WiFiServer server(80);
DHT dht(DHTPIN, DHTTYPE);

void setup() {
    // init    
    Serial.begin(115200);
    delay(10); 
    Serial.println();

    Serial.printf("Connecting to %s \r\n", SSID);
    WiFi.begin(SSID, PASS);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("Connected to internet");

    server.begin(); // init the webserver
    dht.begin(); // initialize the sensor
    Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str());
}

void readTemps(String &status, String &msg) {
    // read humidity
    float humi  = dht.readHumidity();
    // read temperature as Celsius
    float tempC = dht.readTemperature();
    // read temperature as Fahrenheit
    float tempF = dht.readTemperature(true);
    // declare output var
    JSONVar outputObj;
    // check if any reads failed
    if (isnan(humi) || isnan(tempC) || isnan(tempF)) {
      outputObj["message"] = "Failed to read from DHT sensor!";
      status = "500 Internal Server Exception";
    } else {
      outputObj["humidity"] = humi;
      outputObj["tempC"] = tempC;
      outputObj["tempF"] = tempF;
      status = "200 OK";
    }
    msg = JSON.stringify(outputObj);
    
}

String prepareHtmlPage() {
    // init
    String htmlPage;    
    String status = "500 Internal Server Exception";
    String msg = "Error in prepping the response";
    // prep data
    readTemps(status, msg);
    Serial.println(status);
    Serial.println(msg);
    // build packet
    htmlPage.reserve(1024); // prevent ram fragmentation
    htmlPage = "HTTP/1.1 " + status + NEWLINE +
               "Content-Type: application/json" + NEWLINE
               "Connection: close" + NEWLINE + NEWLINE;
    htmlPage += msg;
    // return data
    return htmlPage;
}

void loop() {
    WiFiClient client = server.available();
    if (!client) return;
    
    Serial.println("new client");
    while (!client.available()) delay(1);

    String request = client.readStringUntil('\r');
    if (request.indexOf("/") != -1) client.println(prepareHtmlPage());

    client.flush();
    Serial.println("Client disonnected");
    Serial.println("");
}
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <WiFi.h>            // Include the WiFi library
#include <HTTPClient.h>    

const char* ca_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"LONG RSA KEY INFO HERE \n" \
"-----END CERTIFICATE-----\n";
 
 
const char* device_cert = \
"-----BEGIN CERTIFICATE-----\n" \
"LONG RSA KEY INFO HERE \n" \
"-----END CERTIFICATE-----\n";
 
 
const char* private_key = \
"-----BEGIN RSA PRIVATE KEY-----\n" \
"LONG RSA KEY INFO HERE \n" \
"-----END RSA PRIVATE KEY-----\n";

 
WiFiClientSecure net;
PubSubClient client(net);
 
void sendHttpToWLED(String url, String payloadStr) {
  HTTPClient http;
  http.begin(url);
  http.addHeader("Content-Type", "application/json");
 
  int httpResponseCode = http.POST(payloadStr);
 
  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
  }
 
  http.end();
}
 
void messageReceived(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (unsigned int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
 
  // Convert payload to String
  String payloadStr = String((char*)payload).substring(0, length);
 
  // Define the WLED HTTP API endpoint
  String url = "http://10.0.0.125/json/state";  // Replace with the actual IP of your WLED device
 
  // Send the received MQTT payload directly to the WLED device as an HTTP request
  sendHttpToWLED(url, payloadStr);
}
 
void setup() {
  Serial.begin(115200);
 
  WiFi.begin("--YOUR WIFI SSID--", "--YOUR WIFI PASSWORD--");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
 
  net.setCACert(ca_cert);
  net.setCertificate(device_cert);
  net.setPrivateKey(private_key);
 
  client.setServer("--YOUR AWS IOT ENDPOINT ADDRESS--", 8883);
  client.setCallback(messageReceived);
 
  if (client.connect("-- YOUR AWS IOT CLIENT ID --")) {
    Serial.println("Connected to AWS IoT");
    client.subscribe("-- YOUR AWS IOT TOPIC --");  // Subscribe to the topic where commands will be sent
  } else {
    Serial.print("AWS IoT connection failed, state: ");
    Serial.println(client.state());
  }
}
 
void loop() {
  if (!client.connected()) {
    Serial.println("Disconnected from AWS IoT. Attempting reconnection...");
    while (!client.connect("-- YOUR AWS IOT CLIENT ID --")) {
      delay(1000);
      Serial.print(".");
    }
    Serial.println("Reconnected to AWS IoT");
    client.subscribe("lights");
  }
 
  client.loop();  // Handle AWS IoT MQTT messages
}
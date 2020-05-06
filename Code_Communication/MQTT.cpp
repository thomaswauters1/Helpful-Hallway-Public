#include <Arduino.h> 
#include <WiFi.h>
#include <PubSubClient.h>
#define MQTTpubQos 0

const char* ssid = "telenet-8F4E5";       //replace with current WiFi name (eduroam)
const char* password = "CsbuJ5ft4snu";    //current WiFi password

const char* mqtt_server = "helpfulhallway.duckdns.org";   //Raspberrys address
const char* mqtt_topic = "HelpfulHallway";

WiFiClient espClient;
PubSubClient client(espClient);
const uint8_t* bssid = (const uint8_t*) "8c:3b:ad:27:75:fb";  //change to eduroam info 66:d1:54:54:48:d3
int chann = 1;                                                //channel 11

void setup() {
  Serial.begin(115200);
  setCpuFrequencyMhz(80);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password, chann, bssid); 
  while (WiFi.status() != WL_CONNECTED) {
    delay(5);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void loop() {

  while (!client.connected());
  client.loop();

  
    char testData[] = "";
    client.publish("test", testData);   //Server expects data to be in the following format:  HallwayID;MicValue;TrafficID (vb. 5;250;2)  (TW)
  
}

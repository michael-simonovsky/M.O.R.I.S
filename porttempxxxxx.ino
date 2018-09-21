



#include <ESP8266WiFi.h>
#include <dht.h>
#include <PubSubClient.h>

const char* ssid     = "N"; // Your ssid
const char* password = "A12345678"; // Your Password
const char* mqtt_server = "192.168.0.39";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

int pin = 2;
dht DHT;
char send_array[12];
int RT_value[2];
void setup() {
Serial.begin(115200);
delay(10);
//Serial.println();
DHT.read11(pin);
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
// Connect to WiFi network
WiFi.mode(WIFI_STA);
//Serial.println();
//Serial.println();
//Serial.print("Connecting to ");
///.println(ssid);

WiFi.begin(ssid, password);

while (WiFi.status() != WL_CONNECTED) {
delay(500);
//Serial.print(".");
}
//Serial.println("");
//.println("WiFi connected");

// Print the IP address
//Serial.println(WiFi.localIP());
}


void callback(char* topic, byte* payload, unsigned int length) {
 //// Serial.print("Message arrived [");
 // Serial.print(topic);
  //Serial.print("] ");
  for (int i = 0; i < length; i++) {
    //Serial.print((char)payload[i]);
  }
}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    if (client.connect("arduinoClient")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      //client.publish("temp", "hello world");
      // ... and resubscribe
      client.subscribe("temp1");
    } else {
      //Serial.print("failed, rc=");
      //Serial.print(client.state());
      //Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
  
void loop() {
   if (!client.connected()) {
    reconnect();
  }
  client.loop();
int a;
    int readData = DHT.read11(pin); // Reads the data from the sensor
    int RT_temp = DHT.temperature; // Gets the values of the temperature
    int RT_humi= DHT.humidity; // Gets the values of the humidity
    uint8_t change=RT_temp-RT_value[0];
    change=abs(change);
   if (change>1)
    {    
   //Serial.print("temp " );
   // Serial.println(RT_temp);
    RT_value[0]=RT_temp;
    a=3;
    a=a+(RT_temp*10);
    itoa(a,send_array,10);
   // Serial.println(send_array);
    client.publish("temp1",send_array);
   
    }
    change=RT_humi-RT_value[1];
    change=abs(change);
   if (change>1)
   {
      //Serial.print("humi ");
      //Serial.println(RT_humi);
      RT_value[1]=RT_humi;
      a=2;
      a=a+(RT_humi*10);
      itoa(a,send_array,10);   
      //Serial.println(send_array);
      client.publish("temp1",send_array);
    
    }
  delay(2000);
    client.publish("keep_alive","remote_sensor1_alive");
}

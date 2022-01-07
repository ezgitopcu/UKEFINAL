#include<Wire.h>
#include "ColorSens.cpp"
#include "bme280.cpp"
#include <WiFi.h>
#include <PubSubClient.h>


const char* ssid = "Bulana";
const char* password = "gel.gir0";
const char* mqtt_server = "54.224.57.65";
WiFiClient espClient;
PubSubClient client(espClient);

BME280 bme280;

TCS3200 ColorSensor;
color renk;


void setup() {
  Serial.begin(115200);
  Wire.begin();
  bme280.begin();
  ColorSensor.begin();


  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

}
void loop() {
  if (!client.connected()) {
    reconnect();
    if (client.connected()) {
      client.subscribe("group1/ntc");
      client.subscribe("group1/pulse");
      client.subscribe("group4/Gyro");
      client.subscribe("Group4/distance");
      client.subscribe("group5/Temp");
      client.subscribe("group5/flow");
      client.subscribe("group3/temp");
      client.subscribe("group3/humidity");
      client.subscribe("group3/color");
      
    }
  }
  client.loop();


  delay(500);
  // bme280.begin(4);
  raw_data values = bme280.Read();
  Serial.print("temp:  ");
  Serial.print(values.temp);
  Serial.print("  pres:  ");
  Serial.print(values.pres);
  Serial.print("  hum:  ");
  Serial.println(values.hum);
  renk = ColorSensor.ReadColors();
  Serial.print("kırmızı:  ");
  Serial.print(renk.r);
  Serial.print("  yeşil:  ");
  Serial.print(renk.g);
  Serial.print("  mavi:  ");
  Serial.println(renk.b);
  Serial.println(renk.Hex);
  TCS3200().CheckColors(renk);
  client.publish("group2/humidity", String(values.hum).c_str() );
  client.publish("group2/temp", String(values.temp).c_str() );
  client.publish("group2/pressure", String(values.pres).c_str() );
  client.publish("group2/color", renk.Hex.c_str() );
}

/****************************************************************/
void callback(char* topic, byte* message, unsigned int length) {
  String messageTemp;

  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }


  if (String(topic) == "group1/ntc") {
  Serial.println("group1/ntc: "+messageTemp);
  }
  else if (String(topic) == "group1/pulse") {
  Serial.println("group1/pulse: "+messageTemp);
  }
  else if (String(topic) == "group4/Gyro") {
  Serial.println("group4/Gyro: "+messageTemp);
  }
  else if (String(topic) == "Group4/distance") {
  Serial.println("Group4/distance: "+messageTemp);
  }
  else if (String(topic) == "group5/Temp") {
  Serial.println("group5/Temp: "+messageTemp);
  }
  else if (String(topic) == "group5/flow") {
  Serial.println("group5/flow: "+messageTemp);
  }
  else if (String(topic) == "group3/temp") {
  Serial.println("group3/temp: "+messageTemp);
  }
  else if (String(topic) == "group3/humidity") {
  Serial.println("group3/humidity: "+messageTemp);
  }
  else if (String(topic) == "group3/color") {
  Serial.println("group3/color: "+messageTemp);
  }
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}


void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("colors")) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>

int Soil = 11;
int LDR = 10;


int Relay = 5;
int Relay_2 = 6;
int Relay_3 = 4;


#define DHTPIN 9
#define DHTTYPE DHT22   
DHT dht(DHTPIN, DHTTYPE);
    
int Mode;
const char *ssid = "CEIT-IoT-Lab_Wi-Fi5"; 
const char *password = "1oTCEIT@2022";  
     
    
const char *mqtt_broker = "192.168.3.204"; 
const char *topic_control = "Pump/Control";

const char *topic_temp = "temp";
const char *topic_humi = "humi";
const char *topic_soil = "soil";
const char *topic_ldr = "ldr";

const char *topic_pump = "Pump/State";
const char *topic_fan = "Fan/State";
const char *topic_led = "LED/State";


const int mqtt_port = 1883;
WiFiClient espClient;
PubSubClient client(espClient);
    
void setup() {
//Sensor and Device    
  Serial.begin(115200);

  dht.begin();
  pinMode(Relay,OUTPUT);
  pinMode(Relay_2,OUTPUT);
  pinMode(Relay_3,OUTPUT);
  pinMode(Soil,INPUT);
  pinMode(LDR,INPUT);

  WiFi.begin(ssid, password);
  
  //WiFi and Mqtt connect.
  while (WiFi.status() != WL_CONNECTED) {
  delay(5000);
  Serial.println("Connecting to WiFi..");
  }
     
  Serial.println("Connected to the WiFi network");
     
  client.setServer(mqtt_broker, mqtt_port);
  client.setCallback(Callback);
     
  while (!client.connected()) {
    String client_id = "esp32-client-";
    client_id += String(WiFi.macAddress());
     
    Serial.printf("The client %s connects to mosquitto mqtt broker\n", client_id.c_str());
     
    if (client.connect(client_id.c_str())) {
      Serial.println("Public emqx mqtt broker connected");
     } else {
      Serial.print("failed with state ");
      Serial.print(client.state());
      delay(2000);
     }
  }
     
   client.publish(topic_control, "Connected");
   client.subscribe(topic_control);
}
    
    
void Callback(char *topic_control, byte *payload, unsigned int length) {
  String string;
     
  Serial.print(topic_control);
  for (int i = 0; i < length; i++) {
    string+=((char)payload[i]);  
      
    Serial.println(string);
    //Manual Mode
    if(Mode == 1){
      switch ((char)payload[i]){
        //Plumb
        case '1':
          digitalWrite(Relay,1);
          client.publish(topic_pump,String("Plumb is on").c_str());
          break;
        case '0':
          digitalWrite(Relay,0);
          client.publish(topic_pump,String("Plumb is off").c_str());
          break;
        //Fan
        case '2':
          digitalWrite(Relay_2,1);
          client.publish(topic_fan,String("Fan is on").c_str());
          break;
        case '3':
          digitalWrite(Relay_2,0);
          client.publish(topic_fan,String("Fan is off").c_str());
          break;
        //LED
        case '4':
          digitalWrite(Relay_3,1);
          client.publish(topic_led,String("LED is on").c_str());
          break;
        case '5':
          digitalWrite(Relay_3,0);
          client.publish(topic_led,String("LED is off").c_str());
          break;
        }
         }
    if((char) payload[i] == 'M'){
      Mode = 1;
    }
    if((char) payload[i] == 'A'){
      Mode = 0;
    }

    //  }
     Serial.println(string);
     Serial.println(" - - - - - - - - - - - -");
    }
}


void loop() {
  client.loop();

  int value = analogRead(Soil);
  value = map(value,4095,0,0,100);

  int value_1 = analogRead(LDR);
  value_1 = map(value_1,4095,0,0,100);
       
       
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  client.publish(topic_temp,String(t).c_str());
  client.publish(topic_humi,String(h).c_str());
  client.publish(topic_soil,String(value).c_str());
  client.publish(topic_ldr,String(value_1).c_str());

  Auto_Temperature(t,Mode);
  Auto_Soil(value,Mode);
  Auto_LDR(value_1,Mode);

  delay(2000);
}

void Auto_Temperature(float temp,int M){
  if(temp >25 and M==0){
    digitalWrite(Relay_2, HIGH);
    client.publish(topic_fan,String("Fan is on").c_str());
  }else if(temp <=25 and M==0){
    digitalWrite(Relay_2, LOW);
    client.publish(topic_fan,String("Fan is off").c_str());
  }
}

void Auto_Soil(int v,int M){
  if(v <=60 and M==0){
    digitalWrite(Relay, HIGH);
    client.publish(topic_pump,String("Plumb is on").c_str());
  }else if(v >=80 and M==0){
    digitalWrite(Relay, LOW);
    client.publish(topic_pump,String("Plumb is off").c_str());
  }
}
void Auto_LDR(int v_1,int M){
  if(v_1 <=10 and M==0){
    digitalWrite(Relay_3, HIGH);
    client.publish(topic_led,String("LED is on").c_str());
  }else if(v_1 >10 and M==0){
    digitalWrite(Relay_3, LOW);
    client.publish(topic_led,String("LED is off").c_str());
  }
}
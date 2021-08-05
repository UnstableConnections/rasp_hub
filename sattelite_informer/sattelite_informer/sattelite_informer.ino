#include <ESP8266WiFi.h>

// Replace with your network credentials
const char* ssid     = "UNSTABLE";
const char* password = "CONNECTIONS";

const long TYPE_REGISTER    = 0x0001;
const long TYPE_REGISTER_OK = 0x0002;
const long TYPE_SET_COEF    = 0x0003;
const long TYPE_SET_TASK    = 0x0004;
const long TYPE_GET_Q       = 0x0005;
const long TYPE_PRODUCED_Q  = 0x0006;
const long TYPE_INFORM_Q    = 0x0007;

const long STATE_INITED     = 0;
const long STATE_REGISTERED = 1;
const long STATE_CONFIGURED = 2;
const long STATE_TASK_SET   = 3;
const long STATE_NEED_GET_Q = 4;
const long STATE_INFORM     = 5;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long last_connect_time = 0;

// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

char msg_buff[128];

const int HUB_IDENTIFIER = 998;
unsigned int state = STATE_INITED;
float q_produced = 0;

const int CONNECTION_DELAY = 10000;

#define LED_PIN 2


WiFiClient client;

void setup() {
   pinMode( LED_PIN, OUTPUT);
       
   Serial.begin(115200);
  // Initialize the output variables as outputs

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  digitalWrite(LED_PIN, LOW);
  
}

void build_register_message() {
  unsigned short* msg_header = (unsigned short*)msg_buff;
  unsigned int* hub_id = (unsigned int*)(msg_buff+2);
  unsigned int* hub_state = (unsigned int*)(msg_buff+6);
  unsigned int* tsk_time = (unsigned int*)(msg_buff+10);
  char* msg_end = msg_buff+14;

  *msg_header = TYPE_REGISTER;
  *hub_id = HUB_IDENTIFIER;
  *hub_state = STATE_INFORM;
  *tsk_time = 0;
  *msg_end = '\0';

  client.write(msg_buff, 14);
}


void handle_answer() {
    int attempts = 0;
    if (!client.connected()) {
      return;
    }
    
    while(!client.available()) {
      Serial.println("Wait answer");
      delay(1000);
      attempts++;

      if(attempts > 5){
        Serial.println("No answer from main_hub disconnect");
        client.stop();
        return;
      }
    }

    last_connect_time = millis();
    client.read(msg_buff, 120);
    unsigned int msg_type  = *(unsigned short*)(msg_buff);
    Serial.println("Type:");
    Serial.println(msg_type);
    
    switch(msg_type) {      
      case TYPE_INFORM_Q:
        Serial.println("Get TYPE_INFORM_Q");
        q_produced = (*(unsigned int*)(msg_buff+2)) / 100.;
        Serial.print("Get Q: ");
        Serial.println(q_produced);                                      
        break; 
     
    }
}

void handle_network() {
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);
  
    if (!client.connected()) {
    //delay(1000);
    
    if (client.connect("192.168.0.1", 10000)) {
      Serial.println("Connected to main_hub");
      last_connect_time = millis();
      build_register_message();  
      handle_answer();    
    } else {
      Serial.println("Connection to main_hub failed");
    }
  } else {
    handle_answer();
  }
}



void loop() {
  currentTime = millis();
  bool need_connect = false;

  if(  currentTime - last_connect_time > CONNECTION_DELAY){
    need_connect = true;
  } 
  
 if(need_connect){
  Serial.println("Call handle_network");
  handle_network();
  // update LCD
 }

}

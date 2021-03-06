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

const long STATE_INITED     = 0;
const long STATE_REGISTERED = 1;
const long STATE_CONFIGURED = 2;
const long STATE_TASK_SET   = 3;
const long STATE_NEED_GET_Q = 4;


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long last_connect_time = 0;
// Last timer up time
unsigned long last_inc_time = millis();
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

char msg_buff[128];

const int HUB_IDENTIFIER = 0;
unsigned int state = STATE_INITED;
float coef = 0;
float t_work = 0;
unsigned int t_max = 0;
unsigned int q_produced = 0;
unsigned int task_time = 0;
unsigned int session_init = 0;
const int CONNECTION_DELAY = 40000;
unsigned int wifi_state = 0;

#define LED_PIN 2
#define BTN_PIN 12

WiFiClient client;

void setup() {
   pinMode( LED_PIN, OUTPUT);
   pinMode( BTN_PIN, INPUT);

   digitalWrite(LED_PIN, HIGH);
   
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
  wifi_state = 1;
  
}

void build_register_message() {
  unsigned short* msg_header = (unsigned short*)msg_buff;
  unsigned int* hub_id = (unsigned int*)(msg_buff+2);
  unsigned int* hub_state = (unsigned int*)(msg_buff+6);
  unsigned int* tsk_time = (unsigned int*)(msg_buff+10);
  char* msg_end = msg_buff+14;

  *msg_header = TYPE_REGISTER;
  *hub_id = HUB_IDENTIFIER;
  *hub_state = state;
  *tsk_time = task_time;
  *msg_end = '\0';

  client.write(msg_buff, 14);
}

void build_q_message() {
  unsigned short* msg_header = (unsigned short*)msg_buff;
  unsigned int* hub_id = (unsigned int*)(msg_buff+2);
  unsigned int* hub_state = (unsigned int*)(msg_buff+6);
  unsigned int* tsk_q = (unsigned int*)(msg_buff+10);
  char* msg_end = msg_buff+14;

  q_produced = t_work * coef * 100;

  *msg_header = TYPE_PRODUCED_Q;
  *hub_id = HUB_IDENTIFIER;
  *hub_state = state;
  *tsk_q = q_produced;
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
        if (WiFi.getPersistent() == true) WiFi.persistent(false);   //disable saving wifi config into SDK flash area
        WiFi.mode(WIFI_OFF); // off  WIFI
        WiFi.persistent(true);   //enable saving wifi config into SDK flash area
        wifi_state = 0;
        return;
      }
    }

    last_connect_time = millis();
    client.read(msg_buff, 120);
    unsigned int msg_type  = *(unsigned short*)(msg_buff);
    Serial.println("Type:");
    Serial.println(msg_type);
    
    switch(msg_type) {      
      case TYPE_REGISTER_OK:
        Serial.println("Get TYPE_REGISTER_OK");
        state = STATE_REGISTERED;
        build_register_message();
        Serial.println("Send new state");
        
        break;
      case TYPE_SET_COEF:
        Serial.println("Get TYPE_SET_COEF");
        coef = (*(unsigned int*)(msg_buff+2)) / 100.;
        Serial.print("Get COEF: ");
        Serial.println(coef);
                
        state = STATE_CONFIGURED;
        build_register_message();
        Serial.println("Send new state");        
        break; 
      case TYPE_SET_TASK:
        Serial.println("Get TYPE_SET_TASK");
        t_max = *(unsigned int*)(msg_buff+2);
        t_work = 0;
        task_time = *(unsigned int*)(msg_buff+6);
        Serial.print("Get t_max: ");
        Serial.println(t_max);
        Serial.print("Get task_time: ");
        Serial.println(task_time);
        
        state = STATE_TASK_SET;
        Serial.println("Task set, disconnecting");
        client.stop();
        if (WiFi.getPersistent() == true) WiFi.persistent(false);   //disable saving wifi config into SDK flash area
        WiFi.mode(WIFI_OFF); // off  WIFI
        WiFi.persistent(true);   //enable saving wifi config into SDK flash area
        wifi_state = 0;
        break;
      case TYPE_GET_Q:
        Serial.println("Get TYPE_GET_Q");
        state = STATE_NEED_GET_Q;
        session_init = 0;
        digitalWrite(LED_PIN, HIGH);                
        break;      
    }
}

void handle_network() {
    long rssi = WiFi.RSSI();
    Serial.print("RSSI:");
    Serial.println(rssi);

    if(!wifi_state) {
      WiFi.mode(WIFI_STA);
      WiFi.begin(ssid, password);
      wifi_state = 1;
      while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
      }
      // Print local IP address and start web server
      Serial.println("");
      Serial.println("WiFi connected.");
      Serial.println("IP address: ");
      Serial.println(WiFi.localIP());
    }
  
    if (!client.connected()) {
    //delay(1000);
    
    if (client.connect("192.168.0.1", 10000)) {
      Serial.println("Connected to main_hub");
      last_connect_time = millis();

      if(state == STATE_NEED_GET_Q){
        build_q_message();
      } else build_register_message();      
    } else {
      Serial.println("Connection to main_hub failed");
    }
  } else {
    handle_answer();
  }
}


void inc_timer() {
  unsigned long check_diff = currentTime - last_inc_time;
  
  if(state == STATE_TASK_SET && check_diff > 1000 ) { //once in a sec
    
    if(session_init && t_work < t_max){
      t_work = t_work + 1;
      Serial.print("t_work: ");
      Serial.println(t_work);
    }
    last_inc_time = millis();
  }
}


void loop() {
  currentTime = millis();
  unsigned int pin_state = digitalRead(BTN_PIN);

  if( pin_state == 1 ){
    session_init ^= 1;
     
    if(session_init) {
      digitalWrite(LED_PIN, LOW);
      Serial.println("Session init");
    } else {
      digitalWrite(LED_PIN, HIGH);
    }
    delay(500);
  }
  
  bool need_connect = false;
  switch(state){
    case STATE_TASK_SET:
      if(  currentTime - last_connect_time > CONNECTION_DELAY){
        need_connect = true;
      } else handle_answer();
    break;
    default:
      need_connect = true;
  }


 if(need_connect){
  Serial.println("Call handle_network");
  handle_network();
 }

 inc_timer();

}

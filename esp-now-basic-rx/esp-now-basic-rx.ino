/**
   ESPNOW - Basic communication - Slave
   Date: 26th September 2017
   Author: Arvind Ravulavaru <https://github.com/arvindr21>
   Purpose: ESPNow Communication between a Master ESP32 and a Slave ESP32
   Description: This sketch consists of the code for the Slave module.
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/

   << This Device Slave >>

   Flow: Master
   Step 1 : ESPNow Init on Master and set it in STA mode
   Step 2 : Start scanning for Slave ESP32 (we have added a prefix of `slave` to the SSID of slave for an easy setup)
   Step 3 : Once found, add Slave as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Master to Slave

   Flow: Slave
   Step 1 : ESPNow Init on Slave
   Step 2 : Update the SSID of Slave with a prefix of `slave`
   Step 3 : Set Slave in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Master and Slave have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Master and Slave.
         Any devices can act as master or salve.
*/

#include <esp_now.h>
#include <WiFi.h>

#define CHANNEL 1

#define PRINT_RX_METADATA false
#define PRINT_MSG_ONLY true

#include <ledObject.h>  // see https://github.com/PanGalacticTech/ledObject_library
#define PWM_PIN 2
#define LED_CH 0
#define PWM_FREQ 5000
#define PWM_RESO 8
fadeLED led(PWM_PIN, LED_CH, PWM_FREQ, PWM_RESO);  // Constructor for ESP Boards Includes (PIN, CHANNEL, PWM FREQUENCY, RESOLUTION)
#define INITIAL_BRIGHTNESS 50                      //defines a starting brightness for fadeLED object. byte value from 0 - 255 valid
#define MIN_BRIGHT 50
#define MAX_BRIGHT 255
#define TIME_MS 400

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char msg[32];
  int num;
  float data;
  bool flag;
} struct_message;

// Create a struct_message called myData
struct_message myData;




// Init ESP Now with fallback
void InitESPNow() {
  WiFi.disconnect();
  if (esp_now_init() == ESP_OK) {
    Serial.println("ESPNow Init Success");
  } else {
    Serial.println("ESPNow Init Failed");
    // Retry InitESPNow, add a counte and then restart?
    // InitESPNow();
    // or Simply Restart
    ESP.restart();
  }
}

// config AP SSID
void configDeviceAP() {
  const char *SSID = "moduleRx";
  bool result = WiFi.softAP(SSID, "Slave_1_Password", CHANNEL, 0);
  if (!result) {
    Serial.println("AP Config failed.");
  } else {
    Serial.println("AP Config Success. Broadcasting with AP: " + String(SSID));
    Serial.print("AP CHANNEL ");
    Serial.println(WiFi.channel());
  }
}

void setup() {
  Serial.begin(115200);
  Serial.println("ESPNow - Basic Receiver");
  led.setup(INITIAL_BRIGHTNESS);
  led.turnOn();  // Heartbeat LED
  led.startFading(MIN_BRIGHT, MAX_BRIGHT, TIME_MS);
  //Set device in AP mode to begin with
  WiFi.mode(WIFI_AP);
  // configure device AP mode
  configDeviceAP();
  // This is the mac address of the Slave in AP Mode
  Serial.print("AP MAC: ");
  Serial.println(WiFi.softAPmacAddress());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for recv CB to
  // get recv packer info.
  esp_now_register_recv_cb(OnDataRecv);
}

// callback when data is recv from Master
void OnDataRecv(const uint8_t *mac_addr, const uint8_t *incomingData, int data_len) {
  // print MAC address
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
#if PRINT_RX_METADATA == true
  Serial.print("\n\nLast Packet Recv from: ");
  Serial.println(macStr);
  Serial.println("Last Packet Recv Data: ");
#endif
  // parse data
  memcpy(&myData, incomingData, sizeof(myData));
  printMessage(myData);
#if PRINT_RX_METADATA == true
  Serial.print("Bytes Received: ");
  Serial.print(data_len);
#endif
}



void printMessage(struct_message messageData) {
#if PRINT_MSG_ONLY == true
  Serial.print(messageData.msg);  //  doesnt need to println
#else
  char messageBuffer[64];
  char floatBuffer[8];
  dtostrf(messageData.data, 4, 2, floatBuffer);
  sprintf(messageBuffer, "msg: %s, num: %i, data: %s, flag: %i", messageData.msg, messageData.num, floatBuffer, messageData.flag);
  Serial.println(messageBuffer);
#endif
}


bool flag = true;
void loop() {
  if (flag) {
    Serial.println("Waiting for ESPnow Data Packet..");
    flag = false;
  }
 // led.performFades();  // Heartbeat LED
}

/** 

# ESPnow - UART Sender

   Date: 17/05/2024
   Author: Imogen Wren <https://github.com/ImogenWren>
   Purpose: UART Transmission link using ESPnow
   Description: This sketch consists of the code for the Sender Module
   Resources: (A bit outdated)
   a. https://espressif.com/sites/default/files/documentation/esp-now_user_guide_en.pdf
   b. http://www.esploradores.com/practica-6-conexion-esp-now/
   c. https://randomnerdtutorials.com/esp-now-esp32-arduino-ide/


   << This Device Sender >>
moduleTx
   Flow: UART Sender (moduleTx)
   Step 1 : ESPNow Init on Sender and set it in STA mode
   Step 2 : Start scanning for Receiver ESP32 (we have added a prefix of `moduleRx` to the SSID of moduleRx for an easy setup)
   Step 3 : Once found, add moduleRx as peer
   Step 4 : Register for send callback
   Step 5 : Start Transmitting data from Sender to moduleRx

   Flow: Receiver (moduleRx)
   Step 1 : ESPNow Init on moduleRx
   Step 2 : Update the SSID of moduleRx with a prefix of `moduleRx`
   Step 3 : Set moduleRx in AP mode
   Step 4 : Register for receive callback and wait for data
   Step 5 : Once data arrives, print it in the serial monitor

   Note: Sender and Receiver have been defined to easily understand the setup.
         Based on the ESPNOW API, there is no concept of Sender and Receiver.
         Any devices can act as moduleRx or moduleTx.
*/

/*
As always with ESP32 - Follow RandomNerdTutorials if you want something to work!

ESP-NOW supports the following features:

    - Encrypted and unencrypted unicast communication;
    - Mixed encrypted and unencrypted peer devices;
    - Up to 250-byte payload can be carried;
    - Sending callback function that can be set to inform the application layer of transmission success or failure.

ESP-NOW technology also has the following limitations:

    - Limited encrypted peers.
    - 10 encrypted peers at the most are supported in Station mode;
    - 6 at the most in SoftAP or SoftAP + Station mode;
    - Multiple unencrypted peers are supported, however, their total number should be less than 20, including encrypted peers;
    - Payload is limited to 250 bytes. 

*/

/* ESP32 UART

- https://www.luisllamas.es/en/esp32-uart/
- https://github.com/PanGalacticTech/boxthatscreams - Previous project that incorperated a serial message repeater





*/


//RTC_DATA_ATTR int bootCount = 0;  a way to store data through shutdowns? RESEARCH THIS!

#include <esp_now.h>
#include <WiFi.h>
#include <esp_wifi.h>  // only for esp_wifi_set_channel()



// Global copy of peer data for Receiver
esp_now_peer_info_t moduleRx;
#define CHANNEL 1
#define PRINT_SCAN_SUMMARY false
#define PRINTSCANRESULTS false
#define PRINT_MODULEFOUND false

#define DELETEBEFOREPAIR 0

#define PRINT_TX_STATS false
#define PRINT_REMOTE_STATUS false

#include "esp-wireless.h"

#define STRUCT_MSG_SIZE 128
#define UART_MSG_SIZE 128

// Structure example to send data
// Must match the receiver structure
typedef struct struct_message {
  char msg[STRUCT_MSG_SIZE];
  int num;
  float data;
  bool flag;
} struct_message;


// Create a struct_message called myData
struct_message myData;



uint8_t data = 0;
bool flag = false;



#include <HardwareSerial.h>

HardwareSerial mySerial(0);  // define a Serial for UART1
const int MySerialRX = 3;
const int MySerialTX = 1;


char inputString[STRUCT_MSG_SIZE];  // specify max length of 32 chars? bytes?
char overflowBuffer[STRUCT_MSG_SIZE];
bool stringComplete;

byte byteCount = 0;
byte strLength;


/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/


bool bufferOverflow = false;

void serialEvent() {
  // here we could use our MySerial normally
  while (mySerial.available() > 0) {
    // uint8_t byteFromSerial = MySerial.read();
    char inChar = (char)Serial.read();
    //  Serial.print(inChar);
    //  Serial.print("  byteCount: ");
    //  Serial.println(byteCount);
    // add it to the inputString:
    if (byteCount <= STRUCT_MSG_SIZE) {
      inputString[byteCount] = inChar;       // Compiler has issue with this line
      byteCount++;  // if the incoming character is a newline, set a flag so the main loop can
      strLength = byteCount;
    } else {
      Serial.print("UART Message Exceeds Buffer Size, filling overflow ");
      overflowBuffer[byteCount = STRUCT_MSG_SIZE];
    }

    // do something about it:
    if (inChar == '\n' or byteCount >= UART_MSG_SIZE) {  // if null character reached or buffer is filled, then string is completed
      stringComplete = true;
      byteCount = 0;
      //  Serial.println("\n\nUART Data Received: ");
      Serial.print(inputString);
    }
  }
}



// send data
void sendData() {

  if (stringComplete) {
    // Serial.println(inputString);


    // Package data as required
    // char dataBuffer[32];
    // sprintf(dataBuffer, "Data: %2i, %2i ", data, data + 1);
    strcpy(myData.msg, inputString);
    myData.num = 3;
    myData.data = 2;
    myData.flag = flag;



    // send data
    const uint8_t *peer_addr = moduleRx.peer_addr;
    Serial.print("Sending: ");
    Serial.println(myData.msg);
    esp_err_t result = esp_now_send(peer_addr, (uint8_t *)&myData, sizeof(myData));

    // Analise result
    Serial.print("Send Status: ");
    if (result == ESP_OK) {
      Serial.println("Success");
    } else if (result == ESP_ERR_ESPNOW_NOT_INIT) {
      // How did we get so far!!
      Serial.println("ESPNOW not Init.");
    } else if (result == ESP_ERR_ESPNOW_ARG) {
      Serial.println("Invalid Argument");
    } else if (result == ESP_ERR_ESPNOW_INTERNAL) {
      Serial.println("Internal Error");
    } else if (result == ESP_ERR_ESPNOW_NO_MEM) {
      Serial.println("ESP_ERR_ESPNOW_NO_MEM");
    } else if (result == ESP_ERR_ESPNOW_NOT_FOUND) {
      Serial.println("Peer not found.");
    } else {
      Serial.println("Not sure what happened");
    }
    stringComplete = false;
  }
}



// callback when data is sent from moduleTx to moduleRx
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
#if PRINT_TX_STATS == true
  char macStr[18];
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("Last Packet Sent to: ");
  Serial.println(macStr);
  Serial.print("Last Packet Send Status: ");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
#endif
}



void setup() {
  Serial.begin(115200);
  Serial.print("ESPnow - UART sender Starting");
  //Set device in STA mode to begin with
  WiFi.mode(WIFI_STA);
  esp_wifi_set_channel(CHANNEL, WIFI_SECOND_CHAN_NONE);
  Serial.println("ESPNow/Basic/moduleTx Example");
  // This is the mac address of the moduleTx in Station Mode
  Serial.print("STA MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("STA CHANNEL ");
  Serial.println(WiFi.channel());
  // Init ESPNow with a fallback logic
  InitESPNow();
  // Once ESPNow is successfully Init, we will register for Send CB to
  // get the status of Trasnmitted packet
  esp_now_register_send_cb(OnDataSent);
  // Setup serial port to listen for incoming data
  mySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
}



void loop() {
  // In the loop we scan for moduleRx
  ScanForRx();
  // If moduleRx is found, it would be populate in `moduleRx` variable
  // We will check if `moduleRx` is defined and then we proceed further
  if (moduleRx.channel == CHANNEL) {  // check if moduleRx channel is defined
    // `moduleRx` is defined
    // Add moduleRx as peer if it has not been added already
    bool isPaired = manageRx();
    if (isPaired) {
      // pair success or already paired
      // Send data to device
      sendData();
    } else {
      // moduleRx pair failed
      Serial.println("moduleRx pair failed!");
    }
  } else {
    // No moduleRx found to process
  }

  // wait for 3seconds to run the logic again
  // delay(3000);
}

//===================
/* Using Serial to send and receive structs between two Arduinos
 
 Note: Keep struct size down to 32 bits as this is the size of the serial buffer


*/


#include <HardwareSerial.h>
#include <autoDelay.h>


#define PRINT_SENT_DATA true

HardwareSerial mySerial(1);  // define a Serial for UART1
const int MySerialRX = 16;
const int MySerialTX = 17;

// data to be sent
struct TxStruct {
  char msg[16];      // 16 bytes
  uint8_t val_0;     //  1
  uint8_t val_1;     //  1
  uint8_t val_2;     //  1
  uint8_t val_3;     //  1
  byte padding[10];  // 10
                     //------
                     // 20
};

TxStruct txData = { "xxx", 5, 6, 7, 8,0};

bool newTxData = false;

const byte startMarker = 255;
const byte txDataLen = sizeof(txData);

// timing variables for sending frequency
unsigned long prevUpdateTime = 0;
unsigned long updateInterval = 500;


autoDelay sampleDelay;
#define SAMPLE_DELAY_mS 1000

//=================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n Starting SerialStructSend \n");

  // set up Serial for communication
  //   sendSerial.begin(9600); // to match SoftwareSerial on Uno
  // Setup serial port to listen for incoming data
  mySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
}

//============

void loop() {

  // this function updates the data in txData
  updateDataToSend();
  // this function sends the data if one is ready to be sent
  transmitData();
}

//============

void updateDataToSend() {

  if (sampleDelay.millisDelay(SAMPLE_DELAY_mS)) {
    if (newTxData == false) {  // ensure previous message has been sent
      char sText[] = "SendA";
      strcpy(txData.msg, sText);
      txData.val_0 = 1;
      txData.val_1 = 2;
      txData.val_2 = 3;
      txData.val_3 = 4;
      //TODO check data to make sure it fits in variables
      newTxData = true;
    }
  }
}

//============



void transmitData() {

  if (newTxData == true) {
    mySerial.write(startMarker);
    mySerial.write((byte*)&txData, txDataLen);

    // for demo show the data that as been sent
#if PRINT_SENT_DATA == true
    Serial.print("Sent: ");
    Serial.print(txData.msg);
    Serial.print(' ');
    Serial.print(txData.val_0);
    Serial.print(' ');
    Serial.print(txData.val_1);
    Serial.print(' ');
    Serial.print(txData.val_2);
    Serial.print(' ');
    Serial.print(txData.val_3);
    Serial.println("");
#endif

    newTxData = false;
  }
}
//===================
// Using Serial to send and receive structs between two Arduinos
//      the Tx is sending at 500msec intervals
//      it is assumed that this Rx program can loop in less than half that time

//===================


        // data to be received

struct RxStruct {
  char msg[16];      // 16 bytes
  uint8_t val_0;     //  1
  uint8_t val_1;     //  1
  uint8_t val_2;     //  1
  uint8_t val_3;     //  1
  byte padding[10];  // 10
                     //------
                     // 20
};

RxStruct rxData;

bool newData = false;

const byte startMarker = 255;
const byte rxDataLen = sizeof(rxData);

//#include <SoftwareSerial.h>
//SoftwareSerial recvSerial(6,7); // Rx and Tx


//=================================

void setup() {
    Serial.begin(115200);
    Serial.println("\nStarting SerialStructReceive \n");

        // set up Serial for communication
  //  recvSerial.begin(9600);
}

//============

void loop() {

    recvStructWithKnownLength();
        // this bit checks if a message has been received
    if (newData == true) {
        showNewData();
        newData = false;
    }
}

//=============

void showNewData() {

    Serial.print("This just in:    ");
    Serial.print(rxData.msg);
    Serial.print(' ');
    Serial.print(rxData.val_0);
    Serial.print(' ');
    Serial.print(rxData.val_1);
    Serial.print(' ');
    Serial.print(rxData.val_2);
    Serial.print(' ');
    Serial.print(rxData.val_3);
    Serial.println("");

}

//============

void recvStructWithKnownLength() {
    static byte recvData[rxDataLen];
    byte rb;
    byte * structStart;
    structStart = reinterpret_cast <byte *> (&rxData);
    if (Serial.available() >= rxDataLen + 1 and newData == false) {
        rb = Serial.read();

        if (rb == startMarker) {
                // copy the bytes to the struct
            for (byte n = 0; n < rxDataLen; n++) {
                *(structStart + n) = Serial.read();
            }
                // make sure there is no garbage left in the buffer
            while (Serial.available() > 0) {
                byte dumpTheData = Serial.read();
            }
            newData = true;
        }
    }
}

//============
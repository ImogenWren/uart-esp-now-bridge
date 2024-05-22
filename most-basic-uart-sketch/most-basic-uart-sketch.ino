#include <HardwareSerial.h>

HardwareSerial mySerial(0);  // define a Serial for UART1
const int MySerialRX = 16;
const int MySerialTX = 17;

#define STRUCT_MSG_SIZE 150
#define UART_MSG_SIZE 150

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



void trimCharArray(char *inArray) {
  char nullChar[2] = "\0";             // This must be 2 char long
  int index = strcspn(inArray, "\n");  // Find the location of the newline char
  inArray[index] = *nullChar;
}


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
      inputString[byteCount] = inChar;  // Compiler has issue with this line
      byteCount++;                      // if the incoming character is a newline, set a flag so the main loop can
      strLength = byteCount;
    } else {
      Serial.print("ESP32: UART Message Exceeds Buffer Size, filling overflow ");
      overflowBuffer[byteCount = STRUCT_MSG_SIZE];
    }

    // do something about it:
    if (inChar == '\n' or byteCount >= UART_MSG_SIZE) {  // if null character reached or buffer is filled, then string is completed
      stringComplete = true;
      byteCount = 0;
      //  Serial.println("\n\nUART Data Received: ");
      trimCharArray(inputString);  // added 22/05/2024
#if PRINT_UART_RX == true
      Serial.println(inputString);
#endif
    }
  }
}







void setup() {
  Serial.begin(115200);
  Serial.print("ESPnow - UART sender Starting");
 mySerial.begin(115200, SERIAL_8N1, MySerialRX, MySerialTX);
}


void loop(){

}
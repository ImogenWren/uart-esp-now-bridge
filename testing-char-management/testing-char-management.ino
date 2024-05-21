/*

Testing a small problem to remove the newline char from the middle of a string and ignore the rest of the string while printing


- Found Solution here?
https://www.geeksforgeeks.org/strcspn-in-c/?ref=ml_lbp


- Not that relevent to this problem but seemed useful
https://engineerworkshop.com/blog/pointers-arrays-and-functions-in-arduino-c/

Now this test is working, incorperating function into ESPnow RX

*/


/*

int val = 15;

reference to memory position of val = &val

Dereference this pointer with int = *val

*/


char testString[16] = { "Test \n String" };

char test2string[16];





void trimCharArray(char *inArray) {
  char nullChar[2] = "\0";              // This must be 2 char long
  int index = strcspn(inArray, "\n");  // Find the location of the newline char
  inArray[index] = *nullChar;
}

void demoTrim() {
//  int index = strcspn(testString, "\n");  // Find the location of the newline char
//  Serial.println(index);
  // testString[index] = NULL;    // worked but compiler didnt like
//  testString[index] = *nullChar;  // Worked with zero compuler error
}

void setup() {
  Serial.begin(115200);
  Serial.println(testString);
  trimCharArray(testString);
  Serial.println(testString);
}

void loop() {
  // put your main code here, to run repeatedly:
}

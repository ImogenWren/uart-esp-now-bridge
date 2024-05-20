
char testString[16] = { "Test \n String" };

char test2string[16];

void setup() {
  Serial.begin(115200);
  Serial.println(testString);
  char pointer = strchr(testString,"\n");   // also try strchr
  Serial.println(pointer);

}

void loop() {
  // put your main code here, to run repeatedly:
}

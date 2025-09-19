#include <SoftwareSerial.h>
SoftwareSerial mySerial(4, 3); // 4=RX, 3=TX (we won't TX)
void setup() {
  Serial.begin(9600);
  mySerial.begin(9600);
}
void loop() {
  if (mySerial.available()) {
    int r = mySerial.read();
    Serial.print("0x");
    Serial.println(r, HEX); // show hex so you can spot patterns
  }
}
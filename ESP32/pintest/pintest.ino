const int pin = 39;

void setup() {
  pinMode(pin, OUTPUT);
  Serial.begin(115200);
}

void loop() {
  digitalWrite(pin, HIGH);
  Serial.println("HIGH");
  delay(2000);
  digitalWrite(pin, LOW);
  Serial.println("LOW");
  delay(2000);
}

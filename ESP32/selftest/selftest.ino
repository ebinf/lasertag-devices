#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <IRutils.h>

#define TRIGG 18
#define IROUT 4
#define IR1IN 19
#define IR2IN 35
//#define IR3IN
//#define IR4IN
#define LED1R 23
#define LED1G 22
#define LED1B 21
#define LED2R 0
#define LED2G 2
#define LED2B 15
#define LED3R 32
#define LED3G 33
#define LED3B 25
#define LED4R 14
#define LED4G 12
#define LED4B 13

#define SLEEP_TIME 5000

void fade(const int pin, const bool dodown = true, const bool doup = true) {
  ledcAttachPin(pin, 0);
  unsigned int fader;
  if (dodown && !doup) {
    fader = 255;
  } else {
    fader = 0;
  }
  bool down = !doup;
  while (true) {
    if (down) {
      fader--;
    } else {
      fader++;
    }
    ledcWrite(0, (8191/255) * fader);
    if (fader == 255) {
      down = true;
      if (!dodown) {
        return;
      }
    }
    if (fader == 0) {
      ledcDetachPin(pin);
      return;
    }
    delay(3);
  }
}

void setup() {
  ledcSetup(0, 5000, 13);
  pinMode(TRIGG, INPUT_PULLUP);
  pinMode(IROUT, OUTPUT);
  pinMode(IR1IN, INPUT);
  pinMode(IR2IN, INPUT);
  pinMode(LED1R, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1B, OUTPUT);
  pinMode(LED2R, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2B, OUTPUT);
  pinMode(LED3R, OUTPUT);
  pinMode(LED3G, OUTPUT);
  pinMode(LED3B, OUTPUT);
  pinMode(LED4R, OUTPUT);
  pinMode(LED4G, OUTPUT);
  pinMode(LED4B, OUTPUT);
  
  Serial.begin(115200);
  decode_results results;
  ledcAttachPin(LED1R, 0);
  ledcAttachPin(LED2R, 0);
  ledcAttachPin(LED3R, 0);
  fade(LED4R, false);
  Serial.println("=== BEGIN SELFTEST ===");
  uint64_t chipid = ESP.getEfuseMac();
  Serial.printf("Chip ID: %08X\n", (uint32_t)(chipid>>32));
  Serial.print("Trigger...");
  while(digitalRead(TRIGG) == HIGH) {
    delay(1);
  }
  Serial.println(" done.");
  ledcDetachPin(LED1R);
  ledcAttachPin(LED1G, 0);
  Serial.print("IR Output...");
  {
    IRsend irsend(IROUT);
    irsend.begin();
    irsend.sendRC6(irsend.encodeRC6((uint32_t)(chipid>>32), 12, 0));
    delay(1000);
    irsend.sendRC6(irsend.encodeRC6((uint32_t)(chipid>>32), 12, 0));
    delay(1000);
    irsend.sendRC6(irsend.encodeRC6((uint32_t)(chipid>>32), 12, 0));
    delay(1000);
  }
  Serial.println(" done.");
  ledcDetachPin(LED2R);
  ledcAttachPin(LED2G, 0);
  Serial.print("IR Input 1...");
  {
    IRrecv irrecv(IR1IN, 1024, 15, true);
    irrecv.setUnknownThreshold(12);
    irrecv.enableIRIn();
    while(true) {
      if (irrecv.decode(&results)) {
        if(results.decode_type == RC6) {
          Serial.print(" done. (");
          Serial.printf("%08X", (uint32_t)(results.command));
          Serial.print(" in team ");
          Serial.print(results.address);
          Serial.println(")");
          yield();
          break;
        }
      }
    }
  }
  ledcDetachPin(LED3R);
  ledcAttachPin(LED3G, 0);
  Serial.print("IR Input 2...");
  {
    IRrecv irrecv(IR2IN, 1024, 15, true);
    irrecv.setUnknownThreshold(12);
    irrecv.enableIRIn();
    while(true) {
      if (irrecv.decode(&results)) {
        if(results.decode_type == RC6) {
          Serial.printf(" done. (");
          Serial.printf("%08X", (uint32_t)(results.command));
          Serial.print(" in team ");
          Serial.print(results.address);
          Serial.println(")");
          yield();
          break;
        }
      }
    }
  }
  ledcDetachPin(LED4R);
  ledcAttachPin(LED4G, 0);
  delay(1000);
  fade(LED1G, true, false);
  ledcDetachPin(LED1G);
  ledcDetachPin(LED2G);
  ledcDetachPin(LED3G);
  ledcDetachPin(LED4G);
  delay(1000);
  Serial.print("LED 1 Red...");
  fade(LED1R);
  Serial.print(" Green...");
  fade(LED1G);
  Serial.print(" Blue...");
  fade(LED1B);
  Serial.println(" done.");
  Serial.print("LED 2 Red...");
  fade(LED2R);
  Serial.print(" Green...");
  fade(LED2G);
  Serial.print(" Blue...");
  fade(LED2B);
  Serial.println(" done.");
  Serial.print("LED 3 Red...");
  fade(LED3R);
  Serial.print(" Green...");
  fade(LED3G);
  Serial.print(" Blue...");
  fade(LED3B);
  Serial.println(" done.");
  Serial.print("LED 4 Red...");
  fade(LED4R);
  Serial.print(" Green...");
  fade(LED4G);
  Serial.print(" Blue...");
  fade(LED4B);
  Serial.println(" done.");
  Serial.println("=== END SELFTEST ===");
  ledcAttachPin(LED1G, 0);
  ledcAttachPin(LED2G, 0);
  ledcAttachPin(LED3G, 0);
  fade(LED4G, false);
}

void loop() {
}

#include <IRremote.h>

// Config
const int hits_to_destroy = 3;

// Pin setup
const int LED1R = 3;
const int LED1G = 5;
const int LED1B = 6;
const int LED2R = 9;
const int LED2G = 10;
const int LED2B = 11;
const int BUZZR = 12;
const int IR_EN = 4;
const int IR_SI = 2;

// Used variables
float fader = 0;
float amount = 0.3;

int timer_blue = 0;
int timer_green = 0;
int hit_blue = 0;
int hit_green = 0;
int points_blue = 0;
int points_green = 0;

IRrecv irrecv(IR_SI);
decode_results results;

void setup() {
  Serial.begin(9600);
  
  pinMode(LED1R, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1B, OUTPUT);
  pinMode(LED2R, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2B, OUTPUT);
  pinMode(BUZZR, OUTPUT);
  pinMode(IR_EN, OUTPUT);

  digitalWrite(IR_EN, HIGH);
  
  irrecv.enableIRIn();

  //Serial.println("Setup completed. Starting routine.");
}

void loop() {
  if (irrecv.decode(&results)) {
    //Serial.print("Got IR command: ");
    //Serial.println(results.value, HEX);
    if (results.value == 0xE0E09E61 || results.value == 0xD2B47) {
      //Serial.println("I was tagged.");
      digitalWrite(BUZZR, HIGH);
      float burstfade = 128;
      float burstamount = 1.5;
      while (burstfade > 0) {
        analogWrite(LED1R, burstfade);
        analogWrite(LED1G, burstfade);
        analogWrite(LED1B, burstfade);
        analogWrite(LED2R, burstfade);
        analogWrite(LED2G, burstfade);
        analogWrite(LED2B, burstfade);
        burstfade += burstamount;
        if (burstfade >= 255) {
          burstamount = -2 * burstamount;
        }
        delay(1);
      }
      digitalWrite(BUZZR, LOW);
      if (results.value == 0xE0E09E61) {
        hit_blue += 1;
        timer_blue = 0;
        if (hit_green > 0) {
          hit_green -= 1;
        }
      } else if (results.value == 0xD2B47) {
        hit_green += 1;
        timer_green = 0;
        if (hit_blue > 0) {
          hit_blue -= 1;
        }
      }
      Serial.print("Blue: ");
      Serial.print(hit_blue);
      Serial.print(" | Green: ");
      Serial.println(hit_green);
      if (hit_blue >= hits_to_destroy || hit_green >= hits_to_destroy) {
        analogWrite(LED1R, 0);
        analogWrite(LED1G, 0);
        analogWrite(LED1B, 0);
        analogWrite(LED2R, 0);
        analogWrite(LED2G, 0);
        analogWrite(LED2B, 0);
        delay(5000);
        if (hit_blue >= hits_to_destroy) {
          analogWrite(LED1B, 255);
          analogWrite(LED2B, 255);
          points_blue++;
        } else {
          analogWrite(LED1G, 255);
          analogWrite(LED2G, 255);
          points_green++;
        }
        Serial.print("SCORES Blue: ");
        Serial.print(points_blue);
        Serial.print(" | Green: ");
        Serial.println(points_green);
        fader = 128;
        hit_green = 0;
        hit_blue = 0;
        delay(10000);
        analogWrite(LED1R, 0);
        analogWrite(LED1G, 0);
        analogWrite(LED1B, 0);
        analogWrite(LED2R, 0);
        analogWrite(LED2G, 0);
        analogWrite(LED2B, 0);
        Serial.println("Reactivated.");
        delay(random(1, 5) * 1000);
      } else {
        delay(50);
      }
    }
    irrecv.resume();
    irrecv.enableIRIn();
  }
  analogWrite(LED1B, fader);
  analogWrite(LED2G, 255 - fader);
  fader += amount;
  if (fader > 255 || fader < 0) {
    amount = -amount;
  }
  timer_green++;
  if (timer_green > 5000 && hit_green > 0) {
    hit_green = 0;
  }
  timer_blue++;
  if (timer_blue > 5000 && hit_blue > 0) {
    hit_blue = 0;
  }
  delay(1);
}

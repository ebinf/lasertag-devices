#include <EspMQTTClient.h>
#include <IRremoteESP8266.h>
#include <IRrecv.h>
#include <IRsend.h>
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include <DFMiniMp3.h>

#define TRIGG 34
#define SPCLT 13
#define IROUT 4
#define IRINP 35
#define LED1R 5
#define LED1G 22
#define LED1B 21
#define LED2R 0
#define LED2G 2
#define LED2B 15
#define LED3R 32
#define LED3G 33
#define LED3B 25
#define CPCTS 12
#define CPCTR 14
#define LASER 16
#define RFIDS 17
#define MP3RX 26
#define MP3TX 27

const int L1R = 0;
const int L1G = 1;
const int L1B = 2;
const int L2R = 3;
const int L2G = 4;
const int L2B = 5;
const int L3R = 6;
const int L3G = 7;
const int L3B = 8;

const int LED1[] = {L1R, L1G, L1B};
const int LED2[] = {L2R, L2G, L2B};
const int LED3[] = {L3R, L3G, L3B};
const int LEDA[] = {L1R, L1G, L1B, L2R, L2G, L2B, L3R, L3G, L3B};

const char* ssid     = "YOUR-SSID";
const char* password = "YOUR-PASSWORD";
const char* mqtt_server = "192.168.178.50";
const unsigned int mqtt_port = 1883;

char chipid[4];

int colormode = 0;  // 0 Static
                    // 1 Blinking
                    // 2 Fast blinking
                    // 3 Slow Rainbow
                    // 4 Rainbow
                    // 5 Fast Rainbow
                    // 6 Color switching

int colorjump = 0;
int colorfade = 0;
int coloramount = 0;
const float white[] = {1, 1, 1};
const float red[] = {1, 0, 0};
const float green[] = {0, 1, 0};
const float blue[] = {0, 0, 1};
const float black[] = {0, 0, 0};
float l1_colors[3] = {0, 0, 0};
float l2_colors[3] = {0, 0, 0};
float l3_colors[3] = {0, 0, 0};
const float colorswitch[][3] = {
                          {1.000000, 1.000000, 1.000000}, // White  #FFFFFF
                          {0.000000, 0.000000, 1.000000}, // Blue   #0000FF
                          {0.000000, 1.000000, 0.000000}, // Green  #00FF00
                          {1.000000, 0.000000, 0.000000}, // Red    #FF0000
                          {0.835294, 0.000000, 1.000000}, // Pink   #D500FF
                          {0.333333, 1.000000, 0.082353}, // Lime   #55FF15
                          {1.000000, 1.000000, 0.000000}, // Yellow #FFFF00
                          {0.000000, 0.760784, 0.552941}, // Cyan   #00C28D
                          {1.000000, 0.400000, 0.133333}, // Brown  #FF6622
                          {0.145098, 0.145098, 1.000000}, // Orange #A23300
                          {0.145098, 0.145098, 1.000000}  // Purple #2525FF
                         };


unsigned int language = 1;  // 1 English
                            // 2 German
                            // 3 Dutch
unsigned int reactivationtime = 10;
float reactivationcnt = 0;
int lives = -1;
unsigned int hitstokill = 1;
int ammunition = -1;
unsigned int firerate = 0;
bool friendlyfire = true;
bool friendlydeactivate = true;
int ac_lives = -1;
unsigned int ac_hits = 0;
int ac_ammunition = -1;
unsigned int ac_firerate;
int team = -1;
unsigned int state = 0;  // 0 Pre-Game
                         // 1 Pre-Game activated
                         // 2 In-Game activated
                         // 3 In-Game deactiviated

char sub[64];
char sub2[64];

int irtoggle = 0;
bool israpidfire = false;

int rapidfire = false;
int invisibility = false;
int invulnerability = false;
int camouflage = false;

class Mp3Notify {
public:
  static void PrintlnSourceAction(DfMp3_PlaySources source, const char* action) {}
  static void OnError(uint16_t errorCode) {}
  static void OnPlayFinished(DfMp3_PlaySources source, uint16_t track) {}
  static void OnPlaySourceOnline(DfMp3_PlaySources source) {}
  static void OnPlaySourceInserted(DfMp3_PlaySources source) {}
  static void OnPlaySourceRemoved(DfMp3_PlaySources source) {}
};

EspMQTTClient client(
  ssid,
  password,
  mqtt_server,
  chipid,
  mqtt_port
);
IRsend irsend(IROUT);
IRrecv irrecv(IRINP, 1024, 15, true);
decode_results results;
MFRC522 mfrc522(RFIDS, 40);
SoftwareSerial mp3Serial(MP3RX, MP3TX);
DFMiniMp3<SoftwareSerial, Mp3Notify> mp3(mp3Serial);

void copyarray(const float* olda, float* newa, const int sz = 3) {
  memcpy(newa, olda, sizeof(olda[0])*sz);
}

void fade(const int led[], const float colors[3] = white, const bool dodown = true, const bool doup = true, const int sz = 3) {
  unsigned int fader;
  if (!doup) {
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
    for (int i = 0; i < sz; i++) {
      ledcWrite(led[i], (8191 / 255) * fader * colors[i % 3]);
    }
    if (fader == 255 || (!doup && !dodown)) {
      down = true;
      if (!dodown) {
        return;
      }
    }
    if (fader == 0) {
      return;
    }
    delay(1);
  }
}

unsigned int hexToInt(const char & hex) {
    if (hex >= 'A' && hex <= 'F') {
        switch (hex) {
            case 'F': return 15;
            case 'E': return 14;
            case 'D': return 13;
            case 'C': return 12;
            case 'B': return 11;
            case 'A': return 10;
          }
    }
    if (hex >= '0' && hex <= '9') {
        return (int)hex - 48;
    }
    return false;
}

int hexToInt(const String & hex) {
    int calc = 0;
    for (unsigned int i = 0; i < hex.length(); i++) {
        calc += hexToInt(hex[i]) * pow(16, (hex.length() - i - 1));   
    }
    if (hex[0] == '-') {
      calc = -calc;
    }
    return calc;
}


void mqtt_ammunition(const String & payload) {
  if (payload.length() >= 1 && payload.length() <= 3) {
    ammunition = hexToInt(payload);
    ac_ammunition = ammunition;
    sprintf(sub, "waistcoat/%s/acammunition", chipid);
    client.publish(sub, String(ammunition));
    Serial.printf("New ammunition: %i\n", ammunition);
  }
}

void mqtt_lives(const String & payload) {
  if (payload.length() == 1 || payload.length() == 2) {
    lives = hexToInt(payload);
    ac_lives = lives;
    sprintf(sub, "waistcoat/%s/aclives", chipid);
    client.publish(sub, String(ac_lives));
    Serial.printf("New lives: %i\n", lives);
  }
}

void mqtt_hits_to_kill(const String & payload) {
  if (payload.length() == 1 || payload.length() == 2) {
    hitstokill = hexToInt(payload);
    Serial.printf("New hits to kill: %i\n", hitstokill);
  }
}

void mqtt_reactivationtime(const String & payload) {
  if (payload.length() == 1 || payload.length() == 2) {
    reactivationtime = hexToInt(payload);
    Serial.printf("New reactiviation time: %i seconds\n", reactivationtime);
  }
}

void mqtt_firerate(const String & payload) {
  if (payload.length() == 1 || payload.length() == 2) {
    firerate = hexToInt(payload);
    Serial.printf("New fire rate time: %i\n", firerate);
  }
}

void mqtt_color(const String & topic, const String & payload) {
  if (payload.length() == 3 || payload.length() == 6) {
    float colors[3] = {1, 1, 1};
    if (payload.length() == 6) {
      colors[0] = hexToInt(payload.substring(0, 2)) / 255.0;
      colors[1] = hexToInt(payload.substring(2, 4)) / 255.0;
      colors[2] = hexToInt(payload.substring(4, 6)) / 255.0;
    } else if (payload.length() == 3) {
      colors[0] = hexToInt(payload[0]) / 15.0;
      colors[1] = hexToInt(payload[1]) / 15.0;
      colors[2] = hexToInt(payload[2]) / 15.0;
    } else {
      return;
    }
    if (topic == "1") {
      fade(LED1, l1_colors, true, false);
      fade(LED1, colors, false);
      copyarray(colors, l1_colors);
    } else if (topic == "2") {
      fade(LED2, l2_colors, true, false);
      fade(LED2, colors, false);
      copyarray(colors, l2_colors);
    } else if (topic == "3") {
      fade(LED3, l3_colors, true, false);
      fade(LED3, colors, false);
      copyarray(colors, l3_colors);
    } else if (topic == "all") {
      fade(LEDA, l1_colors, true, false, 9);
      fade(LEDA, colors, false, true, 9);
      copyarray(colors, l1_colors);
      copyarray(colors, l2_colors);
      copyarray(colors, l3_colors);
    }
    Serial.printf("New color for %s: %s\n", topic, payload);
  }
}

void mqtt_colormode(const String & payload) {
  if (payload.length() == 1) {
    colormode = hexToInt(payload[0]);
    ledcAttachPin(LED1R, L1R);
    ledcAttachPin(LED1G, L1G);
    ledcAttachPin(LED1B, L1B);
    ledcAttachPin(LED2R, L2R);
    ledcAttachPin(LED2G, L2G);
    ledcAttachPin(LED2B, L2B);
    ledcAttachPin(LED3R, L3R);
    ledcAttachPin(LED3G, L3G);
    ledcAttachPin(LED3B, L3B);
    for (int i = 0; i < 3; i++) {
      ledcWrite(LED1[i], 8191 * l1_colors[i]);
    }
    for (int i = 0; i < 3; i++) {
      ledcWrite(LED2[i], 8191 * l2_colors[i]);
    }
    for (int i = 0; i < 3; i++) {
      ledcWrite(LED3[i], 8191 * l3_colors[i]);
    }
    switch (colormode) {
      case 1:
        coloramount = 10;
        break;
      case 2:
        coloramount = 20;
        break;
      case 3:
        coloramount = 5;
        break;
      case 4:
        coloramount = 10;
        break;
      case 5:
        coloramount = 40;
        break;
      case 6:
        coloramount = 5;
        break;
      default:
        coloramount = 0;
        break;
    }
    colorfade = 5000;
    colorjump = 0;
    Serial.printf("New color mode: %s\n", payload);
  }
}

void mqtt_powerups(const String & topic, const String & payload = "") {
  sprintf(sub, "waistcoat/%s/acpowerups", chipid);
  if (team != 0) {
    if (topic == "rapidfire") {
      if (payload == "") {
        rapidfire = 100;
      } else {
        rapidfire = hexToInt(payload);
      }
      irtoggle = 0;
      ac_firerate = 0;
    } else if (topic == "-rapidfire") {
      rapidfire = false;
      ac_firerate = firerate;
      client.publish(sub, "rapidfire");
    } else if (topic == "invisibility") {
      if (payload == "") {
        invisibility = 20000;
      } else {
        invisibility = hexToInt(payload) * 1000;
      }
      fade(LEDA, l1_colors, true, false, 9);
    } else if (topic == "-invisibility") {
      invisibility = false;
      fade(LEDA, l1_colors, false, true, 9);
      client.publish(sub, "invisibility");
    } else if (topic == "invulnerability") {
      if (payload == "") {
        invulnerability = 20000;
      } else {
        invulnerability = hexToInt(payload) * 1000;
      }
    } else if (topic == "-invulnerability") {
      invulnerability = false;
      client.publish(sub, "invulnerability");
    } else if (topic == "camouflage") {
      if (payload == "") {
        camouflage = 20000;
      } else {
        camouflage = hexToInt(payload) * 1000;
      }
    } else if (topic == "-camouflage") {
      camouflage = false;
      client.publish(sub, "camouflage");
    } else if (topic == "-all") {
      rapidfire = false;
      invisibility = false;
      invulnerability = false;
      camouflage = false;
      ac_firerate = firerate;
      client.publish(sub, "all");
    }
    Serial.printf("New powerup: %s, %s\n", topic, payload);
  }
}

void mqtt_state(const String & payload) {
  if (payload.length() == 1) {
    const unsigned int newstate = hexToInt(payload);
    if (newstate == 0 || team == -1) {
      if (state == 2 && newstate == 0) {
        mp3.playMp3FolderTrack(5);
      }
      team = -1;
      mqtt_color("all", "000");
      mqtt_colormode("0");
    } else if (newstate == 1) {
      if (team == 0) {
        mqtt_colormode("0");
      } else {
        mqtt_colormode("1");
      }
      mp3.playMp3FolderTrack(3);
    } else if (newstate == 2) {
      if (state == 0) {
        mqtt_state("0");
        return;
      } else if (state == 1) {
        mp3.playMp3FolderTrack(4);
        mqtt_powerups("-all");
        ac_hits = 0;   
      }
      irrecv.enableIRIn();
      mqtt_colormode("0");
    } else if (newstate == 3) {
      fade(LEDA, white, true, false, 9);
      mqtt_powerups("-all");
      reactivationcnt = 0;
      irrecv.disableIRIn();
      mp3.playMp3FolderTrack(2);
      if (hitstokill > 1) {
        ac_hits = 0;
      }
      if (lives > -1) {
        ac_lives--;
        sprintf(sub, "waistcoat/%s/aclives", chipid);
        client.publish(sub, String(ac_lives));
      }
    }
    state = newstate;
    if (team == -1) {
      state = 0;
    }
    Serial.printf("New state: %i\n", state);
  }
}

void mqtt_team(const String & payload) {
  if (payload.length() == 1 || payload.length() == 2) {
    team = hexToInt(payload);
    if (team == 0) {
      mqtt_colormode("0");
    }
    Serial.printf("New team: %i\n", team);
  }
}

void mqtt_sound(const String & payload) {
  if (payload.length() == 1) {
    mp3.playFolderTrack(1, hexToInt(payload));
  } else {
    uint8_t lang = language;
    if (payload[0] == 'S') {
      lang = 1;
    }
    mp3.playFolderTrack(lang, hexToInt(payload));
  }
  Serial.printf("Playing sound: %s\n", payload);
}

void mqtt_friendlyfire(const String & payload) {
  if (payload == "1") {
    friendlyfire = true;
  } else if (payload == "0") {
    friendlyfire = false;
  }
  Serial.printf("New friendly fire: %i\n", friendlyfire);
}

void mqtt_friendlydeactivate(const String & payload) {
  if (payload == "1") {
    friendlydeactivate = true;
  } else if (payload == "0") {
    friendlydeactivate = false;
  }
  Serial.printf("New friendly deactivate: %i\n", friendlydeactivate);
}

void mqtt_ping(const String & payload = "") {
  client.publish("waistcoat/hello", chipid);
}

void mqtt_switch(const String & topic, const String & payload) {
  String topics;
  if (topic.substring(0, 5) == "game/") {
    if (state == 0) {
      return;
    }
    topics = topic.substring(5);
    if (team == 0 && (topics.substring(0, 6) == "color/" || topics.substring(0, 6) == "sound/")) {
      return;
    }
  } else if (topic.substring(0, 6) == "arena/") {
    topics = topic.substring(6);
  } else {
    topics = topic.substring(11 + sizeof(chipid));
  }
  if (topics.substring(0, 6) == "color/") {
    if (topics.substring(6) == "mode") {
      mqtt_colormode(payload);
    } else {
      mqtt_color(topics.substring(6), payload);
    }
  } else if (topics == "team") {
    mqtt_team(payload);
  } else if (topics == "state") {
    mqtt_state(payload);
  } else if (topics == "reactivationtime") {
    mqtt_reactivationtime(payload);
  } else if (topics == "lives") {
    mqtt_lives(payload);
  } else if (topics == "hitstokill") {
    mqtt_hits_to_kill(payload);
  } else if (topics == "ammunition") {
    mqtt_ammunition(payload);
  } else if (topics == "friendlyfire") {
    mqtt_friendlyfire(payload);
  } else if (topics == "friendlydeactivate") {
    mqtt_friendlydeactivate(payload);
  } else if (topics == "sound") {
    mqtt_sound(payload);
  } else if (topics == "firerate") {
    mqtt_firerate(payload);
  } else if (topics.substring(0, 9) == "powerups/") {
    mqtt_powerups(topics.substring(9), payload);
  } 
  return;
}

void onConnectionEstablished() {
  sprintf(sub, "waistcoat/%s/#", chipid);
  client.subscribe(sub, mqtt_switch);
  client.subscribe("game/#", mqtt_switch);
  client.subscribe("arena/#", mqtt_switch);
  client.subscribe("waistcoat/ping", mqtt_ping);
  mqtt_ping();
}

void setup() {
  sprintf(chipid, "%04X", (uint16_t)(ESP.getEfuseMac() >> 32));
  client.enableLastWillMessage("waistcoat/bye", chipid);
  for (int i = 0; i <= 11; i++) {
    ledcSetup(i, 5000, 13);
  }
  pinMode(TRIGG, INPUT);
  pinMode(SPCLT, INPUT_PULLUP);
  pinMode(IROUT, OUTPUT);
  pinMode(IRINP, INPUT);
  pinMode(LED1R, OUTPUT);
  pinMode(LED1G, OUTPUT);
  pinMode(LED1B, OUTPUT);
  pinMode(LED2R, OUTPUT);
  pinMode(LED2G, OUTPUT);
  pinMode(LED2B, OUTPUT);
  pinMode(LED3R, OUTPUT);
  pinMode(LED3G, OUTPUT);
  pinMode(LED3B, OUTPUT);
  pinMode(LASER, OUTPUT);
  pinMode(RFIDS, OUTPUT);
  ledcAttachPin(LED1R, L1R);
  ledcAttachPin(LED1G, L1G);
  ledcAttachPin(LED1B, L1B);
  ledcAttachPin(LED2R, L2R);
  ledcAttachPin(LED2G, L2G);
  ledcAttachPin(LED2B, L2B);
  ledcAttachPin(LED3R, L3R);
  ledcAttachPin(LED3G, L3G);
  ledcAttachPin(LED3B, L3B);

  digitalWrite(LASER, LOW);

  fade(LEDA, red, false, true, 9);

  Serial.begin(115200);
  Serial.println("================== RESTART ==================");
  Serial.print("Chip ID: ");
  Serial.println(chipid);
  

  Serial.println("Starting IR...");
  irsend.begin();
  irrecv.enableIRIn();

  Serial.println("Starting MP3...");
  mp3.begin();
  mp3.setVolume(24);
  fade(LED1, green, false);

  Serial.println("Starting SPI...");
  SPI.begin();
  
  Serial.println("Starting RFID reader...");
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();
  fade(LED2, green, false);

  client.enableDebuggingMessages(true);
  int counter = 15;
  while (client.isWifiConnected() == false) {
    client.loop();
    if (counter <= 0) {
      ESP.restart();
    }
    counter--;
    delay(500);
  }
  //client.enableDebuggingMessages(false);
  fade(LED3, green, false);

  Serial.println("Ready!");
}

void loop() {
  client.loop();
  if (colorfade >= 5000) {
    colorfade = 0;
    if ((colormode >= 1 && colormode <= 2)) {
      if (colorjump == 0) {
        ledcDetachPin(LED3R);
        ledcDetachPin(LED3G);
        ledcDetachPin(LED3B);
        ledcDetachPin(LED2R);
        ledcDetachPin(LED2G);
        ledcDetachPin(LED2B);
        ledcAttachPin(LED1R, L1R);
        ledcAttachPin(LED1G, L1G);
        ledcAttachPin(LED1B, L1B);
        colorjump = 1;
      } else {
        ledcDetachPin(LED1R);
        ledcDetachPin(LED1G);
        ledcDetachPin(LED1B);
        ledcAttachPin(LED3R, L3R);
        ledcAttachPin(LED3G, L3G);
        ledcAttachPin(LED3B, L3B);
        ledcAttachPin(LED2R, L2R);
        ledcAttachPin(LED2G, L2G);
        ledcAttachPin(LED2B, L2B);
        colorjump = 0;
      }
    } else if ((colormode >= 3 && colormode <= 5)) {
      for (int i = 0; i < 9; i++) {
        ledcWrite(LEDA[i], (8191 / 255) * random(0, 255) * (random(1, 100) / 100.0));
      }
    } else if (colormode == 6) {
      fade(LEDA, colorswitch[colorjump], false, false, 9);
      colorjump += 1;
      if (colorjump >= sizeof(colorswitch)/sizeof(colorswitch[0]) - 1) {
        colorjump = 0;
      }
    }
  }
  colorfade += coloramount;
  if (state != 2 && irrecv.decode(&results)) {
    if (results.decode_type == NEC && results.address != (uint16_t)(ESP.getEfuseMac() >> 32) && (int)results.command == 0) {
      Serial.printf("Shot by referee %04X.\n", results.address);
      sprintf(sub, "waistcoat/%s/hit", chipid);
      sprintf(sub2, "%04X", results.address);
      client.publish(sub, sub2);
    }
  }
  if (state == 0 && team != -1) {
    reactivationcnt += 1;
    if (reactivationcnt >= 100) {
      reactivationcnt = 0;
      sprintf(sub2, "");
      delay(1);
      return;
    }
    if (!mfrc522.PICC_IsNewCardPresent()) {
      return;
    }
    if (!mfrc522.PICC_ReadCardSerial()) {
      return;
    }
    sprintf(sub, "");
    for (byte i = 0; i < mfrc522.uid.size; i++) {
      sprintf(sub, "%s%02X", sub, mfrc522.uid.uidByte[i]);
    }
    if (String(sub) != String(sub2)) {
      Serial.println("Membercard recognized.");
      sprintf(sub2, "%s", sub);
      sprintf(sub, "waistcoat/%s/membercard", chipid);
      client.publish(sub, String(sub2));
    }
  } else if (state == 2) {
    if (team == 0) {
      if (digitalRead(TRIGG) == LOW && irtoggle == 0) {
        irtoggle = 1;
        digitalWrite(LASER, HIGH);
        irsend.sendNEC(irsend.encodeNEC((uint16_t)(ESP.getEfuseMac() >> 32), team));
        mp3.playMp3FolderTrack(1);
        digitalWrite(LASER, LOW);
      }
      if (digitalRead(TRIGG) == HIGH && irtoggle == 1) {
        irtoggle = 0;
      }
    } else {
      if (hitstokill > 1) {
        reactivationcnt++;
      }
      if (digitalRead(TRIGG) == LOW && irtoggle == 0 && (ammunition == -1 || ac_ammunition > 0)) {
        irtoggle = 1;
        if (ammunition > 0) {
          ac_ammunition--;
          sprintf(sub, "waistcoat/%s/acammunition", chipid);
          client.publish(sub, String(ac_ammunition));
        }
        if (israpidfire) {
          rapidfire--;
          if (rapidfire <= 0) {
            mqtt_powerups("-rapidfire");
          }
        }
        digitalWrite(LASER, HIGH);
        irsend.sendNEC(irsend.encodeNEC((uint16_t)(ESP.getEfuseMac() >> 32), team));
        mp3.playMp3FolderTrack(1);
        digitalWrite(LASER, LOW);
      }
      if ((digitalRead(TRIGG) == HIGH || rapidfire > 0) && irtoggle > 0) {
        irtoggle++;
        israpidfire = (digitalRead(TRIGG) == LOW);
        if (irtoggle >= 100 + (ac_firerate * 10)) {
          irtoggle = 0;
        }
      }
      if (irrecv.decode(&results)) {
        if (results.decode_type == NEC && results.address != (uint16_t)(ESP.getEfuseMac() >> 32) && ((int)results.command != team || !friendlyfire)) {
          Serial.printf("Shot by %04X in team %i.\n", results.address, results.command);
          if (hitstokill > 1 && (ac_hits + 1) < hitstokill && !invulnerability && ((int)results.command != team || friendlydeactivate)) {
            reactivationcnt = 0;
            fade(LEDA, white, true, false, 9);
            ac_hits++;
            mqtt_state("2");
          } else {
            sprintf(sub, "waistcoat/%s/hit", chipid);
            sprintf(sub2, "%04X", results.address);
            client.publish(sub, sub2);
            if (((int)results.command != team || friendlydeactivate) && (int)results.command != 0 && !invulnerability) {
              mqtt_state("3");
            }
          }
        }
      }
      if (reactivationcnt >= 3000 && hitstokill > 1 && ac_hits > 0) {
        ac_hits = 0;
      }
      if (invisibility > 0) {
        invisibility--;
        if (invisibility <= 0) {
          mqtt_powerups("-invisibility");
        }
      }
      if (invulnerability > 0) {
        invulnerability--;
        if (invulnerability <= 0) {
          mqtt_powerups("-invulnerability");
        }
      }
      if (camouflage > 0) {
        camouflage--;
        if (camouflage <= 0) {
          mqtt_powerups("-camouflage");
        }
      }
    }
  } else if (state == 3) {
    reactivationcnt += 0.001;
    if ((lives == -1 || ac_lives > 0) && reactivationcnt >= reactivationtime) {
      irtoggle = true;
      mqtt_state("2");
    }
  }
  delay(1);
}

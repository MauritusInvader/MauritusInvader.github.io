#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>

const char* ssid = "NuVu";
const char* password = "innov8&cre8";

WebServer server(80);

// LEFT MOTOR
const int ENA = 21;
const int IN1 = 19;
const int IN2 = 18;

// RIGHT MOTOR
const int ENB = 27;
const int IN3 = 26;
const int IN4 = 25;

// ACTUATOR
const int ACT1 = 32;
const int ACT2 = 33;

const int SPEED = 200;

// calibration
float leftTrim = 1.00;
float rightTrim = 0.86;

bool penIsDown = false;
bool actuatorBusy = false;

void enableCORS() {

  server.sendHeader("Access-Control-Allow-Origin", "*");
}

void stopMotors() {

  ledcWrite(ENA, 0);
  ledcWrite(ENB, 0);

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
}

void forward() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, SPEED * leftTrim);
  ledcWrite(ENB, SPEED * rightTrim);
}

void backward() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, SPEED * leftTrim);
  ledcWrite(ENB, SPEED * rightTrim);
}

void turnLeft() {

  digitalWrite(IN1, LOW);
  digitalWrite(IN2, HIGH);

  digitalWrite(IN3, HIGH);
  digitalWrite(IN4, LOW);

  ledcWrite(ENA, SPEED * 0.65);
  ledcWrite(ENB, SPEED * 0.65);
}

void turnRight() {

  digitalWrite(IN1, HIGH);
  digitalWrite(IN2, LOW);

  digitalWrite(IN3, LOW);
  digitalWrite(IN4, HIGH);

  ledcWrite(ENA, SPEED * 0.65);
  ledcWrite(ENB, SPEED * 0.65);
}

void stopActuator() {

  digitalWrite(ACT1, LOW);
  digitalWrite(ACT2, LOW);
}

void penUp() {

  if (!penIsDown) return;
  if (actuatorBusy) return;

  actuatorBusy = true;

  digitalWrite(ACT1, HIGH);
  digitalWrite(ACT2, LOW);

  delay(500);

  stopActuator();

  penIsDown = false;
  actuatorBusy = false;
}

void penDown() {

  if (penIsDown) return;
  if (actuatorBusy) return;

  actuatorBusy = true;

  digitalWrite(ACT1, LOW);
  digitalWrite(ACT2, HIGH);

  delay(500);

  stopActuator();

  penIsDown = true;
  actuatorBusy = false;
}

float msPerUnit = 120;

void moveTimed(void (*motionFn)(), int units) {

  motionFn();

  delay(units * msPerUnit);

  stopMotors();
}

void runLogo(String cmd) {

  cmd.trim();
  cmd.toUpperCase();

  if (cmd.startsWith("F(")) {

    int n = cmd.substring(2, cmd.length() - 1).toInt();

    moveTimed(forward, n);
  }

  else if (cmd.startsWith("B(")) {

    int n = cmd.substring(2, cmd.length() - 1).toInt();

    moveTimed(backward, n);
  }

  else if (cmd.startsWith("L(")) {

    int n = cmd.substring(2, cmd.length() - 1).toInt();

    moveTimed(turnLeft, n * 0.6);
  }

  else if (cmd.startsWith("R(")) {

    int n = cmd.substring(2, cmd.length() - 1).toInt();

    moveTimed(turnRight, n * 0.6);
  }
}

void setup() {

  Serial.begin(115200);

  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);

  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  pinMode(ACT1, OUTPUT);
  pinMode(ACT2, OUTPUT);

  ledcAttach(ENA, 1000, 8);
  ledcAttach(ENB, 1000, 8);

  stopMotors();
  stopActuator();

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {

    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  if (MDNS.begin("logobot")) {

    Serial.println("mDNS started");
    Serial.println("http://logobot.local");
  }

  server.on("/forward", []() {

    forward();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/backward", []() {

    backward();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/left", []() {

    turnLeft();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/right", []() {

    turnRight();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/stop", []() {

    stopMotors();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/penup", []() {

    penUp();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/pendown", []() {

    penDown();

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.on("/runlogo", []() {

    String cmd = server.arg("cmd");

    runLogo(cmd);

    enableCORS();
    server.send(200, "text/plain", "OK");
  });

  server.begin();

  Serial.println("Server started");
}

void loop() {

  server.handleClient();
}
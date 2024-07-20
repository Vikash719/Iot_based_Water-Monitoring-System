#include <ESP8266WiFi.h>
#include <EEPROM.h>

// WiFi credentials
const char* ssid = "your_SSID";       // Replace with your WiFi network name
const char* password = "your_PASSWORD"; // Replace with your WiFi password

// Server endpoint
const char* serverUrl = "http://your_server_ip/update"; // Replace with your server URL

// Pins
const int trigPin = D5;
const int echoPin = D6;
const int DO = D7; // Assuming DO is some digital output pin

// Variables
long duration;
int distance;
int frate = 0; // Flow rate or similar
int mf = 0; // Total measurement or some value

void setup() {
  Serial.begin(9600);
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(DO, OUTPUT);
  pinMode(D5, INPUT);

  // Connect to WiFi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  // Initialize EEPROM
  EEPROM.begin(512);
  int a = EEPROM.read(0);
  int b = EEPROM.read(1);
  int c = EEPROM.read(2);
  int d = EEPROM.read(3);
  frate = (a * 1000) + (b * 100) + (c * 10) + d;
  if (frate > 10000) {
    frate = 0;
  }
}

void loop() {
  ultra(); // Measure distance

  int f = digitalRead(D5);
  if (f == 1) { // Check if digital pin D5 is high
    frate++;
    while (digitalRead(D5) == 1); // Wait until the pin goes low
  }

  mf = frate;
  EEPROM.write(0, mf / 1000);
  EEPROM.write(1, (mf % 1000) / 100);
  EEPROM.write(2, ((mf % 1000) % 100) / 10);
  EEPROM.write(3, ((mf % 1000) % 100) % 10);
  EEPROM.commit(); // Save the data to EEPROM

  Serial.print(distance);
  Serial.print(':');
  Serial.print(frate);
  Serial.print(':');
  Serial.print(mf);
  Serial.println(':');

  // Send data to server
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    if (client.connect(serverUrl, 80)) { // Connect to the server
      String postData = "distance=" + String(distance) + "&frate=" + String(frate) + "&mf=" + String(mf);
      client.println("POST /update HTTP/1.1");
      client.println("Host: your_server_ip");
      client.println("Content-Type: application/x-www-form-urlencoded");
      client.println("Content-Length: " + String(postData.length()));
      client.println();
      client.println(postData);
      client.stop();
    }
  }

  delay(1000); // Wait 1 second before the next loop iteration
}

void ultra() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);
  distance = duration * 0.034 / 2; // Convert to cm
}

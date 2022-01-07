#include "Arduino.h"
class color {
  public:
    int r, g, b;
    String Hex;
    void insertColor(int r, int g, int b) {
      this->r = r;
      this->g = g;
      this->b = b;
      this->Hex = "#" + String(r, HEX) + String(g, HEX) + String(b, HEX);

    }
};

class TCS3200 {
  private:
    int redFrequency = 0;
    int greenFrequency = 0;
    int blueFrequency = 0;
    int redColor = 0;
    int greenColor = 0;
    int blueColor = 0;
  public:
    int S0 =  32;
    int S1 = 33;
    int S2 =  25;
    int S3 = 26;
    int sensorOut = 27;

    color renk;

    void begin(int freq1 = 1, int freq2 = 0) {
      pinMode(S0, OUTPUT);
      pinMode(S1, OUTPUT);
      pinMode(S2, OUTPUT);
      pinMode(S3, OUTPUT);

      pinMode(sensorOut, INPUT);

      //default%20freq
      digitalWrite(S0, freq1);
      digitalWrite(S1, freq2);

    };
    int getRedColor() {
      digitalWrite(S2, LOW);
      digitalWrite(S3, LOW);
      redFrequency = pulseIn(sensorOut, LOW);

      int temps = map(redFrequency, 25, 253, 255, 0);
      if (temps > 255) {
        temps = 255;
      }
      else if (temps < 0) {
        temps = 0;
      }
      return temps;
    }
    int getGreenColor() {
      // Setting GREEN (G) filtered photodiodes to be read
      digitalWrite(S2, HIGH);
      digitalWrite(S3, HIGH);
      greenFrequency = pulseIn(sensorOut, LOW);

      int temps = map(greenFrequency, 27, 310, 255, 0);
      if (temps > 255) {
        temps = 255;
      }
      else if (temps < 0) {
        temps = 0;
      }
      return temps;
    }
    int getBlueColor() {
      digitalWrite(S2, LOW);
      digitalWrite(S3, HIGH);

      // Reading the output frequency
      blueFrequency = pulseIn(sensorOut, LOW);
      // Remaping the value of the BLUE (B) frequency from 0 to 255
      // You must replace with your own values. Here's an example:
      // blueColor = map(blueFrequency, 38, 84, 255, 0);
      int temps = map(blueFrequency, 26, 300, 255, 0);

      if (temps > 255) {
        temps = 255;
      }
      else if (temps < 0) {
        temps = 0;
      }
      return temps;

    }
    color ReadColors() {

      redColor = getRedColor();
      delay(10);
      greenColor = getGreenColor();
      delay(10);
      blueColor = getBlueColor();
      delay(10);
      renk.insertColor(redColor, greenColor, blueColor);
      return renk;
    }
    static  void CheckColors(int r, int g, int b) {
      if (r > g && r > b) {
        Serial.println(" - RED detected!");
      }
      if (g > r && g > b) {
        Serial.println(" - GREEN detected!");
      }
      if (b > r && b > g) {
        Serial.println(" - BLUE detected!");
      }

    }
    static void CheckColors(color renk) {
      int r = renk.r;
      int g = renk.g;
      int b = renk.b;
      if (r > g && r > b) {
        Serial.println(" - RED detected!");
      }
      if (g > r && g > b) {
        Serial.println(" - GREEN detected!");
      }
      if (b > r && b > g) {
        Serial.println(" - BLUE detected!");
      }
    }

};
/*
  TCS3200 ColorSensor;

  color renk;
  void setup() {

  ColorSensor.begin();
  Serial.begin(115200);
  }
  void loop() {
  renk = ColorSensor.ReadColors();
  Serial.print("kırmızı:  ");
  Serial.print(renk.r);
  Serial.print("  yeşil:  ");
  Serial.print(renk.g);
  Serial.print("  mavi:  ");
  Serial.println(renk.b);
   Serial.println(renk.Hex);
  TCS3200().CheckColors(renk);
  delay(300);
  }*/

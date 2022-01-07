#include "Arduino.h"
#include<Wire.h>
class raw_data {
  public:
    float pres;
    float temp;
    float hum;
};
class BME280 {
#define BME280ADDR 0x76

#define BME280SAMPLE_1  1
#define BME280SAMPLE_2  2
#define BME280SAMPLE_4  3
#define BME280SAMPLE_8  4
#define BME280SAMPLE_16 5

#define BME280_REGISTER_CONTROL_HUM  0xF2
#define BME280_REGISTER_STATUS  0xF3
#define BME280_REGISTER_CONTROL  0xF4

#define MODE_SLEEP = 0
#define MODE_FORCED = 1
#define MODE_NORMAL = 3

#define SeaLevel 101325
  public:

    uint8_t m_dig[32];
    int32_t t_fine = 0;

    void WriteOneByte(int addr, int reg, int dat) {
      Wire.beginTransmission(addr);
      Wire.write(reg);
      Wire.write(dat);
      Wire.endTransmission();
    }
    void GetCalibration() {

      Wire.beginTransmission(BME280ADDR);
      Wire.write(0x88);
      Wire.endTransmission();
      Wire.requestFrom(BME280ADDR, 26);
      int  sayim = 0;
      while (Wire.available())   // slave may send less than requested
      {
        m_dig[sayim] = Wire.read();    // receive a byte as character
        sayim++;
      }

      Wire.beginTransmission(BME280ADDR);
      Wire.write(0xE1);
      Wire.endTransmission();
      Wire.requestFrom(BME280ADDR, 7);

      if (Wire.available())   // slave may send less than requested
      {
        m_dig[sayim] = Wire.read();
        sayim++;
      }

    }

    void begin() {

      GetCalibration();


      ReadRaw();
      WriteOneByte(BME280ADDR, 0xF2, 0X01);
      WriteOneByte(BME280ADDR, 0xF4, 0X25);
      WriteOneByte(BME280ADDR, 0xF5, 0XB0);
    }
    raw_data ReadRaw() {

      delay(50);
      WriteOneByte(BME280ADDR, 0xF2, 0X01);
      WriteOneByte(BME280ADDR, 0xF4, 0X25);
      WriteOneByte(BME280ADDR, 0xF5, 0XA0);
      Wire.beginTransmission(BME280ADDR);
      Wire.write(0xF7);
      Wire.endTransmission();
      Wire.requestFrom(BME280ADDR, 8);
      int sayim = 0;
      while (Wire.available())   // slave may send less than requested
      {
        raw_data data = raw_data();
        uint32_t a = Wire.read();
        uint32_t b = Wire.read();
        uint32_t c = Wire.read();

        data.pres = ((a << 16) | (b << 8) | c) >> 4;


        a = Wire.read();
        b = Wire.read();
        c = Wire.read();
        data.temp = ((a << 16) | (b << 8) | c) >> 4;

        a = Wire.read();
        b = Wire.read();
        data.hum = (a << 8) | b;



        return data;
      }

    }

    raw_data ReadCompansatedData(raw_data data) {
      int32_t t_fine;
      float temp = CalculateTemperature(data.temp, t_fine);
      float humidity = CalculateHumidity(data.temp, t_fine);
      float pres = CalculatePressure(data.temp, t_fine);

      raw_data FixedData = raw_data();
      FixedData.temp = temp;
      FixedData.hum = humidity;
      FixedData.pres = pres;
      return FixedData;
    }

    raw_data Read() {
      raw_data dat = ReadRaw();
      //debug
      /*Serial.println(dat.temp);
        Serial.println(dat.hum);
        Serial.println(dat.pres);
        for (int i = 0; i < 32; i++) {
        Serial.print(String(i) + " : ");
        Serial.println(m_dig[i]);
        }*/

      return ReadCompansatedData(ReadRaw());
    }
    float CalculateTemperature(int32_t raw, int32_t& t_fine) {
      // Code based on calibration algorthim provided by Bosch.
      int32_t var1, var2, final;
      uint16_t dig_T1 = (m_dig[1] << 8) | m_dig[0];
      int16_t   dig_T2 = (m_dig[3] << 8) | m_dig[2];
      int16_t   dig_T3 = (m_dig[5] << 8) | m_dig[4];
      var1 = ((((raw >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
      var2 = (((((raw >> 4) - ((int32_t)dig_T1)) * ((raw >> 4) - ((int32_t)dig_T1))) >> 12) * ((int32_t)dig_T3)) >> 14;
      t_fine = var1 + var2;
      final = (t_fine * 5 + 128) >> 8;
      return final / 100.0 ;
    }


    /****************************************************************/
    float CalculateHumidity(int32_t raw, int32_t t_fine) {
      // Code based on calibration algorthim provided by Bosch.
      int32_t var1;
      uint8_t   dig_H1 =   m_dig[24];
      int16_t dig_H2 = (m_dig[26] << 8) | m_dig[25];
      uint8_t   dig_H3 =   m_dig[27];
      int16_t dig_H4 = (m_dig[28] << 4) | (0x0F & m_dig[29]);
      int16_t dig_H5 = (m_dig[30] << 4) | ((m_dig[29] >> 4) & 0x0F);
      int8_t   dig_H6 =   m_dig[31];

      var1 = (t_fine - ((int32_t)76800));
      var1 = (((((raw << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * var1)) +
                ((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)dig_H6)) >> 10) * (((var1 *
                                              ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
                                             ((int32_t)dig_H2) + 8192) >> 14));
      var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
      var1 = (var1 < 0 ? 0 : var1);
      var1 = (var1 > 419430400 ? 419430400 : var1);
      return ((uint32_t)(var1 >> 12)) / 1024.0;
    }


    /****************************************************************/
    float CalculatePressure(int32_t raw, int32_t t_fine) {
      // Code based on calibration algorthim provided by Bosch.
      int64_t var1, var2, pressure;
      float final;

      uint16_t dig_P1 = (m_dig[7]   << 8) | m_dig[6];
      int16_t   dig_P2 = (m_dig[9]   << 8) | m_dig[8];
      int16_t   dig_P3 = (m_dig[11] << 8) | m_dig[10];
      int16_t   dig_P4 = (m_dig[13] << 8) | m_dig[12];
      int16_t   dig_P5 = (m_dig[15] << 8) | m_dig[14];
      int16_t   dig_P6 = (m_dig[17] << 8) | m_dig[16];
      int16_t   dig_P7 = (m_dig[19] << 8) | m_dig[18];
      int16_t   dig_P8 = (m_dig[21] << 8) | m_dig[20];
      int16_t   dig_P9 = (m_dig[23] << 8) | m_dig[22];

      var1 = (int64_t)t_fine - 128000;
      var2 = var1 * var1 * (int64_t)dig_P6;
      var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
      var2 = var2 + (((int64_t)dig_P4) << 35);
      var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
      var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
      if (var1 == 0) {
        return NAN;  // Don't divide by zero.
      }
      pressure   = 1048576 - raw;
      pressure = (((pressure << 31) - var2) * 3125) / var1;
      var1 = (((int64_t)dig_P9) * (pressure >> 13) * (pressure >> 13)) >> 25;
      var2 = (((int64_t)dig_P8) * pressure) >> 19;
      pressure = ((pressure + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);

      final = ((uint32_t)pressure) / 256.0;
      final /= 100.0;
      final = (final/60)+1000;

      return final;
    }
};

#include "Robot.h"

Robot::Robot()
{

}

void Robot::begin(void)
{
    Serial1.begin(115200, SERIAL_8N1, 16, 17);
    Serial2.begin(115200);
    pinMode(PIN_BAT_VOLTS, INPUT);
    pinMode(PIN_BAT_AMPS, INPUT);
    pinMode(PIN_LIDAR_ENABLE, OUTPUT);
    digitalWrite(PIN_LIDAR_ENABLE, LOW);
    Serial.begin(115200);
    ledCommand(0, 0, 0, 0);
}

void Robot::wheelCommand(int wa, int wb, int wc, int wd){
  Serial2.write(0xAA);
  Serial2.write(wa);
  Serial2.write(wb);
  Serial2.write(wc);
  Serial2.write(wd);
  Serial2.write(0x55);
  Serial.printf("wheelCommand(%d, %d, %d, %d)\n", wa, wb, wc, wd);
}

void Robot::ledCommand(int pos, int r, int g, int b){
  Serial2.write(0xAE);
  Serial2.write(r);
  Serial2.write(g);
  Serial2.write(b);
  Serial2.write(0x55);
  Serial.printf("ledCommand(%d, %d, %d, %d)\n", pos, r, g, b);
}

ColorMeasurement Robot::getColorSensorData()
{
    ColorMeasurement measurement;
    return measurement;
}

void Robot::setLidarEnable(bool enable)
{

}

void Robot::getLidarData()
{

}

float Robot::getBatteryVoltage()
{
    int v = analogRead(PIN_BAT_VOLTS);
    float volts = float(v)/10.0f;
    return volts;
}

float Robot::getTotalCurrent()
{
    int v = analogRead(PIN_BAT_AMPS);
    float amps = float(v)/10.0f;
    return amps;
}

float Robot::getMotorTemperature()
{
    return 26.0f;
}
#ifndef __ROBOT_H__
#define __ROBOT_H__
#include <M5Stack.h>

#define PIN_BAT_VOLTS 35
#define PIN_BAT_AMPS 36
#define PIN_LIDAR_ENABLE 5
#define PIN_TEMPERATURE_SENSOR 19

struct ColorMeasurement
{
    uint16_t A;
    uint16_t R;
    uint16_t G;
    uint16_t B;
};

class Robot {
 public:   
    Robot();
    void begin();
    void ledCommand(int pos, int r, int g, int b);
    void wheelCommand(int a, int b, int c, int d);
    ColorMeasurement getColorSensorData();
    void setLidarEnable(bool enable);
    void getLidarData();
    float getBatteryVoltage();
    float getTotalCurrent();
    float getMotorTemperature();
    bool colorSensorAvailable = false;
    bool lidarEnabled = false;
};

#endif

/**
 * @file    sensor.h
 * @brief   Sensor functions/struct
 *
 * @author  Mitchell Clay
 * @date    2/26/2021
**/

#ifndef sensor_H
#define sensor_H

#define SENSOR_TYPE_TEMP            0
#define SENSOR_TYPE_ACCELEROMETER   1
#define SENSOR_TYPE_ALTIMETER       2

struct sensor {
    int sensor_type;
    float reading;
};

#endif
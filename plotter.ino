#include <Wire.h>
#include "DFRobot_INA219.h"


bool reportAverage = true;
const unsigned long reportInterval = 10000; // 100ms in microseconds

DFRobot_INA219_IIC ina219(&Wire, INA219_I2C_ADDRESS4);

// Revise these based on calibration
float ina219Reading_mA = 24;
float extMeterReading_mA = 22;

unsigned long startTime;
unsigned long endTime;
unsigned long duration;

unsigned long startMicros;
unsigned long endMicros;
float timeMs;

// ---- Rolling average tracking ----
float avgDuration = 0;
float avgVoltage = 0;
float avgCurrent = 0; 
int avgCount = 0;
const float alpha = 0.05;  // smoothing factor (0.0 – 1.0)
bool labelsSent = false;
bool recordingData = false;



// ---- 10ms report timer ----
unsigned long lastReportTime = 0;

void setup(void)
{
    Serial.begin(115200);
    while (!Serial);

    pinMode(12, INPUT);   // <-- Added pin D12 input

    Serial.println();
    while (ina219.begin() != true) {
        Serial.println("INA219 begin faild");
        delay(2000);
    }
    ina219.linearCalibrate(ina219Reading_mA, extMeterReading_mA);
}

void loop(void)
{
    if (digitalRead(12) == HIGH) {
        if(labelsSent == false){
            startMicros = micros();
            Serial.println("Voltage:,Current:,Power:");
            labelsSent = true;
            recordingData = true;
        }

        if(reportAverage){
            startTime = micros();
            avgVoltage += ina219.getBusVoltage_V();
            avgCurrent += ina219.getCurrent_mA();
            avgCount++;
            // ---- sensor readout ----

            endTime = micros();
            duration = endTime - startTime;

            // ---- Update rolling average (exponential moving average) ----
            avgDuration = (alpha * duration) + ((1 - alpha) * avgDuration);

            // ---- Print every 10 ms (non-blocking) ----
            if (micros() - lastReportTime >= reportInterval) {
                Serial.print(avgVoltage / avgCount, 2);
                Serial.print(", ");
                Serial.print(avgCurrent / avgCount, 3);
                Serial.print(", ");
                Serial.println((avgVoltage / avgCount * avgCurrent / avgCount) , 1);
                lastReportTime = micros();
                avgVoltage = 0;
                avgCurrent = 0;
                avgCount = 0;
            }
        }
        else{
            Serial.print( ina219.getBusVoltage_V(), 2);
            Serial.print(",");
            Serial.print(ina219.getCurrent_mA(), 3);
            Serial.print(",");
            Serial.println(ina219.getPower_mW(), 1);
        }
    }
    else{
        if(recordingData){
            recordingData = false;
            labelsSent = false;
            endMicros = micros();
            timeMs = (endMicros - startMicros) / 1000.0;
            Serial.printf("Time to encrypt : %.3f ms\n", timeMs);

        }
    }
}
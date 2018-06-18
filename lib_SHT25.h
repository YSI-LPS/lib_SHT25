/** SHT25 class
*
* @purpose       library for SHT25 humidity and temperature sensor
*
* Use to get temperature and humidity
*
* https://www.sensirion.com/products/humidity-sensor/
*
* Example:
* @code
* #include "mbed.h"
* #include "lib_SHT25.h"
*  
* Serial pc(USBTX, USBRX);
* SHT25  sensor(p28, p27);
* 
* int main()
* {
*     float temperature, humidity;
*     while(1)
*     {
*         sensor.getData(&temperature, &humidity);
*         pc.printf("\rtemperature = %6.2f%cC -|- humidity = %6.2f%%RH", temperature, 248, humidity);
*         wait(0.5);
*     }
* }
* @endcode
* @file          lib_SHT25.h 
* @date          Jun 2018
* @author        Yannic Simon
*/
#ifndef SHT25_H
#define SHT25_H

#include "mbed.h"

#define SHT_I2C_ADDR        0x80    //Sensor I2C address
#define SHT_TRIG_TEMP_HOLD  0xE3    //Trigger Temp  with hold master
#define SHT_TRIG_RH_HOLD    0xE5    //Trigger RH    with hold master
#define SHT_TRIG_TEMP       0xF3    //Trigger Temp  with no hold master
#define SHT_TRIG_RH         0xF5    //Trigger RH    with no hold master
#define SHT_WRITE_REG       0xE6    //Write to user register
#define SHT_READ_REG        0xE7    //Read from user register
#define SHT_SOFT_RESET      0xFE    //Soft reset the sensor
#define SHT_SELF_HEATING    0x01    //Keep self heating
#define SHT_PREC_1214       0x00    //RH 12 T 14 - default
#define SHT_PREC_0812       0x01    //RH 8  T 10 
#define SHT_PREC_1013       0x80    //RH 10 T 13
#define SHT_PREC_1111       0x81    //RH 10 T 13

/** SHT25 class
 */
class SHT25
{
    public:
        /** make new SHT25 instance
        * connected to sda, scl I2C pins
        *
        * @param sda I2C pin
        * @param scl I2C pin
        */
        SHT25(PinName sda, PinName scl);
        
        /** return Temperature(°C) and Humidity
        *
        * @param variable address to return Temperature
        * @param variable address to return Humidity
        * @returns none
        */ 
        void getData(float *, float *);
        
        /** return Temperature(°C)
        *
        * @param none
        * @returns Temperature(°C)
        */  
        float getTemperature(void);
        
        /** return Humidity
        *
        * @param none
        * @returns Humidity
        */  
        float getHumidity(void);
        
        /** set data precision 
        *
        * @param precision like SHT_PREC_RHTT (SHT_PREC_1214, SHT_PREC_0812, SHT_PREC_1013, SHT_PREC_1111)
        * @returns I2C acknoledge
        */  
        int setPrecision(char precision);
        
        /** soft reset the sensor
        *
        * @param none
        * @returns boolean on I2C acknoledge
        */  
        bool softReset(void);
    protected:
        I2C     _i2c;
        Timeout _t;
    private:
        void  readData(float *, float *);
        float readTemperature(void);
        float readHumidity(void);
        void  keepSelf(void);
        float _temperature, _humidity;
        bool  _selfHeat;
};

#endif
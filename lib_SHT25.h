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
* #include "lib_SHT25.h"
* 
* SHT25  sensor(I2C_SDA, I2C_SCL);
* 
* int main()
* {
*     while(1)
*     {
*         sensor.waitSafeHeat();
*         float temperature = sensor.getTemperature(), humidity = sensor.getHumidity();
*         printf("\r\ntemperature = %6.2f%cC -|- humidity = %6.2f%%RH", temperature, 248, humidity);
*         sensor.waitSafeHeat();
*         sensor.getData(&temperature, &humidity);
*         printf("\r\ntemperature = %6.2f%cC -|- humidity = %6.2f%%RH", temperature, 248, humidity);
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

#define SHT_I2C_FREQUENCY   100e3   //Sensor I2C Frequency max 400KHz
#define SHT_I2C_ADDR        0x80    //Sensor I2C address
#define SHT_TRIG_TEMP_HOLD  0xE3    //Trigger Temp  with hold master
#define SHT_TRIG_RH_HOLD    0xE5    //Trigger RH    with hold master
#define SHT_TRIG_TEMP_NHOLD 0xF3    //Trigger Temp  with no hold master
#define SHT_TRIG_RH_NHOLD   0xF5    //Trigger RH    with no hold master
#define SHT_WRITE_REG_USER  0xE6    //Write to user register
#define SHT_READ_REG_USER   0xE7    //Read from user register
#define SHT_SOFT_RESET      0xFE    //Soft reset the sensor
#if MBED_MAJOR_VERSION > 5
#define SHT_SELF_HEATING    2s      //Keep self heating
#define SHT_WAIT(ms)        (thread_sleep_for(ms))
#else
#define SHT_SELF_HEATING    0x01    //Keep self heating
#define SHT_WAIT(ms)        (wait_us(1000*ms))
#endif


/** SHT25 class
 */
class SHT25
{
    public:
        /** enumerator of the different precision of the sensor
        */
        typedef enum { SHT_PREC_RH12T14 = 0x00, SHT_PREC_RH08T12 = 0x01, SHT_PREC_RH10T13 = 0x80, SHT_PREC_RH11T11 = 0x81 }
            enum_sht_prec;
        /** make new SHT25 instance
        * connected to sda, scl I2C pins
        *
        * @param sda I2C pin
        * @param scl I2C pin
        * @param precision SHT25 precision for humidity(default 12 bits) and temperature(default 14 bits)
        * @param frequency I2C frequency, default 100KHz and maximum 400KHz
        */
        SHT25(PinName sda, PinName scl, enum_sht_prec precision = SHT_PREC_RH12T14, int frequency = SHT_I2C_FREQUENCY);
        
        /** return Temperature(°C) and Humidity
        *
        * @param tempC address to return Temperature
        * @param relHumidity address to return Humidity
        * @returns none
        */ 
        void getData(float *tempC, float *relHumidity);
        
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
        * @param precision { SHT_PREC_RH12T14 = 0x00, SHT_PREC_RH08T12 = 0x01, SHT_PREC_RH10T13 = 0x80, SHT_PREC_RH11T11 = 0x81 }
        * @returns true on I2C acknoledge
        */  
        bool setPrecision(const enum_sht_prec precision);
        
        /** soft reset the sensor
        *
        * @param none
        * @returns true on I2C acknoledge
        */
        bool softReset(void);
        
        /** wait safe heat for sensor
        *
        * @param none
        * @returns none
        */
        void waitSafeHeat(void);
    protected:
        I2C     _i2c;
        Timeout _t, _h;
    private:
        void  readData(void);
        float readTemperature(void);
        float readHumidity(void);
        void  keepSafeTemperature(void);
        void  keepSafeHumidity(void);
        float _temperature, _humidity;
        bool  _selfHeatTemperature, _selfHeatHumidity;
};

#endif
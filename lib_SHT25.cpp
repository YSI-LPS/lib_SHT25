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
* @file          lib_SHT25.cpp
* @date          Jun 2018
* @author        Yannic Simon
*/
#include "lib_SHT25.h"

SHT25::SHT25(PinName sda, PinName scl, enum_sht_prec precision, int frequency) : _i2c(sda, scl)
{
    _i2c.frequency((frequency<=400e3)?frequency:400e3);
    setPrecision(precision);
    _temperature = _humidity = NAN;
    _selfHeatTemperature = _selfHeatHumidity = false;
    _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
    _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
}

void SHT25::getData(float *tempC, float *relHumidity)
{
    if(_selfHeatTemperature && _selfHeatHumidity) readData();
    *tempC = _temperature;
    *relHumidity = _humidity;
}

void SHT25::readData(void)
{
    _temperature = readTemperature();
    _humidity = readHumidity();
}

float SHT25::getTemperature(void)
{
    if(_selfHeatTemperature) _temperature = readTemperature();
    return _temperature;
}

float SHT25::readTemperature(void) // if I2C Freezing go down PullUp resistor to 2K or slow frequency
{
    char cmd[] = {SHT_TRIG_TEMP_NHOLD}, rx[] = {0xFF, 0xFF, 0xFF};
    _selfHeatTemperature = false;
    _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
    if(!_i2c.write(SHT_I2C_ADDR, cmd, 1))
    {
        SHT_WAIT(66);
        int ack = _i2c.read(SHT_I2C_ADDR, rx, 3);
        if(ack)
        {
            SHT_WAIT(19);
            ack = _i2c.read(SHT_I2C_ADDR, rx, 3);    
        }
        return ack?NAN:-46.85f + 175.72f * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0f);
    }
    return NAN;
}

float SHT25::getHumidity(void)
{
    if(_selfHeatHumidity) _humidity = readHumidity();
    return _humidity;
}

float SHT25::readHumidity(void) // if I2C Freezing go down PullUp resistor to 2K or slow frequency
{
    char cmd[] = {SHT_TRIG_RH_NHOLD}, rx[] = {0xFF, 0xFF, 0xFF};
    _selfHeatHumidity = false;
    _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
    if(!_i2c.write(SHT_I2C_ADDR, cmd, 1))
    {
        SHT_WAIT(29);
        int ack = _i2c.read(SHT_I2C_ADDR, rx, 3);
        return ack?NAN:-6.0f + 125.0f * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0f);
    }
    return NAN;
}

bool SHT25::setPrecision(const enum_sht_prec precision)
{
    char cmd[] = {SHT_WRITE_REG_USER, precision};
    return !_i2c.write(SHT_I2C_ADDR, cmd, 2, false);
}

bool SHT25::softReset()
{
    char cmd[] = {SHT_SOFT_RESET};
    return !_i2c.write(SHT_I2C_ADDR, cmd, 1, false);
}

void SHT25::waitSafeHeat(void)
{
    while(!_selfHeatTemperature || !_selfHeatHumidity) __NOP();
}

void SHT25::keepSafeTemperature(void)
{
    _selfHeatTemperature = true;
}

void SHT25::keepSafeHumidity(void)
{
    _selfHeatHumidity = true;
}
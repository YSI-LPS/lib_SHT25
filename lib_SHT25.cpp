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
* SHT25  sensor(I2C_SDA, I2C_SCL);
* 
* int main()
* {
*     float temperature, humidity;
*     while(1)
*     {
*         temperature = sensor.getTemperature();
*         humidity = sensor.getTemperature();
*         pc.printf("\r\ntemperature = %6.2f%cC -|- humidity = %6.2f%%RH", temperature, 248, humidity);
*         sensor.waitSafeHeat();
*         sensor.getData(&temperature, &humidity);
*         pc.printf("\ntemperature = %6.2f%cC -|- humidity = %6.2f%%RH", temperature, 248, humidity);
*         sensor.waitSafeHeat();
*     }
* }
* @endcode
* @file          lib_SHT25.cpp
* @date          Jun 2018
* @author        Yannic Simon
*/
#include "lib_SHT25.h"

SHT25::SHT25(PinName sda, PinName scl) : _i2c(sda, scl)
{
    wait_ms(SHT_WAIT_SET);
    softReset();
    setPrecision(SHT_PREC_1214);
    _temperature = _humidity = NAN;
    _selfHeatTemperature = _selfHeatHumidity = true;
}

float SHT25::getTemperature(void)
{
    if(_selfHeatTemperature)
    {
        _selfHeatTemperature = false;
        _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
        return _temperature = readTemperature();
    }
    return _temperature;
}

float SHT25::readTemperature(void)
{
    char command[1] = {SHT_TRIG_TEMP}, rx[3] = {0xFF, 0xFF, 0xFF};
    
    if(!_i2c.write(SHT_I2C_ADDR, command, 1, false))
    {
        wait_ms(SHT_WAIT_TEMP);
        if(!_i2c.read(SHT_I2C_ADDR, rx, 3, false))
            return -46.85 + 175.72 * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0);
    }
    return NAN;
}

float SHT25::getHumidity(void)
{
    if(_selfHeatHumidity)
    {
        _selfHeatHumidity = false;
        _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
        return _humidity = readHumidity();
    }
    return _humidity;
}

float SHT25::readHumidity(void)
{
    char command[1] = {SHT_TRIG_RH}, rx[3] = {0xFF, 0xFF, 0xFF};
    
    if(!_i2c.write(SHT_I2C_ADDR, command, 1, false))
    {
        wait_ms(SHT_WAIT_RH);
        if(!_i2c.read(SHT_I2C_ADDR, rx, 3, false))
            return -6.0 + 125.0 * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0);
    }
    return NAN;
}

void SHT25::getData(float *tempC, float *relHumidity)
{
    if(_selfHeatTemperature && _selfHeatHumidity)
    {
        _selfHeatTemperature = _selfHeatHumidity = false;
        _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
        _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
        readData(tempC, relHumidity);
    }
    else
    {
        *tempC = _temperature;
        *relHumidity = _humidity;
    }
}

void SHT25::readData(float *tempC, float *relHumidity)
{
    *tempC = _temperature = readTemperature();
    *relHumidity = _humidity = readHumidity();
}

bool SHT25::setPrecision(char precision)
{
    char command[2] = {SHT_WRITE_REG, precision};

    if(!_i2c.write(SHT_I2C_ADDR, command, 2, false))
    {
        wait_ms(SHT_WAIT_SET);
        return true;
    }
    return false;
}

bool SHT25::softReset()
{
    char command[1] = {SHT_SOFT_RESET};
    
    if (!_i2c.write(SHT_I2C_ADDR, command, 1, false))
    {
        wait_ms(SHT_WAIT_SET);
        return true;
    }
    return false;
}

void SHT25::waitSafeHeat(void)
{
    while(!_selfHeatTemperature || !_selfHeatHumidity)
        wait_ms(SHT_WAIT_SET);
}

void SHT25::keepSafeTemperature(void)
{
    _selfHeatTemperature = true;
}

void SHT25::keepSafeHumidity(void)
{
    _selfHeatHumidity = true;
}
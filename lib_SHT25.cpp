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
#include "lib_SHT25.h"

SHT25::SHT25(PinName sda, PinName scl) : _i2c(sda, scl)
{
    softReset();
    wait_ms(100);
    setPrecision(SHT_PREC_1214);
    _selfHeat = true;
}

float SHT25::getTemperature(void)
{
    if(_selfHeat)
    {
        _selfHeat = false;
        _t.attach(callback(this, &SHT25::keepSelf), SHT_SELF_HEATING);
        return readTemperature();
    }
    return _temperature;
}

float SHT25::readTemperature(void)
{
    char command[1] = {SHT_TRIG_TEMP}, rx[3] = {0xFF, 0xFF, 0xFF};
    
    _i2c.write(SHT_I2C_ADDR, command, 1, false);
    wait_ms(85);
    _i2c.read(SHT_I2C_ADDR, rx, 3, false);
    
    return _temperature = -46.85 + 175.72 * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0);
}

float SHT25::getHumidity(void)
{
    if(_selfHeat)
    {
        _selfHeat = false;
        _t.attach(callback(this, &SHT25::keepSelf), SHT_SELF_HEATING);
        return readHumidity();
    }
    return _humidity;
}

float SHT25::readHumidity(void)
{
    char command[1] = {SHT_TRIG_RH}, rx[3] = {0xFF, 0xFF, 0xFF};
    
    _i2c.write(SHT_I2C_ADDR, command, 1, false);
    wait_ms(29);
    _i2c.read(SHT_I2C_ADDR, rx, 3, false);
    
    return _humidity = -6.0 + 125.0 * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0);
}

void SHT25::getData(float *tempC, float *relHumidity)
{
    if(_selfHeat)
    {
        _selfHeat = false;
        _t.attach(callback(this, &SHT25::keepSelf), SHT_SELF_HEATING);
        readData(tempC, relHumidity);
    }
}

void SHT25::readData(float *tempC, float *relHumidity)
{
    *tempC = readTemperature();
    *relHumidity = readHumidity();
}

int SHT25::setPrecision(char precision)
{
    char command[2] = {SHT_WRITE_REG, precision};

    return _i2c.write(SHT_I2C_ADDR, command, 2, false);
}

bool SHT25::softReset()
{
    char command[1] = {SHT_SOFT_RESET};
    
    if (_i2c.write(SHT_I2C_ADDR, command, 1, false) != 0)
    {
        wait_ms(15);
        return false;
    }
    
    return true;
}

void SHT25::keepSelf(void)
{
    _selfHeat = true;
}
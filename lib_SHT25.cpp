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

SHT25::SHT25(PinName sda, PinName scl, enum_sht_prec prec, int frequency) : _i2c(sda, scl)
{
    _i2c.frequency(frequency);
    setPrecision(prec);
    _temperature = _humidity = NAN;
    _selfHeatTemperature = _selfHeatHumidity = false;
    _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
    _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
}

void SHT25::getData(float *tempC, float *relHumidity)
{
    *tempC = _temperature;
    *relHumidity = _humidity;
    if(_selfHeatTemperature && _selfHeatHumidity)
        readData(tempC, relHumidity);
}

void SHT25::readData(float *tempC, float *relHumidity)
{
    *tempC = _temperature = readTemperature();
    *relHumidity = _humidity = readHumidity();
}

float SHT25::getTemperature(void)
{
    if(_selfHeatTemperature)
        _temperature = readTemperature();
    return _temperature;
}

float SHT25::readTemperature(void)
{
    char cmd[1] = {SHT_TRIG_TEMP_NHOLD}, rx[3] = {0xFF, 0xFF, 0xFF};
    if(!_i2c.write(SHT_I2C_ADDR_WRITE, cmd, 1, false))
    {
        for(int i = 0; _i2c.read(SHT_I2C_ADDR_READ, rx, 3, false) && (i < 1889); i++);  //1889*45ms(time to read at 400KHz) = 85ms(time max to measure T on 14bit)
        _selfHeatTemperature = false;
        _t.attach(callback(this, &SHT25::keepSafeTemperature), SHT_SELF_HEATING);
        return -46.85f + 175.72f * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0f);
    }
    return NAN;
}

float SHT25::getHumidity(void)
{
    if(_selfHeatHumidity)
        _humidity = readHumidity();
    return _humidity;
}

float SHT25::readHumidity(void)
{
    char cmd[1] = {SHT_TRIG_RH_NHOLD}, rx[3] = {0xFF, 0xFF, 0xFF};
    if(!_i2c.write(SHT_I2C_ADDR_WRITE, cmd, 1, false))
    {
        for(int i = 0; _i2c.read(SHT_I2C_ADDR_READ, rx, 3, false) && (i < 667); i++);   //667*45ms(time to read at 400KHz) = 30ms(time max to measure RH on 12bit)
        _selfHeatHumidity = false;
        _h.attach(callback(this, &SHT25::keepSafeHumidity), SHT_SELF_HEATING);
        return -6.0f + 125.0f * ((((rx[0] << 8) | rx[1]) & 0xFFFC) / 65536.0f);
    }
    return NAN;
}

bool SHT25::setPrecision(const enum_sht_prec precision)
{
    char cmd[2] = {SHT_WRITE_REG_USER, precision};
    return !_i2c.write(SHT_I2C_ADDR_WRITE, cmd, 2, false);
}

bool SHT25::softReset()
{
    char cmd[1] = {SHT_SOFT_RESET};
    return !_i2c.write(SHT_I2C_ADDR_WRITE, cmd, 1, false);
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
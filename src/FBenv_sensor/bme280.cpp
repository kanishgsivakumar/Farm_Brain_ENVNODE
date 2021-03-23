#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

bool bme280Init(Adafruit_BME280* bme280,uint8_t addr)
{
    bme280->begin(addr);
    bme280->setSampling(Adafruit_BME280::MODE_FORCED,
                    Adafruit_BME280::SAMPLING_X1, // temperature
                    Adafruit_BME280::SAMPLING_X1, // pressure
                    Adafruit_BME280::SAMPLING_X1, // humidity
                    Adafruit_BME280::FILTER_OFF   );
    if (bme280->sensorID() == 0x60)
    {
        return true;
    }
    else
    {
        return false;
    }
}


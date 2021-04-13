#include <BH1750.h>
/**
 * initialize sensor
 * @param lightmeter BH1750 instance
 * @param sda sda pin number
 * @param scl scl pin number
 * @return a boolean if init is successful
 */
bool bh1750Init(BH1750* lightmeter, int sda,int scl)
{
    Wire.begin(sda,scl);
    return lightmeter -> begin();
}
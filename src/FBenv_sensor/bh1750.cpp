#include <BH1750.h>

bool bh1750Init(BH1750* lightmeter, int sda,int scl)
{
    Wire.begin(sda,scl);
    return lightmeter -> begin();
}
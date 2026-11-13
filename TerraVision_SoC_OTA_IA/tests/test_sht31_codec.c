/** @file test_sht31_codec.c @brief Pruebas de CRC y temperatura SHT31. */
#include "sht31_codec.h"
#include <assert.h>
#include <math.h>
#include <stdio.h>

int main(void)
{
    assert(tv_sht31_crc8((const unsigned char[]){0xBE,0xEF},2)==0x92);
    unsigned char sample[6]={0x66,0x66,0x93,0x80,0x00,0xA2};
    float temp=-99, humidity=-99;
    assert(tv_sht31_decode(sample,&temp,&humidity));
    assert(fabsf(temp-25.0f)<0.05f);
    assert(fabsf(humidity-50.0f)<0.05f);
    sample[2] ^= 0x01;
    assert(!tv_sht31_decode(sample,&temp,&humidity));
    printf("PASS: CRC8 y conversion SHT31\n");
    return 0;
}

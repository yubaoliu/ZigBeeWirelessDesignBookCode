#include "Sensor.h"
#include <ioCC2530.h>

#define ADC_REF_115V 0x00
#define ADC_DEC_256  0x20
#define ADC_CHN_TEMP 0x0e
#define ADC_DEC_064 0x00
#define ADC_CHN_VDD3 0x0f
int8 readTemp(void)
{
    static uint16 reference_voltage;
    static uint8 bCalibrate=TRUE;
    uint16 value;
    int8 temp;
    
    ATEST=0x01;
    TR0|=0x01;
    ADCIF=0;
    ADCCON3=(ADC_REF_115V|ADC_DEC_256|ADC_CHN_TEMP);
    while(!ADCIF);
	ADCIF=0;
	value=ADCL;
	value|=((uint16)ADCH)<<8;
	value>>=4;
	if(bCalibrate)
		{
			reference_voltage=value;
			bCalibrate=FALSE;
		}
	temp=22+((value-reference_voltage)/4);
	return temp;
    
}
unsigned int getVddvalue(void)
{
  unsigned int value;
  unsigned char tmpADCCON3=ADCCON3;
  
  ADCIF=0;
  ADCCON3=(ADC_REF_115V|ADC_DEC_064|ADC_CHN_VDD3);
  while(!ADCIF);
  
  value=ADCH;
  ADCCON3=tmpADCCON3;
  return (value);
}
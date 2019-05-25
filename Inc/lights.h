#ifndef __lights_H
#define __lights_H

//#ifdef __cplusplus
// extern "C" {
// }
 
 
void ShiftRegLightThread(void);
void RGB_LightThread(void); 

void TransmitShiftRegLight(uint8_t *light_sequence);

void RGB_TurnOn_Red();
void RGB_TurnOn_Green();
void RGB_TurnOn_Blue();
void RGB_TurnOff_Red();
void RGB_TurnOff_Green();
void RGB_TurnOff_Blue();

//#endif
#endif
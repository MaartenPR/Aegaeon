#include <stdint.h>
#include <AVR/io.h>
#include <AVR/pgmspace.h>
#include <AVR/eeprom.h>
#include "FX.h"
#include "DCC_Config.h"
#include "Motor.h"
#include "globals.h"

uint8_t FX_Soft_Start_Active;
//uint8_t FX_Soft_Start_Accum[2];
uint32_t FX_Soft_Start_Prev_Time[2];

uint16_t const FX_Animation_Start[] = {
    0x70,   //0
    0x90,   //1
    0xB0,   //2
    0xD0,   //3
    0xF0,   //4
    0x110,  //5
    0x130,  //6
    0x150,  //7
    0x170   //8
    //    0x190   //9
};

void FX_Initialize(void)
{
    //setup outputs. This will vary with decoder!
    DDRA |= ((1 << DDA5) | (1 << DDA6));
    PORTA &= ~((1 << PORTA5) | (1 << PORTA6));
}

void FX_SetFunction( uint8_t new_fx, uint8_t new_fx_group, uint8_t consisted )
{
    uint16_t fx_status;
    uint16_t mask;
    
    //Bits to F for fx_status and mask
    // 9  8  7  6  5  4  3  2  1   0
    // F8 F7 F6 F5 F4 F3 F2 F1 FLr FLf
    // notice that here we only care about FL -- F8!
    
    if(FX_GROUP_1 == new_fx_group)
    {
        fx_status = new_fx;
        mask = 0x003F; //only care about lower 6 bits, mask off the rest
    }
    else //if(FX_GROUP_2 == new_fx_group)
    {
        fx_status = (new_fx << 6); //offset is by 6, as group 2 is F5..F8
        mask = 0x03C0;
    }
    
    if(DCC_consist_address)
    {
        uint16_t temp = (eeprom_read_byte(CV_CONSIST_ADDRESS_ACTIVE_FOR_F1_F8) << 2) | ((eeprom_read_byte(CV_CONSIST_ADDRESS_ACTIVE_FOR_FL_F9_F12) & 0x01) << 1);
        if(consisted)
            mask &= temp;
        else //exclude those functions under control by the consist address
            mask &= ~temp;
    }
    
    uint8_t i;
    for(i = 0; i < 10; ++i) // cycle through FL -- F8
    {
        if(mask & (1<<i))
        {
            uint8_t temp =  eeprom_read_byte((const uint8_t *) CV_OUTPUT_LOCATION_FLF + i);
            //notice that because of the crazy NMRA specs, we need to mess a bit with temp;
            if(i > 4)
                temp = temp << 3;
            if (fx_status & (1 << i))
            {
                FX_Active |= temp;
            }
            else
            {
                FX_Active &= ~temp;
            }
        }
    }
}

void FX_Update(void)
{
    //here is where we update the next brightness value for each of the outputs.
    uint8_t i;
    uint32_t time = millis();
    uint16_t brightness;
    
    for(i = 0; i < 2; ++i) //for each physical output
    {
    	brightness = 0;
        if (FX_Active & (1<<i)) //if that output is currently active
        {
            //////////determine the base-level brightness.
            //first, check direction
            uint8_t on, dim, dim_stop;
            if (_current_speed > 0) //forward
            {
                on = FX_ON_FORWARD;
                dim = FX_DIM_FORWARD;
                dim_stop = FX_DIM_FORWARD_STOP;
            }
            else
            {
                on = FX_ON_REVERSE;
                dim = FX_DIM_REVERSE;
                dim_stop = FX_DIM_REVERSE_STOP;
            }
            if (FX[i] & on)
            {
                //FX_Active |= (1 << i); //turn on output one // WHY!
                
                uint8_t _abs_speed = _current_speed;
                if(_current_speed < 0) _abs_speed = -1*_current_speed;
                if ((FX[i] & dim)
                    || ((FX[i] & dim_stop) && (1 == _abs_speed)) //if automatic Rule 17 features are in effect
                    || (FX_Active & (1<<(i+4)))) //manual Rule 17 features are in effect
                {
                    brightness = FX_Dim_Brightness[i];
                }
                else
                {
                    brightness = FX_Brightness[i];
                }
            }
            
            //////////////////now, check to see if we are animating this output
            //two cases for which we have an animation:
            //    a non-triggerable animation is selected, in which case we always play it,
            // or
            //  a triggerable animation is selected, and the associated output is active
            if( (FX_Animation[i] & FX_ANIMATION_MASK) &&
               ( (FX_Active & (1<<(i+2))) || (0== (FX_Animation[i] & FX_TRIGGERABLE_MASK)) ) )
            {
                //first, figure out where in the animation we are.
                uint16_t cur_frame = FX_Animation_Frame[i] + FX_Animation_Start[(FX_Animation[i]&FX_ANIMATION_MASK) - 1];
                //then, attenuate the brightness by the current frame value
                brightness *= (eeprom_read_byte((const uint8_t *) cur_frame) + 1);
                brightness /= 256;
                //                //and clip, just in case. This should never actually happen.
                //                if(brightness > 255) //should never happen
                //                {
                //                    brightness = 255;
                //                }
            }
            
            //and set the calculated brightness! Attenuating by soft start, if needed
            int16_t diff = (uint16_t)brightness - (uint16_t)Output_Match_Buf[i];
            if( (diff > FX_SOFT_START_THRESHOLD) && (FX[i] & FX_SOFT_START) && (0 == (FX_Soft_Start_Active & (1<<i))) )
                //(brightness > Output_Match_Buf[i]) && ((brightness - Output_Match_Buf[i]) > FX_SOFT_START_THRESHOLD) && (FX[i] & FX_SOFT_START) && (0 == (FX_Soft_Start_Active & (1<<i))) )
            {
                FX_Soft_Start_Prev_Time[i] = time;
                FX_Soft_Start_Active |= (1<<i);
                //implicit: Don't change Output_Match_Buf[i]!
            }
            else if(FX_Soft_Start_Active & (1<<i))
            {
                if(time_delta_32(time, FX_Soft_Start_Prev_Time[i]) >= FX_SOFT_START_PERIOD)
                {
                    Output_Match_Buf[i] += 1;
                    FX_Soft_Start_Prev_Time[i] = time;
                }
                if(Output_Match_Buf[i] >= brightness)
                {
            	    Output_Match_Buf[i] = brightness;
                    FX_Soft_Start_Active &= ~(1<<i);
                }
            }
            else
            {
                Output_Match_Buf[i] = (uint8_t)brightness;
            }
            
        }
        else
        {
            FX_Soft_Start_Active &= ~(1<<i);
            Output_Match_Buf[i] = 0;
        }
        
        if (time_delta_32(time, FX_Prev_Time[i]) >= FX_Period[i]) //one frame every 62 ms, gives a total period of 2s
        {
            ++FX_Animation_Frame[i];
            if (FX_Animation_Frame[i] == 32)
            {
                FX_Animation_Frame[i] = 0;
            }
            FX_Prev_Time[i] = time;
        }
    }
}

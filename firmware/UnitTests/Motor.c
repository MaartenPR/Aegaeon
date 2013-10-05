#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <util/atomic.h>

#include <stdint.h>
#include "DCC_Config.h"
#include "DCC_Decoder.h"
#include "Motor.h"
#include "globals.h"

#define ENABLE (1<<PB1)

volatile uint8_t sample, sample_ready, min_ADC;
int16_t sum_err;

uint8_t PDFF(int16_t set, int16_t measured)
{
//    int16_t i_set = set;
//    int16_t i_measured = measured;
    if (0 == set) //setting a 0 voltage, don't bother.
    {
        //but do measure and set the minimum measured voltage!
        //min_ADC = measured;
        return 0;
    }

    //check to see if BEMF is enabled

    //check to see if we are above the cutoff speed

    int16_t err = set - measured;
    sum_err += err;
    //prevent windup
    if (sum_err > 1024) sum_err = 1024;
    else if (sum_err < -1024) sum_err = 1024;

    int16_t iterm;
    if(sum_err < 0)
    {
        iterm = -1 * (BEMF_Ki * sum_err) / 256;
        iterm *= -1;
    }
    else
    {
        iterm = (BEMF_Ki * sum_err) / 256;
    }

    int16_t fterm;
     if(DCC_consist_address) //if in a consist
     {
             fterm = ((BEMF_Kf_consist * (uint16_t)set) / 256) - measured;
     }
     else
     {
             fterm = ((BEMF_Kf * (uint16_t)set) / 256) - measured;

     }

    int16_t pterm = (BEMF_Kp * fterm) / 256;

    int16_t retval = iterm + pterm + set;
    if(retval < 0) return 0;
    else if(retval > 255) return 255;
    else return retval;
}

void Motor_Initialize(void)
{

    //pre-condition: DCC_Config_Initialize has been called first. Interrupts have been disabled

    //first, compute the accel and decel factors.
    //TODO THIS MAY HAVE TO BE CALLED BY DCC_Config if these CV values change!!
    // (the contents of CV#3*.896)/(number of speed steps in use) = accel factor in seconds/speed_step, a weird unit of measure.

    //do other things to set up the hardware.
    //on ATtiny84A, setup TIMER0 for both motor control PWM on output compare B and a millis timer on compare match overflow

    //set OC0A and OC0B to output; OC0A is on PB2, OC0B is on PA7
    DDRB |= (1 << DDB2);
    DDRA |= (1 << DDA7);

    //set up enable pin too
    DDRB |= (1 << DDB1);
    PORTB |= (1 << PB1); //and set to ENABLE to begin with.

    TCCR0A = (1 << COM0B1) | (1 << WGM00); // //Clear OCB0 on up-count clear on down-count, phase correct PWM
    if(eeprom_read_byte((const uint8_t*)CV_MOTOR_FREQUENCY)) // positive value = high frequency, use fast PWM
        TCCR0A |= (1 << WGM01);
    TCCR0B = (1 << CS00); //use /1 prescaler

    TCNT0 = 0;
    //OCR0A = 0xFF;
    OCR0B = OCR0A = 0; //turn output off.

    TIMSK0 |= (1 << TOIE0); //enable overflow interrupt so we can count micros

    //enable the ADC for BEMF measurement
    DIDR0 |= (1 << ADC3D);
    ADCSRA |= (1 << ADEN) | (1 << ADIE) | (1 << ADPS2) | (1 << ADPS0); //enable ADC, enable interrupt, set prescalar to 32 to get a clock of 250KHz. See pg 135 of ATtiny84a manual
    ADCSRB |= (1 <<  ADLAR); //left justified ADC output for 8-bit reads
    ADMUX = 0x03; //enable ADC3 for analog conversion, use VCC as reference.
    /*
            _millis_counter = 0;
        _micros_rollover = 0;
            _micros_counter = 0;
            PA5_match_buf = 0;
            PA6_match_buf = 0;
            PA5_match = 0;
            PA6_match = 0;
            softcount = 0;
            _goal_speed = 0; //forward, stop
            _current_speed = 0;
            //_voltage_level = 0;
            _max_speed = 0;
            _jog_time = 0;
     */
    _current_speed = _goal_speed = 1; //forward stop
    min_ADC = 0x93;
}

//volatile int8_t _bemf_adjust;

ISR(ADC_vect) //ISR for ADC conversion complete
{
    sample = (uint8_t)((1036*(ADCH - min_ADC))>>8);
    _prev_bemf_time = millis(); //set the time of last measurement
    sample_ready = 1;
}

ISR(TIM0_OVF_vect)
{
    _micros_rollover += 32; //at 8MHz, /1 prescaler, it takes 32 microseconds to cycle through
    _micros_counter += 32;
    if (_micros_rollover >= 1000) //update millis
    {
        ++_millis_counter;
        _micros_rollover -= 1000;
    }

    //also handle FX
    if (softcount++ == 0)
    {
        Output_Match[0] = Output_Match_Buf[0];
        Output_Match[1] = Output_Match_Buf[1];
        if (Output_Match[0])
            PORTA |= (1 << PA5); //only turn on if match is non-zero! Prevents flicker when should be off.
        if (Output_Match[1])
            PORTA |= (1 << PA6);
    }

    if (Output_Match[0] == softcount) //just rolled over, turn output off
    {
        PORTA &= ~(1 << PA5);
    }
    if (Output_Match[1] == softcount) //just rolled over, turn output off
    {
        PORTA &= ~(1 << PA6);
    }
}

uint32_t millis(void)
{
    uint32_t m;

    ATOMIC_BLOCK(ATOMIC_FORCEON)
    {
        m = _millis_counter;
    }
    return m;
}

uint32_t micros(void)
{
    uint32_t m;

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
    {
        m = _micros_counter;
    }
    return m;
}


void Motor_EStop(int8_t dir)
{
    //first, kill the h-bridge
    PORTB &= ~(1 << PB1); //bring ENABLE low, which forces both motor outputs low
    if (dir >= 0)
        _current_speed = _goal_speed = 1; //immediate forward stop
    else
        _current_speed = _goal_speed = -1; //immediate reverse stop
    //_voltage_level = _voltage_adjust = 0; //make sure we don't re-energize!
    OCR0B = OCR0A = 0; //shut it down; TODO anything else we can do?
}

//in all of the following, 0 = stop; 1 is first movement step.

//void Motor_Set_Speed_By_DCC_Speed_Step_14(int8_t notch)
//{
//	Motor_Set_Speed_By_DCC_Speed_Step_128( (notch*18) >> 1);
//}

//wants a value in the range 0..27

void Motor_Set_Speed_By_DCC_Speed_Step_28(int8_t notch)
{
    //valid range is 1..29, -1..-29
    uint8_t new_notch = notch;
    if (notch < 0) new_notch = -notch;
    else new_notch = notch;

    if (new_notch <= 1) new_notch = 1;
    else if (new_notch >= 29) new_notch = 127;
    else new_notch = ((new_notch - 1) *9) >> 1; //notch-1 * 4.5

    if (notch < 0) //reverse it again
        new_notch = -new_notch;
    Motor_Set_Speed_By_DCC_Speed_Step_128(new_notch);
}

//void Motor_Set_Restriction(uint8_t notch)
//{
//notch comes in as a 28-step, need to convert to 128 step;
//valid range is 1..29, -1..-29
//	uint8_t new_notch = notch;
//	if (new_notch <= 1) new_notch = 1;
//	else if (new_notch >= 29) new_notch = 127;
//	else new_notch = ((new_notch - 1) *9) >> 1; //notch-1 * 4.5

//	_max_speed = new_notch;
//}

//void Motor_Remove_Restriction(void)
//{
//	_max_speed = 0;
//}

void Motor_Set_Speed_By_DCC_Speed_Step_128(int8_t notch)
{
    //valid range is -127..-1, 1..127
    //	//converted internally to -127..-1,0..126
    //if(_max_speed)
    //{
    //	if(-notch < _max_speed) //negative
    //	notch = -_max_speed;
    //	else if(notch > _max_speed)
    //	notch = _max_speed;
    //}

    //	if (notch > 0)
    //	_goal_speed = notch - 1;
    //	else
    //	_goal_speed = notch;
    _goal_speed = notch;
}

uint16_t time_delta_16(uint16_t curtime, uint16_t prevtime)
{
    if (curtime < prevtime) //we have a rollover situation
    {
        return (0xFFFF - prevtime) +curtime;
    }
    else
    {
        return curtime - prevtime;
    }
}

uint32_t time_delta_32(uint32_t curtime, uint32_t prevtime)
{
    if (curtime < prevtime) //we have a rollover situation
    {
        return (0xFFFFFFFF - prevtime) +curtime;
    }
    else
    {
        return curtime - prevtime;
    }
}

void Motor_Update(void)
{
    //check to see if we need to start an AD conversion
    uint8_t abs_speed = _current_speed;
    if(_current_speed < 0) abs_speed = -1*_current_speed;
    if ((abs_speed < BEMF_cutoff) & (time_delta_32(millis(), _prev_bemf_time) >= BEMF_period))
    {
        //turn off motor
        PORTB |= (1 << PB1); //bring ENABLE high
        TCCR0A |= (1 << COM0B1) | (1 << COM0A1); //tells motor output to set BOTH high.
        OCR0B = OCR0A = 0xFF;
        //start an AD conversion
        ADCSRA |= (1 << ADSC);
    }

    //first, determine whether we need to do anything
    //we only act if
    // a) BEMF is disabled or not active
    // b) -OR- BEMF is enabled and active, and a new sample is ready
    // (E&A&R) | ~(E&A)
    // ~(E&A) | R
    else if ((abs_speed >= BEMF_cutoff) | sample_ready) //if BEMF is inactive, or there is a sample ready
    {
        uint32_t time = micros();
        uint32_t delta = time_delta_32(time, _prev_time);
        _prev_time = time;

        //first calculate next speed step
        if (_goal_speed != _current_speed) //need to update the current speed!
        {
            //not just if at a stop, but take into account the sign of goal_speed! Don't want a kick-start in reverse!
            if( (1 == abs_speed) &&
            ( ((_current_speed > 0) && (_goal_speed > 0)) || ((_current_speed < 0) && (_goal_speed < 0))) &&
            eeprom_read_byte((const uint8_t *) CV_KICK_START) ) //starting to move from a stop
            {
                _kick_start_time = millis();
            }

            PORTB |= (1 << PB1); //bring ENABLE high, in case it had been disabled for an E-Stop
            uint32_t factor;
            //Acceleration = movement /away/ from 0. //Deceleration = movement /toward/ 0.
            // SEVERAL CASES!
            // current speed > goal speed > 0 : DECEL
            // current speed > 0 > goal speed : DECEL //have to move towards 0 first
            // 0 > current speed > goal speed : ACCEL

            // goal speed > 0 > current speed : DECEL //have to move towards 0 first
            // goal speed > current speed > 0 : ACCEL
            // 0 > goal speed > current speed : DECEL

            if (_current_speed > _goal_speed)
            {
                if (_current_speed < 0) //was < 0
                factor = DCC_accel_rate;
                else
                factor = DCC_decel_rate;
            }
            else //_goal > current
            {
                if (_current_speed > 0) //was > 0
                factor = DCC_accel_rate;
                else
                factor = DCC_decel_rate;
            }

            if (factor == 0) //instant speed, no momentum
            {
                //move us to 0 or to _goal_speed, whichever is closer
                if ((_current_speed < -1) && (-1 < _goal_speed)) //moving from negative speeds to 0; make speed 0
                {
                    _current_speed = 1;
                }
                else if ((_current_speed > 1) && (1 > _goal_speed)) //moving from positrive speeds to -1
                {
                    _current_speed = -1;
                }
                else
                {
                    _current_speed = _goal_speed;
                }
            }
            else //need to calculate the new current speed
            {
                accum += delta;
                if (accum / factor) //if enough time has passed to jump a speed step
                {
                    uint8_t incr = (uint8_t) (accum / factor);
                    //accum -= incr; //get it back down to a fractional amount
                    accum = 0;
                    if (_goal_speed > _current_speed)
                    {
                        if ((_current_speed < 0) && ((_current_speed + incr) >= 0))//negative speed
                        {
                            _current_speed = 1;
                        }
                        else if ((_current_speed + incr) >= _goal_speed)
                        {
                            _current_speed = _goal_speed;
                        }
                        else
                        {
                            _current_speed += incr;
                        }
                    }
                    else
                    {
                        if ((_current_speed > 0) && ((_current_speed - incr) <= 0))//negative speed
                        {
                            _current_speed = -1;
                        }
                        else if ((_current_speed - incr) <= _goal_speed)
                        {
                            _current_speed = _goal_speed;
                        }
                        else
                        {
                            _current_speed -= incr;
                        }
                    }
                }
            }
            //smooth out the PID curve
            //if(_bemf_speed)
            //        PID.last_error = _goal_speed - _bemf_speed;
        }
        else
        {
            accum = 0; //reset the accumulator
        }

        uint8_t tccr0a = TCCR0A;
        int16_t voltage = 0;

        //NOW we set the motor voltage

        //three different possibilities: Jogging the motor (highest priority, because this is communicating with CS), Kick-starting the motor, or setting the motor based on set speed step, either forwards or reverse

        if (_jog_time)
        {
            voltage = 0xFF;
            //if(_current_speed == 1) //if stopped, shove it in reverse!
            //{
            //    tccr0a &= ~(1 << COM0B1);
            //    tccr0a |= (1 << COM0A1); //do it in reverse!
            //}
            if (time_delta_32(millis(), _jog_time) >= 6) ///RP 9.2.3, line 47 = 6ms +/- 1ms;
            {
                _jog_time = 0;
            }
        }
        else if (_kick_start_time)
        {
            voltage = eeprom_read_byte((const uint8_t *) CV_KICK_START);

            if (time_delta_32(_kick_start_time, millis()) >= 6)
            {
                _kick_start_time = 0;
            }
        }
        else //use the commanded speed step to set the motor!
        {
            if (_current_speed < 0) //if negative
            {
                //set OC0A
                tccr0a &= ~(1 << COM0B1);
                tccr0a |= (1 << COM0A1);
                //PORTB &= ~(1 << PB1); //direction pin
                //_voltage_level = DCC_speed_table[(_current_speed * -1) - 1]; //THIS IS WHERE we move from from -1..-127 to 0..-126
                voltage = DCC_speed_table[(_current_speed * -1) - 1]; // + _voltage_adjust;
                if (DCC_reverse_trim)
                {
                    voltage = (voltage * DCC_reverse_trim) / 128;
                    if (voltage > 255) voltage = 255;
                }
            }
            else
            {
                //set OC0B
                tccr0a &= ~(1 << COM0A1);
                tccr0a |= (1 << COM0B1);
                //PORTB |= (1 << PB1);
                //_voltage_level = DCC_speed_table[_current_speed]; //THIS also needs to be converted to the range 0..126 from 1..127
                voltage = DCC_speed_table[_current_speed - 1]; // + _voltage_adjust;
                if (DCC_forward_trim)
                {
                    voltage = (voltage * DCC_forward_trim) / 128;

                }
                //speed += _bemf_adjust;
                if (voltage < 0) voltage = 0;
                if (voltage > 255) voltage = 255;
            }

            //If switching mode is enabled, half the voltage
            //TODO fix motor half speed issue!

            //            if(Motor_Switch_Mode)
            if(FX_Active & (1<<4))
            voltage = (uint8_t)(voltage) >> 1;

            //now, if enabled and active, adjust the output voltage with PDFF
            abs_speed = _current_speed;
            if(_current_speed < 0) abs_speed = -1*_current_speed;
            if (abs_speed < BEMF_cutoff) //if BEMF is enabled and active
            {
                voltage = PDFF(voltage, sample);
                sample_ready = 0;
            }
        }
        OCR0B = OCR0A = voltage;
        TCCR0A = tccr0a;
    }
}

void Motor_Jog(void)
{
    if (DCC_SERVICE_MODE || eeprom_read_byte((const uint8_t *) CV_OPS_MODE_BASIC_ACK))
    {
        //increase voltage by a small amount for a brief period.
        _jog_time = millis(); //RP 9.2.3, line 47 = 6ms +/- 1ms;
    }
}
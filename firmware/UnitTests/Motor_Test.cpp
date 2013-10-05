extern "C"
{
#include "DCC_Config.h"
#include "Motor.h"
#include "DCC_Decoder.h"
#include "reset.h"
#include <avr/interrupt.h>
#include <avr/io.h>
void TIM0_OVF_vect(void);
void ADC_vect(void);

}

#include <CppUTest/TestHarness.h>

extern uint8_t eeprom[];
extern uint8_t _defaults[];
extern uint8_t DCC_CV29;
extern uint32_t _millis_counter, _micros_counter;
extern uint32_t _prev_time;
extern int8_t _goal_speed;
extern int8_t _current_speed;
extern uint8_t _voltage_level, _voltage_adjust;
extern uint8_t DCC_speed_table[];
extern uint32_t _jog_time;
extern uint8_t DCC_flags;
extern uint8_t sample;
extern    uint8_t BEMF_cutoff, BEMF_period;
extern    int16_t BEMF_Kp, BEMF_Ki;
extern    uint16_t BEMF_Kf, BEMF_Kf_consist;

TEST_GROUP(MotorTests)
{

    void setup()
    {
        uint16_t i;
        for (i = 0; i < 106; ++i)
            eeprom[i] = 0xFF;
        soft_reset();
        //first, force ops mode
        DCC_flags |= DCC_FLAGS_OPS_MODE_MASK;
        DCC_Config_Initialize();
    }

    void teardown()
    {

    }
};

TEST(MotorTests, Initialization)
{
    CHECK_EQUAL(0x00, OCR0B);
}

TEST(MotorTests, millis_0)
{
    TIM0_OVF_vect();
    CHECK_EQUAL(0x00, millis());
}

TEST(MotorTests, millis_1)
{
    //uint16_t i = 0;
    while (_micros_counter < 1000)
    {
        TIM0_OVF_vect();
        //i += 32; //number of micros incremented each time ISR is called;
    }
    //i = 0;
    CHECK_EQUAL(0x01, millis());
}

/*********28 speed steps w/ 3-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_28_STOP)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FIRST)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(2);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MID)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(15);
    Motor_Update();
    CHECK_EQUAL(125, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MAX) //MAX SPEED
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(29);
    Motor_Update();
    CHECK_EQUAL(255, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_28_3)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(3);
    Motor_Update();
    CHECK_EQUAL(15, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_4)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(4);
    Motor_Update();
    CHECK_EQUAL(23, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_25)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(25);
    Motor_Update();
    CHECK_EQUAL(216, OCR0B);
}

/*********28 speed steps negative speed w/ 3-step speed table*********/


TEST(MotorTests, TestSpeedSettings_noAccel_28_STOP_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FIRST_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-2);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MID_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-15);
    Motor_Update();
    CHECK_EQUAL(125, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MAX_NEG) //MAX SPEED
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-29);
    Motor_Update();
    CHECK_EQUAL(255, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_28_3_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-3);
    Motor_Update();
    CHECK_EQUAL(15, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_4_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-4);
    Motor_Update();
    CHECK_EQUAL(23, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_25_NEG)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(-25);
    Motor_Update();
    CHECK_EQUAL(216, OCR0B);
}

/*********28 speed steps w/alternate 3-step speed table*********/


TEST(MotorTests, TestSpeedSettings_noAccel_28_STOP_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_28(1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FIRST_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(2);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MID_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(15);
    Motor_Update();
    CHECK_EQUAL(19, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_MAX_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(29);
    Motor_Update();
    CHECK_EQUAL(100, OCR0B);
}

//very tricky!

TEST(MotorTests, TestSpeedSettings_noAccel_28_3_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(3);
    Motor_Update();
    CHECK_EQUAL(11, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_4_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(4);
    Motor_Update();
    CHECK_EQUAL(11, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_16_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(16);
    Motor_Update();
    CHECK_EQUAL(23, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_26_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);

    Motor_Set_Speed_By_DCC_Speed_Step_28(26);
    Motor_Update();
    CHECK_EQUAL(80, OCR0B);
}

/*********28 speed steps w/28-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_STOP)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(1);
    Motor_Update();
    CHECK_EQUAL(0x00, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_FIRST)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(2);
    Motor_Update();
    CHECK_EQUAL(6, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_MID)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(15);
    Motor_Update();
    CHECK_EQUAL(125, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_MAX) //MAX SPEED
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(29);
    Motor_Update();
    CHECK_EQUAL(255, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_3)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(3);
    Motor_Update();
    CHECK_EQUAL(16, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_4)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(4);
    Motor_Update();
    CHECK_EQUAL(24, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_FancyCurve_25)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(25);
    Motor_Update();
    CHECK_EQUAL(216, OCR0B);
}

/*********28 speed steps w/alternate 28-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_STOP)
{
    DCC_Config_Write_CV(29, _defaults[29] | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    CHECK_EQUAL(_defaults[29] | 0x10, eeprom[29]);
    Motor_Set_Speed_By_DCC_Speed_Step_28(1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_FIRST)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(2);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_MID)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(15);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_MAX) //MAX SPEED
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(29);
    Motor_Update();
    CHECK_EQUAL(220, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_3)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(3);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_4)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(4);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_28_AltFancyCurve_25)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_28(25);
    Motor_Update();
    CHECK_EQUAL(138, OCR0B);
}

/*********128 speed steps w/3-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_128_STOP)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FIRST)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(2);
    Motor_Update();
    CHECK_EQUAL(1, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_MID)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(64);
    Motor_Update();
    CHECK_EQUAL(128, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_MAX) //MAX SPEED
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(127);
    Motor_Update();
    CHECK_EQUAL(255, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_128_3)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(3);
    Motor_Update();
    CHECK_EQUAL(3, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_62)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(62);
    Motor_Update();
    CHECK_EQUAL(123, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_66)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(66);
    Motor_Update();
    CHECK_EQUAL(132, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_125)
{
    Motor_Set_Speed_By_DCC_Speed_Step_128(125);
    Motor_Update();
    CHECK_EQUAL(250, OCR0B);
}

/*********128 speed steps w/alternate 3-step speed table*********/
TEST(MotorTests, TestSpeedSettings_noAccel_128_STOP_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(1);
    Motor_Update();
    CHECK_EQUAL(0, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FIRST_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(2);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_MID_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(64);
    Motor_Update();
    CHECK_EQUAL(20, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_MAX_new3step) //MAX SPEED
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(127);
    Motor_Update();
    CHECK_EQUAL(100, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_128_3_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(3);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_62_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(62);
    Motor_Update();
    CHECK_EQUAL(19, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_66_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(66);
    Motor_Update();
    CHECK_EQUAL(22, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_125_new3step)
{
    DCC_Config_Write_CV(CV_V_START, 10);
    DCC_Config_Write_CV(CV_V_MID, 20);
    DCC_Config_Write_CV(CV_V_HIGH, 100);
    Motor_Set_Speed_By_DCC_Speed_Step_128(125);
    Motor_Update();
    CHECK_EQUAL(97, OCR0B);
}

/*********128 speed steps w/28-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_STOP)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(1);
    Motor_Update();
    CHECK_EQUAL(0x00, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_FIRST)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(2);
    Motor_Update();
    CHECK_EQUAL(2, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_MID)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(64);
    Motor_Update();
    CHECK_EQUAL(127, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_MAX) //MAX SPEED
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(127);
    Motor_Update();
    CHECK_EQUAL(0xFF, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_3)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(3);
    Motor_Update();
    CHECK_EQUAL(4, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_62)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(62);
    Motor_Update();
    CHECK_EQUAL(123, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_66)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(66);
    Motor_Update();
    CHECK_EQUAL(131, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_FancyCurve_125)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    Motor_Set_Speed_By_DCC_Speed_Step_128(125);
    Motor_Update();
    CHECK_EQUAL(250, OCR0B);
}

/*********128 speed steps w/alternate 28-step speed table*********/

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_STOP)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(1);
    Motor_Update();
    CHECK_EQUAL(0x00, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_FIRST)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(2);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_MID)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(64);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_MAX) //MAX SPEED
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(127);
    Motor_Update();
    CHECK_EQUAL(220, OCR0B);
}

//trickier…

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_3)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(3);
    Motor_Update();
    CHECK_EQUAL(5, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_62)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(62);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_66)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(66);
    Motor_Update();
    CHECK_EQUAL(10, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_noAccel_128_AltFancyCurve_125)
{
    DCC_Config_Write_CV(29, DCC_CV29 | 0x10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_1, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_2, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_3, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_4, 5);
    DCC_Config_Write_CV(CV_SPEED_TABLE_5, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_6, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_7, 6);
    DCC_Config_Write_CV(CV_SPEED_TABLE_8, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_9, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_10, 7);
    DCC_Config_Write_CV(CV_SPEED_TABLE_11, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_12, 8);
    DCC_Config_Write_CV(CV_SPEED_TABLE_13, 9);
    DCC_Config_Write_CV(CV_SPEED_TABLE_14, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_15, 10);
    DCC_Config_Write_CV(CV_SPEED_TABLE_16, 12);
    DCC_Config_Write_CV(CV_SPEED_TABLE_17, 17);
    DCC_Config_Write_CV(CV_SPEED_TABLE_18, 20);
    DCC_Config_Write_CV(CV_SPEED_TABLE_19, 40);
    DCC_Config_Write_CV(CV_SPEED_TABLE_20, 60);
    DCC_Config_Write_CV(CV_SPEED_TABLE_21, 80);
    DCC_Config_Write_CV(CV_SPEED_TABLE_22, 100);
    DCC_Config_Write_CV(CV_SPEED_TABLE_23, 120);
    DCC_Config_Write_CV(CV_SPEED_TABLE_24, 140);
    DCC_Config_Write_CV(CV_SPEED_TABLE_25, 160);
    DCC_Config_Write_CV(CV_SPEED_TABLE_26, 180);
    DCC_Config_Write_CV(CV_SPEED_TABLE_27, 200);
    DCC_Config_Write_CV(CV_SPEED_TABLE_28, 220);
    Motor_Set_Speed_By_DCC_Speed_Step_128(125);
    Motor_Update();
    CHECK_EQUAL(211, OCR0B);
}


//TODO forward trim, reverse trim


//accel and decel

TEST(MotorTests, TestAccel_v1)
{
    //CV3 * .896 / 128 = sec/step
    DCC_Config_Write_CV(CV_ACCELERATION_RATE, 10);
    Motor_Set_Speed_By_DCC_Speed_Step_28(29); //max speed
    //should take 320 milliseconds to increase one step;
    while (OCR0B < 255)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(8963, _millis_counter); //should really be 8960, but with rounding errors…
}

TEST(MotorTests, TestAccel_v2)
{
    //CV3 * .896 / 128 = sec/step
    DCC_Config_Write_CV(CV_ACCELERATION_RATE, 25);
    Motor_Set_Speed_By_DCC_Speed_Step_28(29); //max speed
    while (OCR0B < 255)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(22401, _millis_counter); //should be 22400 really
}

TEST(MotorTests, TestAccel_v2_NEG_v1)
{
    //CV3 * .896 / 128 = sec/step
    Motor_Set_Speed_By_DCC_Speed_Step_28(-1);
    Motor_Update(); //force immediate speed update
    DCC_Config_Write_CV(CV_ACCELERATION_RATE, 25);
    Motor_Set_Speed_By_DCC_Speed_Step_28(-29); //max speed
    while (OCR0B < 255)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(22401, _millis_counter); //should be 22400 really
}

TEST(MotorTests, TestDecel_v1)
{
    //CV3 * .896 / 128 = sec/step
    Motor_Set_Speed_By_DCC_Speed_Step_28(29); //max speed
    Motor_Update(); //force max speed immediately
    DCC_Config_Write_CV(CV_DECELERATION_RATE, 25);
    Motor_Set_Speed_By_DCC_Speed_Step_28(1); //min speed
    while (OCR0B > 0)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(22401, _millis_counter); //should be 22400 really
}

TEST(MotorTests, TestDecel_v1_NEG)
{
    //CV3 * .896 / 128 = sec/step
    Motor_Set_Speed_By_DCC_Speed_Step_28(-29);
    Motor_Update(); //force a -29 speed immediately
    DCC_Config_Write_CV(CV_DECELERATION_RATE, 25);
    Motor_Set_Speed_By_DCC_Speed_Step_28(-1); //stop reverse
    while(OCR0A > 0)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(22401, _millis_counter); //should be 22400 really
}

TEST(MotorTests, TestAccelDecel_v1)
{
    //CV3 * .896 / 128 = sec/step
    Motor_Set_Speed_By_DCC_Speed_Step_28(-29);
    Motor_Update(); //to force it to -29 speed immediately.

    DCC_Config_Write_CV(CV_DECELERATION_RATE, 10);
    DCC_Config_Write_CV(CV_ACCELERATION_RATE, 25);
    Motor_Set_Speed_By_DCC_Speed_Step_28(29); //max speed
    while (!((OCR0B == 255) && (TCCR0A & (1 << COM0B1))))
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(31436, _millis_counter); //should be ??? really
}

//motor jog

TEST(MotorTests, TestJog)
{
    //need to enable motor jogging!
    DCC_Config_Write_CV(CV_OPS_MODE_BASIC_ACK, 0x01);
    _millis_counter = 1; //need to do this
    Motor_Jog();
    Motor_Update();
    CHECK_EQUAL(0xFF, OCR0B);

    _millis_counter += 6;
    Motor_Update();
    CHECK_EQUAL(0xFF, OCR0B);

    _millis_counter += 2;
    Motor_Update();
    Motor_Update(); //takes two updates for jog off to go into effect.
    CHECK_EQUAL(0x00, OCR0B);
}

//motor jog

TEST(MotorTests, TestJogOpsMode)
{
    _millis_counter = 1; //need to do this
    DCC_Config_Write_CV(CV_OPS_MODE_BASIC_ACK, 1);
    Motor_Update();
    _millis_counter = 12;
    DCC_Config_Write_CV(CV_OPS_MODE_BASIC_ACK, 1);
    Motor_Update();
    CHECK_EQUAL(0xFF, OCR0B);
}

TEST(MotorTests, TestSpeedSettings_eStop_V1)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(29);
    Motor_Update();
    CHECK_EQUAL(255, OCR0B);

    //estop then delay.
    Motor_EStop(1);
    Motor_Update();
    CHECK_EQUAL(0x00, OCR0B);

    //long delay then recheck
    uint8_t i;
    for (i = 0; i < 255; ++i)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(0x00, OCR0B);
}

TEST(MotorTests, TestForwardTrim_V1)
{
    DCC_Config_Write_CV(CV_FORWARD_TRIM, 64); //half voltage
    Motor_Set_Speed_By_DCC_Speed_Step_28(29); //set to full speed, should be half voltage
    Motor_Update();
    CHECK_EQUAL(127, OCR0B);
}

TEST(MotorTests, TestForwardTrim_V2)
{
    DCC_Config_Write_CV(CV_FORWARD_TRIM, 255); //double voltage
    Motor_Set_Speed_By_DCC_Speed_Step_28(15); //set to half speed, should be nearly full voltage
    Motor_Update();
    CHECK_EQUAL(249, OCR0B);
}

TEST(MotorTests, TestReverseTrim_V1)
{
    DCC_Config_Write_CV(CV_REVERSE_TRIM, 64); //half voltage
    Motor_Set_Speed_By_DCC_Speed_Step_28(-29); //set to full speed, should be half voltage
    Motor_Update();
    CHECK_EQUAL(127, OCR0B);
}

TEST(MotorTests, TestReverseTrim_V2)
{
    DCC_Config_Write_CV(CV_REVERSE_TRIM, 255); //double voltage
    Motor_Set_Speed_By_DCC_Speed_Step_28(-15); //set to half speed, should be nearly full voltage
    Motor_Update();
    CHECK_EQUAL(249, OCR0B);
}

//check motor cutout for BEMF WHEN ENABLED
TEST(MotorTests, TestBEMFCutout)
{
    Motor_Set_Speed_By_DCC_Speed_Step_28(14); //50% cruise rate
    Motor_Update();
    CHECK_EQUAL(115, OCR0B);
    uint8_t applied_voltage = OCR0B;

    //set up BEMF
    BEMF_cutoff = 127; //activates BEMF
    BEMF_period = 100; // checks every 100 ms
    BEMF_Kp = 0x32;
    BEMF_Ki = 0x18;
    BEMF_Kf = 0xFF;

    //check how long it takes for the motor to be disconnected
    while(OCR0B != 0xFF)
    {
        TIM0_OVF_vect();
        Motor_Update();
    }
    CHECK_EQUAL(0xFF, OCR0B);
    CHECK_EQUAL(0xFF, OCR0A);
    CHECK_EQUAL((1 << COM0B1) | (1 << COM0A1), TCCR0A & ((1 << COM0B1) | (1 << COM0A1)));
    CHECK_EQUAL(100, _millis_counter);

    //now, check how long it takes to cut out
//    while(0 == (ADCSRA & (1 << ADSC)))
//    {
//        TIM0_OVF_vect();
//        Motor_Update();
//    }
//    CHECK_EQUAL(512, _micros_counter - timer); //should stay off for 500us before triggering ADC measurement, this is the closest we can get

    //advance time by 32 us, then trigger ADC measurement;
    TIM0_OVF_vect();
    //set ADC measurement to right at 115 the motor was set at
    ADCH = applied_voltage;
    ADC_vect();
    CHECK_EQUAL(126, sample);

    //now check that motor has returned to previous voltage
    ADC_vect();
    Motor_Update();
    CHECK_EQUAL(112, OCR0B); //TODO HACK!
}

//TEST(MotorTests, TestPID)
//{
//    eeprom[CV_BEMF_PERIOD] = 100; //every 100 milliseconds
//    eeprom[CV_EMF_FEEDBACK_CUTOUT] = 127; //never cut BEMF measurement off
//    soft_reset();
//    Motor_Set_Speed_By_DCC_Speed_Step_28(14); //50% cruise rate
//    Motor_Update();
//    uint8_t intended_voltage = OCR0B;
//
//
//    ADCH = (intended_voltage - 5);
//    ADC_vect();
//    Motor_Update();
//
//    ADCH = (intended_voltage +5);
//    ADC_vect();
//    Motor_Update();
//
//    ADCH = (intended_voltage -3);
//    ADC_vect();
//    Motor_Update();
//
//    ADCH = (intended_voltage +1);
//    ADC_vect();
//    Motor_Update();
//
//    CHECK_EQUAL(0, _bemf_adjust);
//}
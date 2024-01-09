/* =================================================================================
File name  : main.c
Modify     : MAIWEI /sales@mcudsp.com.tw
Website    : http://www.mcudsp.com.tw
Version    : V1.0
Description: Primary system file for the PB Labs.
==================================================================================*/
#include "easyDSP-Expansion_Board.h"
#include "math.h"
#include "bs2700.h"
#define SAMPLING_FREQ 12000
#define PI 3.14159265358979
#define loop_time 6000 / 2
short DTMF_count = 0, DTMF_flag = 0, sample = 0;
int music_led_array[4] = {-1, -1, -1, -1};
int keyboard_led_array[4] = {-1, -1, -1, -1};
int i = 0;
int sw1 = 0;
int sw2 = 0;
int sw3 = 0;
int sw4 = 0;
int L_output = -1;
int R_output = -1;
float keyboard_gain = 1.1; // ��j�B�Y�p���v
float music_gain = 1.1;    // ��j�B�Y�p���v
int pb1 = 0;
int pb2 = 0;
int pb3 = 0;
int pb4 = 0;
int wave = 1;
//      wave
//  1   sine
//  2   square
//  3   sawtooth
//  4   triangular
short L_sample, R_sample;
int LR_sample = 0;
int row = 0;
unsigned short digital = 15, old_digital = 15; // keyboard
float f[9] = {262.0, 294.0, 330.0, 349.0, 392.0, 440.0, 494.0, 524.0, 588.0};
float key_gain = 0.5;

int mode = 0; // 0�쥻(�R����)  1�^�� 2���� 3�o�i
// echo
#define GAIN 0.6
#define BUF_SIZE 2000
int L_buffer[BUF_SIZE], R_buffer[BUF_SIZE], LR_buffer[BUF_SIZE];
int left_output_echo = 0, right_output_echo = 0, LR_output_echo = 0;
int delayed = 0;
int echo_i = 0;
// ����
#define AM_N 20
int L_baseband[AM_N] = {0};
int R_baseband[AM_N] = {0};
int LR_baseband[AM_N] = {0};
short carrier[AM_N] = {1000, 0, -1000, 0, 1000, 0, -1000, 0, 1000, 0,
                       -1000, 0, 1000, 0, -1000, 0, 1000, 0, -1000, 0}; // 2.4-kHz carrier

short AM_amp = 1, AM_sample = 0;
int L_AM_output, R_AM_output, LR_AM_output;
float tmp;

//�o�i
float fir_L[fir_N], fir_R[fir_N], fir_LR[fir_N];
short fir_i;
float fir_ynl = 0.0, fir_ynr = 0.0, fir_ynlr = 0.0;

int main(void)
{

    Board_Init(); // Initial easyDSP-Expansion_Board
    Setup_Audio_Init(FS_12000_HZ, ADC_GAIN_0DB, DAC_ATTEN_0DB, LINE_INPUT);
    Control_LED_OFF(1);
    Control_LED_OFF(2);
    Control_LED_OFF(3);
    Control_LED_OFF(4);

    while (1)
    {
        sw1 = Read_SW(1); // 0 ���`    or 1 reset
        sw2 = Read_SW(2); // 0 �q�l�^ or 1 ���T�B�z

        sw3 = Read_SW(3); // 0 �S���ĪG or 1 ����Ҧ�
        //       �q�l�^         ���T�B�z
        // PB 1  �j�n           �j�n
        // PB 2  �p�n           �p�n
        // PB 3  ��key          �^��
        // PB 4  ��key          �쭵
        sw4 = Read_SW(4); // 0 �S���ĪG or 1 ����Ҧ�
        //       �q�l�^        ���T�B�z
        // PB 1  sine          ���n�D��X
        // PB 2  square        �k�n�D��X
        // PB 3  sawtooth      ����
        // PB 4  triangular    �쭵

        if (sw1 == 1)
        { // reset
            // LCD ��� reset
            LCD_PUT_CMD(LCD_FIRST_LINE);
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR('R');
            LCD_PUT_CHAR('E');
            LCD_PUT_CHAR('S');
            LCD_PUT_CHAR('E');
            LCD_PUT_CHAR('T');
            LCD_PUT_CHAR(' ');

            // reset led array
            int i = 0;
            for (i = 0; i < 4; i++)
            {
                keyboard_led_array[i] = -1;
                music_led_array[i] = -1;
            }

            // reset gain
            keyboard_gain = 1.3;
            key_gain = 0.5;
            music_gain = 1.1;
            sample = 0;
            // mute
            L_output = -1;
            R_output = -1;

            // off led
            Control_LED_OFF(1);
            Control_LED_OFF(2);
            Control_LED_OFF(3);
            Control_LED_OFF(4);
            US_Delay(300000); // 0.3��
            LCD_PUT_CMD(LCD_FIRST_LINE);
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CMD(LCD_SECOND_LINE);
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');
            LCD_PUT_CHAR(' ');

            // reset echo buffer
            for (echo_i = 0; echo_i < BUF_SIZE; echo_i++)
            {
                L_buffer[echo_i] = 0;
                R_buffer[echo_i] = 0;
                LR_buffer[echo_i] = 0;
            }
            left_output_echo = 0;
            right_output_echo = 0;
            LR_output_echo = 0;
            delayed = 0;
            echo_i = 0;

            // reset AM buffer
            for (AM_sample = 0; AM_sample < AM_N; AM_sample++)
            {
                L_baseband[AM_sample] = 0;
                R_baseband[AM_sample] = 0;
                LR_baseband[AM_sample]= 0;

            }

            AM_sample = 0;

            mode = 0;
        }
        else if (sw1 == 0)
        { // normal
            if (sw2 == 0)
            { // keyboard
                digital = Read_keypad();
                switch (digital) // �C���O�l�����@�ˡA�n��
                {
                case 15:
                    row = 2 * 262.0 + f[0] * key_gain;
                    break;
                case 14:
                    row = 2 * 294.0 + f[1] * key_gain;
                    break;
                case 13:
                    row = 2 * 330.0 + f[2] * key_gain;
                    break;
                case 11:
                    row = 2 * 349.0 + f[3] * key_gain;
                    break;
                case 3:
                    row = 2 * 392.0 + f[4] * key_gain;
                    break;
                case 6:
                    row = 2 * 440.0 + f[5] * key_gain;
                    break;
                case 10:
                    row = 2 * 494.0 + f[6] * key_gain;
                    break;
                case 2:
                    row = 2 * 524.0 + f[7] * key_gain;
                    break;
                case 5:
                    row = 2 * 588.0 + f[8] * key_gain;
                    break;
                case 12:
                    row = 0;
                    break;
                }
                if (DTMF_flag == 1)
                {
                    if (old_digital != digital)
                    {
                        DTMF_count = 0;
                        DTMF_flag = 0;
                    }
                }
                LCD_PUT_CMD(LCD_FIRST_LINE);
                LCD_PUT_CHAR('K');
                LCD_PUT_CHAR('E');
                LCD_PUT_CHAR('Y');
                LCD_PUT_CHAR('B');
                LCD_PUT_CHAR('O');
                LCD_PUT_CHAR('A');
                LCD_PUT_CHAR('R');
                LCD_PUT_CHAR('D');
                if (sw3 == 1 && sw4 == 0)
                {
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR('M');
                    LCD_PUT_CHAR('O');
                    LCD_PUT_CHAR('D');
                    LCD_PUT_CHAR('E');
                    LCD_PUT_CHAR(':');
                    LCD_PUT_CHAR('1');
                }
                else if (sw3 == 0 && sw4 == 1)
                {
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR('M');
                    LCD_PUT_CHAR('O');
                    LCD_PUT_CHAR('D');
                    LCD_PUT_CHAR('E');
                    LCD_PUT_CHAR(':');
                    LCD_PUT_CHAR('2');

                    for (i = 0; i < 4; i++)
                    {
                        if (keyboard_led_array[i] == 1)
                        {
                            Control_LED_ON(i + 1);
                        }
                        else if (keyboard_led_array[i] == -1)
                        {
                            Control_LED_OFF(i + 1);
                        }
                    }
                }
                else if (sw3 == 0 && sw4 == 0)
                {
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    Control_LED_OFF(1);
                    Control_LED_OFF(2);
                    Control_LED_OFF(3);
                    Control_LED_OFF(4);
                }
            }
            else if (sw2 == 1)
            { // music
                LCD_PUT_CMD(LCD_FIRST_LINE);
                LCD_PUT_CHAR('M');
                LCD_PUT_CHAR('U');
                LCD_PUT_CHAR('S');
                LCD_PUT_CHAR('I');
                LCD_PUT_CHAR('C');
                LCD_PUT_CHAR(' ');
                LCD_PUT_CHAR(' ');
                LCD_PUT_CHAR(' ');
                if (sw3 == 1 && sw4 == 0)
                { // mode 1
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR('M');
                    LCD_PUT_CHAR('O');
                    LCD_PUT_CHAR('D');
                    LCD_PUT_CHAR('E');
                    LCD_PUT_CHAR(':');
                    LCD_PUT_CHAR('1');

                    if (mode == 1)
                        Control_LED_ON(3);
                    else
                        Control_LED_OFF(3);

                    if (mode == 0)
                        Control_LED_ON(4);
                    else
                        Control_LED_OFF(4);
                }
                else if (sw3 == 0 && sw4 == 1)
                { // mode 2
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR('M');
                    LCD_PUT_CHAR('O');
                    LCD_PUT_CHAR('D');
                    LCD_PUT_CHAR('E');
                    LCD_PUT_CHAR(':');
                    LCD_PUT_CHAR('2');

                    for (i = 0; i < 2; i++)
                    {
                        if (music_led_array[i] == 1)
                        {
                            Control_LED_ON(i + 1);
                        }
                        else if (music_led_array[i] == -1)
                        {
                            Control_LED_OFF(i + 1);
                        }
                    }

                    if (mode == 2)
                        Control_LED_ON(3);
                    else
                        Control_LED_OFF(3);

                    if (mode == 3)
                        Control_LED_ON(4);
                    else
                        Control_LED_OFF(4);
                    // �o�i
                }
                else if (sw3 == 0 && sw4 == 0)
                {
                    LCD_PUT_CMD(LCD_SECOND_LINE);
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    LCD_PUT_CHAR(' ');
                    Control_LED_OFF(1);
                    Control_LED_OFF(2);
                    Control_LED_OFF(3);
                    Control_LED_OFF(4);
                }
            }
            if(pb1 == 1){
                // �{�{0.4��
                pb1 = 0;
                Control_LED_ON(1);
                US_Delay(200000);
                Control_LED_OFF(1);
            }
            if(pb2 == 1){
                // �{�{0.4��
                pb2 = 0;
                Control_LED_ON(2);
                US_Delay(200000);
                Control_LED_OFF(2);
            }
            if(pb3 == 1){
                // �{�{0.4��
                pb3 = 0;
                Control_LED_ON(3);
                US_Delay(200000);
                Control_LED_OFF(3);
            }
            if(pb4 == 1){
                // �{�{0.4��
                pb4 = 0;
                Control_LED_ON(4);
                US_Delay(200000);
                Control_LED_OFF(4);
            }
        }
    }
}

interrupt void INT4_ISR(void)
{
    if (sw2 == 0) // keyboard
    {
        if (DTMF_count < loop_time)
        {
            // ��ܪi��
            if (wave == 1) // sine
            {
                sample = 10000 * (sin(2.0 * PI * DTMF_count * row / SAMPLING_FREQ));
            }
            else if (wave == 2) // square
            {
                tmp = sin(2.0 * PI * DTMF_count * row / SAMPLING_FREQ);
                if (tmp > 0)
                    sample = 10000;
                if (tmp <= 0)
                    sample = -10000;
            }
            else if (wave == 3) // sawtooth
            {
                double period = SAMPLING_FREQ / row;
                    double fraction = fmod(DTMF_count, period) / period;
                    sample = 10000 * (2 * fraction - 1);
            }
            else if (wave == 4) // triangular
            {
                double period = SAMPLING_FREQ / row;
                    double fraction = fmod(DTMF_count, period) / period;
                    sample = 10000 * (1 - 2 * fabs(0.5 - fraction));
            }
            DTMF_count++;
        }
        else
        {
            sample = 0;
            DTMF_flag = 1;
            old_digital = digital;
            DTMF_count = loop_time;
        }
        sample *= keyboard_gain;
        output_left_sample((short)(sample));
    }
    else if (sw2 == 1) // music
    {
        if(L_output == 1 && R_output == -1)
            L_sample = input_left_sample();

        else if(L_output == -1 && R_output == 1)
            R_sample = input_right_sample();

        else if(L_output == 1 && R_output == 1)
            LR_sample = input_sample();
        else{
            L_sample = 0;
            R_sample = 0;
            LR_sample = 0;
        }
        //LR_sample = L_sample + R_sample;
        // echo
        delayed = L_buffer[echo_i];
        left_output_echo = L_sample + delayed;
        L_buffer[echo_i] = L_sample + delayed * GAIN;
        L_baseband[AM_sample] = L_sample;

        delayed = R_buffer[echo_i];
        right_output_echo = R_sample + delayed;
        R_buffer[echo_i] = R_sample + delayed * GAIN;
        R_baseband[AM_sample] = R_sample;

        delayed = LR_buffer[echo_i];
        LR_output_echo = LR_sample + delayed;
        LR_buffer[echo_i] = LR_sample + delayed * GAIN;
        LR_baseband[AM_sample] = LR_sample;

        if (++AM_sample >= (AM_N - 1))
            AM_sample = 0;
        if (++echo_i >= BUF_SIZE)
            echo_i = 0;

        // ��X
        if (L_output == -1 && R_output == -1)
        {
            output_sample(0);
        }
        else if (L_output == -1 && R_output == 1)
        {
            if (mode == 1) // �^��
            {
                output_right_sample((short)(right_output_echo*music_gain));
            }
            else if (mode == 2) // ����
            {
                R_AM_output = carrier[AM_sample] + ((AM_amp * R_baseband[AM_sample] * carrier[AM_sample] / 10) >> 12);
                output_right_sample((short)(AM_amp * R_AM_output*music_gain));
            }
            else
            {
                output_right_sample((short)((float)(R_sample)*music_gain));
            }
        }
        else if (L_output == 1 && R_output == -1)
        {
            if (mode == 1) // �^��
            {
                output_left_sample((short)(left_output_echo*music_gain));
            }
            else if (mode == 2) // ����
            {
                L_AM_output = carrier[AM_sample] + ((AM_amp * L_baseband[AM_sample] * carrier[AM_sample] / 10) >> 12);
                output_left_sample((short)(AM_amp * L_AM_output*music_gain));
            }
            else
            {
                output_left_sample((short)((float)(L_sample)*music_gain));
            }
        }
        else if (L_output == 1 && R_output == 1)
        {
            if (mode == 1) // �^��
            {
                output_sample((int)(LR_output_echo));
            }
            else
            {
                output_sample(LR_sample);
            }
        }
    }

    return;
}

interrupt void INT5_ISR(void) // LED 1
{
    //       �q�l�^
    // sw3 1  �j�n
    // sw4 1  sine

    if (sw1 == 0)
    {
        if (sw2 == 0)
        { // keyboard
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 1 ���Ҧ�
                keyboard_gain += 0.2;
                if (keyboard_gain > 2.5)
                    keyboard_gain = 2.5;
                pb1 = 1;

            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // sine wave
                wave = 1;
                keyboard_led_array[0] = 1;
                keyboard_led_array[1] = -1;
                keyboard_led_array[2] = -1;
                keyboard_led_array[3] = -1;
            }
        }
        //       ���T�B�z
        // sw3 1  �j�n
        // sw4 1  ���n�D��X

        else if (sw2 == 1)
        { // music
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 1 ���Ҧ�
                music_gain += 0.2;
                if (music_gain > 2.5)
                    music_gain = 2.5;
                pb1 = 1;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // ����n�D��X
                L_output *= -1;
                music_led_array[0] *= -1;
            }
            else return;
        }
        else return;
    }
    return;
}
//-------------------------------
// interrupt service routine #6
//-------------------------------
interrupt void INT6_ISR(void) // LED 2
{
    //       �q�l�^
    // sw3 2  �p�n
    // sw4 2  square

    if (sw1 == 0)
    {
        if (sw2 == 0)
        { // keyboard
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 2 ���Ҧ�
                keyboard_gain -= 0.2;
                if (keyboard_gain < 0.3)
                    keyboard_gain = 0.3;
                pb2 = 1;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 2 ���Ҧ�
                // square wave
                wave = 2;
                keyboard_led_array[0] = -1;
                keyboard_led_array[1] = 1;
                keyboard_led_array[2] = -1;
                keyboard_led_array[3] = -1;
            }
        }
        //       ���T�B�z
        // sw3 2  �p�n
        // sw4 2  �k�n�D��X

        else if (sw2 == 1)
        { // music
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 2 ���Ҧ�
                music_gain -= 0.2;
                if (music_gain < 0.3)
                    music_gain = 0.3;
                // �{�{0.4��
                Control_LED_ON(2);
                //US_Delay(400000);
                pb2 = 1;
                Control_LED_OFF(2);
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 2 ���Ҧ�
                // ����k�n�D��X
                R_output *= -1;
                music_led_array[1] *= -1;
            }
        }
    }
    return;
}

interrupt void INT7_ISR(void)
{
    //       �q�l�^
    // sw3 3  ��key
    // sw4 3  sawtooth

    if (sw1 == 0)
    {
        if (sw2 == 0)
        { // keyboard
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 1 ���Ҧ�
                pb3 = 1;
                key_gain += 0.5;
                if (key_gain > 3)
                    key_gain = 3;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // sawtooth wave
                wave = 3;
                keyboard_led_array[0] = -1;
                keyboard_led_array[1] = -1;
                keyboard_led_array[2] = 1;
                keyboard_led_array[3] = -1;
            }
        }
        //       ���T�B�z
        // sw3 3  �^��
        // sw4 3  ����

        else if (sw2 == 1)
        { // music
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 1 ���Ҧ�
                // echo led
                mode = 1;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // ���� led
                // music_led_array[2] *= -1;
                mode = 2;
            }
        }
    }
    return;
}

interrupt void INT8_ISR(void) // LED 4
{
    //       �q�l�^
    // sw3 3  ��key
    // sw4 3  triangular

    if (sw1 == 0)
    {
        if (sw2 == 0)
        { // keyboard
            if (sw3 == 1 && sw4 == 0)
            { // ���� sw3 = 1 ���Ҧ�
                pb4 = 1;
                key_gain -= 0.5;
                if (key_gain < -2)
                    key_gain = -2;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // triangular wave
                wave = 4;
                keyboard_led_array[0] = -1;
                keyboard_led_array[1] = -1;
                keyboard_led_array[2] = -1;
                keyboard_led_array[3] = 1;
            }
        }
        //       ���T�B�z
        // sw3 4  �쥻
        // sw4 4  �v�i

        else if (sw2 == 1)
        { // music
            if (sw3 == 1 && sw4 == 0)
            {
                mode = 0;
            }
            else if (sw3 == 0 && sw4 == 1)
            { // ���� sw4 = 1 ���Ҧ�
                // �o�i
                // music_led_array[3] *= -1;
                mode = 3;
            }
        }
    }
    return;
}

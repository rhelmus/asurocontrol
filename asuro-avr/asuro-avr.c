/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 ***************************************************************************/

#include "asuro.h"

void sendIRByte(unsigned char byte)
{
    /* 1: Start
    * 2: Stop
    * 4: One
    * 5: Zero
    * This is used to have a 'compatible' way of communicating with the e51
    */

    int i;
    
    UartPutc('1');
    
    for (i=0; i<8; i++)
    {
        if (byte & (1<<i))
            UartPutc('4');
        else
            UartPutc('5');
    }
    
    UartPutc('2');
}

void SendSensors(void)
{
    // Switch
    sendIRByte('S'); // S == msg for switch
    sendIRByte(PollSwitch() & PollSwitch()); // Use pollswitch twice
    
    // All sensor data is converted from 0..1023 to 0..255 to lessen IR IO
    // long: conversion may give int overflows
    const unsigned long adcmax = 1023, sendmax = 255;
    unsigned int adc[2];
    
    // Line sensors
    LineData(adc);
    sendIRByte('L'); // Line
    sendIRByte(adc[0]*sendmax/adcmax);
    sendIRByte(adc[1]*sendmax/adcmax);
    
    // Odo
    OdometryData(adc);
    sendIRByte('O'); // Odo
    sendIRByte(adc[0]*sendmax/adcmax);
    sendIRByte(adc[1]*sendmax/adcmax);
    
    // Battery
    sendIRByte('B'); // Battery
    sendIRByte(Battery()*sendmax/adcmax);    
}

void ReadIR(void)
{
    if (PIND & (1<<PD0))
        return;
    
    int i;
    unsigned char datastr[13], irstat, cmd = 0, data = 0;
    
    /* Read manchester data:
     * First 5 bits: cmd type
     * Remaining 8 bits: value
     */

    for (i=0; i<13; i++)
    {
        // Longer timing than regular RC5: Symbian doesn't seem to provide enough accuracy
        Msleep(15);
        
        irstat = (PIND & (1<<PD0));
        if (irstat)
        {
            datastr[i] = '1';
            if (i < 5)
                cmd |= (1<<(4-i));
            else
                data |= (1<<(7-(i-5)));
        }
        else
            datastr[i] = '0';
        
        while (irstat == (PIND & (1<<PD0)))
            ;
    }
    
    SerPrint("Received IR data: ");
    SerWrite(datastr, 13);
    SerPrint("\ncmd: ");
    PrintInt(cmd);
    SerPrint("\ndata: ");
    PrintInt(data);
    UartPutc('\n');
    
    Msleep(500);
}

int main(void)
{   
    Init();
 
    StatusLED(RED);
    BackLED(OFF, OFF);
    
    unsigned long datatime = 0;
    while (1)
    {
        const unsigned long time = Gettime();
        
        ReadIR();
        
        if (time > datatime)
        {
            datatime = time + 500; // Every 0.5s
            //SendSensors();
        }
        
//         Msleep(100);
    }
    
    return 0;
}

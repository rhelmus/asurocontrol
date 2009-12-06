/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   any later version.                                                    *
 ***************************************************************************/

#include "asuro.h"

void sendIR(const char *text)
{
    /* 1: Start
    * 2: Stop
    * 4: One
    * 5: Zero
    */
    
    char *p = text;
    int i;
    
    while (p && *p)
    {
        UartPutc('1');
        
        for (i=0; i<8; i++)
        {
            if (*p & (1<<i))
                UartPutc('4');
            else
                UartPutc('5');
        }
        
        UartPutc('2');
        p++;
    }    
}

void SendSensors()
{
    // Switch
    char sw[3];
    sw[0] = 'S'; // S == msg for switch
    sw[1] = PollSwitch(); // UNDONE: Call twice?
    sw[2] = 0;
    sendIR(sw);
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
        
        if (time > datatime)
        {
            datatime = time + 500; // Every 0.5s
            SendSensors();
        }
        
        Msleep(100);
    }
    
    return 0;
}

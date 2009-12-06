/****************************************************************************/
/*!
  \file     adc_low.c

  \brief    Low Level Funktion zum Auslesen der ADC-Wandler.

  \version  V001 - 20.01.2008 - m.a.r.v.i.n\n
*****************************************************************************/
/*****************************************************************************
*                                                                            *
*   This program is free software; you can redistribute it and/or modify     *
*   it under the terms of the GNU General Public License as published by     *
*   the Free Software Foundation; either version 2 of the License, or        *
*   any later version.                                                       *
*                                                                            *
*****************************************************************************/

#include "asuro.h"

/****************************************************************************/
/*!
  \brief
  Schaltet den A/D Multiplexer auf den gewuenschten A/D Kanal\n
  startet die A/D Wandlung und gibt den gelesenen Wert zurueck.

  \param  mux Nummer des A/D Kanal Multiplexer
  \param  sleep optionale Wartezeit

  \return
  10 Bit A/D Wert (Bereich 0..1023)
*****************************************************************************/

unsigned int ReadADC(unsigned int mux, unsigned int sleep)
{  
  if ((mux) == (BATTERIE))
    ADMUX = (1 << REFS0) | (1 << REFS1) | (mux); // interne 2.56V Referenz
  else
    ADMUX = (1 << REFS0) | (mux);           // Referenz mit externer Kapazitaet
  if (sleep)
    Sleep(sleep);

  ADCSRA |= (1 << ADSC);                // Starte AD-Wandlung
  while (!(ADCSRA & (1 << ADIF)))       // Ende der AD-Wandlung abwarten
    ;
  ADCSRA |= (1 << ADIF);                // AD-Interupt-Flag zuruecksetzen
  return ADC;                           // Ergebnis als 16-Bit-Wert
}


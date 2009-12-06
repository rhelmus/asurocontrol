/****************************************************************************/
/*!
  \file     asuro.c

  \brief    Init- und Interrupt-Funktionen der Asuro Library.\n
            Timer2 wird mit 36 kHz betrieben, im Gegensatz zur ausgelieferten\n
            Version mit 72 kHz.

  \par      Interrupt-Doku nur ueber die Datei zu sehen:
            Bitte ueber den oben ausgegebenen Link \b 'gehe \b zum \b Quellcode \n
            \b dieser \b Datei' direkt in der Datei nachsehen.\n
            DoxyGen ist nicht zur Erkennung von Interrupt-Funktionen 'bereit'.\n
            Behandelt werden folgende Interrupts:\n
            - SIG_OVERFLOW2   : Timer 2 (fest mit 36 kHz belegt)\n
            - SIG_INTERRUPT1  : Switches (Taster) im Interruptmode\n
            - SIG_ADC         : Analog-Digital-Wandler

  \par      Wichtiger Hinweis:
            Die Init()-Funktion muss von jedem Programm beim Start\n
            aufgerufen werden.\n

  \see      Defines zum setzen von Port's und Konfigurationen in asuro.h\n
            IRTX, LEFT_DIR, PWM, GREEN_LED, RIGHT_DIR, FRONT_LED,\n
            ODOMETRIE_LED, RED_LED, ON, OFF, GREEN, FWD, TRUE, FALSE

  \version  V--- - 10.11.2003 - Jan Grewe - DLR\n
            Original Version von der ASURO CD\n
  \version  V--- - 20.11.2006 - m.a.r.v.i.n\n
            +++ SIGNAL (SIG_ADC)\n
            static Variable toggle mit FALSE initialisiert.\n
            (Bug report von Rolf_Ebert)
  \version  V--- - bis zum 07.01.2007 - \n
            Bitte in Datei CHANGELOG nachsehen.\n
  \version  V001 - 13.01.2007 - m.a.r.v.i.n\n
            +++ Alle Funktionen\n
            Zerlegte Sourcen in einzelne Dateien fuer eine echte Library.
  \version  V002 - 27.01.2007 - Sternthaler\n
            +++ Alle Funktionen\n
            Kommentierte Version (KEINE Funktionsaenderung)
  \version  V003 - 20.02.2007 - m.a.r.v.i.n\n
            Defines fuer Dometrie High/Low Werte aus myasuro.h verwenden
            StopSwitch ersetzt. Deshalb wurde immer die komplette switches.o 
            mitgelinkt
  \version  V004 - 15.11.2007 - m.a.r.v.i.n\n
            RIGHT_DIR und LEFT_DIR waren in der Init Funktion vertauscht
  \version  V005 - 29.01.2008 - m.a.r.v.i.n\n           
            Initialisierung fuer ATmega168\n
            UART Baudrate einstellbar durch Define\n
            Interrupt User Funktionen f�r Timer und A/D Wandler      
          
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
#include "myasuro.h"

#define BAUD_RATE   2400

/****************************************************************************/
/*!
  \brief
  Initialisiert die Hardware: Ports, A/D Wandler, Serielle Schnittstelle, PWM\n
  Die Init Funktion muss von jeden Programm beim Start aufgerufen werden 

  \see
  Die Funktionen Sleep() und Msleep() in time.c werden mit dem hier\n
  eingestellten 36 kHz-Takt betrieben.\n

  \par  Funktionsweise der Zeitfunktionen:
  Msleep() ruft Sleep() auf. In Sleep() wird die globale Variable count36kHz\n
  zur Zeitverzoegerung benutzt. Diese Variable wird jedesmal im Interrupt\n
  SIG_OVERFLOW2 um 1 hochgezaehlt.\n
  Der Interrupt selber wird durch den hier eingestellten Timer ausgeloesst.\n
  Somit ist dieser Timer fuer die Zeitverzoegerung zustaendig.

  \see
  Die globale Variable autoencode fuer die automatische Bearbeitung der\n
  Odometrie-ADC-Wandler wird hier auf FALSE gesetzt.

  \par  Hinweis zur 36 kHz-Frequenz vom Timer 2
  Genau diese Frequenz wird von dem Empfaengerbaustein benoetigt und kann\n
  deshalb nicht geaendert werden.\n
  In der urspruenglichen, vom Hersteller ausgelieferten LIB, war diese\n
  Frequenz allerdings auf 72 kHz eingestellt. Durch eine geschickte\n
  Umkonfigurierung durch waste konnte diese aber halbiert werden.\n
  Sinnvoll ist dies, da der durch diesen Timer2 auch ausgeloesste Timer-\n
  Interrupt dann nur noch die Haelfte an Rechenzeit in Anspruch nimmt.

  \par  Beispiel:
  (Nur zur Demonstration der Parameter/Returnwerte)
  \code
  // Die Init()-Funktion MUSS IMMER zu Anfang aufgerufen werden.
  int main (void)
  {
    int wert;

    Init ();

    while (1)
    (
        // Dein Programm
    }
    return 0;
  }
  \endcode
*****************************************************************************/
void Init (
  void)
{
  /*
    Timer2, zum Betrieb mit der seriellen Schnittstelle, fuer die
    IR-Kommunikation auf 36 kHz eingestellt.
  */
#if defined(__AVR_ATmega168__)
  // fast PWM, set OC2A on compare match, clear OC2A at bottom, clk/1
  TCCR2A = _BV(WGM20) | _BV(WGM21) | _BV(COM2A0) | _BV(COM2A1);
  TCCR2B = _BV(CS20);
  // interrupt on timer overflow
  TIMSK2 |= _BV(TOIE2); 
#else
  // fast PWM, set OC2A on compare match, clear OC2A at bottom, clk/1
  TCCR2 = _BV(WGM20) | _BV(WGM21) | _BV(COM20) | _BV(COM21) | _BV(CS20);
  // interrupt on timer overflow
  TIMSK |= _BV(TOIE2); 
#endif
  // 36kHz carrier/timer
  OCR2  = 0x91;

  /*
    Die serielle Schnittstelle wurde waerend der Boot-Phase schon
    programmiert und gestartet. Hier werden die Parameter auf 2400 1N8 gesetzt.
  */
#if defined(__AVR_ATmega168__)
   UBRR0L = (uint8_t)(F_CPU/(BAUD_RATE*16L)-1);
   UBRR0H = (F_CPU/(BAUD_RATE*16L)-1) >> 8;
   UCSR0B = (1<<RXEN0) | (1<<TXEN0);
   UCSR0C = (1<<UCSZ00) | (1<<UCSZ01);
#else
  UBRRH = (((F_CPU/BAUD_RATE)/16)-1)>>8; 	// set baud rate
  UBRRL = (((F_CPU/BAUD_RATE)/16)-1);
  UCSRB = (1<<RXEN)|(1<<TXEN);  // enable Rx & Tx
  UCSRC = (1<<URSEL)|(1<<UCSZ1)|(1<<UCSZ0);  // config USART; 8N1
#endif

  /*
    Datenrichtung der I/O-Ports festlegen. Dies ist durch die Beschaltung der
    Asuro-Hardware nicht aenderbar.
    Port B: Seriell Senden; Richtungsvorgabe Motor links; Takt fuer die
            Geschwindigkeit beider Motoren; Grueneanteil-Status-LED
    Port D: Richtungsvorgabe Motor rechts; Vordere LED;
            Odometrie-LED (Radsensor); Rotanteil-Status-LED
  */
  DDRB = IRTX | RIGHT_DIR | PWM | GREEN_LED;
  DDRD = LEFT_DIR | FRONT_LED | ODOMETRIE_LED | RED_LED;

  /*
    PWM-Kanaele OC1A und OC1B auf 8-Bit einstellen.
    Sie werden fuer die Geschwindigkeitsvorgaben der Motoren benutzt.
  */
  TCCR1A = _BV(WGM10) | _BV(COM1A1) | _BV(COM1B1);
  TCCR1B = _BV(CS11);                 // tmr1-Timer mit MCU-Takt/8 betreiben.

  /*
    Einstellungen des A/D-Wandlers auf MCU-Takt/64
  */
  ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1);

  /*
    Sonstige Vorbereitungen.
    - Alle LED's ausschalten
    - Motoren stoppen und schon mal auf Vorwaerts einstellen.
    - Globale Variable autoencoder ausschalten.
  */
  ODOMETRIE_LED_OFF;
  FrontLED (OFF);
  BackLED (ON, ON);
  BackLED (OFF, OFF);
  StatusLED (GREEN);

  MotorDir (FWD, FWD);
  MotorSpeed (0, 0);

  autoencode = FALSE;

  Ovr2IntFunc = 0;
  AdcIntFunc = 0;
  /*
    Funktion zum ALLGEMEINEN ZULASSEN von Interrupts.
  */
  sei ();
}



/****************************************************************************/
/*
  \brief
  Interrupt-Funktion fuer Timer-2-Ueberlauf.

  \see
  count36kHz, timebase

  \par
  Der zum Timer gehoerende Zaehler TCNT2 wird so justiert, dass damit die\n
  gewuenschten 36 kHz erreicht werden.\n
  Fuer die Zeitfunktionen werden die globalen Variablen count36kHz und\n
  timebase hochgezaehlt.

  \par 
  Die Variable Ovr2IntFunc kann als Zeiger auf eine User Funktion benutzt werden\n
  und wird dann, falls ungleich 0, von der Interrupt Funktion aus angesprungen.

  \par  Beispiel:
  (Nicht vorhanden)
*****************************************************************************/
SIGNAL (SIG_OVERFLOW2)
{
  TCNT2 += 0x25;
  count36kHz ++;
  if (!count36kHz)
    timebase ++;
  if (Ovr2IntFunc)
    Ovr2IntFunc();
}


/**
 * being used insted TIMER2_OVF_vect during ultrasonic polling
 */
#if defined(__AVR_ATmega168__)
SIGNAL(SIG_OUTPUT_COMPARE2A)
#else
SIGNAL(SIG_OUTPUT_COMPARE2) 
#endif
{
	count36kHz++;
  if (!count36kHz)
    timebase ++;
}

/****************************************************************************/
/*
  \brief
  Interrupt-Funktion fuer den externen Interrupt 1.

  \see  switched

  \par
  Hier wird 'nur' in der globalen Variablen switched vermerkt, dass ein\n
  Switch (Taster) gedrueckt wurde und dieser Interrupt soeben aufgetreten ist.\n
  Damit dieser Interrupt aber nicht permanent aufgerufen wird, solange der\n
  Taster gedrueckt bleibt, wird die Funktion, dass ein Interrupt erzeugt wird,\n
  ueber StopSwitch() abgeschaltet.\n
  Nach einer Bearbeitung im eigenen Hauptprogramm, muss also die Funktion\n
  StartSwitch() wieder Aufgerufen werden, um einen Tastendruck wieder ueber\n
  einen Interrupt zu erkennen.

  \par  Beispiel:
  (Nicht vorhanden)
*****************************************************************************/
SIGNAL (SIG_INTERRUPT1)
{
  switched = 1;
#if defined(__AVR_ATmega168__)
  EIMSK &= ~_BV(INT1);                // Externen Interrupt 1 sperren
#else
  GICR &= ~_BV(INT1);                 // Externen Interrupt 1 sperren
#endif
//  StopSwitch ();
}



/****************************************************************************/
/*
  \brief
  Interrupt-Funktion fuer den AD-Wandler. Kann ueber autoencode gesteuert\n
  die Odometrie-Zaehler in encoder hochzaehlen.

  \see
  Die globale Variable autoencode wird hier ausgewertet. Ist sie nicht FALSE,\n
  dann wird der AD-Wandler-Wert zum Zaehlen der Odometriewerte in der globalen\n
  Variablen encoder benutzt.\n
  Es wird auch der AD-Wandler-Kanal auf die 'andere' Seite der Odometrie\n
  umgeschaltete und der AD-Wandler neu gestartet.\n
  Somit wird erreicht, dass zumindest die Odometriemessung automatisch erfolgt.

  \par 
  Die Variable AdcIntFunc kann als Zeiger auf eine User Funktion benutzt werden\n
  und wird dann, falls ungleich 0, von der Interrupt Funktion aus angesprungen.

  \par  Beispiel:
  (Nicht vorhanden)
*****************************************************************************/
SIGNAL (SIG_ADC)
{
  if (AdcIntFunc)
    AdcIntFunc();
}


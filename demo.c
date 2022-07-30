/*
    Title:    AVR-GCC test program #1 for the STK200 eva board
              (modified by Rolf Johansson <rojo (at) nocrew.org>)
    Author:   Volker Oth
    Date:     4/1999
    Purpose:  Flashes the LEDs on Port B with a hard coded delay loop
    needed
    Software: AVR-GCC
    needed
    Hardware: ATS90S8515/8535/2313/mega on STK200/STK300 board
    Note:     To contact me, mail to
                  volkeroth (at) gmx.de
              You might find more AVR related stuff at my homepage:
                  http://members.xoom.com/volkeroth
*/

#include <avr/io.h>
#include <compat/deprecated.h>

#define F_CPU 16000000UL
#define BAUD 9600
#define UBRR_VALUE ((F_CPU / (BAUD * 16UL)) - 1)

typedef unsigned char  u08;

void myputc(int c)
{
    while (!(UCSRA & (1 << UDRE))) ;
    UDR = c;
}

void myputs(const char *s)
{
    while (*s)
    {
        myputc(*s);
        ++s;
    }
}

char hello[] = "Hello, world\r\n";

int main( void )
{
   u08 led, i, j, k, l, dc, run, rundc;
   char dat[10];

   outp(0xff,DDRB);            /* use all pins on PortB for output */

   led = 1;                  /* init variable representing the LED state */
   dc = 1;                     /* Init the direction (dc) */
   run = 1;
   rundc = 0;

   UBRRH = (UBRR_VALUE >> 8);
   UBRRL = (UBRR_VALUE);
   UCSRB = (1 << RXEN) | (1 << TXEN);
   UCSRC = (1 << URSEL) | (1 << USBS) | (3 << UCSZ0);

   dat[0] = 'H';
   dat[1] = 0;

   for (;;) {
       outp(~led, PORTB);      /* invert the output since a zero means: LED on */
       if (dc==0)
         led >>= 1;              /* move to next LED */
       if (dc==1)
         led <<= 1;              /* move to next LED */
       if (!led) {             /* overflow: start with Port B0 again */
         if (dc == 0) {
           led = 1;
           dc = 1;
         }
         else {
           led = 128;
           dc = 0;
         }
         if (rundc == 0)
           run++;
         if (rundc == 1)
           run--;
       }
       if (run == 1)
         rundc = 0;
       if (run == 255)
         rundc = 1;
       for (i=0; i<run; i++)     /* outer delay loop */
          for (j=0; j<run;j++)   /* inner delay loop */
             for (l=0; l<16;l++) /* inner inner delay loop */
                k++;                /* just do something - could also be a NOP */
   myputc('H');
   myputc('e');
   myputc('l');
   myputc('l');
   myputc('o');
   myputc('\r');
   myputc('\n');
   myputs(dat);
   myputs(hello);
   }
}

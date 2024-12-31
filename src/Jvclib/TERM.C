/*
**      term.c :
*/

#include        <stdio.h>
#include        <stdlib.h>
#include        <conio.h>
#include        <string.h>
#include        <ctype.h>

#include        "term.h"

#define true    1
#define false   0

/* MODEM SUPPORT */
static char mparm = 0;

void Init_Comm( int port, int baud )
{                                      /* initialize comm port stuff          */
    i_m( port );
    Set_Parity( 1 );                   /* 8,n,1                               */
    Set_Baud( baud );                  /* initial baud rate                   */
}

#define B9600   0xe0
#define B4800   0xc0
#define B1200   0x80                   /* baud rate codes                     */

Set_Baud( int n )
{
    if ( n == 1200 )
        mparm = ( mparm & 0x1f ) + B1200;
    else if ( n == 4800 )
        mparm = ( mparm & 0x1f ) + B4800;
    else if ( n == 9600 )
        mparm = ( mparm & 0x1f ) + B9600;
    else
        return 0;                      /* invalid speed                       */
    set_mdm( mparm );
    return n;
}

#define PAREVN  0x18                   /* MCR bits for commands               */
#define PARODD  0x10
#define PAROFF  0x00
#define STOP2   0x40
#define WORD8   0x03
#define WORD7   0x02
#define WORD6   0x01

Set_Parity( int n )
{                                      /* n is parity code                    */
    static int mmode;

    if ( n == 1 )
        mmode = ( WORD8 | PAROFF );    /* off                                 */
    else if ( n == 2 )
        mmode = ( WORD7 | PAREVN );    /* on and even                         */
    else if ( n == 3 )
        mmode = ( WORD7 | PARODD );    /* on and odd                          */
    else
        return 0;                      /* invalid code                        */
    mparm = ( mparm & 0xe0 ) + mmode;
    set_mdm( mparm );
    return n;
}
Write_Modem( int data )
{
    wrtmdm( data );
}
int Read_Modem( )
{
    return rdmdm( );                   /* from int bfr                        */
}
void Term_Comm( )
{                                      /* uninstall comm port driver          */
    u_m( );
}

/*** end of term.c ***/

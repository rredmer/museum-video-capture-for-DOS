/*
**      cterm.h : Function prototypes for cterm.c
*/

extern int Wants_To_Abort( void );
extern int docmd( void );
extern int kbd_wait( void );
extern int kb_file( void );
extern int esc( void );
extern FILE *getfil( void );
extern void getnam( char *, int );
extern char *addext( char *, char * );
extern void put_cap( char );
extern void cap_flush( void );
extern long getmr( void );
extern void Start_Timer( int );
extern int Timer_Expired( void );
extern int Set_Vid( void );
extern void deol( void );
extern void deos( void );
extern void cls( void );
extern void cursor( int );
extern void revvid( int );
extern int putchx( char );
extern int Read_Keyboard( void );
extern void Init_Comm( int, int );
extern int Set_Baud( int );
extern int Set_Parity( int );
extern int Write_Modem( int );
extern int Read_Modem( void );
extern void Term_Comm( void );

/* CH1.ASM functions - modem interfacing */
extern void i_m( int );
extern void set_mdm( int );
extern void wrtmdm( int );
extern int Send_Byte( int );
extern int rdmdm( void );
extern void u_m( void );

/* CH1A.ASM functions - exception handlers */
extern void set_int( void );
extern void rst_int( void );
extern int broke( void );

/* CH2.ASM functions - video interfacing */
extern void _i_v( void );
extern int _wrpos( int, int );
extern void _deol( void );
extern void _cls( void );
extern int _color( int, int );
extern void locate( int, int );

/*** end of cterm.h ***/

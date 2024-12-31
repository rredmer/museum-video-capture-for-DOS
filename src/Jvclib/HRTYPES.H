/*
**      HRTYPES.H
*/

    #ifndef __HRTYPES__
        #define __HRTYPES__

        #define         true    1
        #define         false   0
        typedef int Boolean;
        typedef unsigned int Word;
        typedef unsigned char Byte;


/*================ GRAPHICS ===============*/
        struct Rect
            {
            int left;
            int top;
            int right;
            int bottom;
        };
        typedef struct Rect Rect;

        struct Point
            {
            int v;
            int h;
        };
        typedef struct Point Point;

        struct GPort
            {
            unsigned long baseAddr;
            unsigned long rowBits;
            int psize;
            Rect portRect;
        };
        typedef struct GPort GPort;
        typedef struct GPort *GPtr;

        struct Pix32
            {
            unsigned b : 8;
            unsigned g : 8;
            unsigned r : 8;
            unsigned a : 8;
        };
        typedef struct Pix32 Pix32;

        struct Pix24
            {
            unsigned b : 8;
            unsigned g : 8;
            unsigned r : 8;
        };
        typedef struct Pix24 Pix24;

        struct Pix16
            {
            unsigned b : 5;
            unsigned g : 5;
            unsigned r : 5;
            unsigned a : 1;
        };
        typedef struct Pix16 Pix16;

/*=========================================*/


/*================ EVENT ==================*/
        struct EventRecord
            {
            int what;
            int keycode;
            int message;
            long when;
            Point where;
            int modifiers;
        };
        typedef struct EventRecord EventRecord;

        enum
            {                          /* event code                          */
            idle, keydown, mousemove, mousedown, vstaevt
        };

        enum
            {                          /* key modifiers                       */
            rightShift = 1, leftShift = 2, ctl = 4, alt = 8
        };
/*=========================================*/


/*============ DATA STRUCTURE =============*/
        union LUT
            {
            struct _rgba
                {
                Byte r, g, b, a;
                } RGBA;
            Byte rgba[4];
        };
        typedef union LUT LUT;

        typedef struct
            {
            Byte setR;                 /* RGB setup                           */
            Byte setG;
            Byte setB;
        } SETUP;

        typedef struct
            {
            Byte high;
            Byte low;
        } GAIN;

        typedef struct
            {
            GAIN gainR;                /* RGB gain                            */
            GAIN gainG;
            GAIN gainB;
        } RGBGain;

        typedef struct
            {
            Byte genlock;              /* genlock (master or slave)           */
            Byte sample;         /* sampling clock (942fH/908fH - Targa/Vista)*/
            Byte gamma;                /* gamma on/off                        */
            Byte knee;                 /* knee on/off                         */
            Byte phase;             /* phase control delay 0-15 (0:through HD)*/
            Byte gainNum;              /* current gain table number           */
            SETUP setup[2][4];         /* RGB setup                           */
            RGBGain gain[2][4];        /* RGB gain                            */
        } HRSETUP;

        struct HrProc
            {
            int cmdChar[2];
            Boolean display;
            Boolean enabled;
            char *cmdStr;
            int ( *proc ) ( struct HrProc * );
        };
        typedef struct HrProc HrProc;
/*=========================================*/

/*=============== ENVIRONMEMT =============*/
        struct VistaENV
            {
            int ioBase;                /* AtVista base i/o address            */
            int vistaMem;              /* Vista memory (MB)                   */
            unsigned long vistaStr;    /* vista memory start address          */
            unsigned long vistaEnd;    /* vram end address                    */
            unsigned long commbuf;     /* communication buffer address        */
        };
        typedef struct VistaENV VistaENV;

        struct HvipENV
            {
            Boolean hvipExist;         /* HVIP system flag                    */
            int ioBase;                /* HVIP base i/o address               */
        };
        typedef struct HvipENV HvipENV;

        struct DspCapINFO
            {
            int genlock;               /* AtVista genlock mode                */
            int imageType;             /* image type, same as TGA file type   */
            int pixSize;               /* pixel depth                         */
            int capMode;               /* capture mode                        */
            int horzRes;               /* horizontal resolution               */
            int vertRes;               /* vertical resolution                 */
            int dspMode;               /* display mode                        */
            int horzCrtRes;            /* horizontal crt resolution           */
            int vertCrtRes;            /* vertical crt resolution             */
            GPort imagePort;           /* image data port on AtVista          */
            Rect capRect;              /* capture rectangle in image port     */
            Rect dspRect;   /* current display rectangle in imagePort.portRect*/
            GPort dspPort;             /* display device graf port            */
            GPort devPort;             /* phisical device graf port           */
        };
        typedef struct DspCapINFO DspCapINFO;

/* AT Vista Sync mode */
        enum
            {
            slave, master
        };
/*=========================================*/


    #endif


/*** end of hrtypes.h ***/



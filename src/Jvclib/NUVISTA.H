/***********************************************************************
**
**      GSP Loader error return values and structure typedefs etc.
**
***********************************************************************/

#define ldBadCoffHeader                         -1
#define ldBadMagicNumber                        -2
#define ldNotEnoughSections                     -3
#define ldBadSectionSeek                        -4
#define ldBadSectionRead                        -5
#define ldBadSectionDataSeek            -6
#define ldBadDataRead                           -7
#define ldBadRelocSeek                          -8
#define ldBadRelocRead                          -9

#define ldUnlinkedCoffFile                      -10
#define ldFileIsAbsoulte                        -11
#define ldExecutableOnBitAddress        -12
#define ldBadMalloc                                     -13
#define ldBadReadSections                       -14
#define ldBadRelocateSection            -15
#define ldBadOptHeader                          -16

#define ldNotEnoughHeapSpace            -17

#define ldNotCopyCinitSection           -18

/*
**      Relocation types
*/

#define relocAbs                        0x0000/* absolute, no relocation      */
#define relocPcrWord            0x0010 /* word direct                         */
#define relocRelLong            0x0011 /* 32-bit direct                       */
#define relocOcrLong            0x0018 /* 1's complement 32-bit direct        */
#define relocGspPcr16           0x0019 /* 16-bit relative (in words)          */

/*
** Following are typedefs needed for gsp program load
*/

typedef struct
    {
    unsigned short magicNumber;        /* magic number                        */
    unsigned short numSections;        /* number of section headers           */
    long timeDate;                     /* time data                           */
    long symPtr;                       /* pointer to symbol table             */
    long numSymbols;                   /* number of entries in symbol table   */
    unsigned short numOptBytes;        /* number of bytes in optional header  */
    unsigned short flags;              /* describes type of object file       */
} coffFileHeader;

typedef struct
    {
    short optMagic;                    /* magic number                        */
    short vStamp;                      /* version                             */
    long tSize;                        /* size of text in bytes               */
    long dSize;                        /* size of initialized data in bytes   */
    long bSize;                        /* size of uninitialized data in bytes */
    unsigned long pEntry;              /* entry point                         */
    long textStart;                    /* base address of text                */
    long dataStart;                    /* base address of data                */
} coffOptFileHeader;

typedef struct
    {
    char name[8];                      /* section name                        */
    unsigned long pphyAddr;            /* physical address of section         */
    unsigned long virtAddr;            /* virtual address of section          */
    unsigned long size;                /* size of sections                    */
    unsigned long sectionPtr;          /* pointer to raw data                 */
    unsigned long relPtr;              /* ptr to relocation entries           */
    unsigned long lnNoPtr;             /* file ptr to line number entries     */
    unsigned short nReloc;             /* number of entries                   */
    unsigned short nLnNo;              /* number of line number entries       */
    unsigned short flags;              /* flags equals a section type         */
    unsigned char rsvd;                /* reserved byte                       */
    unsigned char MemPageNum;          /* memory page number                  */
} coffSectionHeader;

typedef struct
    {
    long virtAddr;                    /* virtual address of relocatable symbol*/
    unsigned short symIndex;           /* index into the symbol table         */
    unsigned short reserved;           /* not used, always assume 16 bit      */
    unsigned short type;               /* relocation type entry               */
} coffRelocInfo;

typedef struct
    {
    union
        {
        char string[8];                /* symbol name                         */
        struct
            {
            long zeros;                /* symbol name                         */
            long offset;               /* location in string tabel            */
            } nn;
        char *ovlPtr[2];               /* allows overlaying                   */
        } name;
    unsigned long value;               /* value of symbol                     */
    short sectionNumber;               /* section number                      */
    unsigned short type;               /* symbol type and derived             */
    char storageClass;                 /* symbol stirage calss                */
    char maxAuxEntries;                /* max auxiliary entries               */
} coffSymbolEntry;

/*** end of Nuvista.h ***/

ECHO OFF
CLS
ECHO Compiling/Linking User Application...
cl /c /AL userapp.c
LINK userapp.obj,,,jvclib;
ECHO ON


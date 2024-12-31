gboot.obj : gboot.c
	gspcl -cq -v10 gboot

ghrcap.obj : ghrcap.c ghrcap.h
	gspcl -cq -v10 ghrcap

gdisplay.obj : gdisplay.c ghrcap.h
	gspcl -cq -v10 gdisplay

ggrab.obj : ggrab.c ghrcap.h
	gspcl -cq -v10 ggrab

glive.obj : glive.c ghrcap.h
	gspcl -cq -v10 glive

gimage.obj : gimage.c ghrcap.h
	gspcl -cq -v10 gimage

gimgasm.obj : gimgasm.asm
	gspa gimgasm

gspasm.obj : gspasm.asm
	gspa gspasm.asm

hrcap.out : ghrcap.mk ghrcap.cmd gboot.obj ghrcap.obj ggrab.obj glive.obj \
gimage.obj gimgasm.obj gdisplay.obj gspasm.obj
	gsplnk ghrcap.cmd
	grep -i undefed ghrcap.map

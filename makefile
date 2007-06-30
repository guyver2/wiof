TARGET = wiof
OBJS = main.o codage.o ./Pgck/psp_main.o ./Pgck/Pgck.o ./Pgck/Pson.o

INCDIR = 
CFLAGS = -G4 -Wall -O3 -I/usr/local/pspdev/psp/include/SDL -DPSPFW30X -DPSP
CXXFLAGS = $(CFLAGS)  -fno-exceptions -fno-rtti
ASFLAGS = $(CFLAGS)

PSP_FW_VERSION=390

PSPSDK=$(shell psp-config --pspsdk-path)
PSPBIN = $(PSPSDK)/../bin

LIBDIR =
LDFLAGS =
STDLIBS=  -lSDL_mixer -lSDLmain -lSDL_image -lSDL -lpng -ljpeg -lm -lz \
	-lvorbisfile -lvorbis  -logg \
	-lpspsdk -lpspctrl  -lpsprtc -lpsppower -lpspgu -lpspaudiolib -lpspaudio -lpsphprm
LIBS=$(shell $(PSPBIN)/sdl-config --libs) $(STDLIBS)$(YOURLIBS)


EXTRA_TARGETS = EBOOT.PBP
PSP_EBOOT_TITLE = Who is on fire ?
PSP_EBOOT_ICON = ICON0.png
#PSP_EBOOT_SND0 = SND0.AT3

PSPSDK=$(shell psp-config --pspsdk-path)
DEFAULT_CFLAGS = $(shell $(SDL_CONFIG) --cflags)
include $(PSPSDK)/lib/build.mak


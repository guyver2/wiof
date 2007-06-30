#ifndef PSP_MAIN_H
#define PSP_MAIN_H

#include <zlib.h>
#include "SDL.h"

#include <pspkernel.h>
#include <pspdebug.h>
#include <pspsdk.h>
#include <pspctrl.h>
#include <pspthreadman.h>
#include <psppower.h>
#include <stdlib.h>
#include <stdio.h>


#define printErr pspDebugScreenPrintf

void pspfrequence(int mhz);


#endif

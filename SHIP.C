/*
	File:		SHIP.C

	Contains:   This file contains code to show the Sega logo,
		the Ship splash screen, and handle the options screen.
		It starts the game up by calling play(p), defined in play.c.

	Written by:	Burt Sloane & Randel B. Reiss of Technopop

	Copyright:	c 1990 by Sega of America, Inc., all rights reserved.

	Change History:

	To Do:
*/

#include "ship.h"
#include "..\Gems\Gems.h"

#include "s40.h"
#include "start.h"

extern		play();

char		*paletteptr;
short		copypaletteinVBL;
long		tickcount;
short		dmaglobal1;
short		dmaglobal2;
short		players;
short		cfmaxlives;
short		cffastcharge;
short		cfwrapworld;
short		cfsunfatal;
short		cfsunshowing;
short		cfgravinward;
short		cfgravamount;
short		cffriction;
long		arewewarm;

putstring(s, out, pal)
char		*s;
short		out[];
{
  char		c;
  short		i, j;

  i = 0;
  while (*s != '\0') {
    c = *s++;
    if (c == ' ') out[i++] = (pal == 0) ? 0 : 0x63ff;
    if (c == '>') out[i++] = (pal << sCPShift) + 0x53f;
    if ((c >= 'A') && (c <= 'Z')) out[i++] =
	(pal << sCPShift) + CHARBASE + c - 'A';
    if ((c >= '0') && (c <= '9')) out[i++] =
	(pal << sCPShift) + NUMBERBASE + c - '0';
  }
}

drawstring(h, v, s)
int		h, v;
char		*s;
{
  short		out[80];
  short		i, j;

  i = strlen(s);
  putstring(s, out, 0);
  SetBCharPos(h, v, 6);			/* 64 for scrollwidth */
  for (j=0; j<i; j++) VDPdata = out[j];
}

drawAstring(h, v, s, pal)
int		h, v;
char		*s;
int		pal;
{
  short		out[80];
  short		i, j;

  i = strlen(s);
  putstring(s, out, pal);
  SetACharPos(h, v, 6);			/* 64 for scrollwidth */
  for (j=0; j<i; j++) VDPdata = out[j];
}

drawAchar(h, v, c, pal)
int		h, v;
short		c;
int		pal;
{
  SetACharPos(h, v, 6);			/* 64 for scrollwidth */
  VDPdata = c + (pal << sCPShift);
}

drawnum(h, v, n)
int		h, v, n;
{
  short		out[6];
  short		i, j;

  for (j=0; j<6; j++) out[j] = 0;
  i = n / 10000;
  if (i != 0) out[0] = NUMBERBASE + (i % 10);
  i = n / 1000;
  if (i != 0) out[1] = NUMBERBASE + (i % 10);
  i = n / 100;
  if (i != 0) out[2] = NUMBERBASE + (i % 10);
  i = n / 10;
  if (i != 0) out[3] = NUMBERBASE + (i % 10);
  i = n;
  out[4] = NUMBERBASE + (i % 10);
  SetBCharPos(h, v, 6);			/* 64 for scrollwidth */
  for (j=0; j<5; j++) VDPdata = out[j];
}

dmapalette(paletteptr, whichpal, numpals)
char		*paletteptr;
int		whichpal, numpals;
{
  long		source;
  short		palette[64];
  int		i;
  
  gemsdmastart();

  for (i=0; i<16 * numpals; i++) {
    palette[i] = *(short *)paletteptr;
    paletteptr += 2;
  }

  dmaglobal1 = 0xc000 + (whichpal << 5);
  dmaglobal2 = 0x0080;
  source = (long)&palette[0];
  VDPcontrol = 0x8174;
  VDPcontrol = 0x9300 + 16 * numpals;
  VDPcontrol = 0x9400;
  VDPcontrol = 0x9500 + ((source >> 1) & 0x0ff);
  VDPcontrol = 0x9600 + ((source >> 9) & 0x0ff);
  VDPcontrol = 0x9700 + ((source >> 17) & 0x07f);
  VDPcontrol = dmaglobal1;
  VDPcontrol = dmaglobal2;

  gemsdmaend();
}

goosedma()
{
  long		source;

  dmaglobal1 = 0x6d00;
  dmaglobal2 = 0x0083;
  source = (long)&tickcount;	/* just an address */
  VDPcontrol = 0x8174;
  VDPcontrol = 0x9300 + 80;
  VDPcontrol = 0x9400;
  VDPcontrol = 0x9500 + ((source >> 1) & 0x0ff);
  VDPcontrol = 0x9600 + ((source >> 9) & 0x0ff);
  VDPcontrol = 0x9700 + ((source >> 17) & 0x07f);
  VDPcontrol = dmaglobal1;
  VDPcontrol = dmaglobal2;
}

VBLinterrupt()
{
  if (copypaletteinVBL) {
    gemsholdz80();
	dmapalette(paletteptr, 0, copypaletteinVBL);
    copypaletteinVBL = 0;
    gemsreleasez80();
  }
  tickcount++;
}

hidescrolls ()
{
/* make both scrolls invisible, but don't use char #0	*/
  eraseCRAM();
  loadchars(&zerochar, 0x3ff, 1);	/* char 0x3ff is trasparent */
  fillScrollA(0x63ff);			/* pal 3, char 0x3ff */
  fillScrollB(0x63ff);			/* pal 3, char 0x3ff */
}

int anybuttonpress()
{
  if ((readCtrlA() != 0) || (readCtrlB() != 0)) return 1;
  return 0;
}

int anystartpress()
{
  if ((readCtrlA() | readCtrlB()) & ctrlSTART) return 1;
  return 0;
}

dosega ()
{
  int		i, palindex;
  long		oldtickvalue;

  eraseCRAM();
  eraseVSRAM();
  eraseVRAM();

  copypaletteinVBL = 0;

/* display the SEGA logo and cycle colors		*/
  hidescrolls();

/* load the character data from SEGACHAR.ASM		*/
  loadchars(&segachar, 0, 49);

/* load the image from SEGAART.ASM into scroll A at location ()	*/
  __loadScroll(kMscrAbase + (SEGAVPOS * 64 + SEGAHPOS) * 2,
      &segaart, SEGAWIDTH, SEGAHEIGHT, SCROLLWIDTH);

/* load the first palette from SEGAPAL.ASM		*/
  paletteptr = &segapal;
  copypaletteinVBL = 1;

  for (palindex=0; palindex<NUMSEGAPALETTES-1; palindex++) {
    oldtickvalue = tickcount;
    while (tickcount < oldtickvalue + COLORCYCLEDELAY) {
      if (anystartpress()) {
        while (anystartpress()) ;
        return;
      }
      wait4VBLon;
      wait4VBLoff;
    }
    paletteptr += 32;
    copypaletteinVBL = 1;
  }

/* wait 2 seconds */
  oldtickvalue = tickcount;
  while (tickcount < oldtickvalue + 2 * 60) {
    if (anystartpress()) break;
    wait4VBLon;
    wait4VBLoff;
  }

  while (anystartpress()) ;

  return;
}

int dosplash ()
{
  int		i, palindex;
  long		oldtickvalue;

  eraseCRAM();
  eraseVSRAM();
  eraseVRAM();

  copypaletteinVBL = 0;

/* display the splash screen				*/

  hidescrolls();

/* load the character data from STAMPLIB.ASM		*/
  loadchars(&chardata, 0, ((&chardataend-&chardata) >> 5));

/* load the image from STAMPART.ASM into scroll A at location (0, 0)	*/
  loadScrollB(&cellmap, PICTWIDTH, PICTHEIGHT, SCROLLWIDTH);

/* load the palettes from STAMPPAL.ASM		*/
  loadPalette(&palettedata   , 0);
  loadPalette(&palettedata+32, 1);
  loadPalette(&palettedata+64, 2);
  loadPalette(&palettedata+96, 2);

/* wait 20 seconds */
  oldtickvalue = tickcount;
  while (tickcount < oldtickvalue + 20 * 60) {
    if (anystartpress()) break;
  }
  if (!anystartpress()) return 0;
  while (anystartpress()) ;
  return 1;
}

drawship(h, v, showing)
int	h, v, showing;
{
  if (showing) {
    drawAchar(h, v, 0x4d4, 2);
    drawAchar(h, v+1, 0x4d5, 2);
    drawAchar(h+1, v, 0x4d6, 2);
    drawAchar(h+1, v+1, 0x4d7, 2);
  } else {
    drawAstring(h, v, "  ", 1);
    drawAstring(h, v+1, "  ", 1);
  }
}

updateoptions()
{
  int		i;

  if (cfgravamount == 0x30) drawAstring(6, 6, "  WEAK    ", 1);
  if (cfgravamount == 0xe0) drawAstring(6, 6, "  STRONG  ", 1);
  if (cfgravamount == 0) drawAstring(6, 6, "  OFF     ", 1);
  if (cfgravinward) drawAstring(6, 7, "  INWARD   ", 1);
  if (!cfgravinward) drawAstring(6, 7, "  OUTWARD  ", 1);
  if (cfsunshowing) drawAstring(6, 11, "  VISBLE    ", 1);
  if (!cfsunshowing) drawAstring(6, 11, "  INVISBLE  ", 1);
  if (cfsunfatal) drawAstring(6, 12, "  FATAL       ", 1);
  if (!cfsunfatal) drawAstring(6, 12, "  NON FATAL   ", 1);
  if (cffriction) drawAstring(6, 16, "  ON       ", 1);
  if (!cffriction) drawAstring(6, 16, "  OFF      ", 1);
  if (cfwrapworld) drawAstring(24, 6, "  WRAP    ", 1);
  if (!cfwrapworld) drawAstring(24, 6, "  BOUNCE  ", 1);
  if (cfmaxlives == 200) drawAstring(24, 9, "INFINITE SHIPS   ", 1);
  if (cfmaxlives != 200) drawAstring(24, 9, "SHIPS            ", 1);
  drawAstring(24, 11, " ", 1);
  for (i=0; i<6; i++) drawship(26 + i * 2, 11, (cfmaxlives > i));
  if (cffastcharge) drawAstring(24, 16, "  FAST   ", 1);
  if (!cffastcharge) drawAstring(24, 16, "  SLOW   ", 1);
}

options()
{
  int		cur, i;
  int		butdown, c, lastbuttontime;
  int		suntime, sunpict;

  eraseCRAM();
  eraseVSRAM();
  eraseVRAM();

  hidescrolls();
  loadchars(&sundata, 0x3f0, 16);
  loadchars(&shipdata, 0x4d4, 45);
  loadchars(&letterdata, CHARBASE, 41);
  loadchars(&letterdata, CHARBASE, 41);
  loadchars(&arrowdata, 0x53f, 1);
  loadPalette(&shippalette, 2);
  loadPalette(&bwpalette, 1);

  drawAstring(7, 1, "UNIVERSE CONSTRUCTION KIT", 1);

  drawAstring(6, 5, "GRAVITY", 1);
  drawAstring(6, 10, "SUN", 1);
  drawAstring(6, 15, "FRICTION", 1);
  drawAstring(24, 5, "BOUNDARIES", 1);
  drawAstring(24, 9, "SHIPS", 1);
  drawAstring(24, 15, "RECHARGE RATE", 1);

  cur = 1;
  butdown = 0;
  suntime = tickcount;
  sunpict = 0x3f0;

  while (1) {
    updateoptions();
    if (cur == 1) drawAstring(6, 6, ">", 1);
    if (cur == 2) drawAstring(6, 7, ">", 1);
    if (cur == 3) drawAstring(6, 11, ">", 1);
    if (cur == 4) drawAstring(6, 12, ">", 1);
    if (cur == 5) drawAstring(6, 16, ">", 1);
    if (cur == 6) drawAstring(24, 6, ">", 1);
    if (cur == 7) drawAstring(24, 11, ">", 1);
    if (cur == 8) drawAstring(24, 16, ">", 1);
    while (1) {
      if (tickcount > suntime) {
	suntime = tickcount + 16;
	drawAchar(2, 8, sunpict++, 2);
	drawAchar(2, 9, sunpict++, 2);
	drawAchar(3, 8, sunpict++, 2);
	drawAchar(3, 9, sunpict++, 2);
	if (sunpict == 0x400) sunpict = 0x3f0;
      }
      c = readCtrlA() | readCtrlB();
      if (c == 0) butdown = 0;
      if ((butdown == 0) && (c != 0)) {
	butdown = c;
	lastbuttontime = tickcount;
        if (c & ctrlSTART) {
	  while (anystartpress()) ;
	  return;
        }
        if (c & ctrlUP) {
          cur--;
	  if (cur <= 0) cur = 8;
	  break;
        }
        if (c & ctrlDOWN) {
          cur++;
	  if (cur > 8) cur = 1;
	  break;
        }
        if (c & ctrlLEFT) {
	  switch (cur) {
	    case 1:
	      if (cfgravamount == 0xe0) cfgravamount = 0x30;
		else if (cfgravamount == 0x30) cfgravamount = 0;
		  else if (cfgravamount == 0) cfgravamount = 0xe0;
	      break;
	    case 2:
	      cfgravinward = 1 - cfgravinward;
	      break;
	    case 3:
	      cfsunshowing = 1 - cfsunshowing;
	      break;
	    case 4:
	      cfsunfatal = 1 - cfsunfatal;
	      break;
	    case 5:
	      cffriction = 1 - cffriction;
	      break;
	    case 6:
	      cfwrapworld = 1 - cfwrapworld;
	      break;
	    case 7:
	      cfmaxlives -= 1;
	      if (cfmaxlives > 6) cfmaxlives = 6;
	      if (cfmaxlives < 3) cfmaxlives = 200;
	      break;
	    case 8:
	      cffastcharge = 1 - cffastcharge;
	      break;
	  }
	  break;
        }
        if (c & ctrlRIGHT) {
	  switch (cur) {
	    case 1:
	      if (cfgravamount == 0xe0) cfgravamount = 0;
		else if (cfgravamount == 0x30) cfgravamount = 0xe0;
		  else if (cfgravamount == 0) cfgravamount = 0x30;
	      break;
	    case 2:
	      cfgravinward = 1 - cfgravinward;
	      break;
	    case 3:
	      cfsunshowing = 1 - cfsunshowing;
	      break;
	    case 4:
	      cfsunfatal = 1 - cfsunfatal;
	      break;
	    case 5:
	      cffriction = 1 - cffriction;
	      break;
	    case 6:
	      cfwrapworld = 1 - cfwrapworld;
	      break;
	    case 7:
	      cfmaxlives += 1;
	      if (cfmaxlives > 200) cfmaxlives = 3;
	      if (cfmaxlives > 6) cfmaxlives = 200;
	      break;
	    case 8:
	      cffastcharge = 1 - cffastcharge;
	      break;
	  }
	  break;
        }
        if (lastbuttontime + 30 * 60 < tickcount) return;
      }
      wait4VBLon;
      wait4VBLoff;
    }
  }
}

#define PICKV 23
#define PICKH 16
int pickstart()
{
  int		starttime;
  int		pick, c;

  starttime = tickcount;

  loadchars(&letterdata, CHARBASE, 41);
  loadchars(&arrowdata, 0x53f, 1);
  loadPalette(&bwpalette, 1);

  drawAstring(PICKH, PICKV, "ONE PLAYER", 1);
  drawAstring(PICKH, PICKV + 2, "TWO PLAYER", 1);
  drawAstring(PICKH, PICKV + 4, "OPTIONS", 1);

  pick = 2;

  while (1) {
    drawAstring(PICKH - 2, PICKV, " ", 1);
    drawAstring(PICKH - 2, PICKV + 2, " ", 1);
    drawAstring(PICKH - 2, PICKV + 4, " ", 1);
    if (pick == 1) drawAstring(PICKH - 2, PICKV, ">", 1);
    if (pick == 2) drawAstring(PICKH - 2, PICKV + 2, ">", 1);
    if (pick == 3) drawAstring(PICKH - 2, PICKV + 4, ">", 1);
    while (1) {
      c = readCtrlA() | readCtrlB();
      if (c & ctrlSTART) {
	while (anystartpress()) ;
	return pick;
      }
      if (c & ctrlUP) {
        pick--;
	if (pick <= 0) pick = 3;
	while (anybuttonpress()) ;
	starttime = tickcount;
	break;
      }
      if (c & ctrlDOWN) {
        pick++;
	if (pick >= 4) pick = 1;
	while (anybuttonpress()) ;
	starttime = tickcount;
	break;
      }
      if (starttime + 30 * 60 < tickcount) return 0;
      wait4VBLon;
      wait4VBLoff;
    }
  }
}

/* s40.h sets the initial VDP state before calling here */
main()
{
  int		c;

  copypaletteinVBL = 0;
/*
  z80Reset;
  z80BusReq;
*/

  goosedma();


  gemsinit(&patchbank, &envbank, &seqbank, &sampbank);
  gemslockchannel(15);		/* lock a channel for sound effects */

  copypaletteinVBL = 0;
  tickcount = 0;
  VDPcontrol = kRmode2 + 0x70;		/* enable V interrupt */

  if (arewewarm != 0xa5a55a5a) {
    arewewarm = 0xa5a55a5a;
    cfgravamount = 0x30;
    cfgravinward = 1;
    cfsunshowing = 1;
    cfsunfatal = 1;
    cfwrapworld = 1;
    cfmaxlives = 5;
    cffastcharge = 1;
    cffriction = 0;
  }

  while (1) {
    dosega();
    c = dosplash();
    if (c != 0) {
      c = pickstart();
      if (c == 1) play(1);
      if (c == 2) play(2);
      if (c == 3) options();
    }
  }
}


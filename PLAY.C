/*
	File:		PLAY.C

	Contains:	This file contains the main code for the game.
		The game is started by calling a function named play(p),
		where p is the intended number of players.

	Written by:	Burt Sloane & Randel B. Reiss of Technopop

	Copyright:	c 1990 by Sega of America, Inc., all rights reserved.

	Change History:

	To Do:
*/

#include "ship.h"
#include "..\Gems\Gems.h"

#include "s40.h"

#define NUMCYCLERS 8

struct TShip	playerA;
struct TShip	playerB;
short		noiseon;
char		paused;
char		redrawscore;
short		drawpause;
short		scorearea[80];
short		sunstate;
short		palette[64];
short		*cycleptrinit[NUMCYCLERS];
short		*cycleptr[NUMCYCLERS];
short		cycletime[NUMCYCLERS];
short		cyclecurtime[NUMCYCLERS];
short		cyclepal[NUMCYCLERS];
short		cycleentry[NUMCYCLERS];

extern short	cfmaxlives;
extern short	cffastcharge;
extern short	cfwrapworld;
extern short	cfsunfatal;
extern short	cfsunshowing;
extern short	cfgravinward;
extern short	cfgravamount;
extern short	cffriction;
extern long	tickcount;
extern short	players;
extern char	*paletteptr;
extern short	copypaletteinVBL;
extern		drawstring();
extern		drawnum();
extern		goosedma();
extern short	cycle1;
extern short	cycle2;
extern short	cycle3;
extern short	cycle4;
extern short	cycle5;
extern short	cycle6;
extern short	cycle7;

fdrawstring(h, v, s)
int		h, v;
char		*s;
{
  char		c;

  if (v == 27) h += 40;
  while (*s != '\0') {
    c = *s++;
    if (c == ' ') scorearea[h++] = 0;
    if ((c >= 'A') && (c <= 'Z')) scorearea[h++] = CHARBASE + c - 'A';
    if ((c >= '0') && (c <= '9')) scorearea[h++] = NUMBERBASE + c - '0';
  }
}

fdrawnum(h, v, n)
int		h, v, n;
{
  int		i;

  if (v == 27) h += 40;
  i = n / 10000;
  if (i == 0) scorearea[h++] = 0;
    else scorearea[h++] = NUMBERBASE + (i % 10);
  i = n / 1000;
  if (i == 0) scorearea[h++] = 0;
    else scorearea[h++] = NUMBERBASE + (i % 10);
  i = n / 100;
  if (i == 0) scorearea[h++] = 0;
    else scorearea[h++] = NUMBERBASE + (i % 10);
  i = n / 10;
  if (i == 0) scorearea[h++] = 0;
    else scorearea[h++] = NUMBERBASE + (i % 10);
  i = n;
  scorearea[h++] = NUMBERBASE + (i % 10);
}


initship(ship, h, v)
struct TShip	*ship;
int		h;
int		v;
{
  int		i;

  ship->h = h << 16;
  ship->v = v << 16;
  ship->fired = 0;
  ship->thrusting = 0;
  ship->startdown = 0;
  ship->shieldon = 0;
  ship->blowupstate = 0;
  ship->firesound = 0;
  ship->score = 0;
  ship->dir = 0;
  ship->energy = 40 << 7;
  ship->lives = cfmaxlives;
  ship->blinkindex = 0;
  ship->blinkcounter = 5;
  for (i=0; i<MAXSHOTS; i++) {
    ship->shots[i].h = 0x400000;
    ship->shots[i].v = 0x400000;
    ship->shots[i].dh = 0;
    ship->shots[i].dv = 0;
  }
}

int barchar(val, pal)
int	val, pal;
{
  int	base = 0x530 + (pal << sCPShift);

  if (val < 0) return base + 2;
  if (val > 8) return base + 10;
  return base + 2 + val;
}

/* 5 chars across, max value of amt = 40 */
drawbar(h, amt, pal)
int	h, amt, pal;
{
  scorearea[h++] = 0x530 + 1;
  scorearea[h++] = barchar(amt     , pal);
  scorearea[h++] = barchar(amt - 8 , pal);
  scorearea[h++] = barchar(amt - 16, pal);
  scorearea[h++] = barchar(amt - 24, pal);
  scorearea[h++] = barchar(amt - 32, pal);
  scorearea[h++] = 0x530;
}

drawscores()
{
  int		i, c, l;

  for (i=0; i<80; i++) scorearea[i] = 0;
  l = playerA.lives;
  if (l > 5) l = 5;
  for (i=0; i<l - 1; i++) {
    scorearea[ 0 + (i << 1)] = (1 << sCPShift) + 0x4d4;
    scorearea[ 1 + (i << 1)] = (1 << sCPShift) + 0x4d4 + 2;
    scorearea[40 + (i << 1)] = (1 << sCPShift) + 0x4d4 + 1;
    scorearea[41 + (i << 1)] = (1 << sCPShift) + 0x4d4 + 3;
  }
  fdrawnum(13, 26, playerA.score);
  if (playerA.lives > 0) drawbar(52, playerA.energy >> 7, 1);
    else fdrawstring(6, 27, "GAME OVER");
  if ((playerA.energy >> 7) < 15) c = 0x00e;
    else if ((playerA.energy >> 7) < 23) c = 0x0ee;
      else c = 0x0e0;
  palette[16 + 5] = c;

  if (players < 2) return;
  l = playerB.lives;
  if (l > 5) l = 5;
  for (i=0; i<l - 1; i++) {
    scorearea[20 + (i << 1)] = (2 << sCPShift) + 0x4d4;
    scorearea[21 + (i << 1)] = (2 << sCPShift) + 0x4d4 + 2;
    scorearea[60 + (i << 1)] = (2 << sCPShift) + 0x4d4 + 1;
    scorearea[61 + (i << 1)] = (2 << sCPShift) + 0x4d4 + 3;
  }
  fdrawnum(33, 26, playerB.score);
  if (playerB.lives > 0) drawbar(72, playerB.energy >> 7, 2);
    else fdrawstring(26, 27, "GAME OVER");
  if ((playerB.energy >> 7) < 15) c = 0x00e;
    else if ((playerB.energy >> 7) < 23) c = 0x0ee;
      else c = 0x0e0;
  palette[32 + 5] = c;
}

int outofbounds(h, v, size)
int h, v, size;
{
  if (h + (size << 16) < 0x800000) return 1;
  if (h > 0x1c00000) return 1;
  if (v + (size << 16) < 0x800000) return 1;
  if (v > 0x1600000) return 1;
  return 0;
}

drawshot(shot, sprite)
struct TShot	*shot;
int		sprite;
{
  initsprite(sprite,
    sHSize8+sVSize8+sprite+1,		/* 8x8 sprite, linked to sprite+1 */
    shot->h >> 16, shot->v >> 16,
    (1 << sCPShift)+0x500);		/* palette 1, char $500 */
}

fireshot(ship, shot)
struct TShip	*ship;
struct TShot	*shot;
{
  ship->firesound = 20;
  gemsstartsong(1);
  shot->h = ship->h + 0x40000;
  shot->v = ship->v + 0x40000;
  shot->dh = (hdirtable[ship->dir divDIR] << 5) + ship->dh;
  shot->dv = (vdirtable[ship->dir divDIR] << 5) + ship->dv;
  shot->h += shot->dh << 1;
  shot->v += shot->dv << 1;
  shot->stepsremaining = 48;
}

int collision(h, v, eh, ev, esize, sensitivity)
int h, v, eh, ev, esize, sensitivity;
{
  esize = esize << 16;
  sensitivity = sensitivity << 16;
  if (eh + esize < h + sensitivity) return 0;
  if (eh > h + 0x100000 - sensitivity) return 0;
  if (ev + esize < v + sensitivity) return 0;
  if (ev > v + 0x100000 - sensitivity) return 0;
  return 1;
}

dogravity(g, h, v, dh, dv)
short	g;
int	h, v;
int	*dh;
int	*dv;
{
  int	deltah, deltav, sundistsquared, gh, gv;
  short sh, sv;

  deltah = h - (SUNHLOC << 16);
  if (deltah < 0) deltah = -deltah;
  sh = deltah >> 12;
  deltav = v - (SUNVLOC << 16);
  if (deltav < 0) deltav = -deltav;
  sv = deltav >> 12;
  sundistsquared = ((sh * sh) +
		    (sv * sv)) >> 8;
  if (sundistsquared > 0) {
    gh = (g * (deltah / sundistsquared)) >> 8;
    gv = (g * (deltav / sundistsquared)) >> 8;
    if (!cfgravinward) {
      gh = -gh;
      gv = -gv;
    }
    if (h > (SUNHLOC << 16))
      *dh -= gh;
      else *dh += gh;
    if (v > (SUNVLOC << 16))
      *dv -= gv;
      else *dv += gv;
  }
}

movestuff(ship)
struct TShip	*ship;
{
  int		i;
  int		wereA;

  wereA = (ship == &playerA);
  for (i=0; i<MAXSHOTS; i++) {
    if (ship->shots[i].h != 0x400000) {
      ship->shots[i].h += ship->shots[i].dh;
      ship->shots[i].v += ship->shots[i].dv;
      if (cfgravamount > 0) {
	dogravity((cfgravamount << 2) + 0x140,
		  ship->shots[i].h, ship->shots[i].v,
		  &ship->shots[i].dh, &ship->shots[i].dv);
      }
      if (cfwrapworld) {
	if (ship->shots[i].h > 0x1bc0000) ship->shots[i].h = 0x7c0000;
	if (ship->shots[i].h < 0x7c0000) ship->shots[i].h = 0x1bc0000;
	if (ship->shots[i].v > 0x15c0000) ship->shots[i].v = 0x7c0000;
	if (ship->shots[i].v < 0x7c0000) ship->shots[i].v = 0x15c0000;
      } else {
	if ((ship->shots[i].h > 0x1bc0000) && (ship->shots[i].dh > 0))
		ship->shots[i].dh = -ship->shots[i].dh;
	if ((ship->shots[i].h < 0x7c0000) && (ship->shots[i].dh < 0))
		ship->shots[i].dh = -ship->shots[i].dh;
	if ((ship->shots[i].v > 0x15c0000) && (ship->shots[i].dv > 0))
		ship->shots[i].dv = -ship->shots[i].dv;
	if ((ship->shots[i].v < 0x7c0000) && (ship->shots[i].dv < 0))
		ship->shots[i].dv = -ship->shots[i].dv;
      }
      ship->shots[i].stepsremaining -= 1;
      if ((ship->shots[i].stepsremaining == 0) ||
	  collision(SUNHLOC << 16, SUNVLOC << 16,
		    ship->shots[i].h, ship->shots[i].v, 8, 3)) {
	ship->shots[i].h = 0x400000;
	ship->shots[i].v = 0x400000;
	ship->shots[i].dh = 0;
	ship->shots[i].dv = 0;
      }
      drawshot(&ship->shots[i], (wereA) ? i : i+6);
    }
  }
  /* actually move the ship */
  ship->h += ship->dh;
  ship->v += ship->dv;
  /* check bounds */
  if (cfwrapworld) {
    if (ship->h > 0x1b80000) ship->h -= 0x1500000;
    if (ship->h < 0x780000) ship->h += 0x1500000;
    if (ship->v > 0x1580000) ship->v -= 0xf00000;
    if (ship->v < 0x780000) ship->v += 0xf00000;
  } else {
    if ((ship->h > 0x1b80000) && (ship->dh > 0)) ship->dh = -ship->dh;
    if ((ship->h < 0x780000) && (ship->dh < 0)) ship->dh = -ship->dh;
    if ((ship->v > 0x1580000) && (ship->dv > 0)) ship->dv = -ship->dv;
    if ((ship->v < 0x780000) && (ship->dv < 0)) ship->dv = -ship->dv;
  }
  /* damping */
  if (cffriction) {
    if (ship->dh > 0) ship->dh -= ship->dh >> 5;
    if (ship->dh < 0) ship->dh += (-ship->dh) >> 5;
    if (ship->dv > 0) ship->dv -= ship->dv >> 5;
    if (ship->dv < 0) ship->dv += (-ship->dv) >> 5;
  }
  /* sun gravity */
  if (cfgravamount > 0)
    dogravity(cfgravamount, ship->h, ship->v, &ship->dh, &ship->dv);
}

shipcontrol(ship, ctrlInfo)
struct TShip	*ship;
int		ctrlInfo;
{
  int		i;
  int		wasthrusting;
  int		wereA;

  wereA = (ship == &playerA);
  if ((ctrlInfo & ctrlSTART) && (!ship->startdown)) {
    ship->startdown = 1;
    paused = 1 - paused;
    drawpause = 1;
    if (paused) gemspauseall(); else gemsresumeall();
  }
  if (!(ctrlInfo & ctrlSTART)) ship->startdown = 0;
  wasthrusting = ship->thrusting;
  ship->thrusting = 0;
  if (!paused) {

    movestuff(ship);

    if (ship->blowupstate == 0) { /* do normal motion commands */
      if (ctrlInfo & ctrlLEFT) ship->dir -= 1;
      if (ctrlInfo & ctrlRIGHT) ship->dir += 1;
      ship->dir &= (16 timesDIR) - 1;
      ship->energy += (cffastcharge) ? 4 : 1;
      if ((ctrlInfo & ctrlB) && (ship->energy > (2 << 7))) {
	if (!wasthrusting) {
	  gemsprogchange(15, 17);
	  gemsnoteon(15, 24);
	}
	ship->thrusting = 1;
	ship->energy -= 16;
	ship->dh += hdirtable[ship->dir divDIR];
	ship->dv += vdirtable[ship->dir divDIR];
	if (cffriction) {
	  ship->dh += hdirtable[ship->dir divDIR] << 1;
	  ship->dv += vdirtable[ship->dir divDIR] << 1;
	}
	if (ship->dh > 0) {
	  if (ship->dh > MAXSPEED) ship->dh = MAXSPEED;
	} else {
	  if (-ship->dh > MAXSPEED) ship->dh = -MAXSPEED;
	}
	if (ship->dv > 0) {
	  if (ship->dv > MAXSPEED) ship->dv = MAXSPEED;
	} else {
	  if (-ship->dv > MAXSPEED) ship->dv = -MAXSPEED;
	}
      }
      if (wasthrusting && (ship->thrusting == 0)) {
	gemsnoteoff(15, 24);
      }
      if ((ctrlInfo & ctrlA) && !ship->shieldbuttondown) {
	ship->shieldon = 1;
	ship->shieldbuttondown = 1;
      }
      if (!(ctrlInfo & ctrlA)) {
	ship->shieldbuttondown = 0;
	ship->shieldon = 0;
      }
      if (ship->energy < (6 << 7)) ship->shieldon = 0;
      if (ship->shieldon) ship->energy -= 32;
      if ((ctrlInfo & ctrlC) && !ship->fired && !ship->shieldon) {
	ship->fired = 1;
	for (i=0; i<MAXSHOTS; i++) {
	  if (ship->shots[i].h == 0x400000) {
	    fireshot(ship, &ship->shots[i]);
	    drawshot(&ship->shots[i], (wereA) ? i : i+6);
	    break;
	  }
	}
      }
      if (!(ctrlInfo & ctrlC)) ship->fired = 0;
      if (ship->energy < 0) ship->energy = 0;
      if (ship->energy > (40 << 7)) ship->energy = 40 << 7;
    } else { /* the ship is blowing up... */
      ship->blowupstate += 1;
      if (ship->blowupstate >= BLOWUPFRAMES timesBLOWUP) { /* done */
	if (ship->lives < 100) ship->lives--;
	if (wasthrusting) gemsnoteoff(15, 24);
	ship->energy = 40 << 7;
	ship->blowupstate = 0;
	ship->dh = 0;
	ship->dv = 0;
	ship->dir = ((wereA) ? playerB.dir : playerA.dir) + (8 timesDIR);
	ship->dir &= (16 timesDIR) - 1;
	ship->h = (wereA) ? playerB.h : playerA.h;
	ship->v = (wereA) ? playerB.v : playerA.v;
	ship->h += hdirtable[ship->dir divDIR] << 9;
	ship->v += vdirtable[ship->dir divDIR] << 9;
	ship->h += hdirtable[ship->dir divDIR] << 8;
	ship->v += vdirtable[ship->dir divDIR] << 8;
	if (ship->lives == 0) {
	  ship->h = 0x400000;
	  ship->v = 0x400000;
	}
	if (wereA) {
	  if (playerB.lives == 0) {
	    ship->h = 176 << 16;
	    ship->v = 176 << 16;
	  }
	} else { /* were B */
	  if (playerA.lives == 0) {
	    ship->h = 400 << 16;
	    ship->v = 272 << 16;
	  }
	}
	while (collision(ship->h, ship->v, SUNHLOC << 16, SUNVLOC << 16,
			 16, 0)) {
	  ship->h += hdirtable[ship->dir divDIR] << 8;
	  ship->v += vdirtable[ship->dir divDIR] << 8;
	}
      }
    }
  }
}

playexplosionsound()
{
  gemsnoteoff(15, 24);		/* turn off thrust */
  gemsprogchange(15, 18);
  gemsnoteon(15, 48);
}

hitcheck(shipA, shipB)
struct TShip	*shipA;
struct TShip	*shipB;
{
  int		i;

  if (shipA->shieldon) return;
  if (shipA->blowupstate != 0) return;
  for (i=0; i<MAXSHOTS; i++) {
    if (shipB->shots[i].h != 0x400000) {
      if (collision(shipA->h, shipA->v,
		    shipB->shots[i].h, shipB->shots[i].v, 8, 4)) {
	shipA->blowupstate = 1;
	playexplosionsound();
	shipB->score += 100;
	redrawscore = 1;
	shipB->shots[i].h = 0x400000;
	shipB->shots[i].v = 0x400000;
	shipB->shots[i].dh = 0;
	shipB->shots[i].dv = 0;
	drawshot(&shipB->shots[i], (shipB == &playerA) ? i : i+6);
      }
    }
  }
}

otherhitchecks()
{
  long	xtemp;

  if (cfsunfatal) {
    if (collision(playerA.h, playerA.v, SUNHLOC << 16, SUNVLOC << 16, 16, 6) &&
        !playerA.blowupstate) {
      playerA.blowupstate = 1;
      playexplosionsound();
    }
    if (collision(playerB.h, playerB.v, SUNHLOC << 16, SUNVLOC << 16, 16, 6) &&
        !playerB.blowupstate) {
      playerB.blowupstate = 1;
      playexplosionsound();
    }
  }
  if (collision(playerA.h, playerA.v, playerB.h, playerB.v, 16, 7)) {
    xtemp = playerA.dh;
    playerA.dh = playerB.dh;
    playerB.dh = xtemp;
    xtemp = playerA.dv;
    playerA.dv = playerB.dv;
    playerB.dv = xtemp;
  }
  if (collision(playerA.h, playerA.v, playerB.h, playerB.v, 16, 8) &&
      !playerA.shieldon && !playerB.shieldon &&
      !playerA.blowupstate && !playerB.blowupstate) {
    playerA.blowupstate = 1;
    playexplosionsound();
    playerB.blowupstate = 3;
    xtemp = playerA.dh;
    playerA.dh = playerB.dh;
    playerB.dh = xtemp;
    xtemp = playerA.dv;
    playerA.dv = playerB.dv;
    playerB.dv = xtemp;
  }
}

updateshipsprites(ship, whichpal, sprite, splink, shield, shlink)
struct TShip	*ship;
int		whichpal, sprite, splink, shield, shlink;
{
  if ((ship->lives <= 0) ||
      (ship->blowupstate > BLOWUPTHRESH timesBLOWUP)) { /* ship gone */
    initsprite(sprite,
      sHSize16+sVSize16+splink,
      0x40, 0x40,
      (whichpal << sCPShift)+0x400);
  } else {					/* ship showing */
    if (ship->thrusting) {
      initsprite(sprite,
	sHSize16+sVSize16+splink,
	ship->h >> 16,
	ship->v >> 16,
	(whichpal << sCPShift)+shipTdirtable[ship->dir divDIR]);
    } else {
      initsprite(sprite,
	sHSize16+sVSize16+splink,
	ship->h >> 16,
	ship->v >> 16,
	(whichpal << sCPShift)+shipdirtable[ship->dir divDIR]);
    }
  }
  if (ship->blowupstate > 0) {
    initsprite(shield,
      sHSize32+sVSize32+shlink,
      (ship->h >> 16) - 8,
      (ship->v >> 16) - 8,
      (1 << sCPShift)+(0xf0 & ((ship->blowupstate divBLOWUP) << 4)) +
	  0x400);
  } else {
    if (ship->shieldon && (ship->lives > 0)) {
      initsprite(shield,
	sHSize16+sVSize16+shlink,
	ship->h >> 16,
	ship->v >> 16,
	(1 << sCPShift)+0x4fc);
    } else {
      initsprite(shield,
	sHSize16+sVSize16+shlink,
	0x40,
	0x40,
	(1 << sCPShift)+0x4fc);
    }
  }
}

updatesprites()
{
  updateshipsprites(&playerA, 1, playerAsprite, playerAsplink,
				 playerAshield, playerAshlink);
  updateshipsprites(&playerB, 2, playerBsprite, playerBsplink,
				 playerBshield, playerBshlink);

  playerA.blinkcounter--;
  if (playerA.blinkcounter <= 0) {
    playerA.blinkcounter = 5;
    palette[16 + 12] = blinkcolor[playerA.blinkindex++];
    if (playerA.blinkindex > 14) playerA.blinkindex = 0;
  }
  playerB.blinkcounter--;
  if (playerB.blinkcounter <= 0) {
    playerB.blinkcounter = 4;
    palette[32 + 12] = blinkcolor[playerB.blinkindex++];
    if (playerB.blinkindex > 14) playerB.blinkindex = 0;
  }
  if (cfsunshowing) {
    initsprite(sunsprite,
      sHSize16+sVSize16,		/* 16x16 sprite, not linked */
      SUNHLOC, SUNVLOC,			/* starts here */
      (1 << sCPShift)+0x3f0+(((sunstate++ >> SUNFACTOR) << 2) & 0x0c));
  } else {
    initsprite(sunsprite,
      sHSize16+sVSize16,		/* 16x16 sprite, not linked */
      0x400000, 0x400000,		/* not showing */
      (1 << sCPShift)+0x3f0);
  }
}

playloop()
{
  int	i;
  char	*pause;

  if (playerA.lives > 0) shipcontrol(&playerA, readCtrlA());
  if (playerB.lives > 0) shipcontrol(&playerB, readCtrlB());

  hitcheck(&playerA, &playerB);
  hitcheck(&playerB, &playerA);

  otherhitchecks();

  updatesprites();

  drawscores();

  if (drawpause) {
    drawpause = 0;
    if (paused) {
      pause = "PAUSED";
    } else {
      pause = "      ";
    }
    drawstring(17, 25, (long)pause);
  }

  VDPaddr = 0x6d000003;
  for (i=0; i<40; i++) VDPdata = scorearea[i];
  VDPaddr = 0x6d800003;
  for (i=40; i<80; i++) VDPdata = scorearea[i];

  for (i=0; i<NUMCYCLERS; i++) {
    if (cycleptr[i] != 0) {
      cyclecurtime[i]--;
      if (cyclecurtime[i] <= 0) {
	cyclecurtime[i] = cycletime[i];
	if (*cycleptr[i] == 0xfff) cycleptr[i] = cycleptrinit[i];
	palette[(cyclepal[i] << 4) + cycleentry[i]] = *cycleptr[i]++;
      }
    }
  }

  paletteptr = &palette[0];
  copypaletteinVBL = 3;
  while (copypaletteinVBL != 0) ;
}

play(p)
int	p;
{
  int	i, j, k;
  long	t;
  short *ptr;

  eraseCRAM();
  eraseVSRAM();
  eraseVRAM();

  gemsstartsong(0);

  loadchars(&expdata, 0x400, 192);
  loadchars(&sundata, 0x3f0, 16);
  loadchars(&shipdata, 0x4d4, 45);
  loadchars(&letterdata, CHARBASE, 41);
  loadchars(&barchars, 0x530, 11);
  loadPalette(&bwpalette, 0);
  loadPalette(&shippalette, 1);
  loadPalette(&shippalette+32, 2);
/* load the character data from STARCHAR.ASM		*/
  loadchars(&stardata, 0, ((&stardataend-&stardata) >> 5));

/* load the image from STARART.ASM into scroll A at location (0, 0)	*/
  loadScrollB(&starart, PICTWIDTH, PICTHEIGHT, SCROLLWIDTH);


  ptr = (short *)&bwpalette;
  for (i=0; i<16; i++) palette[i] = *ptr++;
  ptr = (short *)&shippalette;
  for (i=16; i<48; i++) palette[i] = *ptr++;
  for (i=0; i<NUMCYCLERS; i++) {
    cycleptr[i] = 0;
    cyclecurtime[i] = 1;
  }

  cycleptrinit[0] = &cycle1;  cycleptr[0] = &cycle1;
  cycletime[0] = 29;
  cyclepal[0] = 0;  cycleentry[0] = 1;

  cycleptrinit[1] = &cycle2;  cycleptr[1] = &cycle2;
  cycletime[1] = 29;
  cyclepal[1] = 0;  cycleentry[1] = 2;

  cycleptrinit[2] = &cycle3;  cycleptr[2] = &cycle3;
  cycletime[2] = 29;
  cyclepal[2] = 0;  cycleentry[2] = 3;

  cycleptrinit[3] = &cycle4;  cycleptr[3] = &cycle4;
  cycletime[3] = 23;
  cyclepal[3] = 0;  cycleentry[3] = 4;

  cycleptrinit[4] = &cycle5;  cycleptr[4] = &cycle5;
  cycletime[4] = 27;
  cyclepal[4] = 0;  cycleentry[4] = 5;

  cycleptrinit[5] = &cycle6;  cycleptr[5] = &cycle6;
  cycletime[5] = 27;
  cyclepal[5] = 0;  cycleentry[5] = 6;

  cycleptrinit[6] = &cycle7;  cycleptr[6] = &cycle7;
  cycletime[6] = 27;
  cyclepal[6] = 0;  cycleentry[6] = 7;

  t = 0;
  noiseon = 0;
  paused = 0;
  sunstate = 0;
  redrawscore = 1;
  drawpause = 0;
  for (i=0; i<80; i++) scorearea[i] = 0;

  initship(&playerA, 176, 176);
  initship(&playerB, 400, 272);
  players = p;
  if (p < 2) {
    playerB.h = 0x400000;
    playerB.v = 0x400000;
    playerB.lives = 0;
  }

  for (i=0; i<12; i++) {		/* sprites 0-11 are shots */
    initsprite(i,			/* sprite i */
	sHSize8+sVSize8+i+1,		/* 8x8 sprite, linked to i+1 */
	0x400000, 0x400000,		/* starts here */
	(1 << sCPShift)+0x500);		/* palette 1, char $500 */
  }

  goosedma();

  updatesprites();

  while (1) {

    if ((playerA.lives + playerB.lives) == 0) {
      if (t == 0) t = tickcount;
        else if (t + 1200 < tickcount) break;
      if ((readCtrlA() | readCtrlB()) & ctrlSTART) {
	while ((readCtrlA() | readCtrlB()) & ctrlSTART) ;
	break;
      }
    }

    playloop();
    
  }

  gemsstopsong(0);
}


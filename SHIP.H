#define MAXSHOTS 4
#define playerAshield 12
#define playerAshlink 13
#define playerAsprite 13
#define playerAsplink 14
#define playerBshield 14
#define playerBshlink 15
#define playerBsprite 15
#define playerBsplink 16
#define sunsprite 16
#define MAXSPEED 0x50000
#define BLOWUPTHRESH 6
#define BLOWUPFRAMES 12
#define timesDIR << 2
#define divDIR >> 2
#define timesBLOWUP << 2
#define divBLOWUP >> 2
#define SUNFACTOR 4
#define NUMBERBASE 0x51b
#define CHARBASE 0x501
#define	SUNHLOC 0x110
#define	SUNVLOC 0xe0
#define PICTWIDTH 40
#define PICTHEIGHT 28
#define SEGAWIDTH 12
#define SEGAHEIGHT 4
#define SEGAHPOS 14
#define SEGAVPOS 10
#define COLORCYCLEDELAY 3
#define NUMSEGAPALETTES 21
#define SCROLLWIDTH 64

struct TShot {
	long	h, v;
	long	dh, dv;
	short	stepsremaining;
};

struct TShip {
	long		h, v;
	long		dh, dv;
	short		dir;
	char		fired;
	char		thrusting;
	char		startdown;
	char		shieldon;
	char		shieldbuttondown;
	short		blowupstate;
	short		score;
	short		firesound;
	short		energy;
	short		lives;
	short		blinkindex;
	short		blinkcounter;
	struct TShot	shots[MAXSHOTS];
};

extern char	shipdata;
extern char	expdata;
extern char	sundata;
extern char	letterdata;
extern char	arrowdata;
extern short	blinkcolor[];
extern char	barchars;
extern char	shippalette;
extern char	bwpalette;
extern short	shipdirtable[];
extern short	shipTdirtable[];
extern long	hdirtable[];
extern long	vdirtable[];
extern char	palettedata;
extern char	cellmap;
extern char	chardata;
extern char	chardataend;
extern char	zerochar;
extern char	blackpalette;
extern char	segachar;
extern char	segaart;
extern char	segapal;
extern char	stardata;
extern char	stardataend;
extern char	starart;


extern char patchbank;
extern char envbank;
extern char seqbank;
extern char sampbank;


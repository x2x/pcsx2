/*  ZeroGS KOSMOS
 *  Copyright (C) 2005-2006 zerofrog@gmail.com
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef __GS_H__
#define __GS_H__

#ifdef _WIN32

#include <windows.h>
#include <windowsx.h>

extern HWND GShwnd;

#else // linux basic definitions

#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/extensions/xf86vmode.h>
#include <gtk/gtk.h>
//#include <sys/stat.h>
#include <sys/types.h>

#endif

#include <stdio.h>
#include <malloc.h>
#include <assert.h>

extern float fFPS;

#define GSdefs
#include "PS2Edefs.h"

// need C definitions -- no mangling please!
extern "C" u32   CALLBACK PS2EgetLibType(void);
extern "C" u32   CALLBACK PS2EgetLibVersion2(u32 type);
extern "C" char* CALLBACK PS2EgetLibName(void);

#include "zerogsmath.h"

#include <assert.h>

#include <vector>
#include <string>
using namespace std;

extern u32 THR_KeyEvent; // value for passing out key events beetwen threads
extern bool THR_bShift;
extern std::string s_strIniPath; // Air's new (r2361) new constant for ini file path 

#ifndef _WIN32
#if !defined(_MSC_VER) && !defined(HAVE_ALIGNED_MALLOC)

// declare linux equivalents
static __forceinline void* pcsx2_aligned_malloc(size_t size, size_t align)
{
	assert( align < 0x10000 );
	char* p = (char*)malloc(size+align);
	int off = 2+align - ((int)(uptr)(p+2) % align);

	p += off;
	*(u16*)(p-2) = off;

	return p;
}

static __forceinline void pcsx2_aligned_free(void* pmem)
{
	if( pmem != NULL ) {
		char* p = (char*)pmem;
		free(p - (int)*(u16*)(p-2));
	}
}

#define _aligned_malloc pcsx2_aligned_malloc
#define _aligned_free pcsx2_aligned_free

#endif

#include <sys/timeb.h>	// ftime(), struct timeb

inline unsigned long timeGetTime()
{
#ifdef _WIN32
	_timeb t;
	_ftime(&t);
#else
	timeb t;
	ftime(&t);
#endif

	return (unsigned long)(t.time*1000+t.millitm);
}

#define max(a,b)			(((a) > (b)) ? (a) : (b))
#define min(a,b)			(((a) < (b)) ? (a) : (b))

struct RECT
{
	int left, top;
	int right, bottom;
};

/*typedef struct {
	Display *dpy;
	int screen;
	Window win;
	GLXContext ctx;
	XSetWindowAttributes attr;
	Bool fs;
	Bool doubleBuffered;
	XF86VidModeModeInfo deskMode;
	int x, y;
	unsigned int width, height;
	unsigned int depth;	
} GLWindow;*/

#define GL_X11_WINDOW

class GLWindow
{
	private:
#ifdef GL_X11_WINDOW
		Display *glDisplay;
		Window glWindow;
		int glScreen;
		GLXContext context;
		XSetWindowAttributes attr;
		XF86VidModeModeInfo deskMode;
#endif
		bool fullScreen, doubleBuffered;
		s32 x, y;
		u32 width, height, depth;

	public:
		void SwapGLBuffers();
		void SetTitle(char *strtitle);
		bool CreateWindow(void *pDisplay);
		bool ReleaseWindow();
		void CloseWindow();
		bool DisplayWindow(int _width, int _height);
		void ResizeCheck();
};

extern GLWindow GLWin;

#else

#define GL_WIN32_WINDOW

class GLWindow
{
	private:
		bool fullScreen, doubleBuffered;
		s32 x, y;
		u32 width, height, depth;

	public:
		void SwapGLBuffers();
		void SetTitle(char *strtitle);
		bool CreateGLWindow(void *pDisplay);
		bool ReleaseWindow();
		void CloseWindow();
		bool DisplayWindow(int _width, int _height);
		void ResizeCheck();
};

extern GLWindow GLWin;

#endif // linux basic definitions

struct Vector_16F
{
	u16 x, y, z, w;
};

/////////////////////
// define when releasing
// The only code that uses it is commented out!
//#define ZEROGS_CACHEDCLEAR // much better performance
//#define RELEASE_TO_PUBLIC
// fixme - We should use ZEROGS_DEVBUILD to determine devel/debug builds from "public release" builds.
//  Means a lot of search-and-replace though. (air)

#ifdef ZEROGS_DEVBUILD
#define GS_LOG __Log
#else
#define GS_LOG 0&&
#endif

#define ERROR_LOG __LogToConsole
//Logging for errors that are called often should have a time counter.

#ifdef __LINUX__
static u32 __attribute__((unused)) lasttime = 0; 
static u32 __attribute__((unused)) BigTime = 5000;
static bool __attribute__((unused)) SPAM_PASS;
#else
static u32 lasttime = 0; 
static u32 BigTime = 5000;
static bool SPAM_PASS;
#endif

#define ERROR_LOG_SPAM(text) { \
	if( timeGetTime() - lasttime > BigTime ) { \
		ERROR_LOG(text); \
		lasttime = timeGetTime(); \
	} \
}
// The same macro with one-argument substitution.
#define ERROR_LOG_SPAMA(fmt, value) { \
	if( timeGetTime() - lasttime > BigTime ) { \
		ERROR_LOG(fmt, value); \
		lasttime = timeGetTime(); \
	} \
}

#define ERROR_LOG_SPAM_TEST(text) {\
	if( timeGetTime() - lasttime > BigTime ) { \
		ERROR_LOG(text); \
		lasttime = timeGetTime(); \
		SPAM_PASS = true; \
	} \
	else \
		SPAM_PASS = false; \
}

#if DEBUG_PROF
#define FILE_IS_IN_CHECK ((strcmp(__FILE__, "targets.cpp") == 0) || (strcmp(__FILE__, "ZZoglFlush.cpp") == 0))

#define FUNCLOG {\
	static bool Was_Here = false; \
	static unsigned long int waslasttime = 0; \
	if (!Was_Here && FILE_IS_IN_CHECK) { \
		Was_Here = true;\
		ERROR_LOG("%s:%d %s\n", __FILE__, __LINE__, __func__); \
		waslasttime = timeGetTime(); \
	} \
	if (FILE_IS_IN_CHECK && (timeGetTime() - waslasttime > BigTime ))  { \
		Was_Here = false; \
	} \
}
#else
#define FUNCLOG
#endif

#define DEBUG_LOG printf

#ifdef RELEASE_TO_PUBLIC
#define WARN_LOG 0&&
#define PRIM_LOG 0&&
#else
#define WARN_LOG printf
#define PRIM_LOG if (conf.log & 0x00000010) GS_LOG
#endif

#ifndef GREG_LOG
#define GREG_LOG 0&&
#endif
#ifndef PRIM_LOG
#define PRIM_LOG 0&&
#endif
#ifndef WARN_LOG
#define WARN_LOG 0&&
#endif

#define REG64(name) \
union name			\
{					\
	u64 i64;		\
	u32 ai32[2];	\
	struct {		\

#define REG128(name)\
union name			\
{					\
	u64 ai64[2];	\
	u32 ai32[4];	\
	struct {		\

#define REG64_(prefix, name) REG64(prefix##name)
#define REG128_(prefix, name) REG128(prefix##name)

#define REG_END }; };
#define REG_END2 };

#define REG64_SET(name) \
union name			\
{					\
	u64 i64;		\
	u32 ai32[2];	\

#define REG128_SET(name)\
union name			\
{					\
	u64 ai64[2];	\
	u32 ai32[4];	\

#define REG_SET_END };

REG64_(GSReg, BGCOLOR)
	u32 R:8;
	u32 G:8;
	u32 B:8;
	u32 _PAD1:8;
	u32 _PAD2:32;
REG_END

REG64_(GSReg, BUSDIR)
	u32 DIR:1;
	u32 _PAD1:31;
	u32 _PAD2:32;
REG_END

REG64_(GSReg, CSR)
	u32 SIGNAL:1;
	u32 FINISH:1;
	u32 HSINT:1;
	u32 VSINT:1;
	u32 EDWINT:1;
	u32 ZERO1:1;
	u32 ZERO2:1;
	u32 _PAD1:1;
	u32 FLUSH:1;
	u32 RESET:1;
	u32 _PAD2:2;
	u32 NFIELD:1;
	u32 FIELD:1;
	u32 FIFO:2;
	u32 REV:8;
	u32 ID:8;
	u32 _PAD3:32;
REG_END

REG64_(GSReg, DISPFB) // (-1/2)
	u32 FBP:9;
	u32 FBW:6;
	u32 PSM:5;
	u32 _PAD:12;
	u32 DBX:11;
	u32 DBY:11;
	u32 _PAD2:10;
REG_END

REG64_(GSReg, DISPLAY) // (-1/2)
	u32 DX:12;
	u32 DY:11;
	u32 MAGH:4;
	u32 MAGV:2;
	u32 _PAD:3;
	u32 DW:12;
	u32 DH:11;
	u32 _PAD2:9;
REG_END

REG64_(GSReg, EXTBUF)
	u32 EXBP:14;
	u32 EXBW:6;
	u32 FBIN:2;
	u32 WFFMD:1;
	u32 EMODA:2;
	u32 EMODC:2;
	u32 _PAD1:5;
	u32 WDX:11;
	u32 WDY:11;
	u32 _PAD2:10;
REG_END

REG64_(GSReg, EXTDATA)
	u32 SX:12;
	u32 SY:11;
	u32 SMPH:4;
	u32 SMPV:2;
	u32 _PAD1:3;
	u32 WW:12;
	u32 WH:11;
	u32 _PAD2:9;
REG_END

REG64_(GSReg, EXTWRITE)
	u32 WRITE;
	u32 _PAD2:32;
REG_END

REG64_(GSReg, IMR)
	u32 _PAD1:8;
	u32 SIGMSK:1;
	u32 FINISHMSK:1;
	u32 HSMSK:1;
	u32 VSMSK:1;
	u32 EDWMSK:1;
	u32 _PAD2:19;
	u32 _PAD3:32;
REG_END

REG64_(GSReg, PMODE)
	u32 EN1:1;
	u32 EN2:1;
	u32 CRTMD:3;
	u32 MMOD:1;
	u32 AMOD:1;
	u32 SLBG:1;
	u32 ALP:8;
	u32 _PAD:16;
	u32 _PAD1:32;
REG_END

REG64_(GSReg, SIGLBLID)
	u32 SIGID:32;
	u32 LBLID:32;
REG_END

REG64_(GSReg, SMODE1)
	u32 RC:3;
	u32 LC:7;
	u32 T1248:2;
	u32 SLCK:1;
	u32 CMOD:2;
	u32 EX:1;
	u32 PRST:1;
	u32 SINT:1;
	u32 XPCK:1;
	u32 PCK2:2;
	u32 SPML:4;
	u32 GCONT:1;
	u32 PHS:1;
	u32 PVS:1;
	u32 PEHS:1;
	u32 PEVS:1;
	u32 CLKSEL:2;
	u32 NVCK:1;
	u32 SLCK2:1;
	u32 VCKSEL:2;
	u32 VHP:1;
	u32 _PAD1:27;
REG_END

REG64_(GSReg, SMODE2)
	u32 INT:1;
	u32 FFMD:1;
	u32 DPMS:2;
	u32 _PAD2:28;
	u32 _PAD3:32;
REG_END

REG64_(GSReg, SIGBLID)
	u32 SIGID;
	u32 LBLID;
REG_END

extern int g_LastCRC;
extern u8* g_pBasePS2Mem;

#define PMODE ((GSRegPMODE*)(g_pBasePS2Mem+0x0000))
#define SMODE1 ((GSRegSMODE1*)(g_pBasePS2Mem+0x0010))
#define SMODE2 ((GSRegSMODE2*)(g_pBasePS2Mem+0x0020))
// SRFSH
#define SYNCH1 ((GSRegSYNCH1*)(g_pBasePS2Mem+0x0040))
#define SYNCH2 ((GSRegSYNCH2*)(g_pBasePS2Mem+0x0050))
#define SYNCV ((GSRegSYNCV*)(g_pBasePS2Mem+0x0060))
#define DISPFB1 ((GSRegDISPFB*)(g_pBasePS2Mem+0x0070))
#define DISPLAY1 ((GSRegDISPLAY*)(g_pBasePS2Mem+0x0080))
#define DISPFB2 ((GSRegDISPFB*)(g_pBasePS2Mem+0x0090))
#define DISPLAY2 ((GSRegDISPLAY*)(g_pBasePS2Mem+0x00a0))
#define EXTBUF ((GSRegEXTBUF*)(g_pBasePS2Mem+0x00b0))
#define EXTDATA ((GSRegEXTDATA*)(g_pBasePS2Mem+0x00c0))
#define EXTWRITE ((GSRegEXTWRITE*)(g_pBasePS2Mem+0x00d0))
#define BGCOLOR ((GSRegBGCOLOR*)(g_pBasePS2Mem+0x00e0))
#define CSR ((GSRegCSR*)(g_pBasePS2Mem+0x1000))
#define IMR ((GSRegIMR*)(g_pBasePS2Mem+0x1010))
#define BUSDIR ((GSRegBUSDIR*)(g_pBasePS2Mem+0x1040))
#define SIGLBLID ((GSRegSIGBLID*)(g_pBasePS2Mem+0x1080))

#define GET_GSFPS (((SMODE1->CMOD&1) ? 50 : 60) / (SMODE2->INT ? 1 : 2))

//
// sps2tags.h
//
#define GET_GIF_REG(tag, reg) \
	(((tag).ai32[2 + ((reg) >> 3)] >> (((reg) & 7) << 2)) & 0xf)

//
// GIFTag
REG128(GIFTag)
	u32 NLOOP:15;
	u32 EOP:1;
	u32 _PAD1:16;
	u32 _PAD2:14;
	u32 PRE:1;
	u32 PRIM:11;
	u32 FLG:2; // enum GIF_FLG
	u32 NREG:4;
	u64 REGS:64;
REG_END

typedef struct {
	int x, y, w, h;
} Rect;

typedef struct {
	int x, y;
} Point;

typedef struct {
	int x0, y0;
	int x1, y1;
} Rect2;

typedef struct {
	int x, y, c;
} PointC;

#define GSOPTION_FULLSCREEN 	0x2
#define GSOPTION_TGASNAP 	0x4
#define GSOPTION_CAPTUREAVI 	0x8

#define GSOPTION_WINDIMS	0x30
#define GSOPTION_WIN640		0x00
#define GSOPTION_WIN800		0x10
#define GSOPTION_WIN1024	0x20
#define GSOPTION_WIN1280	0x30
#define GSOPTION_WIDESCREEN	0x40

#define GSOPTION_WIREFRAME	0x100
#define GSOPTION_LOADED		0x8000

//Configuration values.
typedef struct 
{
	u8 mrtdepth; // write color in render target
	u8 interlace; // intelacing mode 0, 1, 3-off
	u8 aa;	// antialiasing 0 - off, 1 - 2x, 2 - 4x, 3 - 8x, 4 - 16x
	u8 negaa; // negative aliasing
	u8 bilinear; // set to enable bilinear support. 0 - off, 1 -- on, 2 -- force (use for textures that usually need it)
	u32 options; // game options -- different hacks.
	u32 gamesettings;// default game settings
	int width, height; // View target size, has no impact towards speed
	bool isWideScreen; // Widescreen support
#ifdef GS_LOG
	u32 log;
#endif
} GSconf;

// PS2 vertex
struct VertexGPU
{
	// gained from XYZ2, XYZ3, XYZF2, XYZF3, 
	// X -- bits 0-15, Y-16-31. Z - 32-63 if no F used, 32-55 otherwise, F (fog) - 56-63
	// X, Y stored in 12d3 format, 
	s16 x, y, f, resv0;		// note: xy is 12d3
	// Vertex color settings. RGB -- luminance of red/green/blue, A -- alpha. 1.0 == 0x80.
	// Goes grom RGBAQ register, bits 0-7, 8-15, 16-23 and 24-31 accordingly
	u32 rgba;
	u32 z;
	// Texture coordinates. S & T going from ST register (bits 0-31, and 32-63).
	// Q goes from RGBAQ register, bits 32-63
	float s, t, q;
};

// Almost same with previous, controlled by prim.fst flagf
struct Vertex
{
	u16 x, y, f, resv0;		// note: xy is 12d3
	u32 rgba;
	u32 z;
	float s, t, q;
	// Texel coordinate of vertex. Used if prim.fst == 1
	// Bits 0-14 and 16-30 of UV
	u16 u, v;
};

extern int g_GameSettings;
extern GSconf conf;
extern int ppf;

// PSM values
// PSM types == Texture Storage Format
enum PSM_value{
	PSMCT32		= 0,		// 000000
	PSMCT24		= 1,		// 000001
	PSMCT16		= 2,		// 000010
	PSMCT16S	= 10,		// 001010
	PSMT8		= 19,		// 010011
	PSMT4		= 20,		// 010100
	PSMT8H		= 27,		// 011011
	PSMT4HL		= 36,		// 100100
	PSMT4HH		= 44,		// 101100
	PSMT32Z		= 48,		// 110000
	PSMT24Z		= 49,		// 110001
	PSMT16Z		= 50,		// 110010
	PSMT16SZ	= 58,		// 111010
};

// CLUT = Color look up table. Set proper color to table according CLUT table.
// Used for PSMT8, PSMT8H, PSMT4, PSMT4HH, PSMT4HL textures
inline bool PSMT_ISCLUT(int psm) { return ((psm & 0x7) > 2);}

// PSMCT16, PSMCT16S, PSMT16Z, PSMT16SZ is 16-bit targets and usually there is
// two of them in each 32-bit word.
inline bool PSMT_IS16BIT(int psm) { return ((psm & 0x7) == 2);}

// PSMT32Z, PSMT24Z, PSMT16Z, PSMT16SZ is Z-buffer textures
inline bool PSMT_ISZTEX(int psm) {return ((psm & 0x30) == 0x30);}

// PSMCT16, PSMCT16S, PSMT8, PSMT8H, PSMT16Z and PSMT16SZ use only half 16 bit per pixel.
inline bool PSMT_ISHALF(int psm) {return ((psm & 2) == 2);}

// PSMT8 and PSMT8H use IDTEX8 CLUT, PSMT4H, PSMT4HL, PSMT4HH -- IDTEX4.
// Don't use it on non clut entries, please!
inline bool PSMT_IS8CLUT(int psm) {return ((psm & 3) == 3);}

// PSM16Z and PSMT16SZ use -1 offset to z-buff. Need to check this thesis.
inline bool PSMT_IS16Z(int psm) {return ((psm & 0x32) == 0x32);}

// Check target bit mode. PSMCT32 and 32Z return 0, 24 and 24Z - 1
// 16, 16S, 16Z, 16SZ -- 2, PSMT8 and 8H - 3, PSMT4, 4HL, 4HH -- 4.
inline int PSMT_BITMODE(int psm) {return (psm & 0x7);}

//----------------------- Data from registers -----------------------
// EE part. Data transfer packet description
typedef struct {
	int nloop;
	int eop;
	int nreg;
} tagInfo;

typedef struct {
	int mode;
	int regn;
	u64 regs;
	tagInfo tag;
} pathInfo;

typedef union {
	s64 SD;
	u64 UD;
	s32 SL[2];
	u32 UL[2];
	s16 SS[4];
	u16 US[4];
	s8  SC[8];
	u8  UC[8];
} reg64;

/* general purpose regs structs */
typedef struct {
	int fbp;
	int fbw;
	int fbh;
	int psm;
	u32 fbm;
} frameInfo;

// Create frame structure from known data
inline frameInfo CreateFrame(int fbp, int fbw, int fbh, int psm, u32 fbm){
	frameInfo frame;
	frame.fbp = fbp;
	frame.fbw = fbw;
	frame.fbh = fbh;
	frame.psm = psm;
	frame.fbm = fbm;
	return frame;
}

typedef struct {
	u16 prim;
	
	union {
		struct {
			u16 iip : 1;
			u16 tme : 1;
			u16 fge : 1;
			u16 abe : 1;
			u16 aa1 : 1;
			u16 fst : 1;
			u16 ctxt : 1;
			u16 fix : 1;
			u16 resv : 8;
		};
		u16 _val;
	};
} primInfo;

extern primInfo *prim;

typedef union {
	struct {
		u32 ate : 1;
		u32 atst : 3;
		u32 aref : 8;
		u32 afail : 2;
		u32 date : 1;
		u32 datm : 1;
		u32 zte : 1;
		u32 ztst : 2;
		u32 resv : 13;
	};
	u32 _val;
} pixTest;

typedef struct {
	int bp;
	int bw;
	int psm;
} bufInfo;

typedef struct {
	int tbp0;
	int tbw;
	int cbp;
	u16 tw, th;
	u8 psm;
	u8 tcc;
	u8 tfx;
	u8 cpsm;
	u8 csm;
	u8 csa;
	u8 cld;
} tex0Info;

#define TEX_MODULATE 0
#define TEX_DECAL 1
#define TEX_HIGHLIGHT 2
#define TEX_HIGHLIGHT2 3

typedef struct {
	int lcm;
	int mxl;
	int mmag;
	int mmin;
	int mtba;
	int l;
	int k;
} tex1Info;

typedef struct {
	int wms;
	int wmt;
	int minu;
	int maxu;
	int minv;
	int maxv;
} clampInfo;

typedef struct {
	int cbw;
	int cou;
	int cov;
} clutInfo;

typedef struct {
	int tbp[3];
	int tbw[3];
} miptbpInfo;

typedef struct {
	u16 aem;
	u8 ta[2];
	float fta[2];
} texaInfo;

typedef struct {
	int sx;
	int sy;
	int dx;
	int dy;
	int dir;
} trxposInfo;

typedef struct {
	union {
		struct {
			u8 a : 2;
			u8 b : 2;
			u8 c : 2;
			u8 d : 2;
		};
		u8 abcd;
	};

	u8 fix : 8;
} alphaInfo;

typedef struct {
	u16 zbp;		// u16 address / 64
	u8 psm;
	u8 zmsk;
} zbufInfo;

typedef struct {
	int fba;
} fbaInfo;

typedef struct {	
	Vertex gsvertex[3];
	u32 rgba;
	float q;
	Vertex vertexregs;
	
	int primC;		// number of verts current storing
	int primIndex;	// current prim index
	int nTriFanVert;

	int prac;
	int dthe;
	int colclamp;
	int fogcol;
	int smask;
	int pabe;
	u64 buff[2];
	int buffsize;
	int cbp[2];		// internal cbp registers

	u32 CSRw;

	primInfo _prim[2];
	bufInfo srcbuf, srcbufnew;
	bufInfo dstbuf, dstbufnew;

	clutInfo clut;
	
	texaInfo texa;
	trxposInfo trxpos, trxposnew;

	int imageWtemp, imageHtemp;

	int imageTransfer;		
	int imageWnew, imageHnew, imageX, imageY, imageEndX, imageEndY;

	pathInfo path1;
	pathInfo path2;
	pathInfo path3;

} GSinternal;

extern GSinternal gs;

extern FILE *gsLog;

void __Log(const char *fmt, ...);
void __LogToConsole(const char *fmt, ...);

void LoadConfig();
void SaveConfig();

extern void (*GSirq)();

void *SysLoadLibrary(char *lib);		// Loads Library
void *SysLoadSym(void *lib, char *sym);	// Loads Symbol from Library
char *SysLibError();					// Gets previous error loading sysbols
void SysCloseLibrary(void *lib);		// Closes Library
void SysMessage(const char *fmt, ...);

#ifdef __LINUX__
#include "Utilities/MemcpyFast.h"
#define memcpy_amd memcpy_fast
#else
extern "C" void * memcpy_amd(void *dest, const void *src, size_t n);
extern "C" u8 memcmp_mmx(const void *dest, const void *src, int n);
#endif

template <typename T>
class CInterfacePtr
{
public:
	inline CInterfacePtr() : ptr(NULL) {}
	inline explicit CInterfacePtr(T* newptr) : ptr(newptr) { if ( ptr != NULL ) ptr->AddRef(); }
	inline ~CInterfacePtr() { if( ptr != NULL ) ptr->Release(); }

	inline T* operator* () { assert( ptr != NULL); return *ptr; }
	inline T* operator->() { return ptr; }
	inline T* get() { return ptr; }

	inline void release() { 
		if( ptr != NULL ) { ptr->Release(); ptr = NULL; }
	}

	inline operator T*() { return ptr; }

	inline bool operator==(T* rhs) { return ptr == rhs; }
	inline bool operator!=(T* rhs) { return ptr != rhs; }

	inline CInterfacePtr& operator= (T* newptr) {
		if( ptr != NULL ) ptr->Release();
		ptr = newptr;

		if( ptr != NULL ) ptr->AddRef();
		return *this;
	}

private:
	T* ptr;
};

#define RGBA32to16(c) \
	(u16)((((c) & 0x000000f8) >>  3) | \
	(((c) & 0x0000f800) >>  6) | \
	(((c) & 0x00f80000) >>  9) | \
	(((c) & 0x80000000) >> 16)) \

#define RGBA16to32(c) \
	(((c) & 0x001f) <<  3) | \
	(((c) & 0x03e0) <<  6) | \
	(((c) & 0x7c00) <<  9) | \
	(((c) & 0x8000) ? 0xff000000 : 0) \

// converts float16 [0,1] to BYTE [0,255] (assumes value is in range, otherwise will take lower 8bits)
// f is a u16
static __forceinline u16 Float16ToBYTE(u16 f) {
	//assert( !(f & 0x8000) );
	if( f & 0x8000 ) return 0;

	u16 d = ((((f&0x3ff)|0x400)*255)>>(10-((f>>10)&0x1f)+15));
	return d > 255 ? 255 : d;
}

static __forceinline u16 Float16ToALPHA(u16 f) {
	//assert( !(f & 0x8000) );
	if( f & 0x8000 ) return 0;

	// round up instead of down (crash and burn), too much and charlie breaks
	u16 d = (((((f&0x3ff)|0x400))*255)>>(10-((f>>10)&0x1f)+15));
	d = (d)>>1;
	return d > 255 ? 255 : d;
}

#ifndef COLOR_ARGB
#define COLOR_ARGB(a,r,g,b) \
	((u32)((((a)&0xff)<<24)|(((r)&0xff)<<16)|(((g)&0xff)<<8)|((b)&0xff)))
#endif

// assumes that positive in [1,2] (then extracts fraction by just looking at the specified bits)
#define Float16ToBYTE_2(f) ((u8)(*(u16*)&f>>2))
#define Float16To5BIT(f) (Float16ToBYTE(f)>>3)

#define Float16Alpha(f) (((*(u16*)&f&0x7c00)>=0x3900)?0x8000:0) // alpha is >= 1

// converts an array of 4 u16s to a u32 color
// f is a pointer to a u16
#define Float16ToARGB(f) COLOR_ARGB(Float16ToALPHA(f.w), Float16ToBYTE(f.x), Float16ToBYTE(f.y), Float16ToBYTE(f.z));

#define Float16ToARGB16(f) (Float16Alpha(f.w)|(Float16To5BIT(f.x)<<10)|(Float16To5BIT(f.y)<<5)|Float16To5BIT(f.z))

// used for Z values
#define Float16ToARGB_Z(f) COLOR_ARGB((u32)Float16ToBYTE_2(f.w), Float16ToBYTE_2(f.x), Float16ToBYTE_2(f.y), Float16ToBYTE_2(f.z))
#define Float16ToARGB16_Z(f) ((Float16ToBYTE_2(f.y)<<8)|Float16ToBYTE_2(f.z))


inline float Clamp(float fx, float fmin, float fmax)
{
	if( fx < fmin ) return fmin;
	return fx > fmax ? fmax : fx;
}

// IMPORTANT: For every Register there must be an End
void DVProfRegister(char* pname);			// first checks if this profiler exists in g_listProfilers
void DVProfEnd(u32 dwUserData);
void DVProfWrite(char* pfilename, u32 frames = 0);
void DVProfClear();						// clears all the profilers

#define DVPROFILE
#ifdef DVPROFILE

class DVProfileFunc
{
public:
	u32 dwUserData;
	DVProfileFunc(char* pname) { DVProfRegister(pname); dwUserData = 0; }
	DVProfileFunc(char* pname, u32 dwUserData) : dwUserData(dwUserData) { DVProfRegister(pname); }
	~DVProfileFunc() { DVProfEnd(dwUserData); }
};

#else

class DVProfileFunc
{
public:
	u32 dwUserData;
	static __forceinline DVProfileFunc(char* pname) {}
	static __forceinline DVProfileFunc(char* pname, u32 dwUserData) { }
	~DVProfileFunc() {}
};

#endif

// PSMT16, 16S have shorter color per pixel, also cluted textures with half storage.
inline bool PSMT_ISHALF_STORAGE(const tex0Info& tex0) { 
	if (PSMT_IS16BIT(tex0.psm) || (PSMT_ISCLUT(tex0.psm) && tex0.cpsm > 1))
		return true;
	else
		return false;
}

//--------------------------- Inlines for bitwise ops
//--------------------------- textures
// Tex0Info (TEXD_x registers) bits, lower word 
// The register is really 64-bit, but we use 2 32bit ones to represent it
// Obtain tbp0 -- Texture Buffer Base Pointer (Word Address/64) -- from data. Bits 0-13.
inline int
ZZOglGet_tbp0_TexBits(u32 data) { 
	return (data	  ) & 0x3fff;
}

// Obtain tbw -- Texture Buffer Width (Texels/64) -- from data, do not multiply to 64. Bits 14-19
// ( data & 0xfc000 ) >> 14
inline int
ZZOglGet_tbw_TexBits(u32 data) { 
	return (data >> 14) & 0x3f;
}

// Obtain tbw -- Texture Buffer Width (Texels) -- from data, do multiply to 64, never return 0.
inline int
ZZOglGet_tbw_TexBitsMult(u32 data) { 
	int result = ZZOglGet_tbw_TexBits(data);
	if (result == 0)			
		return 64;
	else	
		return (result << 6);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25. 
// (data & 0x3f00000) >> 20
inline int
ZZOglGet_psm_TexBits(u32 data) { 
	return 	((data >> 20) & 0x3f);
}

// Obtain psm -- Pixel Storage Format -- from data. Bits 20-25. Fix incorrect psm == 9 
inline int
ZZOglGet_psm_TexBitsFix(u32 data) { 
	int result = ZZOglGet_psm_TexBits(data) ;
//	printf ("result %d\n", result);
	if ( result == 9 )
		result = 1;
	return result;
}

// Obtain tw -- Texture Width (Width = 2^TW) -- from data. Bits 26-29
// (data & 0x3c000000)>>26
inline u16
ZZOglGet_tw_TexBits(u32 data) { 
	return 	((data >> 26) & 0xf);
}

// Obtain tw -- Texture Width (Width = TW) -- from data. Width could newer be more than 1024.
inline u16
ZZOglGet_tw_TexBitsExp(u32 data) { 
	u16 result = ZZOglGet_tw_TexBits(data);
	if (result > 10) 
		result = 10;
	return (1<<result);
}

// TH set at the border of upper and higher words.
// Obtain th -- Texture Height (Height = 2^TH) -- from data. Bits 30-31 lower, 0-1 higher
// (dataLO & 0xc0000000) >> 30 + (dataHI & 0x3) * 0x4
inline u16
ZZOglGet_th_TexBits(u32 dataLO, u32 dataHI) { 
	return (((dataLO >> 30) & 0x3) | ((dataHI & 0x3) << 2));
}

// Obtain th --Texture Height (Height = 2^TH) -- from data. Height could newer be more than 1024.
inline u16
ZZOglGet_th_TexBitsExp(u32 dataLO, u32 dataHI) { 
	u16 result = ZZOglGet_th_TexBits(dataLO, dataHI);
	if (result > 10) 
		result = 10;
	return 	(1<<result);
}

// Tex0Info bits, higher word.
// Obtain tcc -- Tecture Color Component 0=RGB, 1=RGBA + use Alpha from TEXA reg when not in PSM -- from data. Bit 3
// (data & 0x4)>>2
inline u8
ZZOglGet_tcc_TexBits(u32 data) { 
	return  ((data >>  2) & 0x1);
}

// Obtain tfx -- Texture Function (0=modulate, 1=decal, 2=hilight, 3=hilight2) -- from data. Bit 4-5
// (data & 0x18)>>3
inline u8
ZZOglGet_tfx_TexBits(u32 data) { 
	return  ((data >>  3) & 0x3);
}

// Obtain cbp from data -- Clut Buffer Base Pointer (Address/256) -- Bits 5-18
// (data & 0x7ffe0)>>5
inline int
ZZOglGet_cbp_TexBits(u32 data) { 
	return  ((data >>  5) & 0x3fff);
}

// Obtain cpsm from data -- Clut pixel Storage Format -- Bits 19-22. 22nd is at no use.
// (data & 0x700000)>>19
inline u8
ZZOglGet_cpsm_TexBits(u32 data) { 
	return ((data >> 19) & 0xe);
}

// Obtain csm -- I don't know what is it -- from data. Bit 23
// (data & 0x800000)>>23
inline u8
ZZOglGet_csm_TexBits(u32 data) { 
	return ((data >> 23) & 0x1);
}

// Obtain csa -- -- from data. Bits 24-28
// (data & 0x1f000000)>>24
inline u8
ZZOglGet_csa_TexBits(u32 data) { 
	if ((data & 0x700000) == 0 ) // it is cpsm < 2 check
		return ((data >> 24) & 0xf);
	else	
		return ((data >> 24) & 0x1f);
}

// Obtain cld --   -- from data. Bits 29-31
// (data & 0xe0000000)>>29
inline u8
ZZOglGet_cld_TexBits(u32 data) { 
	return  ((data >> 29) & 0x7);
}

//-------------------------- frames 
// FrameInfo bits.
// Obtain fbp -- frame Buffer Base Pointer (Word Address/2048) -- from data. Bits 0-15
inline int
ZZOglGet_fbp_FrameBits(u32 data) { 
	return ((data      ) & 0x1ff);
}

// So we got adress / 64, henceby frame fbp and tex tbp have the same dimension -- "real adress" is x64.
inline int
ZZOglGet_fbp_FrameBitsMult(u32 data) { 
	return (ZZOglGet_fbp_FrameBits(data) << 5);
}

// Obtain fbw -- width (Texels/64) -- from data. Bits 16-23
inline int
ZZOglGet_fbw_FrameBits(u32 data) { 
	return ((data >> 16) & 0x3f);
}

inline int
ZZOglGet_fbw_FrameBitsMult(u32 data) { 
	return (ZZOglGet_fbw_FrameBits(data) << 6);
}


// Obtain psm -- Pixel Storage Format -- from data. Bits 24-29.
// (data & 0x3f000000) >> 24
inline int
ZZOglGet_psm_FrameBits(u32 data) { 
	return 	((data >> 24) & 0x3f);
}

// Function for calculating overal height from frame data.
inline int 
ZZOgl_fbh_Calc (int fbp, int fbw, int psm) {
	int fbh = ( 1024 * 1024 - 64 * fbp ) / fbw;
	fbh &= ~0x1f;
	if (PSMT_ISHALF(psm))
		fbh *= 2;
	if (fbh > 1024)
		fbh = 1024;
	return fbh ;
}
inline int
ZZOgl_fbh_Calc (frameInfo frame) {
	return ZZOgl_fbh_Calc(frame.fbp, frame.fbw, frame.psm);
}

// Calculate fbh from data, It does not set in register
inline int
ZZOglGet_fbh_FrameBitsCalc (u32 data) {
	int fbh = 0;
	int fbp = ZZOglGet_fbp_FrameBits(data);
	int fbw = ZZOglGet_fbw_FrameBits(data);
	int psm = ZZOglGet_psm_FrameBits(data);
	if (fbw > 0) 
		fbh = ZZOgl_fbh_Calc(fbp, fbw, psm) ;
	return fbh ;
}

// Obtain fbm -- frame mask -- from data. All higher word.
inline u32
ZZOglGet_fbm_FrameBits(u32 data) { 
	return 	(data);
}	

// Obtain fbm -- frame mask -- from data. All higher word. Fixed from psm == PCMT24 (without alpha)
inline u32
ZZOglGet_fbm_FrameBitsFix(u32 dataLO, u32 dataHI) { 
	if (PSMT_BITMODE(ZZOglGet_psm_FrameBits(dataLO)) == 1)
		return (dataHI | 0xff000000);
	else
		return dataHI;
}

// obtain colormask RED
inline u32
ZZOglGet_fbmRed_FrameBits(u32 data) { 
	return (data & 0xff);
}

// obtain colormask Green
inline u32
ZZOglGet_fbmGreen_FrameBits(u32 data) { 
	return ((data >> 8) & 0xff);
}

// obtain colormask Blue
inline u32
ZZOglGet_fbmBlue_FrameBits(u32 data) { 
	return ((data >> 16) & 0xff);
}

// obtain colormask Alpha
inline u32
ZZOglGet_fbmAlpha_FrameBits(u32 data) { 
	return ((data >> 24) & 0xff);
}

// obtain colormask Alpha
inline u32
ZZOglGet_fbmHighByte(u32 data) { 
	return (!!(data & 0x80000000));
}



//-------------------------- tex0 comparison
// Check if old and new tex0 registers have only clut difference
inline bool 
ZZOglAllExceptClutIsSame( u32* oldtex, u32* newtex) {
	return ((oldtex[0] == newtex[0]) && ((oldtex[1] & 0x1f) == (newtex[1] & 0x1f)));
}

// Check if the CLUT registers are same, except CLD
inline bool
ZZOglClutMinusCLDunchanged( u32* oldtex, u32* newtex) {
	return ((oldtex[1] & 0x1fffffe0) == (newtex[1] & 0x1fffffe0));
}

// Check if CLUT storage mode is not changed (CSA, CSM and CSPM)
inline bool
ZZOglClutStorageUnchanged( u32* oldtex, u32* newtex) {
	return ((oldtex[1] & 0x1ff10000) == (newtex[1] & 0x1ff10000));
}

// CSA and CPSM bitmask 0001 1111 0111 1000 ...
//                         60   56   52
#define CPSM_CSA_BITMASK 0x1f780000
#define CPSM_CSA_NOTMASK 0xe0870000

#endif
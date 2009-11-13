
/*************************************************
 *  imedefs.h                                    *
 *                                               *
 *  Copyright (C) 1995-1999 Microsoft Inc.       *
 *                                               *
 *************************************************/

#ifndef __IMEDEFS_H__
#define __IMEDEFS_H__
#define ENABLE_BHJDEBUG
#include "bhjdebug.h" 
#define NATIVE_CHARSET          GB2312_CHARSET
#define NATIVE_ANSI_CP          936
#define NATIVE_LANGUAGE         0x0804
// resource ID
#define IDI_IME                 0x0100


#define IDC_STATIC              -1

#define IDM_HLP                 0x0400
#define IDM_OPTGUD              0x0403
#define IDM_VER                 0x0401
#define IDM_PROP                0x0402

#define IDM_IME                 0x0450

#define DlgPROP                 101
#define DlgUIMODE               102
#define IDC_UIMODE1             1001
#define IDC_UIMODE2             1002
#define IDC_USER1               1003

// setting offset in .SET file
#define OFFSET_MODE_CONFIG      0
#define OFFSET_READLAYOUT       4

// state of composition
#define CST_INIT                0
#define CST_INPUT               1
#define CST_CHOOSE              2
#define CST_SYMBOL              3
#define CST_ALPHANUMERIC        7	// not in iImeState
#define CST_INVALID             8	// not in iImeState
#define CST_INVALID_INPUT       9	// not in iImeState
#define CST_CAPITAL             11

// state engin
#define ENGINE_COMP             0
#define ENGINE_ASCII            1
#define ENGINE_RESAULT          2
#define ENGINE_CHCAND           3
#define ENGINE_BKSPC            4
#define ENGINE_MULTISEL         5
#define ENGINE_ESC              6

#define CANDPERPAGE             9

#define MAXSTRLEN               40
#define MAXCAND                 256
#define IME_MAXCAND             94
#define IME_XGB_MAXCAND         190
#define IME_UNICODE_MAXCAND     256
#define MAXCODE                 12

// set ime character
#define SIC_INIT                0
#define SIC_SAVE2               1
#define SIC_READY               2
#define SIC_MODIFY              3
#define SIC_SAVE1               4

#define BOX_UI                  0


#define IMEINDEXNUM             1

#define IME_CMODE_INDEX_FIRST   0x1000

#define INDEX_UNICODE           0


// border for UI
#define UI_MARGIN               4
#define COMP_TEXT_Y             17

#define STATUS_DIM_X            20
#define STATUS_DIM_Y            20
#define STATUS_NAME_MARGIN      20

#define uCandHome               0
#define uCandUp                 1
#define uCandDown               2
#define uCandEnd                3
#define CandGBinfoLen           64
// the flag for an opened or start UI
#define IMN_PRIVATE_COMPOSITION_SIZE          0x0002
#define IMN_PRIVATE_UPDATE_QUICK_KEY          0x0004

#define MSG_ALREADY_OPEN                0x000001
#define MSG_ALREADY_OPEN2               0x000002
#define MSG_OPEN_CANDIDATE              0x000010
#define MSG_OPEN_CANDIDATE2             0x000020
#define MSG_CLOSE_CANDIDATE             0x000100
#define MSG_CLOSE_CANDIDATE2            0x000200
#define MSG_CHANGE_CANDIDATE            0x001000
#define MSG_CHANGE_CANDIDATE2           0x002000
#define MSG_ALREADY_START               0x010000
#define MSG_START_COMPOSITION           0x020000
#define MSG_END_COMPOSITION             0x040000
#define MSG_COMPOSITION                 0x080000
#define MSG_IMN_COMPOSITIONPOS          0x100000
#define MSG_IMN_UPDATE_STATUS           0x000400
#define MSG_IN_IMETOASCIIEX             0x800000
#define MSG_IMN_DESTROYCAND             0x004000
#define MSG_BACKSPACE                   0x000800
#define GEN_MSG_MAX             6


#define VK_OEM_SEMICLN                  0xba	//  ;    :
#define VK_OEM_EQUAL                    0xbb	//  =    +
#define VK_OEM_SLASH                    0xbf	//  /    ?
#define VK_OEM_LBRACKET                 0xdb	//  [    {
#define VK_OEM_BSLASH                   0xdc	//  \    |
#define VK_OEM_RBRACKET                 0xdd	//  ]    }
#define VK_OEM_QUOTE                    0xde	//  '    "

#define MAXMBNUMS                       40

// for ime property info
#define MAXNUMCODES                     48

#define LINE_LEN                80
#define CLASS_LEN               24

#define NumsSK                  13

#define NEAR_CARET_FIRST_TIME   0x0001
#define NEAR_CARET_CANDIDATE    0x0002

typedef DWORD UNALIGNED FAR *LPUNADWORD;
typedef WORD UNALIGNED FAR *LPUNAWORD;

typedef UINT u32;
typedef unsigned char u8;
typedef unsigned short u16;

typedef struct _tagImeG {
	int xChiCharWi;
	int yChiCharHi;

	int xStatusWi;				// width of status window
	int yStatusHi;				// high of status window
	RECT rcStatusText;			// text position relative to status window
	RECT rcImeIcon;				// ImeIcon position relative to status window
	RECT rcImeName;				// ImeName position relative to status window
	RECT rcSymbol;				// symbol relative to status window
	TCHAR szStatusErr[8];

	int iPara;
	int iPerp;
	int iParaTol;
	int iPerpTol;
} IMEG;


typedef IMEG *PIMEG;
typedef IMEG NEAR *NPIMEG;
typedef IMEG FAR *LPIMEG;


typedef struct tagNEARCARET {	// for near caret offset calculatation
	int iLogFontFacX;
	int iLogFontFacY;
	int iParaFacX;
	int iPerpFacX;
	int iParaFacY;
	int iPerpFacY;
} NEARCARET;

typedef NEARCARET *PNEARCARET;
typedef NEARCARET NEAR *NPNEARCARET;
typedef NEARCARET FAR *LPNEARCARET;

extern HINSTANCE hInst;
extern IMEG sImeG;
extern HDC ST_UI_hDC;
extern u32 uCaps;
extern TCHAR szUIClassName[];
extern TCHAR szCompClassName[];
extern TCHAR szStatusClassName[];
extern TCHAR szCMenuClassName[];
extern TCHAR szHandCursor[];
extern TCHAR szChinese[];
extern TCHAR *szImeName;
extern TCHAR szImeXGBName[];
extern TCHAR szSymbol[];
extern TCHAR szNoSymbol[];
extern TCHAR szEnglish[];
extern TCHAR szCode[];
extern TCHAR szEudc[];
extern TCHAR szNone[];
extern TCHAR szPerp[];
extern TCHAR szPara[];
extern TCHAR szPerpTol[];
extern TCHAR szParaTol[];
extern TCHAR szRegImeIndex[];
extern const NEARCARET ncUIEsc[], ncAltUIEsc[];
extern const POINT ptInputEsc[], ptAltInputEsc[];
extern BYTE VirtKey48Map[];


LRESULT CALLBACK UIWndProc(HWND, u32, WPARAM, LPARAM);	// ui.c
LRESULT PASCAL UIPaint(HWND);	// ui.c


class input_context;

void PASCAL SetCompPosition(input_context&);	// compui.c
void PASCAL MoveDefaultCompPosition(HWND);	// compui.c
void PASCAL StartComp(HWND);	// compui.c
LRESULT CALLBACK CompWndProc(HWND, u32, WPARAM, LPARAM);	// compui.c

void show_status_wnd();
void hide_status_wnd();

void show_comp_wnd();
void hide_comp_wnd();

void PASCAL OpenStatus(HWND);	// statusui.c
LRESULT CALLBACK StatusWndProc(HWND, u32, WPARAM, LPARAM);
void PASCAL InitStatusUIData();

// dialog procedure
const char* msg_name(u32 msg);
extern HWND g_hCompWnd, g_hStatusWnd;
#endif

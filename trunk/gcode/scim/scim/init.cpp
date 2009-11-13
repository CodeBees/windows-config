#include <windows.h>
#include <winerror.h>
#include <memory.h>
#include <immdev.h>
#include <imedefs.h>
#include <regstr.h>

int strbytelen(LPTSTR);

void PASCAL InitStatusUIData()
{

	int iContentHi;

	// iContentHi is to get the maximum value of predefined STATUS_DIM_Y and
    //int cxBorder, int cyBorder	// a real Chinese character's height in the current HDC.

	iContentHi = STATUS_DIM_Y;

	if (iContentHi < sImeG.yChiCharHi) {
		iContentHi = sImeG.yChiCharHi;
	}

	// right bottom of status
	sImeG.rcStatusText.left = 0;

	sImeG.rcStatusText.top = 0;

	sImeG.rcStatusText.right =
		sImeG.rcStatusText.left +
		strbytelen(szImeName) * sImeG.xChiCharWi / 2 +
		STATUS_NAME_MARGIN + STATUS_DIM_X * 4;

	sImeG.rcStatusText.bottom = sImeG.rcStatusText.top + iContentHi;

	sImeG.xStatusWi =
		STATUS_DIM_X * 2 + STATUS_NAME_MARGIN +
		strbytelen(szImeName) * sImeG.xChiCharWi / 2;

	sImeG.yStatusHi = iContentHi;

	// left bottom of imeicon bar
	sImeG.rcImeIcon.left = sImeG.rcStatusText.left;

	sImeG.rcImeIcon.top = sImeG.rcStatusText.top;

	sImeG.rcImeIcon.right = sImeG.rcImeIcon.left + STATUS_DIM_X;

	sImeG.rcImeIcon.bottom = sImeG.rcImeIcon.top + iContentHi;

	// left bottom of imename bar
	sImeG.rcImeName.left = sImeG.rcImeIcon.right;

	sImeG.rcImeName.top = sImeG.rcStatusText.top;

	sImeG.rcImeName.right =
		sImeG.rcImeName.left +
		strbytelen(szImeName) * sImeG.xChiCharWi / 2 + STATUS_NAME_MARGIN;

	sImeG.rcImeName.bottom = sImeG.rcImeName.top + iContentHi;

	sImeG.rcSymbol.left = sImeG.rcImeName.right;

	sImeG.rcSymbol.top = sImeG.rcStatusText.top;

	sImeG.rcSymbol.right = sImeG.rcSymbol.left + STATUS_DIM_X;

	sImeG.rcSymbol.bottom = sImeG.rcSymbol.top + iContentHi;

	return;

}

void PASCAL InitImeGlobalData(HINSTANCE hInstance)
{
	TCHAR szChiChar[4] = {0x9999, 0};

	SIZE lTextSize;

	hInst = hInstance;

	szUIClassName = L"bhj_ime_ui";
	szCompClassName = L"bhj_ime_cand";
	szStatusClassName = L"bhj_ime_status";

	HDC hDC = GetDC(NULL);

	if (!GetTextExtentPoint (hDC, (LPTSTR) szChiChar, 1, &lTextSize)) {
		memset(&lTextSize, 0, sizeof(SIZE));
	}
	ReleaseDC(NULL, hDC);

	sImeG.xChiCharWi = lTextSize.cx;
	sImeG.yChiCharHi = lTextSize.cy;


	InitStatusUIData();

	sImeG.iPara = 0;
	sImeG.iPerp = sImeG.yChiCharHi;

	sImeG.iParaTol = sImeG.xChiCharWi * 4;
	sImeG.iPerpTol = lTextSize.cy;

	return;

}

void PASCAL RegisterImeClass(HINSTANCE hInstance, HINSTANCE hInstL)
{

	WNDCLASSEX wcWndCls;

	// IME UI class
	// Register IME UI class
	wcWndCls.cbSize = sizeof(WNDCLASSEX);

	wcWndCls.cbClsExtra = 0;

	wcWndCls.cbWndExtra = sizeof(INT_PTR) * 2;

	wcWndCls.hIcon =
		LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
				  IMAGE_ICON, 32, 32, LR_DEFAULTCOLOR);

	wcWndCls.hInstance = hInstance;

	wcWndCls.hCursor = LoadCursor(NULL, IDC_ARROW);

	wcWndCls.hbrBackground = GetStockObject(NULL_BRUSH);

	wcWndCls.lpszMenuName = (LPTSTR) NULL;

	wcWndCls.hIconSm =
		LoadImage(hInstL, MAKEINTRESOURCE(IDI_IME),
				  IMAGE_ICON, 16, 16, LR_DEFAULTCOLOR);

	// IME UI class
	if (!GetClassInfoEx(hInstance, szUIClassName, &wcWndCls)) {

		wcWndCls.style = CS_IME;

		wcWndCls.lpfnWndProc = UIWndProc;

		wcWndCls.lpszClassName = (LPTSTR) szUIClassName;

		RegisterClassEx(&wcWndCls);

	}

	wcWndCls.style = CS_IME | CS_HREDRAW | CS_VREDRAW;

	wcWndCls.hbrBackground = GetStockObject(WHITE_BRUSH);

	// IME composition class
	// register IME composition class
	if (!GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {

		wcWndCls.lpfnWndProc = CompWndProc;

		wcWndCls.lpszClassName = (LPTSTR) szCompClassName;

		RegisterClassEx(&wcWndCls);

	}

	// IME status class
	// register IME status class
	if (!GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {

		wcWndCls.lpfnWndProc = StatusWndProc;

		wcWndCls.lpszClassName = (LPTSTR) szStatusClassName;

		RegisterClassEx(&wcWndCls);

	}

	return;

}


/**********************************************************************/
/* ImeDllInit()                                                       */
/* Return Value:                                                      */
/*      TRUE - successful                                             */
/*      FALSE - failure                                               */
/**********************************************************************/
BOOL CALLBACK DllMain(HINSTANCE hInstance,	// instance handle of this library
					  DWORD fdwReason,	// reason called
					  LPVOID lpvReserve)	// reserve pointer
{

	switch (fdwReason) {

	case DLL_PROCESS_ATTACH:
		szImeName = L"\x5305\x5305\x4e94\x7b14"; //包包五笔

		if (!hInst) {
			InitImeGlobalData(hInstance);
		}

		RegisterImeClass(hInstance, hInstance);

		break;

	case DLL_PROCESS_DETACH:
		{

			WNDCLASSEX wcWndCls;

			if (GetClassInfoEx(hInstance, szCMenuClassName, &wcWndCls)) {

				UnregisterClass(szCMenuClassName, hInstance);

			}

			if (GetClassInfoEx(hInstance, szStatusClassName, &wcWndCls)) {

				UnregisterClass(szStatusClassName, hInstance);

			}

			if (GetClassInfoEx(hInstance, szCompClassName, &wcWndCls)) {

				UnregisterClass(szCompClassName, hInstance);

			}

			if (GetClassInfoEx(hInstance, szUIClassName, &wcWndCls) && UnregisterClass(szUIClassName, hInstance)) {

				DestroyIcon(wcWndCls.hIcon);
				DestroyIcon(wcWndCls.hIconSm);

			}

		}

		break;

	default:

		break;

	}

	return (TRUE);

}

int strbytelen(LPTSTR lpStr) //used for calculating text size
{

	int i, len, iRet;

	len = lstrlen(lpStr);

	for (i = 0, iRet = 0; i < len; i++, iRet++) {

		if (lpStr[i] > 0x100)

			iRet++;

	}

	return iRet;

}

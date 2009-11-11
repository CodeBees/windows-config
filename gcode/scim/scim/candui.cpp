/*++

Copyright (c) 1990-1999 Microsoft Corporation, All Rights Reserved

Module Name:

    candui.c

++*/

#include <windows.h>
#include <immdev.h>
#include <imedefs.h>
#define ENABLE_BHJDEBUG
#include "bhjdebug.h"

HWND PASCAL GetCandWnd(HWND hUIWnd)	// UI window
{
	return (hCandWnd);
}

void PASCAL CalcCandPos(HIMC hIMC, HWND hUIWnd, LPPOINT lpptWnd)	// the composition window position
{
	POINT ptNew;
	RECT rcWorkArea;

	rcWorkArea = sImeG.rcWorkArea;


	ptNew.x = lpptWnd->x + lpImeL->xCompWi + UI_MARGIN;
	if (ptNew.x + sImeG.xCandWi > rcWorkArea.right) {
		// exceed screen width
		ptNew.x = lpptWnd->x - sImeG.xCandWi - UI_MARGIN;
	}

	ptNew.y = lpptWnd->y + lpImeL->cyCompBorder - sImeG.cyCandBorder;
	if (ptNew.y + sImeG.yCandHi > rcWorkArea.bottom) {
		// exceed screen high
		ptNew.y = rcWorkArea.bottom - sImeG.yCandHi;
	}

	lpptWnd->x = ptNew.x;
	lpptWnd->y = ptNew.y;

	return;
}

void AdjustCandPos(HIMC hIMC, LPPOINT lpptWnd)	// the composition window position
{
	LPINPUTCONTEXT lpIMC;
	LONG ptFontHi;
	UINT uEsc;
	RECT rcWorkArea;

	rcWorkArea = sImeG.rcWorkArea;

	lpIMC = (LPINPUTCONTEXT) ImmLockIMC(hIMC);
	if (!lpIMC) {
		return;
	}

	if (lpIMC->lfFont.A.lfHeight > 0) {
		ptFontHi = lpIMC->lfFont.A.lfHeight;
	} else if (lpIMC->lfFont.A.lfWidth == 0) {
		ptFontHi = lpImeL->yCompHi;
	} else {
		ptFontHi = -lpIMC->lfFont.A.lfHeight;
	}

	if (ptFontHi > lpImeL->yCompHi * 8) {
		ptFontHi = lpImeL->yCompHi * 8;
	}

	if (ptFontHi < sImeG.yChiCharHi) {
		ptFontHi = sImeG.yChiCharHi;
	}
	// -450 to 450 index 0
	// 450 to 1350 index 1
	// 1350 to 2250 index 2
	// 2250 to 3150 index 3
	uEsc = (UINT) ((lpIMC->lfFont.A.lfEscapement + 450) / 900 % 4);

	// find the location after IME do an adjustment
	ptFontHi = ptFontHi * ptInputEsc[uEsc].y;

	if (lpptWnd->y + ptFontHi + sImeG.yCandHi <= rcWorkArea.bottom) {
		lpptWnd->y += ptFontHi;
	} else {
		lpptWnd->y -= (ptFontHi + sImeG.yCandHi);
	}

	ImmUnlockIMC(hIMC);
	return;
}

void PASCAL AdjustCandRectBoundry(LPINPUTCONTEXT lpIMC, LPPOINT lpptCandWnd)	// the caret position
{
	RECT rcExclude, rcUIRect, rcInterSect;
	UINT uEsc;
	RECT rcWorkArea;

	rcWorkArea = sImeG.rcWorkArea;

	// be a normal rectangle, not a negative rectangle
	if (lpIMC->cfCandForm[0].rcArea.left >
		lpIMC->cfCandForm[0].rcArea.right) {
		LONG tmp;

		tmp = lpIMC->cfCandForm[0].rcArea.left;
		lpIMC->cfCandForm[0].rcArea.left =
			lpIMC->cfCandForm[0].rcArea.right;
		lpIMC->cfCandForm[0].rcArea.right = tmp;
	}

	if (lpIMC->cfCandForm[0].rcArea.top >
		lpIMC->cfCandForm[0].rcArea.bottom) {
		LONG tmp;

		tmp = lpIMC->cfCandForm[0].rcArea.top;
		lpIMC->cfCandForm[0].rcArea.top =
			lpIMC->cfCandForm[0].rcArea.bottom;
		lpIMC->cfCandForm[0].rcArea.bottom = tmp;
	}
	// translate from client coordinate to screen coordinate
	rcExclude = lpIMC->cfCandForm[0].rcArea;

	rcExclude.left += lpptCandWnd->x - lpIMC->cfCandForm[0].ptCurrentPos.x;
	rcExclude.right +=
		lpptCandWnd->x - lpIMC->cfCandForm[0].ptCurrentPos.x;

	rcExclude.top += lpptCandWnd->y - lpIMC->cfCandForm[0].ptCurrentPos.y;
	rcExclude.bottom +=
		lpptCandWnd->y - lpIMC->cfCandForm[0].ptCurrentPos.y;

	// if original point is OK, we use it
	*(LPPOINT) & rcUIRect = *lpptCandWnd;

	if (rcUIRect.left < rcWorkArea.left) {
		rcUIRect.left = rcWorkArea.left;
	} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
		rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
	} else {
	}

	if (rcUIRect.top < rcWorkArea.top) {
		rcUIRect.top = rcWorkArea.top;
	} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
		rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
	} else {
	}

	rcUIRect.right = rcUIRect.left + sImeG.xCandWi;
	rcUIRect.bottom = rcUIRect.top + sImeG.yCandHi;

	if (!IntersectRect(&rcInterSect, &rcExclude, &rcUIRect)) {
		*lpptCandWnd = *(LPPOINT) & rcUIRect;
		return;
	}

	uEsc = (UINT) ((lpIMC->lfFont.A.lfEscapement + 450) / 900 % 4);

	if (uEsc & 0x0001) {
		// 900 & 2700 we need change x coordinate
		if (ptInputEsc[uEsc].x > 0) {
			rcUIRect.left = rcExclude.right;
		} else {
			rcUIRect.left = rcExclude.left - sImeG.xCandWi;
		}
	} else {
		// 0 & 1800 we do not change x coordinate
		rcUIRect.left = lpptCandWnd->x;
	}

	if (rcUIRect.left < rcWorkArea.left) {
		rcUIRect.left = rcWorkArea.left;
	} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
		rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
	} else {
	}

	if (uEsc & 0x0001) {
		// 900 & 2700 we do not change y coordinate
		rcUIRect.top = lpptCandWnd->y;
	} else {
		// 0 & 1800 we need change y coordinate
		if (ptInputEsc[uEsc].y > 0) {
			rcUIRect.top = rcExclude.bottom;
		} else {
			rcUIRect.top = rcExclude.top - sImeG.yCandHi;
		}
	}

	if (rcUIRect.top < rcWorkArea.top) {
		rcUIRect.top = rcWorkArea.top;
	} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
		rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
	} else {
	}

	rcUIRect.right = rcUIRect.left + sImeG.xCandWi;
	rcUIRect.bottom = rcUIRect.top + sImeG.yCandHi;

	// the candidate window not overlapped with exclude rectangle
	// so we found a position
	if (!IntersectRect(&rcInterSect, &rcExclude, &rcUIRect)) {
		*lpptCandWnd = *(LPPOINT) & rcUIRect;
		return;
	}
	// adjust according to
	*(LPPOINT) & rcUIRect = *lpptCandWnd;

	if (uEsc & 0x0001) {
		// 900 & 2700 we prefer adjust x
		if (ptInputEsc[uEsc].x > 0) {
			rcUIRect.left = rcExclude.right;
		} else {
			rcUIRect.left = rcExclude.left - sImeG.xCandWi;
		}

		if (rcUIRect.left < rcWorkArea.left) {
		} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
		} else {
			if (rcUIRect.top < rcWorkArea.top) {
				rcUIRect.top = rcWorkArea.top;
			} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
				rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
			}

			*lpptCandWnd = *(LPPOINT) & rcUIRect;
			return;
		}

		// negative try
		if (ptInputEsc[uEsc].x > 0) {
			rcUIRect.left = rcExclude.left - sImeG.xCandWi;
		} else {
			rcUIRect.left = rcExclude.right;
		}

		if (rcUIRect.left < rcWorkArea.left) {
		} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
		} else {
			if (rcUIRect.top < rcWorkArea.top) {
				rcUIRect.top = rcWorkArea.top;
			} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
				rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
			}

			*lpptCandWnd = *(LPPOINT) & rcUIRect;
			return;
		}

		// negative try failure again, we use positive plus display adjust
		if (ptInputEsc[uEsc].x > 0) {
			rcUIRect.left = rcExclude.right;
		} else {
			rcUIRect.left = rcExclude.left - sImeG.xCandWi;
		}

		if (rcUIRect.left < rcWorkArea.left) {
			rcUIRect.left = rcWorkArea.left;
		} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
			rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
		}

		if (rcUIRect.top < rcWorkArea.top) {
			rcUIRect.top = rcWorkArea.top;
		} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
			rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
		}

		*lpptCandWnd = *(LPPOINT) & rcUIRect;
	} else {
		// 0 & 1800 we prefer adjust y
		if (ptInputEsc[uEsc].y > 0) {
			rcUIRect.top = rcExclude.bottom;
		} else {
			rcUIRect.top = rcExclude.top - sImeG.yCandHi;
		}

		if (rcUIRect.top < rcWorkArea.top) {
		} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
		} else {
			if (rcUIRect.left < rcWorkArea.left) {
				rcUIRect.left = rcWorkArea.left;
			} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
				rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
			}

			*lpptCandWnd = *(LPPOINT) & rcUIRect;
			return;
		}

		// negative try
		if (ptInputEsc[uEsc].y > 0) {
			rcUIRect.top = rcExclude.top - sImeG.yCandHi;
		} else {
			rcUIRect.top = rcExclude.bottom;
		}

		if (rcUIRect.top < rcWorkArea.top) {
		} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.right) {
		} else {
			if (rcUIRect.left < rcWorkArea.left) {
				rcUIRect.left = rcWorkArea.left;
			} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
				rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
			}

			*lpptCandWnd = *(LPPOINT) & rcUIRect;
			return;
		}

		// negative try failure again, we use positive plus display adjust
		if (ptInputEsc[uEsc].y > 0) {
			rcUIRect.top = rcExclude.bottom;
		} else {
			rcUIRect.top = rcExclude.top - sImeG.yCandHi;
		}

		if (rcUIRect.left < rcWorkArea.left) {
			rcUIRect.left = rcWorkArea.left;
		} else if (rcUIRect.left + sImeG.xCandWi > rcWorkArea.right) {
			rcUIRect.left = rcWorkArea.right - sImeG.xCandWi;
		}

		if (rcUIRect.top < rcWorkArea.top) {
			rcUIRect.top = rcWorkArea.top;
		} else if (rcUIRect.top + sImeG.yCandHi > rcWorkArea.bottom) {
			rcUIRect.top = rcWorkArea.bottom - sImeG.yCandHi;
		}

		*lpptCandWnd = *(LPPOINT) & rcUIRect;
	}

	return;
}

void PASCAL AdjustCandBoundry(LPPOINT lpptCandWnd)	// the position
{

	RECT rcWorkArea;

	rcWorkArea = sImeG.rcWorkArea;
	if (lpptCandWnd->x < rcWorkArea.left) {
		lpptCandWnd->x = rcWorkArea.left;
	} else if (lpptCandWnd->x + sImeG.xCandWi > rcWorkArea.right) {
		lpptCandWnd->x = rcWorkArea.right - sImeG.xCandWi;
	}

	if (lpptCandWnd->y < rcWorkArea.top) {
		lpptCandWnd->y = rcWorkArea.top;
	} else if (lpptCandWnd->y + sImeG.yCandHi > rcWorkArea.bottom) {
		lpptCandWnd->y = rcWorkArea.bottom - sImeG.yCandHi;
	}

	return;
}


void PASCAL ShowCand(HWND hUIWnd, int nShowCandCmd)
{
	BHJDEBUG(" ShowCand %d", nShowCandCmd);
	ShowWindow(hCandWnd, nShowCandCmd);
	return;
}

void PASCAL OpenCand(HWND hUIWnd)
{
	BHJDEBUG(" ");
	HIMC hIMC;
	LPINPUTCONTEXT lpIMC;
	POINT ptWnd;

	hIMC = (HIMC) GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
	if (!hIMC) {
		goto OpenCandUnlockUIPriv;
	}

	lpIMC = (LPINPUTCONTEXT) ImmLockIMC(hIMC);
	if (!lpIMC) {
		goto OpenCandUnlockUIPriv;
	}

	if (lpIMC->cfCandForm[0].dwIndex == 0) {

		ptWnd = lpIMC->cfCandForm[0].ptCurrentPos;

		ClientToScreen(lpIMC->hWnd, &ptWnd);

		if (lpIMC->cfCandForm[0].dwStyle & CFS_FORCE_POSITION) {
		} else if (lpIMC->cfCandForm[0].dwStyle == CFS_EXCLUDE) {
			POINT ptCaret;

			AdjustCandBoundry(&ptWnd);
			if ((!GetCaretPos(&ptCaret))) {

				if (GetCompWnd(hUIWnd)) {
					ptWnd.x = ptWnd.y = 0;
					ClientToScreen(lpIMC->hWnd, &ptWnd);
					ptWnd.x -= lpImeL->cxCompBorder + 1;
					ptWnd.y -= lpImeL->cyCompBorder + 1;
				} else {
					ptWnd.x = lpImeL->cxCompBorder + 1;
					ptWnd.y = lpImeL->cyCompBorder + 1;
				}

				CalcCandPos(hIMC, hUIWnd, &ptWnd);

				lpIMC->cfCandForm[0].dwStyle |= CFS_CANDIDATEPOS;
				lpIMC->cfCandForm[0].ptCurrentPos = ptWnd;
				ScreenToClient(lpIMC->hWnd,
							   &lpIMC->cfCandForm[0].ptCurrentPos);
			} else {
				AdjustCandPos(hIMC, &ptWnd);
			}
		} else if (lpIMC->cfCandForm[0].dwStyle == CFS_CANDIDATEPOS) {
			AdjustCandBoundry(&ptWnd);
		} else {
			if (1) {//FIXME
				ptWnd.x = ptWnd.y = 0;
				ClientToScreen(hCompWnd, &ptWnd);
			} else {
				ptWnd = lpIMC->cfCompForm.ptCurrentPos;
				ClientToScreen(lpIMC->hWnd, &ptWnd);
			}

			ptWnd.x -= lpImeL->cxCompBorder + 1;
			ptWnd.y -= lpImeL->cyCompBorder + 1;

			CalcCandPos(hIMC, hUIWnd, &ptWnd);

			lpIMC->cfCandForm[0].dwStyle |= CFS_CANDIDATEPOS;
			lpIMC->cfCandForm[0].ptCurrentPos = ptWnd;
			ScreenToClient(lpIMC->hWnd,
						   &lpIMC->cfCandForm[0].ptCurrentPos);
		}
	} else {
		// make cand windows trace comp window !
		if (1) { //FIXME
			ptWnd.x = ptWnd.y = 0;
			ClientToScreen(hCompWnd, &ptWnd);
		} else {
			ptWnd = lpIMC->cfCompForm.ptCurrentPos;
			ClientToScreen(lpIMC->hWnd, &ptWnd);
		}

		ptWnd.x -= lpImeL->cxCompBorder + 1;
		ptWnd.y -= lpImeL->cyCompBorder + 1;

		CalcCandPos(hIMC, hUIWnd, &ptWnd);

		lpIMC->cfCandForm[0].dwStyle |= CFS_CANDIDATEPOS;
		lpIMC->cfCandForm[0].ptCurrentPos = ptWnd;
		ScreenToClient(lpIMC->hWnd, &lpIMC->cfCandForm[0].ptCurrentPos);
	}

	ImmUnlockIMC(hIMC);

	if (hCandWnd) {
		SetWindowPos(hCandWnd,
					 NULL,
					 ptWnd.x,
					 ptWnd.y,
					 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	} else {
		hCandWnd =
			CreateWindowEx(WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
						   szCandClassName, NULL, WS_POPUP | WS_DISABLED,
						   ptWnd.x, ptWnd.y, sImeG.xCandWi, sImeG.yCandHi,
						   hUIWnd, (HMENU) NULL, hInst, NULL);

		SetWindowLong(hCandWnd, UI_MOVE_OFFSET,
					  WINDOW_NOT_DRAG);
		SetWindowLong(hCandWnd, UI_MOVE_XY, 0L);
	}

	ShowCand(hUIWnd, SW_SHOWNOACTIVATE);

  OpenCandUnlockUIPriv:
	return;
}

void PASCAL CloseCand(HWND hUIWnd)
{
	BHJDEBUG(" ");
	ShowCand(hUIWnd, SW_HIDE);
	return;
}

void PASCAL DestroyCandWindow(HWND hCandWnd)
{
	hCandWnd = (HWND) NULL;
	return;
}


void PASCAL PaintCandWindow(HWND hCandWnd, HDC hDC)
{
	HIMC hIMC;
	LPINPUTCONTEXT lpIMC;
	LPCANDIDATEINFO lpCandInfo;
	LPCANDIDATELIST lpCandList;
	LPPRIVCONTEXT imcPrivPtr;
	HGDIOBJ hOldFont;
	DWORD dwStart, dwEnd;
	TCHAR szStrBuf[2 * MAXSTRLEN * sizeof(WCHAR) / sizeof(TCHAR) + 1];
	int i;
	HBITMAP hCandIconBmp, hCandInfBmp;
	HBITMAP hOldBmp, hCandHBmp, hCandUBmp, hCandDBmp, hCandEBmp;
	HDC hMemDC;

	hIMC =
		(HIMC) GetWindowLongPtr(GetWindow(hCandWnd, GW_OWNER),
								IMMGWLP_IMC);
	if (!hIMC) {
		return;
	}

	lpIMC = (LPINPUTCONTEXT) ImmLockIMC(hIMC);
	if (!lpIMC) {
		return;
	}

	if (!lpIMC->hCandInfo) {
		goto UpCandW2UnlockIMC;
	}

	lpCandInfo = (LPCANDIDATEINFO) ImmLockIMCC(lpIMC->hCandInfo);
	if (!lpCandInfo) {
		goto UpCandW2UnlockIMC;
	}

	if (!lpIMC->hPrivate) {
		goto UpCandW2UnlockCandInfo;
	}

	imcPrivPtr = (LPPRIVCONTEXT) ImmLockIMCC(lpIMC->hPrivate);
	if (!imcPrivPtr) {
		goto UpCandW2UnlockCandInfo;
	}
	// set font
	if (sImeG.fDiffSysCharSet) {
		LOGFONT lfFont;
		ZeroMemory(&lfFont, sizeof(lfFont));
		hOldFont = GetCurrentObject(hDC, OBJ_FONT);
		lfFont.lfHeight = -MulDiv(12, GetDeviceCaps(hDC, LOGPIXELSY), 72);
		lfFont.lfCharSet = NATIVE_CHARSET;
		lstrcpy(lfFont.lfFaceName, TEXT("Simsun"));
		SelectObject(hDC, CreateFontIndirect(&lfFont));
	}

	lpCandList = (LPCANDIDATELIST) ((LPBYTE) lpCandInfo +
									lpCandInfo->dwOffset[0]);

	dwStart = lpCandList->dwSelection;
	dwEnd = dwStart + lpCandList->dwPageSize;

	if (dwEnd > lpCandList->dwCount) {
		dwEnd = lpCandList->dwCount;
	}
	// draw CandWnd Layout

	RECT rcWnd;

	GetClientRect(hCandWnd, &rcWnd);

	SetTextColor(hDC, RGB(0x00, 0x00, 0x00));
	SetBkColor(hDC, RGB(0xC0, 0xC0, 0xC0));


	ExtTextOut(hDC, sImeG.rcCandText.left, sImeG.rcCandText.top,
			   ETO_OPAQUE, &sImeG.rcCandText, NULL, 0, NULL);
	szStrBuf[0] = TEXT('1');
	szStrBuf[1] = TEXT(':');

	for (i = 0; dwStart < dwEnd; dwStart++, i++) {
		int iLen;

		iLen = 0;

		szStrBuf[0] = szDigit[i + CAND_START];


		WORD wCode;
		wCode =
			*(LPUNAWORD) ((LPBYTE) lpCandList +
						  lpCandList->dwOffset[dwStart]);

		szStrBuf[2] = wCode;
		szStrBuf[3] = TEXT('\0');
		iLen = 2 / sizeof(TCHAR);



		ExtTextOut(hDC, sImeG.rcCandText.left,
				   sImeG.rcCandText.top + i * sImeG.yChiCharHi,
				   (UINT) 0, NULL, szStrBuf, iLen + 2, NULL);

		// QW/GB info
		{

			int iMyLen;
			WORD wCode, wGB;
			TCHAR AbSeq[5];
			TCHAR GbSeq[5];
			TCHAR szMyStrBuf[12 * sizeof(WCHAR) / sizeof(TCHAR)];
			RECT GBARInfo;



			wCode =
				*(LPUNAWORD) ((LPBYTE) lpCandList +
							  lpCandList->dwOffset[dwStart]);
			AbSeq[0] = wCode;
			AbSeq[1] = TEXT('\0');
			//  change the CP_ACP to 936, so that it can work under Multilingul Env.
			WideCharToMultiByte(NATIVE_ANSI_CP, WC_COMPOSITECHECK, AbSeq,
								1, (LPSTR) GbSeq, sizeof(GbSeq), NULL,
								NULL);

			wGB = HIBYTE(GbSeq[0]) | (LOBYTE(GbSeq[0]) << 8);

			wsprintf(GbSeq, TEXT("%04lx"), wGB);	// get GB string
			wGB -= 0xa0a0;
			wsprintf(AbSeq, TEXT("%02d%02d"), HIBYTE(wGB), LOBYTE(wGB));

			lstrcpy(szMyStrBuf, TEXT("("));
			lstrcat(szMyStrBuf, GbSeq);
			lstrcat(szMyStrBuf, TEXT(", "));
			wsprintf(AbSeq, TEXT("%04lx"), wCode);
			lstrcat(szMyStrBuf, AbSeq);
			lstrcat(szMyStrBuf, TEXT(")"));
			iMyLen = lstrlen(szMyStrBuf);


			GBARInfo.top = sImeG.rcCandText.top + i * sImeG.yChiCharHi;
			GBARInfo.left = sImeG.rcCandText.left;
			GBARInfo.right = sImeG.rcCandText.right;
			GBARInfo.bottom = sImeG.rcCandText.bottom;
			DrawText(hDC, szMyStrBuf, lstrlen(szMyStrBuf),
					 &GBARInfo, DT_RIGHT | DT_SINGLELINE);
		}
	}



	// load all bitmap
	hCandInfBmp = LoadBitmap(hInst, TEXT("Candinf"));

	hMemDC = CreateCompatibleDC(hDC);

	if (hMemDC != NULL) {

		hCandIconBmp = LoadBitmap(hInst, TEXT("CandSel"));

		if (hCandIconBmp) {

			hOldBmp = SelectObject(hMemDC, hCandIconBmp);

			BitBlt(hDC, sImeG.rcCandIcon.left, sImeG.rcCandIcon.top,
				   sImeG.rcCandIcon.right - sImeG.rcCandIcon.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);

			DeleteObject(hCandIconBmp);
		}

		if (hCandInfBmp) {

			hOldBmp = SelectObject(hMemDC, hCandInfBmp);

			BitBlt(hDC, sImeG.rcCandInf.left, sImeG.rcCandInf.top,
				   sImeG.rcCandInf.right - sImeG.rcCandInf.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);
		}

		hCandHBmp = LoadBitmap(hInst, TEXT("CandH"));

		if (hCandHBmp) {
			hOldBmp = SelectObject(hMemDC, hCandHBmp);

			BitBlt(hDC, sImeG.rcCandBTH.left, sImeG.rcCandBTH.top,
				   sImeG.rcCandBTH.right - sImeG.rcCandBTH.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);

			DeleteObject(hCandHBmp);
		}


		hCandUBmp = LoadBitmap(hInst, TEXT("CandU"));

		if (hCandUBmp) {
			hOldBmp = SelectObject(hMemDC, hCandUBmp);

			BitBlt(hDC, sImeG.rcCandBTU.left, sImeG.rcCandBTU.top,
				   sImeG.rcCandBTU.right - sImeG.rcCandBTU.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);

			DeleteObject(hCandUBmp);
		}


		hCandDBmp = LoadBitmap(hInst, TEXT("CandD"));

		if (hCandDBmp) {

			hOldBmp = SelectObject(hMemDC, hCandDBmp);

			BitBlt(hDC, sImeG.rcCandBTD.left, sImeG.rcCandBTD.top,
				   sImeG.rcCandBTD.right - sImeG.rcCandBTD.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);

			DeleteObject(hCandDBmp);
		}

		hCandEBmp = LoadBitmap(hInst, TEXT("CandE"));

		if (hCandEBmp) {
			hOldBmp = SelectObject(hMemDC, hCandEBmp);

			BitBlt(hDC, sImeG.rcCandBTE.left, sImeG.rcCandBTE.top,
				   sImeG.rcCandBTE.right - sImeG.rcCandBTE.left,
				   STATUS_DIM_Y, hMemDC, 0, 0, SRCCOPY);

			SelectObject(hMemDC, hOldBmp);

			DeleteObject(hCandEBmp);
		}

		DeleteDC(hMemDC);

	}

	if (hCandInfBmp)
		DeleteObject(hCandInfBmp);

	if (sImeG.fDiffSysCharSet) {
		DeleteObject(SelectObject(hDC, hOldFont));
	}

	ImmUnlockIMCC(lpIMC->hPrivate);
  UpCandW2UnlockCandInfo:
	ImmUnlockIMCC(lpIMC->hCandInfo);
  UpCandW2UnlockIMC:
	ImmUnlockIMC(hIMC);

	return;
}

LRESULT CALLBACK
CandWndProc(HWND hCandWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	BHJDEBUG("received msg %s", msg_name(uMsg));
	switch (uMsg) {
	case WM_DESTROY:
		DestroyCandWindow(hCandWnd);
		break;
	case WM_IME_NOTIFY:
		break;
	case WM_PAINT:
		{
			HDC hDC;
			PAINTSTRUCT ps;

			hDC = BeginPaint(hCandWnd, &ps);
			PaintCandWindow(hCandWnd, hDC);
			EndPaint(hCandWnd, &ps);
		}
		break;
	case WM_MOUSEACTIVATE:
		return (MA_NOACTIVATE);
	default:
		BHJDEBUG(" msg %s not handled", msg_name(uMsg));
		return DefWindowProc(hCandWnd, uMsg, wParam, lParam);
	}

	return (0L);
}

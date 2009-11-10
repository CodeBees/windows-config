
/*++

Copyright (c) 1990-1999 Microsoft Corporation, All Rights Reserved

Module Name:

    ui.c


++*/


#include <windows.h>
#include <immdev.h>
#include <imedefs.h>
#include <regstr.h>



/**********************************************************************/
/* CMenuDestryed()                                                    */
/**********************************************************************/
void PASCAL CMenuDestroyed(             // context menu window
                                        // already destroyed
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

    lpUIPrivate->hCMenuWnd = NULL;

    GlobalUnlock(hUIPrivate);
}

void PASCAL CreateUIWindow(             // create composition window
    HWND hUIWnd)
{
    HGLOBAL hUIPrivate;

    // create storage for UI setting
    hUIPrivate = GlobalAlloc(GHND, sizeof(UIPRIV));
    if (!hUIPrivate) {     
        return;
    }

    SetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE, (LONG_PTR)hUIPrivate);

    // set the default position for UI window, it is hide now
    SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOACTIVATE|SWP_NOZORDER);

    ShowWindow(hUIWnd, SW_SHOWNOACTIVATE);

    return;
}

void PASCAL DestroyUIWindow(            // destroy composition window
    HWND hUIWnd)
{
    HGLOBAL  hUIPrivate;
    LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {     
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {    
        return;
    }

    //destroy ContextMenuWnd
    if (lpUIPrivate->hCMenuWnd) {
        SetWindowLongPtr(lpUIPrivate->hCMenuWnd, CMENU_HUIWND,(LONG_PTR)0);
        PostMessage(lpUIPrivate->hCMenuWnd, WM_USER_DESTROY, 0, 0);
    }


    // composition window need to be destroyed
    if (lpUIPrivate->hCompWnd) {
        DestroyWindow(lpUIPrivate->hCompWnd);
    }

    // candidate window need to be destroyed
    if (lpUIPrivate->hCandWnd) {
        DestroyWindow(lpUIPrivate->hCandWnd);
    }

    // status window need to be destroyed
    if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    }


    GlobalUnlock(hUIPrivate);

    // free storage for UI settings
    GlobalFree(hUIPrivate);

    return;
}

/**********************************************************************/
/* StatusWndMsg()                                                     */
/**********************************************************************/
void PASCAL StatusWndMsg(       // set the show hide state and
    HWND        hUIWnd,
    BOOL        fOn)
{
    HGLOBAL  hUIPrivate;
    HIMC     hIMC;
    HWND     hStatusWnd;

    register LPUIPRIV lpUIPrivate;

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return;
    }

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        GlobalUnlock(hUIPrivate);
    return;
    }

    if (fOn) {
        lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;

        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(
                hUIWnd);
        }
    } else {
        lpUIPrivate->fdwSetContext &= ~(ISC_OPEN_STATUS_WINDOW);
    }

    hStatusWnd = lpUIPrivate->hStatusWnd;

    GlobalUnlock(hUIPrivate);

    if (!hStatusWnd) {
        return;
    }

    if (!fOn) {
        register DWORD fdwSetContext;

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
                hUIWnd, SW_HIDE);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
                hUIWnd, SW_HIDE);
        }

        ShowStatus(
            hUIWnd, SW_HIDE);
    } else if (hIMC) {
        ShowStatus(
            hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowStatus(
            hUIWnd, SW_HIDE);
    }

    return;
}

/**********************************************************************/
/* ShowUI()                                                           */
/**********************************************************************/
void PASCAL ShowUI(             // show the sub windows
    HWND   hUIWnd,
    int    nShowCmd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HGLOBAL        hUIPrivate;
    LPUIPRIV       lpUIPrivate;

    if (nShowCmd == SW_HIDE) {
    } else if (!(hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC))) {
        nShowCmd = SW_HIDE;
    } else if (!(lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC))) {
        nShowCmd = SW_HIDE;
    } else if (!(lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate))) {
        ImmUnlockIMC(hIMC);
        nShowCmd = SW_HIDE;
    } else {
    }

    if (nShowCmd == SW_HIDE) {
        ShowStatus(hUIWnd, nShowCmd);
        ShowComp(hUIWnd, nShowCmd);
        ShowCand(hUIWnd, nShowCmd);
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {          // can not darw status window
        goto ShowUIUnlockIMCC;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {         // can not draw status window
        goto ShowUIUnlockIMCC;
    }

    lpUIPrivate->fdwSetContext |= ISC_SHOWUICOMPOSITIONWINDOW;

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICOMPOSITIONWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_START)) {
        if (lpUIPrivate->hCompWnd) {

            if (lpUIPrivate->nShowCompCmd != SW_HIDE) {
                // some time the WM_NCPAINT is eaten by the app
                RedrawWindow(lpUIPrivate->hCompWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

			SendMessage(lpUIPrivate->hCompWnd, WM_IME_NOTIFY,
						IMN_SETCOMPOSITIONWINDOW, 0);

            if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
                ShowComp(hUIWnd, nShowCmd);
            }
        } else {
            StartComp(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCompCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_COMP_WINDOW;
    } else {
        ShowComp(hUIWnd, SW_HIDE);
    }

    if ((lpUIPrivate->fdwSetContext & ISC_SHOWUICANDIDATEWINDOW) &&
        (lpImcP->fdwImeMsg & MSG_ALREADY_OPEN)) {
        if (lpUIPrivate->hCandWnd) {
            if (lpUIPrivate->nShowCandCmd != SW_HIDE) {
                // some time the WM_NCPAINT is eaten by the app
                RedrawWindow(lpUIPrivate->hCandWnd, NULL, NULL,
                    RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
            }

			SendMessage(lpUIPrivate->hCandWnd, WM_IME_NOTIFY,
						IMN_SETCANDIDATEPOS, 0x0001);

            if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
                ShowCand(hUIWnd, nShowCmd);
            }
        } else {
            OpenCand(hUIWnd);
        }
    } else if (lpUIPrivate->nShowCandCmd == SW_HIDE) {
    } else if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        lpUIPrivate->fdwSetContext |= ISC_HIDE_CAND_WINDOW;
    } else {
        ShowCand(hUIWnd, SW_HIDE);
    }

    if (lpUIPrivate->fdwSetContext & ISC_OPEN_STATUS_WINDOW) {
        if (!lpUIPrivate->hStatusWnd) {
            OpenStatus(hUIWnd);
        }
        if (lpUIPrivate->nShowStatusCmd != SW_HIDE) {
            // some time the WM_NCPAINT is eaten by the app
            RedrawWindow(lpUIPrivate->hStatusWnd, NULL, NULL,
                RDW_FRAME|RDW_INVALIDATE|RDW_ERASE);
        }

        SendMessage(lpUIPrivate->hStatusWnd, WM_IME_NOTIFY,
            IMN_SETSTATUSWINDOWPOS, 0);
        if (lpUIPrivate->nShowStatusCmd == SW_HIDE) {
            ShowStatus(hUIWnd, nShowCmd);
        }
        else{
            ShowStatus(hUIWnd, nShowCmd);
        }
    } else if (lpUIPrivate->hStatusWnd) {
        DestroyWindow(lpUIPrivate->hStatusWnd);
    } 

    // we switch to this hIMC
    lpUIPrivate->hIMC = hIMC;

    GlobalUnlock(hUIPrivate);

ShowUIUnlockIMCC:
    ImmUnlockIMCC(lpIMC->hPrivate);
    ImmUnlockIMC(hIMC);

    return;
}

/**********************************************************************/
/* ShowGuideLine                                                      */
/**********************************************************************/
void PASCAL ShowGuideLine(
    HWND hUIWnd)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPGUIDELINE    lpGuideLine;

    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
    if (!hIMC) {
        return;
    }

    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);

    if (!lpGuideLine) {
    } else if (lpGuideLine->dwLevel == GL_LEVEL_ERROR) {
        MessageBeep((UINT)-1);
        MessageBeep((UINT)-1);
    } else if (lpGuideLine->dwLevel == GL_LEVEL_WARNING) {
        MessageBeep((UINT)-1);
    } else {
    }

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMC(hIMC);

    return;
}

/**********************************************************************/
/* UpdateStatusWindow()                                               */
/* Return Value:                                                      */
/*     none                                                             */
/**********************************************************************/
BOOL UpdateStatusWindow(
    HWND   hUIWnd)
{
    HWND           hStatusWnd;

    if (!(hStatusWnd = GetStatusWnd(hUIWnd))) {
        return (FALSE);
    }

    InvalidateRect(hStatusWnd, &(sImeG.rcStatusText), TRUE);
    UpdateWindow(hStatusWnd);

    return (TRUE);
}


/**********************************************************************/
/* NotifyUI()                                                         */
/**********************************************************************/
void PASCAL NotifyUI(
    HWND        hUIWnd,
    WPARAM      wParam,
    LPARAM      lParam)
{
    HWND hStatusWnd;

    switch (wParam) {
    case IMN_OPENSTATUSWINDOW:
        //PostStatus(hUIWnd, TRUE);
        StatusWndMsg(hUIWnd, TRUE);
        break;
    case IMN_CLOSESTATUSWINDOW:
        //PostStatus(hUIWnd, FALSE);
        StatusWndMsg(hUIWnd, FALSE);
        break;
    case IMN_OPENCANDIDATE:
        if (lParam & 0x00000001) {
            OpenCand(hUIWnd);
        }
        break;
    case IMN_CHANGECANDIDATE:
        if (lParam & 0x00000001) {
            HDC  hDC;
            HWND hCandWnd;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            hDC = GetDC(hCandWnd);
            PaintCandWindow(hCandWnd, hDC);
            ReleaseDC(hCandWnd, hDC);
        }
        break;
    case IMN_CLOSECANDIDATE:
        if (lParam & 0x00000001) {
            CloseCand(hUIWnd);
        }
        break;
    case IMN_SETSENTENCEMODE:
        break;
    case IMN_SETOPENSTATUS:
    case IMN_SETCONVERSIONMODE:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (!hStatusWnd) {
            return;
        }

        {
            RECT rcRect;

            rcRect = sImeG.rcStatusText;
            
            // off by 1
            rcRect.right += 1;
            rcRect.bottom += 1;

            RedrawWindow(hStatusWnd, &rcRect, NULL, RDW_INVALIDATE);
        }
        break;
    case IMN_SETCOMPOSITIONFONT:
        // we are not going to change font, but an IME can do this if it want
        break;
    case IMN_SETCOMPOSITIONWINDOW:
        {
            HWND hCompWnd;

            hCompWnd = GetCompWnd(hUIWnd);
            if (!hCompWnd) {
                return;
            }

            PostMessage(hCompWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
    case IMN_SETCANDIDATEPOS:
        {
            HWND hCandWnd;

            hCandWnd = GetCandWnd(hUIWnd);
            if (!hCandWnd) {
                return;
            }

            PostMessage(hCandWnd, WM_IME_NOTIFY, wParam, lParam);
        }
        break;
    case IMN_SETSTATUSWINDOWPOS:
        hStatusWnd = GetStatusWnd(hUIWnd);
        if (hStatusWnd) {
            PostMessage(hStatusWnd, WM_IME_NOTIFY, wParam, lParam);
        } else {
        }
        break;
    case IMN_GUIDELINE:
        ShowGuideLine(hUIWnd);

        break;
    case IMN_PRIVATE:
        switch (lParam) {
        case IMN_PRIVATE_UPDATE_STATUS:
            UpdateStatusWindow(hUIWnd);
            break;
        case IMN_PRIVATE_DESTROYCANDWIN:
            SendMessage(GetCandWnd(hUIWnd), WM_DESTROY, (WPARAM)0, (LPARAM)0);
            break;
        case IMN_PRIVATE_CMENUDESTROYED:
            CMenuDestroyed(hUIWnd);
            break;
        }
        break;
    default:
        break;
    }

    return;
}

/**********************************************************************/
/* SetContext()                                                       */
/**********************************************************************/
void PASCAL SetContext(         // the context activated/deactivated
    HWND   hUIWnd,
    BOOL   fOn,
    LPARAM lShowUI)
{
    HIMC           hIMC;
    LPINPUTCONTEXT lpIMC;
    LPPRIVCONTEXT  lpImcP;
    HGLOBAL        hUIPrivate;
    register LPUIPRIV lpUIPrivate;
    RECT            rcWorkArea;

    rcWorkArea = sImeG.rcWorkArea;


    hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);

    if (!hIMC) {          
        return ;
    }

    // get lpIMC
    lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
    if (!lpIMC) {
        return;
    }

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        ImmUnlockIMC(hIMC);
        return;
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        ImmUnlockIMC(hIMC);
        return;
    }

    if (fOn) {
        register DWORD fdwSetContext;

        lpUIPrivate->fdwSetContext = lpUIPrivate->fdwSetContext &
            ~(ISC_SHOWUIALL);

        lpUIPrivate->fdwSetContext |= (lShowUI & ISC_SHOWUIALL);

        {
        HKEY hKey;
        DWORD bcData; 
        char buf[256];
        bcData=256;
        if(RegOpenKeyEx (HKEY_CURRENT_USER,
                         TEXT("Control Panel\\Input Method"),
                         0,
                         KEY_ENUMERATE_SUB_KEYS |
                         KEY_EXECUTE |
                         KEY_QUERY_VALUE,
                         &hKey)){
            goto SetShowStatus;
        }
        if(RegQueryValueEx (hKey, TEXT("show status"),
                     NULL,
                     NULL,             //null-terminate string
                     (LPBYTE)buf,              //&bData,
                     &bcData) != ERROR_SUCCESS){
                     // Set default as ON if no entry in registry
                     lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;
                 goto SetShowStatus;
             }
        if(strcmp(buf, "1")==0)
            lpUIPrivate->fdwSetContext |= ISC_OPEN_STATUS_WINDOW;
        else
            lpUIPrivate->fdwSetContext &= ~ISC_OPEN_STATUS_WINDOW;
SetShowStatus:
        RegCloseKey(hKey);
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICOMPOSITIONWINDOW|ISC_HIDE_COMP_WINDOW);

        if (fdwSetContext == ISC_HIDE_COMP_WINDOW) {
            ShowComp(
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_COMP_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_COMP_WINDOW);
        } else {
        }

        fdwSetContext = lpUIPrivate->fdwSetContext &
            (ISC_SHOWUICANDIDATEWINDOW|ISC_HIDE_CAND_WINDOW);

        if (fdwSetContext == ISC_HIDE_CAND_WINDOW) {
            ShowCand(
                hUIWnd, SW_HIDE);
        } else if (fdwSetContext & ISC_HIDE_CAND_WINDOW) {
            lpUIPrivate->fdwSetContext &= ~(ISC_HIDE_CAND_WINDOW);
        } else {
        }

        if (lpIMC->cfCandForm[0].dwIndex != 0) {
            lpIMC->cfCandForm[0].dwStyle = CFS_DEFAULT;
        }


        lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
        if (!lpImcP) {
            GlobalUnlock(hUIPrivate);
            ImmUnlockIMC(hIMC);
            return;
        }


        if(SaTC_Trace == 0) {
            int UI_MODE;

            lpImcP->iImeState = CST_INIT;
            CompCancel(hIMC, lpIMC);
            
            // init fields of hPrivate
            lpImcP->fdwImeMsg = (DWORD)0;
            lpImcP->dwCompChar = (DWORD)0;
            lpImcP->fdwGcsFlag = (DWORD)0;
            lpImcP->uSYHFlg = 0x00000000;
            lpImcP->uDYHFlg = 0x00000000;

            // change compwnd size

            // init fields of hIMC
            lpIMC->fOpen = TRUE;

            SendMessage(GetCandWnd(hUIWnd), WM_DESTROY, (WPARAM)0, (LPARAM)0);
            
            // set cand window data
			UI_MODE = BOX_UI;
            InitCandUIData(
                GetSystemMetrics(SM_CXBORDER),
                GetSystemMetrics(SM_CYBORDER), UI_MODE);
        }
            
        SaTC_Trace = 1;

        // init Caps
        {
            BYTE  lpbKeyState[256];
            DWORD fdwConversion;

            if (!GetKeyboardState(lpbKeyState))
                lpbKeyState[VK_CAPITAL] = 0;
                         
            if (lpbKeyState[VK_CAPITAL] & 0x01) {
                // 10.11 add
                uCaps = 1;
                // change to alphanumeric mode
                fdwConversion = lpIMC->fdwConversion & ~(IME_CMODE_CHARCODE |
                    IME_CMODE_NATIVE | IME_CMODE_EUDC);
            } else {
                // change to native mode
                if(uCaps == 1) {
                        fdwConversion = (lpIMC->fdwConversion | IME_CMODE_NATIVE) &
                        ~(IME_CMODE_CHARCODE | IME_CMODE_EUDC);
                } else {
                    fdwConversion = lpIMC->fdwConversion;
                }
                uCaps = 0;
            }
            ImmSetConversionStatus(hIMC, fdwConversion, lpIMC->fdwSentence);
        }

        if ((lpIMC->cfCompForm.dwStyle & CFS_FORCE_POSITION)) {
            POINT ptNew;            // new position of UI
            POINT ptSTWPos;

            ImmGetStatusWindowPos(hIMC, (LPPOINT)&ptSTWPos);

            ptNew.x = ptSTWPos.x + sImeG.xStatusWi + UI_MARGIN;
            if((ptSTWPos.x + sImeG.xStatusWi + sImeG.xCandWi + lpImeL->xCompWi + 2 * UI_MARGIN) >=
              rcWorkArea.right) { 
                ptNew.x = ptSTWPos.x - lpImeL->xCompWi - UI_MARGIN;
            }
            ptNew.x += lpImeL->cxCompBorder;
            ptNew.y = ptSTWPos.y + lpImeL->cyCompBorder;
            lpIMC->cfCompForm.ptCurrentPos = ptNew;

            ScreenToClient(lpIMC->hWnd, &lpIMC->cfCompForm.ptCurrentPos);
            lpIMC->cfCompForm.dwStyle = CFS_DEFAULT;
        }
    } else {
        lpUIPrivate->fdwSetContext &= ~ISC_SETCONTEXT_UI;
    }

    GlobalUnlock(hUIPrivate);

    UIPaint(hUIWnd);

    ImmUnlockIMC(hIMC);
    return;
}


/**********************************************************************/
/* GetCompWindow()                                                    */
/**********************************************************************/
LRESULT PASCAL GetCompWindow(
    HWND              hUIWnd,
    LPCOMPOSITIONFORM lpCompForm)
{
    HWND hCompWnd;
    RECT rcCompWnd;

    hCompWnd = GetCompWnd(hUIWnd);

    if (!hCompWnd) {
        return (1L);
    }

    if (!GetWindowRect(hCompWnd, &rcCompWnd)) {
        return (1L);
    }

    lpCompForm->dwStyle = CFS_POINT|CFS_RECT;
    lpCompForm->ptCurrentPos = *(LPPOINT)&rcCompWnd;
    lpCompForm->rcArea = rcCompWnd;

    return (0L);
}

/**********************************************************************/
/* SelectIME()                                                        */
/**********************************************************************/
void PASCAL SelectIME(          // switch IMEs
    HWND hUIWnd,
    BOOL fSelect)
{
    if (!fSelect) {
        ShowUI(hUIWnd, SW_HIDE);
    } else {

        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
          
    }

    return;
}

/**********************************************************************/
/* UIPaint()                                                          */
/**********************************************************************/
LRESULT PASCAL UIPaint(
    HWND        hUIWnd)
{
    PAINTSTRUCT ps;
    MSG         sMsg;
    HGLOBAL     hUIPrivate;
    LPUIPRIV    lpUIPrivate;

    // for safety
    BeginPaint(hUIWnd, &ps);
    EndPaint(hUIWnd, &ps);

    // some application will not remove the WM_PAINT messages
    PeekMessage(&sMsg, hUIWnd, WM_PAINT, WM_PAINT, PM_REMOVE|PM_NOYIELD);

    hUIPrivate = (HGLOBAL)GetWindowLongPtr(hUIWnd, IMMGWLP_PRIVATE);
    if (!hUIPrivate) {
        return (0L);
    }

    lpUIPrivate = (LPUIPRIV)GlobalLock(hUIPrivate);
    if (!lpUIPrivate) {
        return (0L);
    }

    if (lpUIPrivate->fdwSetContext & ISC_SETCONTEXT_UI) {
        ShowUI(hUIWnd, SW_SHOWNOACTIVATE);
    } else {
        ShowUI(hUIWnd, SW_HIDE);
    }

    GlobalUnlock(hUIPrivate);

    return (0L);
}

/**********************************************************************/
/* UIWndProc()                                                        */
/**********************************************************************/
LRESULT CALLBACK UIWndProc(
    HWND   hUIWnd,
    UINT   uMsg,
    WPARAM wParam,
    LPARAM lParam)
{

    switch (uMsg) {
    case WM_CREATE:
        CreateUIWindow(hUIWnd);
        break;
    case WM_DESTROY:
        DestroyUIWindow(hUIWnd);
        break;
    case WM_IME_STARTCOMPOSITION:
        // you can create a window as the composition window here
        StartComp(hUIWnd);
        break;
    case WM_IME_COMPOSITION:
		if (lParam & GCS_RESULTSTR) {
            MoveDefaultCompPosition(hUIWnd);
        } else {
        }

        {
            HWND hCompWnd;

            hCompWnd = GetCompWnd(hUIWnd);

            if (hCompWnd) {
                RECT rcRect;

                rcRect = lpImeL->rcCompText;
                // off by 1
                rcRect.right += 1;
                rcRect.bottom += 1;

                RedrawWindow(hCompWnd, &rcRect, NULL, RDW_INVALIDATE);
            }
        }
        break;
    case WM_IME_ENDCOMPOSITION:
        // you can destroy the composition window here
        EndComp(hUIWnd);
        break;
    case WM_IME_NOTIFY:
        NotifyUI(hUIWnd, wParam, lParam);
        break;
    case WM_IME_SETCONTEXT:
        SetContext(hUIWnd, (BOOL)wParam, lParam);
        
        if (wParam && GetWindowLongPtr(hUIWnd, IMMGWLP_IMC))
            SetWindowPos(hUIWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE|SWP_NOACTIVATE|SWP_NOMOVE);

        break;
    case WM_IME_CONTROL:
        switch (wParam) {
        case IMC_GETCANDIDATEPOS:
            return (1L);                    // not implemented yet
        case IMC_GETCOMPOSITIONFONT:
            return (1L);                    // not implemented yet
        case IMC_GETCOMPOSITIONWINDOW:
            return GetCompWindow(hUIWnd, (LPCOMPOSITIONFORM)lParam);
        case IMC_GETSTATUSWINDOWPOS:
            {
                HWND   hStatusWnd;
                RECT   rcStatusWnd;
                LPARAM lParam;

                hStatusWnd = GetStatusWnd(hUIWnd);
                if (!hStatusWnd) {
                    return (0L);    // fail, return (0, 0)?
                }

                if (!GetWindowRect(hStatusWnd, &rcStatusWnd)) {
                     return (0L);    // fail, return (0, 0)?
                }

                lParam = MAKELRESULT(rcStatusWnd.left, rcStatusWnd.top);

                return (lParam);
            }
            return (0L);
        case IMC_SETSTATUSWINDOWPOS:
            {
                HIMC            hIMC;
                LPINPUTCONTEXT  lpIMC;
                LPPRIVCONTEXT   lpImcP;
                //COMPOSITIONFORM CompForm;
                POINT           ptPos;
                RECT            rcWorkArea;

                rcWorkArea = sImeG.rcWorkArea;

                ptPos.x = ((LPPOINTS)&lParam)->x;
                ptPos.y = ((LPPOINTS)&lParam)->y;

                hIMC = (HIMC)GetWindowLongPtr(hUIWnd, IMMGWLP_IMC);
                if (!hIMC) {
                    return (1L);
                }

                if(!ImmSetStatusWindowPos(hIMC, &ptPos)) {
                    return (1L);
                }

                // set comp window position when TraceCuer
                lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                if (!lpIMC) {
                    return (1L);
                }

                lpImcP = (LPPRIVCONTEXT)ImmLockIMCC(lpIMC->hPrivate);
                if (!lpImcP) {
                    return (1L);
                }

                ImmUnlockIMCC(lpIMC->hPrivate);
                ImmUnlockIMC(hIMC);

                return (0L);
            }
            return (1L);
        default:
            return (1L);
        }
        break;
    case WM_IME_COMPOSITIONFULL:
        return (0L);
    case WM_IME_SELECT:

        SetContext(hUIWnd, (BOOL)wParam, 0);
        return (0L);
    case WM_MOUSEACTIVATE:
        return (MA_NOACTIVATE);
    case WM_PAINT:
        return UIPaint(hUIWnd);
    default:
        return DefWindowProc(hUIWnd, uMsg, wParam, lParam);
    }
    return (0L);
}

void DrawConvexRect(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hPen, hOldPen;
    
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2);
    hPen = CreatePen(PS_SOLID, 1, RGB(128, 128, 128));

    if ( hPen )
    {
        hOldPen = SelectObject (hDC, hPen);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x2-1, y1);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x1, y2-1);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);          
    }
}

void DrawConvexRectP(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hPen, hOldPen;
    
    SelectObject(hDC, GetStockObject(LTGRAY_BRUSH));
    SelectObject(hDC, GetStockObject(WHITE_PEN));
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x2 - 1, y1);
    MoveToEx(hDC, x1, y1,NULL);
    LineTo(hDC, x1, y2 - 1);
    hPen = CreatePen(PS_SOLID, 1, RGB(0, 0, 0));

    if ( hPen )
    {
        hOldPen = SelectObject (hDC, hPen);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x2-1, y1);
        MoveToEx(hDC, x2-1, y2-1,NULL);
        LineTo(hDC, x1, y2-1);
        SelectObject(hDC, hOldPen);
        DeleteObject(hPen);          
    }
}

void DrawConcaveRect(HDC hDC, int x1, int y1, int x2, int y2)
{
    HPEN hLtPen = CreatePen(PS_SOLID, 1, 0x00808080);

    if ( hLtPen != NULL )
    {
        HPEN OldPen = SelectObject(hDC, hLtPen);
        MoveToEx(hDC, x1, y1,NULL);
        LineTo(hDC, x2, y1);
        MoveToEx(hDC, x1, y1,NULL);
        LineTo(hDC, x1, y2);
        SelectObject(hDC, GetStockObject(WHITE_PEN));
        MoveToEx(hDC, x2  , y2,NULL);
        LineTo(hDC, x2  , y1-1);
        MoveToEx(hDC, x2  , y2,NULL);
        LineTo(hDC, x1-1, y2);
        SelectObject(hDC, OldPen);
        DeleteObject(hLtPen);     
    }
}



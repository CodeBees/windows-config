#include <windows.h>
#include <immdev.h>
#include "imedefs.h"
#include <regstr.h>
#include "imewnd.h"

static int comp_wnd_width()
{
	return comp_dft_width;
}

static int comp_wnd_height()
{
	return comp_dft_height;
}

BOOL PASCAL FitInLazyOperation(LPPOINT lpptOrg, LPPOINT lpptNearCaret, 
							   LPRECT lprcInputRect, u32 uEsc)
{
	return false;
	POINT ptDelta, ptTol;
	RECT rcUIRect, rcInterRect;

	ptDelta.x = lpptOrg->x - lpptNearCaret->x;

	ptDelta.x = (ptDelta.x >= 0) ? ptDelta.x : -ptDelta.x;

	ptTol.x = sImeG.iParaTol * ncUIEsc[uEsc].iParaFacX +
		sImeG.iPerpTol * ncUIEsc[uEsc].iPerpFacX;

	ptTol.x = (ptTol.x >= 0) ? ptTol.x : -ptTol.x;

	if (ptDelta.x > ptTol.x) {
		return FALSE;
	}

	ptDelta.y = lpptOrg->y - lpptNearCaret->y;

	ptDelta.y = (ptDelta.y >= 0) ? ptDelta.y : -ptDelta.y;

	ptTol.y = sImeG.iParaTol * ncUIEsc[uEsc].iParaFacY +
		sImeG.iPerpTol * ncUIEsc[uEsc].iPerpFacY;

	ptTol.y = (ptTol.y >= 0) ? ptTol.y : -ptTol.y;

	if (ptDelta.y > ptTol.y) {
		return FALSE;
	}
	// build up the UI rectangle (composition window)
	rcUIRect.left = lpptOrg->x;
	rcUIRect.top = lpptOrg->y;
	rcUIRect.right = rcUIRect.left + comp_wnd_width();
	rcUIRect.bottom = rcUIRect.top + comp_wnd_height();

	if (IntersectRect(&rcInterRect, &rcUIRect, lprcInputRect)) {
		return FALSE;
	}

	return (TRUE);
}

void PASCAL GetNearCaretPosition(LPPOINT lpptFont,
								 u32 uEsc,
								 u32 uRot,
								 LPPOINT lpptCaret,
								 LPPOINT lpptNearCaret, 
								 BOOL fFlags)
{
	LONG lFontSize;
	LONG xWidthUI, yHeightUI, xBorder, yBorder;
	RECT rcWorkArea;

	if ((uEsc + uRot) & 0x0001) {
		lFontSize = lpptFont->x;
	} else {
		lFontSize = lpptFont->y;
	}

	xWidthUI = comp_wnd_width();
	yHeightUI = comp_wnd_height();
	xBorder = 0;
	yBorder = 0;

	if (fFlags & NEAR_CARET_FIRST_TIME) {
		lpptNearCaret->x = lpptCaret->x +
			lFontSize * ncUIEsc[uEsc].iLogFontFacX +
			sImeG.iPara * ncUIEsc[uEsc].iParaFacX +
			sImeG.iPerp * ncUIEsc[uEsc].iPerpFacX;

		if (ptInputEsc[uEsc].x >= 0) {
			lpptNearCaret->x += xBorder * 2;
		} else {
			lpptNearCaret->x -= xWidthUI - xBorder * 2;
		}

		lpptNearCaret->y = lpptCaret->y +
			lFontSize * ncUIEsc[uEsc].iLogFontFacY +
			sImeG.iPara * ncUIEsc[uEsc].iParaFacY +
			sImeG.iPerp * ncUIEsc[uEsc].iPerpFacY;

		if (ptInputEsc[uEsc].y >= 0) {
			lpptNearCaret->y += yBorder * 2;
		} else {
			lpptNearCaret->y -= yHeightUI - yBorder * 2;
		}
	} else {
		lpptNearCaret->x = lpptCaret->x +
			lFontSize * ncAltUIEsc[uEsc].iLogFontFacX +
			sImeG.iPara * ncAltUIEsc[uEsc].iParaFacX +
			sImeG.iPerp * ncAltUIEsc[uEsc].iPerpFacX;

		if (ptAltInputEsc[uEsc].x >= 0) {
			lpptNearCaret->x += xBorder * 2;
		} else {
			lpptNearCaret->x -= xWidthUI - xBorder * 2;
		}

		lpptNearCaret->y = lpptCaret->y +
			lFontSize * ncAltUIEsc[uEsc].iLogFontFacY +
			sImeG.iPara * ncAltUIEsc[uEsc].iParaFacY +
			sImeG.iPerp * ncAltUIEsc[uEsc].iPerpFacY;

		if (ptAltInputEsc[uEsc].y >= 0) {
			lpptNearCaret->y += yBorder * 2;
		} else {
			lpptNearCaret->y -= yHeightUI - yBorder * 2;
		}
	}

	rcWorkArea = get_wa_rect();

	if (lpptNearCaret->x < rcWorkArea.left) {
		lpptNearCaret->x = rcWorkArea.left;
	} else if (lpptNearCaret->x + xWidthUI > rcWorkArea.right) {
		lpptNearCaret->x = rcWorkArea.right - xWidthUI;
	}

	if (lpptNearCaret->y < rcWorkArea.top) {
		lpptNearCaret->y = rcWorkArea.top;
	} else if (lpptNearCaret->y + yHeightUI > rcWorkArea.bottom) {
		lpptNearCaret->y = rcWorkArea.bottom - yHeightUI;
	}

	return;
}

BOOL PASCAL AdjustCompPosition(
	input_context& ic, 
	LPPOINT lpptOrg, 
	LPPOINT lpptNew)
{
	POINT ptNearCaret, ptOldNearCaret;
	u32 uEsc, uRot;
	RECT rcUIRect, rcInputRect, rcInterRect;
	POINT ptFont;

	// we need to adjust according to font attribute
	if (ic->lfFont.A.lfWidth > 0) {
		ptFont.x = ic->lfFont.A.lfWidth * 2;
	} else if (ic->lfFont.A.lfWidth < 0) {
		ptFont.x = -ic->lfFont.A.lfWidth * 2;
	} else if (ic->lfFont.A.lfHeight > 0) {
		ptFont.x = ic->lfFont.A.lfHeight;
	} else if (ic->lfFont.A.lfHeight < 0) {
		ptFont.x = -ic->lfFont.A.lfHeight;
	} else {
		ptFont.x = comp_wnd_height();
	}

	if (ic->lfFont.A.lfHeight > 0) {
		ptFont.y = ic->lfFont.A.lfHeight;
	} else if (ic->lfFont.A.lfHeight < 0) {
		ptFont.y = -ic->lfFont.A.lfHeight;
	} else {
		ptFont.y = ptFont.x;
	}

	// if the input char is too big, we don't need to consider so much
	if (ptFont.x > comp_wnd_height() * 8) {
		ptFont.x = comp_wnd_height() * 8;
	}
	if (ptFont.y > comp_wnd_height() * 8) {
		ptFont.y = comp_wnd_height() * 8;
	}

	if (ptFont.x < sImeG.xChiCharWi) {
		ptFont.x = sImeG.xChiCharWi;
	}

	if (ptFont.y < sImeG.yChiCharHi) {
		ptFont.y = sImeG.yChiCharHi;
	}
	// -450 to 450 index 0
	// 450 to 1350 index 1
	// 1350 to 2250 index 2
	// 2250 to 3150 index 3
	uEsc = (u32) ((ic->lfFont.A.lfEscapement + 450) / 900 % 4);
	uRot = (u32) ((ic->lfFont.A.lfOrientation + 450) / 900 % 4);

	// decide the input rectangle
	rcInputRect.left = lpptNew->x;
	rcInputRect.top = lpptNew->y;

	// build up an input rectangle from escapemment
	rcInputRect.right = rcInputRect.left + ptFont.x * ptInputEsc[uEsc].x;
	rcInputRect.bottom = rcInputRect.top + ptFont.y * ptInputEsc[uEsc].y;

	// be a normal rectangle, not a negative rectangle
	if (rcInputRect.left > rcInputRect.right) {
		LONG tmp;

		tmp = rcInputRect.left;
		rcInputRect.left = rcInputRect.right;
		rcInputRect.right = tmp;
	}

	if (rcInputRect.top > rcInputRect.bottom) {
		LONG tmp;

		tmp = rcInputRect.top;
		rcInputRect.top = rcInputRect.bottom;
		rcInputRect.bottom = tmp;
	}

	GetNearCaretPosition(&ptFont, uEsc, uRot, lpptNew, &ptNearCaret,
						 NEAR_CARET_FIRST_TIME);

	// 1st, use the adjust point
	// build up the new suggest UI rectangle (composition window)
	rcUIRect.left = ptNearCaret.x;
	rcUIRect.top = ptNearCaret.y;
	rcUIRect.right = rcUIRect.left + comp_wnd_width();
	rcUIRect.bottom = rcUIRect.top + comp_wnd_height();

	ptOldNearCaret = ptNearCaret;

	// OK, no intersect between the near caret position and input char
	if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
	} else
		if (FitInLazyOperation(lpptOrg, &ptNearCaret, &rcInputRect, uEsc))
	{
		// happy ending!!!, don't change position
		return FALSE;
	} else {
		*lpptOrg = ptNearCaret;

		// happy ending!!
		return (TRUE);
	}

	// unhappy case
	GetNearCaretPosition(&ptFont, uEsc, uRot, lpptNew, &ptNearCaret, 0);

	// build up the new suggest UI rectangle (composition window)
	rcUIRect.left = ptNearCaret.x;
	rcUIRect.top = ptNearCaret.y;
	rcUIRect.right = rcUIRect.left + comp_wnd_width();
	rcUIRect.bottom = rcUIRect.top + comp_wnd_height();

	// OK, no intersect between the adjust position and input char
	if (IntersectRect(&rcInterRect, &rcUIRect, &rcInputRect)) {
	} else
		if (FitInLazyOperation(lpptOrg, &ptNearCaret, &rcInputRect, uEsc))
	{
		return FALSE;
	} else {
		*lpptOrg = ptNearCaret;

		return (TRUE);
	}

	*lpptOrg = ptOldNearCaret;

	return (TRUE);
}

void PASCAL SetCompPosition(input_context& ic)
{
	POINT ptWnd;
	BOOL fChange = FALSE;

	ptWnd.x = 0;
	ptWnd.y = 0;

	ClientToScreen(ic.get_comp_wnd(), &ptWnd);

	POINT ptNew;			// new position of UI
	ptNew.x = ic->cfCompForm.ptCurrentPos.x;
	ptNew.y = ic->cfCompForm.ptCurrentPos.y;
	if (ptNew.x == 0 && ptNew.y == 0) {
		ptNew.x = ic->cfCandForm[0].ptCurrentPos.x;
		ptNew.y = ic->cfCandForm[0].ptCurrentPos.y;
	}
	ClientToScreen((HWND) ic->hWnd, &ptNew);
	fChange = AdjustCompPosition(ic, &ptWnd, &ptNew);


	if (!fChange) {
		return;
	}
	SetWindowPos(ic.get_comp_wnd(), NULL,
				 ptWnd.x, ptWnd.y,
				 0, 0, SWP_NOACTIVATE | SWP_NOSIZE | SWP_NOZORDER);
	return;
}

void PASCAL MoveDefaultCompPosition(HWND hUIWnd)
{
	input_context ic(hUIWnd);
	if (!ic) {
		return;
	}


	if (!ic.get_comp_wnd()) {
		bhjerr("Error: MoveDefaultCompPosition with NULL comp");
	}

	SetCompPosition(ic);
	return;
}

void show_comp_wnd(HWND hUIWnd)
{
	if (g_comp_str.empty()) {
		return hide_comp_wnd(hUIWnd);
	}
	ShowWindow(get_comp_wnd(hUIWnd), SW_SHOWNOACTIVATE);
}

void hide_comp_wnd(HWND hUIWnd)
{
	ShowWindow(get_comp_wnd(hUIWnd), SW_HIDE);
}

void PASCAL StartComp(HWND hUIWnd)
{
	input_context ic(hUIWnd);
	if (!ic) {
		return;
	}

	if (!get_comp_wnd(hUIWnd)) {
		HWND comp = CreateWindowEx(WS_EX_TOPMOST, get_comp_class_name().c_str(), NULL, WS_POPUP | WS_DISABLED,
									0, 0, comp_dft_width, comp_dft_height, hUIWnd,
									(HMENU) NULL, g_hInst, NULL);
		set_comp_wnd(hUIWnd, comp);
	}

	SetCompPosition(ic);
	show_comp_wnd(hUIWnd);

	return;
}

void high_light(HDC hdc, const CRect& rect)
{
	HDC hdc_mem = CreateCompatibleDC(hdc); 

	HBITMAP hbitmap = CreateCompatibleBitmap(hdc, 
											 rect.right-rect.left,
											 rect.bottom-rect.top);
	HBITMAP hbitmap_old = SelectObject(hdc_mem, hbitmap); 
	HBRUSH hbrush = CreateSolidBrush(0x2837df);
    
	RECT rect_mem = {0, 0, rect.Width(), rect.Height()};
	FillRect(hdc_mem, &rect_mem, hbrush);

	BitBlt(hdc, 
		   rect.left, rect.top,
		   rect.Width(), rect.Height(),
		   hdc_mem,
		   0,0, 
		   SRCINVERT);

	DeleteObject(hbrush);
	SelectObject(hdc_mem, hbitmap_old);
	DeleteObject(hbitmap);
	DeleteObject(hdc_mem);
}

// class debug_u32 {
// public:
// 	debug_u32(const string& name, u32 *u32_ptr) {
// 		m_name = name;
// 		m_u32_ptr = u32_ptr;
// 		BHJDEBUG(" %s is %d at beginning", name.c_str(), *u32_ptr);
// 	}
// 	~debug_u32() {
// 		BHJDEBUG(" %s is %d at end", m_name.c_str(), *m_u32_ptr);
// 	}
// private:
// 	string m_name;
// 	u32* m_u32_ptr;
// };

static void 
draw_cands(HDC hdc, const CRect& rect, const vector<string>& cands)
{
	hdc_with_font dc_lucida(hdc, L"Lucida Console", 10);

	int left = rect.left;
	CRect rc_text = rect;
	wstring seq = L"0:";

	if (g_first_cand + g_last_cand + g_active_cand == 0 && 
		map_has_key(g_cand_hist, g_comp_str) &&
		g_cand_hist[g_comp_str] < cands.size() &&
		g_cand_hist[g_comp_str] >= 0) {

		g_first_cand = 0;
		g_active_cand = g_cand_hist[g_comp_str];

		for (size_t i=0; i < cands.size(); i++) {

			if (seq[0] == L'9') {
				seq[0] = L'0';
			} else {
				seq[0] += 1;
			}
			u32 seq_width = dc_lucida.get_text_width(seq);
			wstring cand = to_wstring(cands[i]);
			u32 cand_width = dc_lucida.get_text_width(cand);

			left += seq_width + cand_width + 6;

			if (left > rect.right && //can't draw this one
				i > g_first_cand) { //make sure at least one is drawn
				g_first_cand = i--;
				left = rect.left;
				seq[0] = L'0';
				continue;
			} 

			if (g_active_cand == i) {
				g_last_cand = g_first_cand;
				break;
			}
		}
	}
			
	if (g_first_cand > g_last_cand && g_first_cand != g_active_cand) {
		for (int i=g_last_cand; i>=0; i--) {
			if (seq[0] == L'9') {
				seq[0] = L'0';
			} else {
				seq[0] += 1;
			}
			
			u32 seq_width = dc_lucida.get_text_width(seq);
			wstring cand = to_wstring(cands[i]);
			u32 cand_width = dc_lucida.get_text_width(cand);

			left += seq_width + cand_width + 6;
			if (left > rect.right && i != g_last_cand) {
				g_first_cand = i+1;
				break;
			} else {
				g_first_cand = i;
			}
		}
	} else {
		g_last_cand = g_first_cand;
	}

	left = rect.left;
	seq[0] = L'0';
	if (g_first_cand <= g_last_cand) {
		for (size_t i=g_first_cand; i<cands.size() && i-g_first_cand < (u32)10; i++) {
			if (seq[0] == L'9') {
				seq[0] = L'0';
			} else {
				seq[0] += 1;
			}

			u32 seq_width = dc_lucida.get_text_width(seq);
			wstring cand = to_wstring(cands[i]);
			u32 cand_width = dc_lucida.get_text_width(cand);
			if (left + seq_width + cand_width + 6 > (u32)rect.right
				&& i != g_first_cand) {
				g_last_cand = i-1;
				break;
			} else {
				g_last_cand = i;
			}
				
			rc_text.left = left;
			left += seq_width;
			rc_text.right = left;
			left += 2;
			SetTextColor(hdc, RGB(0, 0, 0));
			dc_lucida.draw_text(seq, rc_text);


			rc_text.left = left;
			left += cand_width;
			rc_text.right = left;
			left += 4;
			dc_lucida.draw_text(cand, rc_text);

			if (i == g_active_cand) {
				high_light(hdc, rc_text);
			} 
		}
	}
}

void PASCAL PaintCompWindow(HWND hWnd, HDC hdc)
{
	CRect rcWnd;
	GetClientRect(hWnd, &rcWnd);
	Rectangle(hdc, rcWnd.left, rcWnd.top, rcWnd.right, rcWnd.bottom);

	rcWnd.left += 5;
	rcWnd.right -= 5;

	CRect rc_top = rcWnd;
	rc_top.bottom = (rcWnd.top+rcWnd.bottom)/2;

	CRect rc_bot = rcWnd;
	rc_bot.top = rc_top.bottom;



	if (g_comp_str.size()) {
		string str_on_comp = g_comp_str;
		if (map_has_key(g_trans_rule, g_comp_str)) {
			list<string> trans;
			for(rule_trans_t::mapped_type::iterator i = g_trans_rule[g_comp_str].begin();
				i != g_trans_rule[g_comp_str].end();
				i++) {
				trans.push_back(i->first);
			}
			u32 s1 = trans.size();
			trans.sort();
			trans.unique();
			if (trans.size()) {
				str_on_comp.push_back('[');
				for (list<string>::iterator i = trans.begin(); i != trans.end(); i++) {
					str_on_comp += *i;
				}
				str_on_comp.push_back(']');
			}
		}
		wstring wstr = to_wstring(str_on_comp);
		DrawText(hdc, wstr.c_str(), wstr.size(), &rc_top, DT_VCENTER|DT_SINGLELINE);
	} 

	if (g_quail_rules.find(g_comp_str) != g_quail_rules.end()) {
		draw_cands(hdc, rc_bot, g_quail_rules[g_comp_str]);
	}

	MoveToEx(hdc, rcWnd.left, (rcWnd.top+rcWnd.bottom)/2, NULL);
	LineTo(hdc, rcWnd.right, (rcWnd.top+rcWnd.bottom)/2);
	return;
}

LRESULT CALLBACK CompWndProc(HWND hWnd, u32 uMsg, WPARAM wParam, LPARAM lParam)
{

	//BHJDEBUG("received msg %s", msg_name(uMsg));

	switch (uMsg) {
	case WM_CREATE:
		break;
	case WM_DESTROY:
		break;
	case WM_IME_NOTIFY:
		// must not delete this case, because DefWindowProc will hang the IME
		break;
	case WM_PAINT:
		{
			HDC hDC;
			PAINTSTRUCT ps;

			hDC = BeginPaint(hWnd, &ps);
			PaintCompWindow(hWnd, hDC);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_MOUSEACTIVATE:
		return (MA_NOACTIVATE);
	default:
		//BHJDEBUG(" msg %s not handled", msg_name(uMsg));
		return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
	return (0L);
}

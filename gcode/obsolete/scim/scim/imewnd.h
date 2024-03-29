#ifndef __IME_WND_H__
#define __IME_WND_H__
#include <windows.h>
#include <atltypes.h>
#include <immdev.h>
#include <string>
#include <map>
#include <vector>
#include <list>
#define ENABLE_BHJDEBUG
#include "bhjdebug.h" 

#define RGB_TRANS_KEY RGB(255, 255, 254)
using namespace std;

typedef UINT u32;
typedef map<string, vector<string>> rule_map_t;
typedef map<string, map<string, int>> rule_trans_t;
typedef map<string, u32> cand_hist_t;

typedef UINT64 u64;

HWND get_comp_wnd(HWND);
HWND get_status_wnd(HWND);

void set_comp_wnd(HWND, HWND);
void set_status_wnd(HWND, HWND);

struct ui_private_data
{
	HWND h_comp_wnd;
	HWND h_stat_wnd;
	
	void set_comp(HWND comp) {
		if (h_comp_wnd) {
			bhjerr("Error: comp wnd reset when already inited");
		}
		h_comp_wnd = comp;
	};
	
	void set_stat(HWND stat) {
		if (h_stat_wnd) {
			bhjerr("Error: stat wnd reset when already inited");
		}
		h_stat_wnd = stat;
	};

	ui_private_data() {
		h_comp_wnd = NULL;
		h_stat_wnd = NULL;
	};
	
	~ui_private_data() {
		if (h_comp_wnd) {
			DestroyWindow(h_comp_wnd);
		}
		if (h_stat_wnd) {
			DestroyWindow(h_stat_wnd);
		}
	}
};

typedef map<HWND, ui_private_data> ui_private_t;

extern ui_private_t g_ui_private;

void debug_rect(const CRect& rect);
void FillSolidRect(HDC hdc, const CRect& rect, COLORREF rgb);

class input_context
{
public:	
	HIMC get_handle() {
		return m_himc;
	}

	HWND get_ui_wnd() {
		if (!m_hUIWnd) {
			bhjerr("Error: get_ui_wnd with NULL m_hUIWnd");
		}
		return m_hUIWnd;
	}

	HWND get_comp_wnd() {
		return ::get_comp_wnd(get_ui_wnd()); //NULL m_hUIWnd is checked
	}
	
	HWND get_status_wnd() {
		return ::get_comp_wnd(get_ui_wnd());
	}

	input_context(HIMC himc, LPTRANSMSG msg_buf, u32 msg_buf_size = 0);
	input_context(HWND hwnd);
	~input_context() {
		if (m_himc) {
			ImmUnlockIMC(m_himc);
		}
	}

	int send_text(const string&);
	
	LPINPUTCONTEXT operator->() {
		return m_ic;
	}
	
	operator bool() {
		if (m_ic) {
			return true;
		} else {
			return false;
		}
	}

	bool add_msg(u32 msg, WPARAM wp = 0, LPARAM lp = 0);
	u32 return_ime_msgs();
	bool add_show_comp_msg();
	bool add_update_status_msg();
private:
	bool enlarge_msg_buf(u32 n);
	bool copy_old_msg();

private:
	//to prevent copying..
	input_context(const input_context&);
	input_context& operator=(const input_context&); 

private:
	LPINPUTCONTEXT m_ic;
	HIMC m_himc;
	const LPTRANSMSG m_msg_buf;
	const u32 m_msg_buf_size;
	u32 m_num_msg;
	HWND m_hUIWnd;
};

class hdc_with_font
{
public:
	hdc_with_font(HDC, wstring, int size=12);
	~hdc_with_font();
	void use_this_font();
	void draw_text(const wstring& text, CRect& rect);
	int get_text_width(const wstring& text);
private:
	HFONT m_old_font, m_this_font;
	HDC m_dc;
};

extern string g_comp_str;

// int MultiByteToWideChar(
//   UINT CodePage, 
//   DWORD dwFlags,         
//   LPCSTR lpMultiByteStr, 
//   int cbMultiByte,       
//   LPWSTR lpWideCharStr,  
//   int cchWideChar        
// );

wstring to_wstring(const string& str);
string to_string(const wstring& wstr);
CRect get_wa_rect();
void promote_cand_for_key(u32 cand_num, const string& key);
void self_make_cand_for_key(const string& cand, const string& key);

const int comp_dft_width = 600;
const int comp_dft_height = 60;

extern rule_map_t g_quail_rules;
extern rule_map_t g_reverse_rules;
extern rule_trans_t g_trans_rule;
extern cand_hist_t g_cand_hist;

extern u32 g_first_cand, g_last_cand, g_active_cand;
extern string g_ime_name;
extern const char *const ime_off;
extern const char *const ime_on;
extern list<wchar_t> g_history_list;
extern "C" char *strcasestr(const char *S, const char *FIND);
template<class key_type, class mapped_type> 
bool map_has_key(const map<key_type, mapped_type>& map_query, const key_type& key)
{
	if (map_query.find(key) != map_query.end()) {
		return true;
	} else {
		return false;
	}
}
wstring get_ui_class_name();
wstring get_comp_class_name();
wstring get_status_class_name();
void high_light(HDC hdc, const CRect& rect);
#endif

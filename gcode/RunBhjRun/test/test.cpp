// test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "test.h"
#include "../bhjlib.h"
#define ENABLE_BHJDEBUG
#include "bhjdebug.h" 
using namespace bhj;

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
	}
	else
	{
		// TODO: code your application's behavior here.
		CString strHello;
		strHello.LoadString(IDS_HELLO);
		cout << (LPCTSTR)strHello << endl;
	}
	BHJDEBUG(" dirname is %s", bce_dirname("q:\\").c_str());
	BHJDEBUG(" basename is `%s'", basename("/").c_str());
	
	WIN32_FIND_DATA wfd;
	HANDLE hfile = FindFirstFile("c:/***", &wfd);
	while (hfile != INVALID_HANDLE_VALUE) {
		BHJDEBUG(" found a file %s", wfd.cFileName);
		if (FindNextFile(hfile, &wfd) == 0) {
			break;
		}
	}
	return nRetCode;
}
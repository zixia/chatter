#include "AfxHeader.h"
#include "WinMain.h"
#include "ChatClient.h"
#include "ClientSocket.h"

CChatter::CChatter()
{
	hBrush	= CreateSolidBrush(RGB(0,255,0));
	//hbrInput = CreateSolidBrush(RGB(0,255,0));
}

CChatter::~CChatter()
{
	FILE* pFile;
	pFile	= fopen("Zixia.dat","w");
	if(pFile){
		if(!strcmp(m_szID, "TELNET")){
			fprintf(pFile, "%s\n", "Zixia_");
		}else{
			fprintf(pFile, "%s\n", m_szID);
		}
		fprintf(pFile, "%u\n", uFlags);
		fclose(pFile);
	}

}

bool CChatter::Input(LPCTSTR szStr)
{
	char szTemp[MAX_STR_LENGTH];
	szTemp[0]	= 0;

	if(!Socket.m_sServer){
		return View("现在你没有和任何人聊天，也没有使用 Telnet 功能。\r\n");
	}

    switch(Socket.m_iFlags){
	case CALL:
		strcpy(szTemp, m_szID);
		strcat(szTemp, ": ");
		//strcat(szTemp, szStr);
		break;
	}

	strcat(szTemp,szStr);
	strcat(szTemp,"\r\n");

	Socket.Send(szTemp);

	return View(szTemp);
}

bool CChatter::View(LPCTSTR szStr)
{
	int length;

	length	= SendMessage(hView, WM_GETTEXTLENGTH, NULL, NULL);
	SendMessage(hView, EM_SETSEL, length, length);
	SendMessage(hView, EM_REPLACESEL, FALSE, (LPARAM)(szStr)); 
	return true;
}

bool CChatter::ViewChar(unsigned char* ch)
{
	int length;
	char szTemp[2];

	strcpy(szTemp, (const char*)ch);
	szTemp[1]=0;

	length	= SendMessage(hView, WM_GETTEXTLENGTH, NULL, NULL);
	SendMessage(hView, EM_SETSEL, length, length);

	SendMessage(hView, EM_REPLACESEL, FALSE, (LPARAM)(szTemp)); 
	return true;
}

bool CChatter::CreatePopupMenu()
{
	hMenu		= ::CreatePopupMenu();
	
	if(!hMenu)
		return false;

	AppendMenu(hMenu, MF_STRING, ZXM_CALL, "Call 别人...");
	AppendMenu(hMenu, MF_STRING, ZXM_TELNET, "Telnet...");
	AppendMenu(hMenu, MF_STRING, ZXM_EXIT, "断开连接");	
	AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(hMenu, MF_STRING, ZXM_DISABLE, "不接受别人的呼叫");
	AppendMenu(hMenu, MF_STRING, ZXM_TOPMOST, "总在最前面");
	AppendMenu(hMenu, MF_SEPARATOR, NULL, NULL);	
	AppendMenu(hMenu, MF_STRING, ZXM_ABOUT, "关于...");
	AppendMenu(hMenu, MF_STRING, ZXM_QUIT, "丢掉紫虾ＢＰCALL");
	return true;
}

void CChatter::TrackPopupMenu()
{
	POINT point;
	GetCursorPos(&point);

	EnableMenuItem(hMenu, ZXM_CALL, Socket.m_sServer?MF_GRAYED:MF_ENABLED);
	EnableMenuItem(hMenu, ZXM_TELNET, Socket.m_sServer?MF_GRAYED:MF_ENABLED);
	EnableMenuItem(hMenu, ZXM_EXIT, Socket.m_sServer?MF_ENABLED:MF_GRAYED);
	CheckMenuItem(hMenu, ZXM_DISABLE, (uFlags&ZXF_CALLER_OFF?MF_CHECKED:MF_UNCHECKED));
	CheckMenuItem(hMenu, ZXM_TOPMOST, (uFlags&ZXF_ALWAYS_TOP?MF_CHECKED:MF_UNCHECKED));
	
	::TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, point.x, point.y, NULL, hWnd, NULL); //GetDesktopWindow
}

bool CChatter::CreateWnd(HINSTANCE hInstance)
{
	hInst	= hInstance;
	SystemParametersInfo(SPI_GETWORKAREA, NULL, &rect, SPIF_SENDCHANGE);

	rect.top	= rect.bottom-CHATTER_HEIGHT;
	rect.left	= rect.right-CHATTER_WIDTH;

	hWnd	= CreateWindowEx(/*WS_EX_OVERLAPPEDWINDOW |*/ 
		WS_EX_TOOLWINDOW & ~WS_EX_CLIENTEDGE & ~WS_EX_DLGMODALFRAME & ~WS_EX_WINDOWEDGE,
		CHATTER_NAME,CHATTER_NAME CHATTER_VER,
		//(WS_OVERLAPPEDWINDOW |  WS_MAXIMIZEBOX | WS_MINIMIZEBOX) & ~WS_SYSMENU, //|WS_VISIBLE, //WM_MAXIMIZE
		WS_POPUP & ~WS_CAPTION & ~WS_BORDER & ~WS_DLGFRAME,
		rect.left,rect.top,
		rect.right-rect.left,
		rect.bottom-rect.top,
		NULL,(HMENU)NULL, hInstance,NULL);
	
	RECT rect;
	GetClientRect(hWnd, &rect);


	hView	= CreateWindow("EDIT", NULL,
		ES_AUTOVSCROLL|ES_LEFT|ES_READONLY|ES_MULTILINE|WS_VISIBLE|WS_CHILD|WS_VSCROLL,
		VIEW_XO,VIEW_YO,rect.right-2*VIEW_XO,rect.bottom-2*VIEW_YO,
		hWnd, (HMENU)IDC_VIEW, hInstance, NULL);

	hInput	= CreateWindow("EDIT", NULL,
		ES_AUTOHSCROLL|ES_LEFT|WS_VISIBLE|WS_CHILD,
		//rect.left,rect.bottom-INPUT_HEIGHT,rect.right-rect.left,INPUT_HEIGHT,
		INPUT_XO, INPUT_YO, rect.right-rect.left-2*INPUT_XO,INPUT_HEIGHT,
		hWnd, (HMENU)IDC_INPUT, hInstance, NULL);

	SetFocus();
	
	NotifyIconData.cbSize	= sizeof(NOTIFYICONDATA);
	NotifyIconData.hWnd		= hWnd;
	NotifyIconData.uID		= IDI_TRAY;
	NotifyIconData.uFlags	= NIF_ICON|NIF_MESSAGE|NIF_TIP;
	NotifyIconData.uCallbackMessage	= WM_ZX_ICON;
	NotifyIconData.hIcon	= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
	strcpy(NotifyIconData.szTip, CHATTER_NAME CHATTER_VER);

	Shell_NotifyIcon(NIM_ADD, &NotifyIconData);
	GetUserName();
	return true;
}

void CChatter::ShowWindow()
{
	hRgn	= CreateEllipticRgn(0, 0, rect.right-rect.left, rect.bottom-rect.top);
	SetWindowRgn(hWnd, hRgn, true);

	::SetWindowPos(hWnd, hWndInsertAfter, //IsWindowActive
		rect.left,rect.top,
		rect.right-rect.left,rect.bottom-rect.top,
		//uFlags&ZXF_CALLER_VISIBLE?(uFlags&ZXF_CALLER_ACTIVE?SWP_SHOWWINDOW:SWP_NOACTIVATE):SWP_HIDEWINDOW);
		uFlags&ZXF_CALLER_VISIBLE?SWP_SHOWWINDOW:SWP_HIDEWINDOW);
	if(uFlags&ZXF_CALLER_VISIBLE)
		::SetForegroundWindow(hWnd);
}

void CChatter::OnSize(int nWidth, int nHeight)
{
	SetWindowPos(
		hView,
		HWND_TOP,
		0,0,
		nWidth, nHeight-INPUT_HEIGHT,
		SWP_SHOWWINDOW);
	SetWindowPos(
		hInput,
		HWND_TOP,
		0,nHeight-INPUT_HEIGHT,
		nWidth,nHeight,
		SWP_SHOWWINDOW);
	GetWindowRect(hWnd, (LPRECT)&rect);

}

void CChatter::OnReturn()
{
	char szBuffer[MAX_STR_LENGTH];

	Edit_GetText(hInput, szBuffer, MAX_STR_LENGTH);
	Edit_SetSel(hInput, 0, -1);
	
	Input(szBuffer);
}

void CChatter::OnClickMenuItem(int nID)
{
	char szTemp[MAX_STR_LENGTH];

	switch(nID)
	{
	case ZXM_CALL:
		Socket.m_iFlags	= CALL;
		if(!ShowRemoteDialog(hWnd))
			return;
		Socket.Connect(hWnd, m_szRemote, m_nPort);
		break;
	case ZXM_TELNET:
		Socket.m_iFlags	= TELNET;
		if(!ShowRemoteDialog(hWnd))
			return;
		Socket.Connect(hWnd, m_szRemote, m_nPort);
		break;
	case ZXM_EXIT:
		switch(Socket.m_iFlags){
		case CALL:
			strcpy(szTemp, "只见");
			strcat(szTemp, m_szID);
			strcat(szTemp, "的身影越来越远，终于渐渐的消失了……");
			uFlags	&= ~ZXF_CALLER_VISIBLE;
			ShowWindow();
			break;
		case TELNET:
			strcpy(szTemp, "\r\n连接断开了。");
			uFlags	&= ~ZXF_CALLER_VISIBLE;
			ShowWindow();
			break;
		}
		Input(szTemp);
		closesocket(Socket.m_sServer);
		Socket.m_sServer	= NULL;
		break;
	case ZXM_DISABLE:
		uFlags ^= ZXF_CALLER_OFF;
		break;
	case ZXM_TOPMOST:
		uFlags ^= ZXF_ALWAYS_TOP;
		hWndInsertAfter = uFlags&ZXF_ALWAYS_TOP?HWND_TOPMOST:HWND_NOTOPMOST;
		ShowWindow();
		break;
 	case ZXM_ABOUT:
		ShowAboutDialog();
		break;
	case ZXM_QUIT:
		OnClickMenuItem(ZXM_EXIT);
		PostMessage(hWnd, WM_CLOSE, NULL,NULL);
		break;
	}
}

void CChatter::OnClickIcon(LPARAM lParam)
{
	switch(lParam)
	{
	case WM_LBUTTONUP:
		uFlags	^= ZXF_CALLER_VISIBLE;
		ShowWindow();
		break;
	case WM_RBUTTONUP:
		TrackPopupMenu();
		break;
	default:
		break;
	}



}

void CChatter::SetFocus()		//使输入焦点始终在Input栏
{
	::SetFocus(hInput);
}

void CChatter::Focus(bool b)
{
	if(b){//收到 Focus
		uFlags |= ZXF_CALLER_ACTIVE;
	}else{//...
		uFlags &= ~ZXF_CALLER_ACTIVE;
	}
}

void CChatter::SetServer(bool bServer)
{
	bServer ? uFlags |= ZXF_CALLER_SERVER : uFlags &= ~ZXF_CALLER_SERVER;
}

bool CChatter::ShowRemoteDialog(HWND hWnd)
{
	int nResult		= ::DialogBox(hInst,
      "IDD_GET_REMOTE",
      hWnd,
      (DLGPROC)RemoteDlgProc);
	return nResult==1?true:false;
}

void CChatter::GetUserName()
{
	int error;
	FILE* pFile;

	pFile	= fopen("Zixia.dat","r");
	if( !pFile ){
		Socket.m_iFlags	= GET_NAME;
		ShowRemoteDialog(hWnd);
		uFlags |= ZXF_CALLER_ACTIVE | ZXF_ALWAYS_TOP;
	}else{
		error = fscanf(pFile, "%s", m_szID);
		if(  !error || error == -1 ){
			Socket.m_iFlags	= GET_NAME;
			ShowRemoteDialog(hWnd);
		}
		char szTemp[10];
		error = fscanf(pFile, "%s", szTemp);
		if(  !error || error == -1 ){
			uFlags |= ZXF_CALLER_ACTIVE | ZXF_ALWAYS_TOP;
		}else{
			uFlags = atoi(szTemp);
		}
		fclose(pFile);
	}

	pFile	= fopen("Zixia.dat","w");
	if(pFile){
		fprintf(pFile, "%s\n", m_szID);
		fprintf(pFile, "%u\n", uFlags);
		fclose(pFile);
	}	

	hWndInsertAfter = uFlags&ZXF_ALWAYS_TOP?HWND_TOPMOST:HWND_NOTOPMOST;
}

void CChatter::ActiveMe()
{
	uFlags	|= ZXF_CALLER_VISIBLE;
	ShowWindow();
	//DrawBackground();
}

void CChatter::ShowAboutDialog()
{
	::DialogBox(hInst,             // current instance         
      "IDD_ABOUT",                 //     /* resource to use          
      hWnd,                              //* parent handle            
      (DLGPROC)AboutDlgProc);                       //* instance address        update
}

void CChatter::DrawBackground(HDC hDC)
{
	/*HBITMAP hBitMap;
	HDC		hbmDC;
	//DEVMODE devMode;

	hBitMap = LoadBitmap(hInst, MAKEINTRESOURCE(IDB_BACKGROUND));
	hbmDC = CreateDC("DISPLAY","Zixia Caller Background", NULL, NULL);//&devMode);
	SelectObject(hbmDC, hBitMap);
	if(!BitBlt(hDC, 0,0, CHATTER_WIDTH, CHATTER_HEIGHT, hbmDC, 0,0, SRCCOPY))
		Beep(500,500);*/

    //draw ellipse with out any border 

	SelectObject(hDC, (HGDIOBJ)NULL_PEN);//dc. SelecStockObject (NULL_PEN); 
	
    //get the RGB colour components of the sphere color 
    COLORREF color= RGB( 0 , 0 , 255); 
    BYTE byRed =GetRValue (color); 
    BYTE byGreen = GetGValue (color); 
    BYTE byBlue = GetBValue (color); 
	
    // get the size of the view window 
    RECT rect ; 
    GetClientRect(hWnd, &rect); 
	
    // get minimun number of units 
    int nUnits =min (rect.right , rect.bottom ); 
	int nIndex;
	
    //calculate he horiaontal and vertical step size 
    float fltStepHorz = (float) rect.right /nUnits ; 
    float fltStepVert = (float) rect.bottom /nUnits ; 
	
    int nEllipse = nUnits/3; // calculate how many to draw 

	
    HBRUSH hBrush ;       // bursh used for ellipse fill color 
    HBRUSH hBrushOld;     // previous brush that was selected into dc 
	
    //draw ellipse , gradually moving towards upper-right corner 
    for (nIndex = 0 ; nIndex < + nEllipse ; nIndex ++) 
	{ 
		//creat solid brush  CreateBrush
		hBrush = CreateSolidBrush(RGB ( 
			( (nIndex *byRed ) /nEllipse ), 
			( ( nIndex * byGreen ) /nEllipse ), 
			( ( nIndex * byBlue ) / nEllipse ) ) 
			); 
		
		//select brush into dc 
		hBrushOld = (HBRUSH)SelectObject (hDC, hBrush); 
		
		//draw ellipse 
		Ellipse (hDC,
			(int) fltStepHorz * 2, (int) fltStepVert * nIndex , 
			rect. right -( (int) fltStepHorz * nIndex )+ 1, 
			rect . bottom -( (int) fltStepVert * (nIndex *2) ) +1) ; 
		
		//delete the brush 
		//brush. DelecteObject ( ); 
    } 
} 


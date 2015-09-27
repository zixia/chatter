#include "AfxHeader.h"
#include "WinMain.h"
#include "ChatClient.h"
#include "ClientSocket.h"


CChatter Chatter;
CSocket Socket(&Chatter);

char szBuffer[MAX_STR_LENGTH];

int WINAPI WinMain(HINSTANCE	hInstance,
				   HINSTANCE	hPreInstance,
				   LPSTR		szCmdLine,
				   int			nCmdShow)
{
	SetLastError((DWORD)0);
	::CreateMutex(NULL, true, "Single Zixia Chatter"); // 禁止
	if(GetLastError() == ERROR_ALREADY_EXISTS)
		return 0;

	MSG		msg;
	HACCEL	hAccelerator;

	if(!hPreInstance){
		if(!InitApp(hInstance)){
			return false;
		}
	}

	if(!InitInstance(hInstance, nCmdShow)){
		return false;
	}

/*	if(!GetSystemMetrics(SM_PENWINDOWS)){
		return false;
	}*/

	hAccelerator = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_ACCELERATOR));
	if(!hAccelerator)	return false;

	while(GetMessage((LPMSG)&msg, NULL, 0, 0)){
		if(!TranslateAccelerator(Chatter.hWnd, hAccelerator, (LPMSG)&msg)){
			TranslateMessage((LPMSG)&msg);
			DispatchMessage((LPMSG)&msg);
		}
	}

	return 0;
}

LRESULT WINAPI ChatterProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
//	char	szTemp[80]; ellipse

	switch(msg)
	{
	case WM_CTLCOLOR:
		return((LRESULT)GetStockObject(LTGRAY_BRUSH));//RGB(0,255,0);//GetStockObject(LTGRAY_BRUSH);//(HGDIOBJ)(Chatter.hBrush);
		break;

	/*case WM_ERASEBKGND:
		//Beep(500,500);
		Chatter.DrawBackground((HDC)wParam);
		return 0;
		break;*/
	case WM_NCHITTEST:
		return HTCAPTION;
		break;
	case WM_CREATE:
		Socket.Listen(hWnd, true);
		break;
	case WM_SIZE:
		Chatter.OnSize(LOWORD(lParam), HIWORD(lParam));
		break;
	case WM_MOVE:
		GetWindowRect(Chatter.hWnd, &(Chatter.rect));
		break;
	case WM_SETFOCUS:
		Chatter.SetFocus();
		break;
	case WM_KILLFOCUS:
		// Chatter.Focus(msg==WM_SETFOCUS?true:false);
		break;
	case WSA_ACCEPT:
		SOCKET tmpSock;
		tmpSock	= Socket.OnAccept(hWnd, wParam, lParam);
		if( Socket.m_sServer || Chatter.uFlags & ZXF_CALLER_OFF){
			Socket.LeaveWord(tmpSock, hWnd);
			break;

		}else{
			Socket.m_sServer	= tmpSock;
		}
		Beep(1000, 500);
		Chatter.ActiveMe();
		Chatter.View("\r\n接收到呼叫：滴滴滴... ...\r\n");
		Socket.m_iFlags	= CALL;
		break;
	case WSA_READ:
		if(Socket.m_sServer == (SOCKET)wParam)
			Socket.OnRead(hWnd, wParam, lParam);
		else
			;//Socket.LeaveWord((SOCKET)wParam, false); CSocket
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDK_RETURN:
			Chatter.OnReturn();
			break;

		case ZXM_CALL:
		case ZXM_TELNET:
		case ZXM_EXIT:
		case ZXM_DISABLE:
		case ZXM_TOPMOST:
		case ZXM_ABOUT:
		case ZXM_QUIT:
			Chatter.OnClickMenuItem(LOWORD(wParam));
			break;

		default:
			break;
		}
		if (HIWORD(wParam)==EN_SETFOCUS)
			Chatter.SetFocus();
		break;
	
	case WM_ZX_ICON:
		Chatter.OnClickIcon(lParam);
		break;

	case WM_CLOSE:
	case WM_DESTROY:
		if(Socket.m_sServer)
			Chatter.Input("将★紫虾ＢＰCALL★丢掉了... ... :~~(");

		Shell_NotifyIcon(NIM_DELETE, &(Chatter.NotifyIconData));
		DestroyWindow(Chatter.hInput);
		DestroyWindow(Chatter.hView);
		DestroyMenu(Chatter.hMenu);
		PostQuitMessage(0);
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

bool InitApp(HINSTANCE hInstance)
{
	WNDCLASS wc;

	wc.style		= CS_NOCLOSE;
	wc.lpfnWndProc	= ChatterProc;
	wc.cbClsExtra	= 0;
	wc.cbWndExtra	= 0;
	wc.hInstance	= hInstance;
	wc.hIcon		= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
	wc.hCursor		= LoadCursor(hInstance, MAKEINTRESOURCE(IDC_CURSOR_MOVE));
	wc.hbrBackground= CreatePatternBrush(LoadBitmap(hInstance, MAKEINTRESOURCE(IDB_BACKGROUND))); //(HBRUSH)GetStockObject(NULL_BRUSH);
	wc.lpszMenuName = NULL;
	wc.lpszClassName= CHATTER_NAME;

	if(!RegisterClass((LPWNDCLASS)&wc)){
		return false;
	}
	return true;
}

bool InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	if(!(Chatter.CreateWnd(hInstance)&&Chatter.CreatePopupMenu()))
		return false;
	
	Chatter.ShowWindow();
	Chatter.View(ZIXIA_MESSAGE);
	return true;
}

BOOL CALLBACK RemoteDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	char szTemp[MAX_STR_LENGTH]; // WM_CTLCOLOR

	switch(msg){
	case WM_INITDIALOG:
		switch(Socket.m_iFlags){
		case CALL:
			EnableWindow(GetDlgItem(hDlg, IDC_REMOTE), true);
			EnableWindow(GetDlgItem(hDlg, IDC_PORT), false);
			EnableWindow(GetDlgItem(hDlg, IDC_ID), true);
			SetWindowText(GetDlgItem(hDlg, IDC_REMOTE), Chatter.m_szRemote);
			SetWindowText(GetDlgItem(hDlg, IDC_ID), Chatter.m_szID);
			break;
		case TELNET:
			EnableWindow(GetDlgItem(hDlg, IDC_REMOTE), true);
			EnableWindow(GetDlgItem(hDlg, IDC_ID), false);
			EnableWindow(GetDlgItem(hDlg, IDC_PORT), true);
			SetWindowText(GetDlgItem(hDlg, IDC_REMOTE), Chatter.m_szRemote);
			SetWindowText(GetDlgItem(hDlg, IDC_PORT), itoa(23, szTemp, 10)); //atoi
			SetWindowText(GetDlgItem(hDlg, IDC_ID), Chatter.m_szID);
			break;
		case GET_NAME:
			EnableWindow(GetDlgItem(hDlg, IDC_REMOTE), false);
			EnableWindow(GetDlgItem(hDlg, IDC_ID), true);
			EnableWindow(GetDlgItem(hDlg, IDC_PORT), false);
			EnableWindow(GetDlgItem(hDlg, IDCANCLE), false);
			SetWindowText(GetDlgItem(hDlg, IDC_ID), Chatter.m_szID);
			break;
		}
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
			Chatter.m_szRemote[0]	= 0;
			Chatter.m_szID[0]		= 0;
			Chatter.m_nPort			= 0;

			switch(Socket.m_iFlags){
			case CALL:
				Chatter.m_nPort	= LISTEN_PORT;
				GetDlgItemText(hDlg, IDC_REMOTE, Chatter.m_szRemote, 80);
				GetDlgItemText(hDlg, IDC_ID, Chatter.m_szID, 80);
				break;
			case TELNET:
				GetDlgItemText(hDlg, IDC_PORT, Chatter.m_szRemote,80);
				Chatter.m_nPort	= atoi(Chatter.m_szRemote);

				GetDlgItemText(hDlg, IDC_REMOTE, Chatter.m_szRemote, 80);

				Chatter.GetUserName();
				
				break;
			case GET_NAME:
				GetDlgItemText(hDlg, IDC_ID, Chatter.m_szID, 80);
				strcpy(Chatter.m_szRemote, "Zixia");
				Chatter.m_nPort	= 23;
				break;
			}

			if( !Chatter.m_szRemote[0] || !Chatter.m_szID[0] || !Chatter.m_nPort){
				MessageBox(hDlg, "我想听的你不告诉我，还想叫我干活！？", "有些事情我还不明白：", MB_OK);
				return false;
			}
			EndDialog(hDlg, true);
			break;
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return false;
		default:
			break;
		}
		break;
	}
	return false;
}

BOOL CALLBACK AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//char szTemp[MAX_STR_LENGTH];

	switch(msg){
	case WM_INITDIALOG:
		break;
	case WM_COMMAND:
		switch(LOWORD(wParam)){
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
 			break;
		}
		break;
	}
	return 0;
}

DWORD WINAPI PendingLeaveWord(LPVOID lpParam)
{
	SOCKET sock;
	sock = *((SOCKET*)lpParam);

	WSAAsyncSelect(sock, Chatter.hWnd, NULL, NULL);	
	
	unsigned long	ulTmp	= 0;
	ulTmp	= ioctlsocket(sock, FIONBIO, &ulTmp);

	static unsigned int		nCount	= 0;
	char	szReceived[MAX_STR_LENGTH], 
		szReturn[MAX_STR_LENGTH],
		szBuf1[MAX_STR_LENGTH],
		szBuf2[128];
	char	szTime[128];
	char	szDate[128];

	_tzset();
    _strtime( szTime );
    _strdate( szDate );
	
	strcpy(szBuf1,"\r\n你呼叫的机器无法接受你的呼叫，\r\n您可以给他（她）留言三句^_^，\r\n请输入：\r\n");
	::send(sock, szBuf1, strlen(szBuf1), NULL);
	::recv(sock, szReceived, sizeof(szReceived), NULL);
//	::recv(sock, szReceived, sizeof(szReceived), NULL); SetWindowRgn HRGN 

	for(int iTmp=0;iTmp<3;iTmp++){
		memset(szBuf1, 0, sizeof(szBuf1));
		::recv(sock, szBuf1, sizeof(szBuf1), NULL);
		strcat(szReceived, szBuf1);
	}

	memset(szBuf1, 0, sizeof(szBuf1));
	sscanf(szReceived, "%s: ", szBuf1);
	sprintf(szBuf2, "收到 %s 的留言：\r\n", szBuf1);
	strcpy(szReturn, szBuf2);
	sprintf(szBuf2, "%s\r\n", szReceived);
	strcat(szReturn, szBuf2);
	sprintf(szBuf2, "        时间：%s\r\n", szTime);
	strcat(szReturn, szBuf2);
	sprintf(szBuf2, "        日期：%s\r\n", szDate);
	strcat(szReturn, szBuf2);
	
	
	strcpy(szBuf1, "留言接收完毕, 以后见。:*>\r\n");
	::send(sock, szBuf1, sizeof(szBuf1), NULL );
	closesocket(sock);
	
	strcpy(szReceived,szReturn);
	strcpy(szReturn, "\r\n☆☆☆☆☆☆☆☆☆☆☆☆\r\n");
	strcat(szReturn, szReceived);
	strcat(szReturn, "☆☆☆☆☆☆☆☆☆☆☆☆\r\n");
	
	Chatter. View(szReturn);

	if(!(Chatter.uFlags&ZXF_CALLER_OFF)){
 		MessageBox(Chatter.hWnd, szReturn, "收到留言：", MB_OK);
	}

	return 0;
}
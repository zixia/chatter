
//WND_CHATTER.flags 的位值，表示 Chatter 的状态。ZXF: Zixia Flags
#define ZXF_ALWAYS_TOP		0x00000001		//总在最前面
#define ZXF_CALLER_OFF		0x00000002		//不让别人呼叫
#define ZXF_CALLER_VISIBLE	0x00000004		//现在窗口是否在桌面上（相对与桌面的 Visible：）
#define ZXF_CALLER_CONNECTED 0x00000008		//是否有活动连接
#define ZXF_CALLER_ACTIVE	0x00000010		//是否是当前窗口
#define ZXF_CALLER_SERVER	0x00000020		//是否为网络 Server
//#define ZXF_CALLER_CLIENT	0x00000040		//是否为网络 Client

//SysTray 接受鼠标消息
#define WM_ZX_ICON		(WM_USER + 100)			

//弹出菜单ID ZXM: Zixia Menus
#define ZXM_CALL			41000
#define ZXM_DISABLE			41001
#define ZXM_TOPMOST			41002
#define ZXM_ABOUT			41003
#define ZXM_QUIT			41004
#define ZXM_EXIT			41005
#define	ZXM_TELNET			41006

//控制ID
#define IDC_INPUT		1101	//输入窗口ID
#define IDC_VIEW		1102	//视窗口ID


class CSocket;

class CChatter{
public:
	HWND			hWnd;
	HWND			hInput;
	HWND			hView;
	HWND			hWndInsertAfter;
	HMENU			hMenu;
	RECT			rect;
	UINT			uFlags;
	NOTIFYICONDATA	NotifyIconData;
	HINSTANCE		hInst;
	HRGN			hRgn;

	HBRUSH			hBrush;
	//HBRUSH			hbrInput;

public:
	char			m_szRemote[80];
	char			m_szID[80];
	UINT			m_nPort;
	char			m_lpszLocalName[80];
	char			m_lpszLocalIP[80];

public:
	CChatter();
	~CChatter();

	bool Input(LPCTSTR szStr);
	bool View(LPCTSTR szStr);
	bool ViewChar(unsigned char* ch);

	bool CreatePopupMenu();
	void TrackPopupMenu();

	bool ShowRemoteDialog(HWND hWnd);

	bool CreateWnd(HINSTANCE hInstance);
	void ShowWindow();
	void SetFocus();
	void Focus(bool b);

	void OnReturn();
	void OnClickMenuItem(int nID);
	void OnClickIcon(LPARAM lParam);
	void OnSize(int nWidth, int nHeight);

	void SetServer(bool bServer);
	void GetUserName();
	void ActiveMe();
	void ShowAboutDialog();
	void DrawBackground(HDC hDC);
};

extern CChatter Chatter;
extern CSocket Socket;
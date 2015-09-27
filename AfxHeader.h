#include <windows.h>
#include <windowsx.h>
//#include <string.h>
//#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "resource.h"

//一些常数
#define INPUT_HEIGHT	20
#define CHATTER_HEIGHT	200
#define CHATTER_WIDTH	400
#define MAX_STR_LENGTH	512
#define VIEW_XO			44
#define VIEW_YO			44
#define INPUT_XO		90
#define INPUT_YO		160


#define CHATTER_NAME	"紫虾ＢＰCALL "
#define CHATTER_VER		"Ver 0.5Beta"
#define ZIXIA_MESSAGE	"\
〖欢迎使用〗\r\n\
    超级烂货之★紫虾ＢＰCall★  \r\n\
    CopyRight 1998.10 writen by Zixia\r\n\
【功能简介】\r\n\
    『用鼠标右键单击图标显示菜单 :)』\r\n\
    『如果对方的机器上面也运行了 Call 机的话，你可以呼叫它聊天』\r\n\
    『可以用 Telnet 功能玩 Mud 等(暂不支持颜色)』\r\n\
【Bugs】\r\n\
    『很多，呵呵，有什么建议欢迎和我联系』\r\n\
【注意】\r\n\
    『此Call机必须在你有写权的目录执行因为它要写你的id到zixia.dat』"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
bool InitApp(HINSTANCE);
bool InitInstance(HINSTANCE, int);
LRESULT WINAPI ChatterProc(HWND, UINT, WPARAM, LPARAM);
bool InitChatter(HINSTANCE);
BOOL CALLBACK RemoteDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI PendingLeaveWord(LPVOID lpParam);
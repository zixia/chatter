#include <windows.h>
#include <windowsx.h>
//#include <string.h>
//#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "resource.h"

//һЩ����
#define INPUT_HEIGHT	20
#define CHATTER_HEIGHT	200
#define CHATTER_WIDTH	400
#define MAX_STR_LENGTH	512
#define VIEW_XO			44
#define VIEW_YO			44
#define INPUT_XO		90
#define INPUT_YO		160


#define CHATTER_NAME	"��Ϻ�£�CALL "
#define CHATTER_VER		"Ver 0.5Beta"
#define ZIXIA_MESSAGE	"\
����ӭʹ�á�\r\n\
    �����û�֮����Ϻ�£�Call��  \r\n\
    CopyRight 1998.10 writen by Zixia\r\n\
�����ܼ�顿\r\n\
    ��������Ҽ�����ͼ����ʾ�˵� :)��\r\n\
    ������Է��Ļ�������Ҳ������ Call ���Ļ�������Ժ��������졻\r\n\
    �������� Telnet ������ Mud ��(�ݲ�֧����ɫ)��\r\n\
��Bugs��\r\n\
    ���ܶ࣬�Ǻǣ���ʲô���黶ӭ������ϵ��\r\n\
��ע�⡿\r\n\
    ����Call������������дȨ��Ŀ¼ִ����Ϊ��Ҫд���id��zixia.dat��"


int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int);
bool InitApp(HINSTANCE);
bool InitInstance(HINSTANCE, int);
LRESULT WINAPI ChatterProc(HWND, UINT, WPARAM, LPARAM);
bool InitChatter(HINSTANCE);
BOOL CALLBACK RemoteDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
BOOL CALLBACK AboutDlgProc( HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam);
DWORD WINAPI PendingLeaveWord(LPVOID lpParam);
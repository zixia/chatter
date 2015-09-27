#include "AfxHeader.h"
#include "ClientSocket.h"
#include "ChatClient.h"

/////
//#include <memory.h>


#define 		M(a)		MessageBox(hWnd,a,"msg",MB_OK)
/////
// class CChatter;

CSocket::CSocket(CChatter* pChatter)
{
	m_pChatter		= pChatter;
    m_acc_sin_len	= sizeof( m_acc_sin );
	
	WSADATA WsaData;
	if(WSAStartup(MAKEWORD(1,1), &WsaData)){
		MessageBox(m_pChatter->hWnd, "请确认您的通讯网络设置正常。\r\n如果有其他问题，请与 Zixia@SMTH 联系", "网络启动失败：", MB_OK);
		PostQuitMessage(0);
	}
	
}

CSocket::~CSocket()
{
	m_pChatter	= NULL;
	WSACleanup();
}

/////////

bool CSocket::Listen(HWND hWnd, bool bBind)
{	
	m_sListen	= socket(AF_INET, SOCK_STREAM, 0);
	if(m_sListen	== INVALID_SOCKET){
		M("create socket error");
		return false;
	}
	
	m_local_sin.sin_family	= AF_INET;
	
	gethostname(m_szBuffer,sizeof(m_szBuffer));
	
	m_local_sin.sin_addr.s_addr	= INADDR_ANY;
	
	m_local_sin.sin_port			= htons(LISTEN_PORT);
	
	if(true){	
		if(bind(m_sListen, (struct sockaddr FAR*)&m_local_sin, sizeof(m_local_sin))==SOCKET_ERROR){
			sprintf(m_szBuffer,"%d is errro",WSAGetLastError());
			M(m_szBuffer);
			return false;
		}
	}
	
	if(listen(m_sListen, 5)<0){
		M("listen error");
		return false;
	}
	
	if(m_status= WSAAsyncSelect(m_sListen, hWnd, WSA_ACCEPT, FD_ACCEPT) > 0){
		M("WSAAysncSelect Error");
		return false;
	}
	
	return true;
	
	
}

SOCKET CSocket::OnAccept(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	if (WSAGETSELECTERROR( lParam ) != 0) {
		WSAAsyncSelect( m_sListen, hWnd, 0, 0);
		return NULL;
	}
	
	m_acc_sin_len	= sizeof(m_acc_sin);
	m_sAccept = accept( m_sListen,(struct sockaddr FAR *) &m_acc_sin,
		(int FAR *) &m_acc_sin_len );
	
	if (m_sAccept == INVALID_SOCKET) {
		return NULL;
	}
	
	if(!m_sServer){
		if ((m_status = WSAAsyncSelect( m_sAccept, hWnd, WSA_READ, FD_READ | FD_CLOSE )) > 0){
			return NULL;
		}
	}
		
	return m_sAccept;
}

bool CSocket::OnRead(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch(Socket.m_iFlags){
	case TELNET:
		{
			SOCKET sock		= (SOCKET)wParam;
			int				n;
			unsigned char	c[2];
			c[1]=0;
			unsigned char	cmd, opt;
			BOOL echo	= false;
			char	term[]="dumb";     /* 终端类型: 哑终端   */ 
			n=recv(sock,(char*)c, 1, NULL);           /* 读取一个字节           */ 
			if( n<=0 ) 
				return false;
			if (*c == 255)                  // IAC字符,后跟telnet命令和协商选项 
			{ 
				cmd = receiveChar();         //读入命令 
				opt = receiveChar();         //读入选项 
				switch(opt) 
				{ 
				case 1:            // echo协商选项 
					switch(cmd) 
					{
					case 253:    //处理IAC DO ECHO 
						echo=TRUE; 
						sendChar(255); //发送 IAC WILL ECHO 
						sendChar(251); 
						sendChar(1); 
						break; 
					case 254:   //处理IAC DON'T ECHO 
						echo=FALSE; 
						sendChar(255);  // 发送 IAC WON'T ECHO 
						sendChar(252); 
						sendChar(1); 
						break; 
					case 251:    //处理IAC WILL  ECHO 
						sendChar(255);  // 发送 IAC DO  ECHO 
						sendChar(253); 
						sendChar(1); 
					} 
					break; 
					case 3:          // supress go-ahead(抑制向前选项) 
						break; 
					case 24:        // terminal type(终端类型选项) 
						if (cmd == 253) 
						{ 
							// IAC WILL terminal-type 将告诉server终端类型 
							sendChar(255); 
							sendChar(251); 
							sendChar(24); 
							// IAC SB terminal-type IS <term> IAC SE 
							sendChar(255); //传送终端类型字符串 
							sendChar(250); 
							sendChar(24); 
							sendChar(0); 
							sendString(term); 
							sendChar(255); 
							sendChar(240); 
						} 
						else if (cmd == 250) 
						{          //SB和SE要配对出现 
							while(receiveChar() != 240) 
								; 
						} 
						break; 
						
					default:        // some other command1 
						if (cmd == 253) 
						{ 
							// IAC DONT whatever 
							sendChar(255); //其它的协商选项均不同意 
							sendChar(252); 
							sendChar(opt); 
						} 
				} 
			} 
			else                                     //处理字符数据 
			{
				//MessageBox(Chatter.hWnd, (const char*)c, "receive", MB_OK);
				Chatter.ViewChar(c);
			}
		}
		break;
	case CALL:
		if (WSAGETSELECTEVENT(lParam) == FD_READ){ // Received Data
			m_status = recv((SOCKET)wParam, m_szBuffer, sizeof(m_szBuffer), NULL);
			if ((SOCKET)wParam != m_sServer)
				return false;
			if (m_status) {
				m_szBuffer[ m_status ] = '\0';
				
				m_pChatter->View(m_szBuffer);
				
			}else{
				closesocket((SOCKET)wParam);
				m_sServer	= NULL;
				return false;
			}
			return true;
		} // Socket Closed.
		m_sServer	= NULL;
		return false;
		break;
	}
	return false;
}

bool CSocket::Connect(HWND hWnd, LPSTR szRemote, int nPort)
{
	if(m_sServer){
		MessageBox(Chatter.hWnd, "已经存在连接，暂时不支持多人 chat. :( ", "等待升级吧！*_^", MB_OK);
		return false;
	}
	SOCKADDR_IN dest_sin;  /* DESTination Socket INternet  SOCKET */
	PHOSTENT phe;
	
	memset(&dest_sin, 0, sizeof(SOCKADDR_IN));

	gethostname(Chatter.m_lpszLocalName, sizeof(Chatter.m_lpszLocalName) );

	strcpy(Chatter.m_lpszLocalIP, inet_ntoa( *(LPIN_ADDR)* ((gethostbyname(Chatter.m_lpszLocalName))->h_addr_list)) );

	phe = gethostbyname(szRemote);

	if(!phe){
		MessageBox(Chatter.hWnd, "如果你给的地址没有错误的话，那就是 Call 机的通讯电路出现了毛病。", "Call 机冒烟乐！", MB_OK);
		closesocket(m_sConnect);
		return false;
	}


	if( !strcmp(Chatter.m_lpszLocalIP ,
		inet_ntoa( *(LPIN_ADDR)*(phe->h_addr_list) )
		)
	){
   		MessageBox(Chatter.hWnd, "　　从前呀，有一个 MM ，天天想着有个 GG 来陪自己聊天。可是她等呀等呀，却没有一个 GG 来陪她……　于是呢，她就开始每天自己和自己聊天了……", "好无聊呀泥！*_^", MB_OK);
		return false;
	}

	m_sConnect = socket( AF_INET, SOCK_STREAM, 0);
	
	if (m_sConnect == INVALID_SOCKET) {
		MessageBox(Chatter.hWnd, "Call 机信号生成器错误……", "呼机冒烟了！", MB_OK);
		return false;
	}
	
	dest_sin.sin_family		= AF_INET;
    
	for(int i=strlen(szRemote);i>=0;i--){
		if(szRemote[i-1]==' ')
			szRemote[i-1]	= 0;
	}

	if(IsIP(szRemote)){
		dest_sin.sin_addr.s_addr	= inet_addr(szRemote);

	}else{
		memcpy((char FAR *)&(dest_sin.sin_addr), phe->h_addr, phe->h_length);
	}
	
	dest_sin.sin_port	= htons(nPort);
	
    bind( m_sConnect, (SOCKADDR*)&dest_sin, sizeof(dest_sin));

	if (connect( m_sConnect, (PSOCKADDR) &dest_sin, sizeof( dest_sin)) != 0) {
		closesocket( m_sConnect);
		MessageBox(Chatter.hWnd, "对方有呼机吗？我呼不到它耶……", "呼叫失败！", MB_OK);
		return false;
	}

	
	if ((m_status = WSAAsyncSelect( m_sConnect, m_pChatter->hWnd, WSA_READ, FD_READ | FD_CLOSE )) > 0) {
		MessageBox( m_pChatter->hWnd, "信号事件选择器发生错误……", "呼机冒烟了！", MB_OK);
		closesocket( m_sConnect );
		return false;
	}
	m_sServer = m_sConnect;

	Chatter.ActiveMe();
	Chatter.View("\r\n连接成功！\r\n");
	if(Socket.m_iFlags == CALL){
		sprintf(m_szBuffer, "从 %s(%s) 连入！", Chatter.m_lpszLocalName, Chatter.m_lpszLocalIP);
		Chatter.Input(m_szBuffer);
	}

	return true;
	
}

bool CSocket::Send(LPCTSTR szStr)
{
	m_status	= ::send(m_sServer, szStr, strlen(szStr), NULL);
	return ((m_status==SOCKET_ERROR)?false:true);
}

bool CSocket::IsIP(LPSTR szRemote)
{
	int iLen;

	iLen	= strlen(szRemote);
	for(int i=0;i<iLen;i++){
		if(!isdigit(szRemote[i]) && szRemote[i]!='.')
			return false;
	}
	return true;
}

void CSocket::LeaveWord(SOCKET& sock, HWND hWnd)
{
	CreateThread(NULL, 0, PendingLeaveWord, (LPVOID)&sock, 0, &m_iThreadIdentifier);
}

char CSocket::receiveChar()
{
	char	c;
	recv(m_sServer, &c, 1, NULL);
	return c;
}

void CSocket::sendChar(const int ch)
{
	send(m_sServer, (char*)&ch, 1, NULL);
}

void CSocket::sendString(char p[])       //从socket向server发送一个字符串 
{ 
	send(m_sServer, p, strlen(p), NULL);
} 

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
		MessageBox(m_pChatter->hWnd, "��ȷ������ͨѶ��������������\r\n������������⣬���� Zixia@SMTH ��ϵ", "��������ʧ�ܣ�", MB_OK);
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
			char	term[]="dumb";     /* �ն�����: ���ն�   */ 
			n=recv(sock,(char*)c, 1, NULL);           /* ��ȡһ���ֽ�           */ 
			if( n<=0 ) 
				return false;
			if (*c == 255)                  // IAC�ַ�,���telnet�����Э��ѡ�� 
			{ 
				cmd = receiveChar();         //�������� 
				opt = receiveChar();         //����ѡ�� 
				switch(opt) 
				{ 
				case 1:            // echoЭ��ѡ�� 
					switch(cmd) 
					{
					case 253:    //����IAC DO ECHO 
						echo=TRUE; 
						sendChar(255); //���� IAC WILL ECHO 
						sendChar(251); 
						sendChar(1); 
						break; 
					case 254:   //����IAC DON'T ECHO 
						echo=FALSE; 
						sendChar(255);  // ���� IAC WON'T ECHO 
						sendChar(252); 
						sendChar(1); 
						break; 
					case 251:    //����IAC WILL  ECHO 
						sendChar(255);  // ���� IAC DO  ECHO 
						sendChar(253); 
						sendChar(1); 
					} 
					break; 
					case 3:          // supress go-ahead(������ǰѡ��) 
						break; 
					case 24:        // terminal type(�ն�����ѡ��) 
						if (cmd == 253) 
						{ 
							// IAC WILL terminal-type ������server�ն����� 
							sendChar(255); 
							sendChar(251); 
							sendChar(24); 
							// IAC SB terminal-type IS <term> IAC SE 
							sendChar(255); //�����ն������ַ��� 
							sendChar(250); 
							sendChar(24); 
							sendChar(0); 
							sendString(term); 
							sendChar(255); 
							sendChar(240); 
						} 
						else if (cmd == 250) 
						{          //SB��SEҪ��Գ��� 
							while(receiveChar() != 240) 
								; 
						} 
						break; 
						
					default:        // some other command1 
						if (cmd == 253) 
						{ 
							// IAC DONT whatever 
							sendChar(255); //������Э��ѡ�����ͬ�� 
							sendChar(252); 
							sendChar(opt); 
						} 
				} 
			} 
			else                                     //�����ַ����� 
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
		MessageBox(Chatter.hWnd, "�Ѿ��������ӣ���ʱ��֧�ֶ��� chat. :( ", "�ȴ������ɣ�*_^", MB_OK);
		return false;
	}
	SOCKADDR_IN dest_sin;  /* DESTination Socket INternet  SOCKET */
	PHOSTENT phe;
	
	memset(&dest_sin, 0, sizeof(SOCKADDR_IN));

	gethostname(Chatter.m_lpszLocalName, sizeof(Chatter.m_lpszLocalName) );

	strcpy(Chatter.m_lpszLocalIP, inet_ntoa( *(LPIN_ADDR)* ((gethostbyname(Chatter.m_lpszLocalName))->h_addr_list)) );

	phe = gethostbyname(szRemote);

	if(!phe){
		MessageBox(Chatter.hWnd, "�������ĵ�ַû�д���Ļ����Ǿ��� Call ����ͨѶ��·������ë����", "Call ��ð���֣�", MB_OK);
		closesocket(m_sConnect);
		return false;
	}


	if( !strcmp(Chatter.m_lpszLocalIP ,
		inet_ntoa( *(LPIN_ADDR)*(phe->h_addr_list) )
		)
	){
   		MessageBox(Chatter.hWnd, "������ǰѽ����һ�� MM �����������и� GG �����Լ����졣��������ѽ��ѽ��ȴû��һ�� GG �����������������أ����Ϳ�ʼÿ���Լ����Լ������ˡ���", "������ѽ�࣡*_^", MB_OK);
		return false;
	}

	m_sConnect = socket( AF_INET, SOCK_STREAM, 0);
	
	if (m_sConnect == INVALID_SOCKET) {
		MessageBox(Chatter.hWnd, "Call ���ź����������󡭡�", "����ð���ˣ�", MB_OK);
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
		MessageBox(Chatter.hWnd, "�Է��к������Һ�������Ү����", "����ʧ�ܣ�", MB_OK);
		return false;
	}

	
	if ((m_status = WSAAsyncSelect( m_sConnect, m_pChatter->hWnd, WSA_READ, FD_READ | FD_CLOSE )) > 0) {
		MessageBox( m_pChatter->hWnd, "�ź��¼�ѡ�����������󡭡�", "����ð���ˣ�", MB_OK);
		closesocket( m_sConnect );
		return false;
	}
	m_sServer = m_sConnect;

	Chatter.ActiveMe();
	Chatter.View("\r\n���ӳɹ���\r\n");
	if(Socket.m_iFlags == CALL){
		sprintf(m_szBuffer, "�� %s(%s) ���룡", Chatter.m_lpszLocalName, Chatter.m_lpszLocalIP);
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

void CSocket::sendString(char p[])       //��socket��server����һ���ַ��� 
{ 
	send(m_sServer, p, strlen(p), NULL);
} 

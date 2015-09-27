//#include "ChatClient.h"

#define		WSA_ACCEPT		(WM_USER+200)
#define		WSA_READ		(WM_USER+201)

#define MAX_PENDING_CONNECTS 4  /* The backlog allowed for listen() */
#define NO_FLAGS_SET         0  /* Used with recv()/send()          */
#define LISTEN_PORT			 42079

#define TELNET			1
#define CALL			2
#define	GET_NAME		3

class CChatter;

class CSocket
{
private:
	char	receiveChar();
	void	sendChar(const int ch);
	void	sendString(char p[]);       //从socket向server发送一个字符串 

public:
	SOCKET		m_sListen;
	SOCKET		m_sAccept;
	SOCKET		m_sServer;
	SOCKET		m_sConnect;
//	PHOSTENT	m_pHostent;
	CChatter*	m_pChatter;

	unsigned long	m_lAddr;

	int			m_iFlags;
	
private:
	int			m_status;
	char		m_szBuffer[ MAX_STR_LENGTH ];

	SOCKADDR_IN m_local_sin;  /* Local socket - internet style */
    SOCKADDR_IN m_acc_sin;    /* Accept socket address - internet style */
    int			m_acc_sin_len;        /* Accept socket address length */

	DWORD		m_iThreadIdentifier;

public:
	CSocket(CChatter*);
	~CSocket();

	SOCKET OnAccept(HWND hWnd, WPARAM wParam, LPARAM lParam);
	bool OnRead(HWND hWnd, WPARAM wParam, LPARAM lParam);

	bool Listen(HWND hWnd, bool bBind);
	bool Connect(HWND hWnd, LPSTR szRemote, int nPort);

	bool Send(LPCTSTR szStr);
	void LeaveWord(SOCKET& sock, HWND hWnd);
	bool IsIP(LPSTR szRemote);
};

extern CSocket Socket;
extern CChatter Chatter;
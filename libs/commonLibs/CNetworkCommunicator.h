#pragma once

/*
	제작 : 정규호

	Boost::asio, Boost::thread 를 이용하여 비동기 소켓 라이브러리 

*/

#include <vector>
#include <string>

#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include "IoServicePool.hpp"

#include "commonSocket.h"

const LPCSTR			IMONCNA_PACKET_IDENTITY = "iMonCNA";
const LPCSTR			IMONCNAV2_PACKET_IDENTITY = "iMonCNAv2";

typedef int SOCK_INDEX;
typedef void (*tySigNewConnection)( const PVOID, const SOCK_INDEX, const tySpCommonSocket& );
typedef void (*tySigDeleteConnection)( const PVOID, const SOCK_INDEX );

/*!
	CCommonSocket 클래스를 이용하여 TCP 통신을 관리하는 클래스

	해당 클래스는 기본적으로 내부적으로 사용되는 소켓에 대해서 신경쓰지 않고 사용할 수 있도록 되어있음. 
	서버용으로 사용하려면 bind_and_accept 함수를 호출하여 특정 포트에 대하여 연결을 대기하도록 하면 되며 클라이언트용으로 사용하려면 connectTo 함수를 이용하여 서버에 접속을 시도한다. 

	사내에서 구글 프로토콜 버퍼를 이용한 메시지 전송/수신의 규칙 몇 가지 

	1. 클라이언트는 각 메시지를 구분할 수 있도록 각 메시지마다 고유의 메시지 번호를 부여한다. ( 패킷의 msgId 필드, 소켓 클래스내에서 자동으로 부여함 )
	2. 서버에서 특정 패킷에 대해 응답을 전송할 때는 요청 패킷과 같은 메시지 번호를 사용한다. ( 예). 2번 메시지 번호에 대한 응답은 2번 메시지 번호를 그대로 사용 )

*/
class IMONSOCKET_DLL_API CNetworkCommunicator
{
public:
	CNetworkCommunicator( boost::asio::io_service& ioService );
	~CNetworkCommunicator();

	/// 소켓이 연결 입력을 받을 수 있도록 바인딩함. ipAddress 에 *.*.*.* 을 선택하면 모든 IP 의 연결을 허용함
	bool bind_and_accept( unsigned short portNumber, const std::string& ipAddress = "*.*.*.*" );
	/*!
		지정한 주소에 연결합니다. 

		@return <= 0, 연결 실패
		@return > 0, 해당 소켓에 대한 색인번호
	*/
	SOCK_INDEX connectTo( const std::string& ipAddress, unsigned short portNumber );
	/*!
		지정한 소켓 번호에 연결된 소켓의 네트워크 연결을 끊습니다. 

		해당 번호에 연결된 소켓이 없으면 아무 일도 일어나지 않습니다
	*/
	void disconnect( SOCK_INDEX socketIndex );
	void disconnectAll();

	SOCK_INDEX getSocketIndex( const CCommonSocket* connection );
    SOCK_INDEX getSocketIndex( tySpCommonSocket connection );

	tySpCommonSocket getSocket( SOCK_INDEX socketIndex )
	{	
        EnterCriticalSection( &_csMapIndexToSocket );
        if( _mapIndexToSocket.find( socketIndex ) == _mapIndexToSocket.end() )
        {
            LeaveCriticalSection( &_csMapIndexToSocket );
            return CCommonSocket::createSocket( CIoServicePool::getInstance()->getNextIoService() );
        }
        tySpCommonSocket _socket = _mapIndexToSocket[ socketIndex ];
        LeaveCriticalSection( &_csMapIndexToSocket );
        return _socket;
    }

	void registerNewConnectionNotify( tySigNewConnection sltMessage, const PVOID pParam );
	void registerDisconnectionNotify( tySigDeleteConnection sltMessage, const PVOID pParam );

private:
	static void receiveDisconnectNotify( const PVOID pParam, tySpCommonSocket connection );

	std::pair< SOCK_INDEX, tySpCommonSocket >		createSocket();

	void socket_accept( const boost::system::error_code& err, tySpCommonSocket conn );
	SOCK_INDEX registerSocket( const tySpCommonSocket& connection );
    void deregisterSocket( const SOCK_INDEX socketIndex );

	// 클라이언트로부터 연결을 받기 위한 변수들
	boost::asio::ip::tcp::acceptor												_acceptor;
	boost::asio::ip::tcp::endpoint												_endpoint;

	CRITICAL_SECTION															_csMapIndexToSocket;
	long																		_socketIndex;
	boost::unordered_map< SOCK_INDEX, tySpCommonSocket >			_mapIndexToSocket;

	std::vector< std::pair< tySigNewConnection, PVOID > >						_vecSltMessage;
	std::vector< std::pair< tySigDeleteConnection, PVOID > >					_vecSltDisconnect;
};

//////////////////////////////////////////////////////////////////////////

/*!
	DLL 에서 new 로 할당한 메모리는 반드시 DLL 에서 해제해야하며, 이를 주 스레드 등 다른 스레드에서 해제하면 문제가 발생할 수 있다. 
	
	하지만, 이에 대한 사항을 잊지 않고 점검하여 DLL 에서 마련해둔 해제 코드에 정상적으로 포인터를 넘기는 것이 번거럽고 잊기 쉽기 때문에, 

	이를 자동적으로 처리해주는 boost::shared_ptr 를 이용하여 전달함.
*/

IMONSOCKET_DLL_API boost::shared_ptr< CNetworkCommunicator > createNetworkCommunicator();

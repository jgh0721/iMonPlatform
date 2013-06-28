#include "stdafx.h"

#include "CNetworkCommunicator.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnConvert;

CNetworkCommunicator::CNetworkCommunicator( boost::asio::io_service& ioService ) 
	: _acceptor( ioService ), _socketIndex( 0 )
{
	SYSTEM_INFO sysinfo;
	ZeroMemory( &sysinfo, sizeof(SYSTEM_INFO) );
	GetSystemInfo( &sysinfo );

	CIoServicePool::getInstance( sysinfo.dwNumberOfProcessors )->run( );

	InitializeCriticalSectionAndSpinCount( &_csMapIndexToSocket, 4000 );

	LM_TRACE_TO( "network/networkCommunicator", L"%1", L"네트워크 관리자 생성 완료" );
}

CNetworkCommunicator::~CNetworkCommunicator()
{
	CIoServicePool::getInstance()->stop();

	DeleteCriticalSection( &_csMapIndexToSocket );

	LM_TRACE_TO( "network/networkCommunicator", L"%1", L"네트워크 관리자 삭제 완료" );
}

bool CNetworkCommunicator::bind_and_accept( unsigned short portNumber, const std::string& ipAddress /*= "*.*.*.*" */ )
{
	bool isSuccess = false;

	do 
	{
		boost::system::error_code ec;
		assert( portNumber > 0 && portNumber <= 65535 );

		std::pair< SOCK_INDEX, tySpCommonSocket > acceptSocket = createSocket();

		if( ipAddress.empty() == true || ipAddress.compare( "*.*.*.*" ) == 0 )
			_endpoint = boost::asio::ip::tcp::endpoint( boost::asio::ip::tcp::v4(), portNumber );
		else
			_endpoint = boost::asio::ip::tcp::endpoint( boost::asio::ip::address_v4::from_string( ipAddress ), portNumber );

		_acceptor.open( _endpoint.protocol() );
		_acceptor.set_option( boost::asio::ip::tcp::acceptor::reuse_address( true ) );
		_acceptor.bind( _endpoint, ec );

		if( ec )
		{
			LM_ERROR_TO( "network/networkCommunicator", L"소켓 오류 : %1, %2", ec.value(), CA2U( ec.message() ) );

            acceptSocket.second->socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
            acceptSocket.second->socket().close();
			break;
		}

		_acceptor.async_accept( 
			acceptSocket.second->socket(), 
			boost::bind( &CNetworkCommunicator::socket_accept, this, boost::asio::placeholders::error, acceptSocket.second ) );

		_acceptor.listen();

		LM_TRACE_TO( "network/networkCommunicator", L"IP : %1, 포트번호 : %2 로 연결을 대기합니다", 
			ipAddress.empty() == true || ipAddress.compare( "*.*.*.*" ) == 0 ? L"모든주소" : CA2U( ipAddress ).c_str(),
			portNumber );

		isSuccess = true;
	} while( false );

	return isSuccess;
}

SOCK_INDEX CNetworkCommunicator::connectTo( const std::string& ipAddress, unsigned short portNumber )
{
	assert( ipAddress.empty() == false );
	assert( portNumber > 0 );
	assert( portNumber < 65535 );

	boost::asio::io_service& ioService = CIoServicePool::getInstance()->getNextIoService();

	boost::asio::ip::tcp::resolver				resolver( ioService );
	boost::asio::ip::tcp::resolver::query		queryHost( ipAddress, format( "%d", portNumber ) );
	boost::asio::ip::tcp::resolver::iterator	endpoint = resolver.resolve( queryHost );

	boost::system::error_code error = boost::asio::error::host_not_found;

	std::pair< SOCK_INDEX, tySpCommonSocket > connectSocket = createSocket();

	connectSocket.second->socket().connect( *endpoint, error );

	if( error )
	{
        connectSocket.second->socket().close();
		LM_ERROR_TO( "network/networkCommunicator", L"주소 : %1:%2 에 접속하는데 실패하였습니다 - %3", CA2U( ipAddress ), portNumber, CA2U( error.message() ) );

        connectSocket.second->socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, error );
        connectSocket.second->socket().close();

        deregisterSocket( connectSocket.first );

        return -1;
	}

	for( std::vector< std::pair< tySigNewConnection, PVOID > >::iterator it = _vecSltMessage.begin(); it != _vecSltMessage.end(); ++it )
		(*it->first)( it->second, connectSocket.first, connectSocket.second );

	connectSocket.second->receivePacket();

	LM_DEBUG_TO( "network/networkCommunicator", L"주소 : %1:%2 에 접속하였습니다, 소켓 색인 번호 : %3", CA2U( ipAddress ), portNumber, connectSocket.first );

	return connectSocket.first;
}

void CNetworkCommunicator::disconnect( SOCK_INDEX socketIndex )
{
	EnterCriticalSection( &_csMapIndexToSocket );

	if( _mapIndexToSocket.find( socketIndex ) == _mapIndexToSocket.end() )
	{
		LeaveCriticalSection( &_csMapIndexToSocket );
		return ;
	}

//		_mapIndexToSocket.find( socketIndex )->second->socket().local_endpoint().address().to_string().c_str(),
// 	LM_DEBUG_TO( "network/networkCommunicator", "소켓 색인 번호 %1 연결 끊기 요청/통보, 소켓 주소 : %2:%3",
// 		socketIndex, 
// 		_mapIndexToSocket.find( socketIndex )->second->socket().local_endpoint().address().to_string().c_str(),
// 		_mapIndexToSocket.find( socketIndex )->second->socket().local_endpoint().port() );

	boost::system::error_code ec;

	_mapIndexToSocket.find( socketIndex )->second->socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
	_mapIndexToSocket.find( socketIndex )->second->socket().close();
	_mapIndexToSocket.erase( socketIndex );

	LeaveCriticalSection( &_csMapIndexToSocket );

	for( std::vector< std::pair< tySigDeleteConnection, PVOID > >::iterator it = _vecSltDisconnect.begin(); it != _vecSltDisconnect.end(); ++it )
		(*it->first)( it->second, socketIndex );
}

void CNetworkCommunicator::disconnectAll()
{
	EnterCriticalSection( &_csMapIndexToSocket );

	for ( boost::unordered_map< SOCK_INDEX, tySpCommonSocket >::iterator iter = _mapIndexToSocket.begin(); iter != _mapIndexToSocket.end(); ++iter)
	{
		if( _mapIndexToSocket.find( iter->first ) == _mapIndexToSocket.end() )
		{
			LeaveCriticalSection( &_csMapIndexToSocket );
			return ;
		}

		for( std::vector< std::pair< tySigDeleteConnection, PVOID > >::iterator it = _vecSltDisconnect.begin(); it != _vecSltDisconnect.end(); ++it )
			(*it->first)( it->second, iter->first );

		LM_DEBUG_TO( "network/networkCommunicator", "소켓 색인 번호 %1 연결 끊기 요청/통보",
			iter->first );

		boost::system::error_code ec;

		_mapIndexToSocket.find( iter->first )->second->socket().shutdown( boost::asio::ip::tcp::socket::shutdown_both, ec );
		_mapIndexToSocket.find( iter->first )->second->socket().close();
	}

	_mapIndexToSocket.clear();
	LeaveCriticalSection( &_csMapIndexToSocket );
}

void CNetworkCommunicator::socket_accept( const boost::system::error_code& err, tySpCommonSocket conn )
{
	if( !err )
	{
		// 정상적으로 연결을 받았을 때 
		int socketIndex = -1;

		EnterCriticalSection( &_csMapIndexToSocket );

		for( boost::unordered_map< SOCK_INDEX, tySpCommonSocket >::iterator it = _mapIndexToSocket.begin(); it != _mapIndexToSocket.end(); ++it)
		{
			if( it->second == conn )
			{
				socketIndex = it->first;
				break;
			}
		}

		LeaveCriticalSection( &_csMapIndexToSocket );

		if( socketIndex == -1 )
			socketIndex = registerSocket( conn );

		boost::system::error_code errSocket;
		boost::asio::ip::tcp::endpoint remote_endpoint = conn->socket().remote_endpoint( errSocket );

		if( !errSocket )
		{
			LM_TRACE_TO( "network/networkCommunicator", L"주소 %1:%2 와 연결이 생성되었습니다, 소켓 색인번호 : %3", 
				CA2U( remote_endpoint.address().to_string() ),
				remote_endpoint.port(), socketIndex );
		}

		conn->receivePacket();

		for( std::vector< std::pair< tySigNewConnection, PVOID > >::iterator it = _vecSltMessage.begin(); it != _vecSltMessage.end(); ++it )
			(*it->first)( it->second, socketIndex, conn );
	}
	else
	{
		// 오류 발생
		LM_ERROR_TO( "network/networkCommunicator", L"클라이언트 소켓 연결 실패 : %1", CA2U( err.message() ) );
	}

	// 새로운 연결 대기
	std::pair< SOCK_INDEX, tySpCommonSocket > acceptSocket = createSocket();
	_acceptor.async_accept( 
		acceptSocket.second->socket(), 
		boost::bind( &CNetworkCommunicator::socket_accept, this, boost::asio::placeholders::error, acceptSocket.second ) );
}

std::pair< int, tySpCommonSocket > CNetworkCommunicator::createSocket()
{
	boost::asio::io_service& ioService = CIoServicePool::getInstance()->getNextIoService();

	tySpCommonSocket commonSocket = CCommonSocket::createSocket( ioService );

	int socketIndex = registerSocket( commonSocket );

	LM_TRACE_TO( "network/networkCommunicator", L"소켓 생성 완료, 소켓 색인 번호 : %1", socketIndex );

	return std::make_pair( socketIndex, commonSocket );
}

int CNetworkCommunicator::registerSocket( const tySpCommonSocket& connection )
{
	EnterCriticalSection( &_csMapIndexToSocket );

	InterlockedIncrement( &_socketIndex );
	while( _mapIndexToSocket.find( _socketIndex ) != _mapIndexToSocket.end() )
		InterlockedIncrement( &_socketIndex );

	_mapIndexToSocket.insert( std::make_pair( _socketIndex, connection ) );

	LeaveCriticalSection( &_csMapIndexToSocket );

	connection->registerDisconnect( &CNetworkCommunicator::receiveDisconnectNotify, this );

	return _socketIndex;
}

void CNetworkCommunicator::deregisterSocket( const SOCK_INDEX socketIndex )
{
    EnterCriticalSection( &_csMapIndexToSocket );

    while( _mapIndexToSocket.find( _socketIndex ) != _mapIndexToSocket.end() )
        _mapIndexToSocket.erase( socketIndex );

    LeaveCriticalSection( &_csMapIndexToSocket );
}

int CNetworkCommunicator::getSocketIndex( const CCommonSocket* connection )
{
	EnterCriticalSection( &_csMapIndexToSocket );

	for( boost::unordered_map< SOCK_INDEX, tySpCommonSocket >::iterator it = _mapIndexToSocket.begin(); it != _mapIndexToSocket.end(); ++it )
	{
		if( it->second.get() == connection )
		{
			LeaveCriticalSection( &_csMapIndexToSocket );
			return it->first;
		}
	}

	LeaveCriticalSection( &_csMapIndexToSocket );
	return -1;
}

SOCK_INDEX CNetworkCommunicator::getSocketIndex( tySpCommonSocket connection )
{
    EnterCriticalSection( &_csMapIndexToSocket );

    for( boost::unordered_map< SOCK_INDEX, tySpCommonSocket >::iterator it = _mapIndexToSocket.begin(); it != _mapIndexToSocket.end(); ++it )
    {
        if( it->second == connection )
        {
            LeaveCriticalSection( &_csMapIndexToSocket );
            return it->first;
        }
    }

    LeaveCriticalSection( &_csMapIndexToSocket );
    return -1;
}

void CNetworkCommunicator::registerNewConnectionNotify( tySigNewConnection sltMessage, const PVOID pParam )
{
	_vecSltMessage.push_back( std::make_pair( sltMessage, pParam ) );
}

void CNetworkCommunicator::registerDisconnectionNotify( tySigDeleteConnection sltMessage, const PVOID pParam )
{
	_vecSltDisconnect.push_back( std::make_pair( sltMessage, pParam ) );
}

void CNetworkCommunicator::receiveDisconnectNotify( const PVOID pParam, tySpCommonSocket connection )
{
	CNetworkCommunicator* pNetworkManager = reinterpret_cast< CNetworkCommunicator * >( pParam );
	pNetworkManager->disconnect( pNetworkManager->getSocketIndex( connection.get() ) );
}

//////////////////////////////////////////////////////////////////////////

boost::shared_ptr< CNetworkCommunicator > createNetworkCommunicator()
{
	return boost::shared_ptr< CNetworkCommunicator >( new CNetworkCommunicator( CIoServicePool::getInstance()->getNextIoService() ) );
}

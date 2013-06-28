#pragma once

/*
	���� : ����ȣ

	Boost::asio, Boost::thread �� �̿��Ͽ� �񵿱� ���� ���̺귯�� 

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
	CCommonSocket Ŭ������ �̿��Ͽ� TCP ����� �����ϴ� Ŭ����

	�ش� Ŭ������ �⺻������ ���������� ���Ǵ� ���Ͽ� ���ؼ� �Ű澲�� �ʰ� ����� �� �ֵ��� �Ǿ�����. 
	���������� ����Ϸ��� bind_and_accept �Լ��� ȣ���Ͽ� Ư�� ��Ʈ�� ���Ͽ� ������ ����ϵ��� �ϸ� �Ǹ� Ŭ���̾�Ʈ������ ����Ϸ��� connectTo �Լ��� �̿��Ͽ� ������ ������ �õ��Ѵ�. 

	�系���� ���� �������� ���۸� �̿��� �޽��� ����/������ ��Ģ �� ���� 

	1. Ŭ���̾�Ʈ�� �� �޽����� ������ �� �ֵ��� �� �޽������� ������ �޽��� ��ȣ�� �ο��Ѵ�. ( ��Ŷ�� msgId �ʵ�, ���� Ŭ���������� �ڵ����� �ο��� )
	2. �������� Ư�� ��Ŷ�� ���� ������ ������ ���� ��û ��Ŷ�� ���� �޽��� ��ȣ�� ����Ѵ�. ( ��). 2�� �޽��� ��ȣ�� ���� ������ 2�� �޽��� ��ȣ�� �״�� ��� )

*/
class IMONSOCKET_DLL_API CNetworkCommunicator
{
public:
	CNetworkCommunicator( boost::asio::io_service& ioService );
	~CNetworkCommunicator();

	/// ������ ���� �Է��� ���� �� �ֵ��� ���ε���. ipAddress �� *.*.*.* �� �����ϸ� ��� IP �� ������ �����
	bool bind_and_accept( unsigned short portNumber, const std::string& ipAddress = "*.*.*.*" );
	/*!
		������ �ּҿ� �����մϴ�. 

		@return <= 0, ���� ����
		@return > 0, �ش� ���Ͽ� ���� ���ι�ȣ
	*/
	SOCK_INDEX connectTo( const std::string& ipAddress, unsigned short portNumber );
	/*!
		������ ���� ��ȣ�� ����� ������ ��Ʈ��ũ ������ �����ϴ�. 

		�ش� ��ȣ�� ����� ������ ������ �ƹ� �ϵ� �Ͼ�� �ʽ��ϴ�
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

	// Ŭ���̾�Ʈ�κ��� ������ �ޱ� ���� ������
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
	DLL ���� new �� �Ҵ��� �޸𸮴� �ݵ�� DLL ���� �����ؾ��ϸ�, �̸� �� ������ �� �ٸ� �����忡�� �����ϸ� ������ �߻��� �� �ִ�. 
	
	������, �̿� ���� ������ ���� �ʰ� �����Ͽ� DLL ���� �����ص� ���� �ڵ忡 ���������� �����͸� �ѱ�� ���� ���ŷ��� �ر� ���� ������, 

	�̸� �ڵ������� ó�����ִ� boost::shared_ptr �� �̿��Ͽ� ������.
*/

IMONSOCKET_DLL_API boost::shared_ptr< CNetworkCommunicator > createNetworkCommunicator();

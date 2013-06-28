#pragma once

#include <vector>
#include <string>
#include <algorithm>

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

#include "IoServicePool.hpp"

#pragma warning( disable: 4251 )

#if defined(COMMONSOCKET_EXPORTS) && defined(TEST_EXE_INCLUDE)
	#define IMONSOCKET_DLL_API	__declspec(dllexport)
#else
	#if defined(TEST_EXE_INCLUDE) && !defined(COMMONSOCKET_EXPORTS)
		#define IMONSOCKET_DLL_API
	#else
		#define IMONSOCKET_DLL_API	__declspec(dllimport)
	#endif
#endif

class CCommonSocket;

typedef std::vector< boost::uint8_t >				tyDataBuffer;
typedef boost::shared_ptr< CCommonSocket >			tySpCommonSocket;

typedef void (*tySigMessage)( const PVOID, tySpCommonSocket, const std::vector< boost::uint8_t >& );
typedef void (*tySigDisconnect)( const PVOID, tySpCommonSocket );

/*!
	소켓 클래스 ( 구글 프로토콜 버퍼를 위한 소켓 클래스 )

	Boost::asio 라이브러리를 사용함. 
	
	본 클래스는 factory 패턴을 사용하여 정적 함수인 createSocket 함수를 사용하여 소켓을 생성한다.

	본 클래스는 네트워크 관리자 클래스와 연동하여 사용하며, 비동기 전송 / 수신 모드만을 지원함. 
*/
class IMONSOCKET_DLL_API CCommonSocket
	: public boost::enable_shared_from_this< CCommonSocket >
{
public:
	~CCommonSocket();

	static tySpCommonSocket createSocket( boost::asio::io_service& ioService )
	{	return tySpCommonSocket( new CCommonSocket( ioService ) ); }

	boost::asio::ip::tcp::socket&			socket()
	{	return _socket; }

	int										sendPacket( const tyDataBuffer& dataBuffer );
	void									receivePacket();

    void									registerQueuePacket( tySigMessage sltMessage, const PVOID pParam );
	void									registerDisconnect( tySigDisconnect sltDisconnect, const PVOID pParam );

    std::string								getRemoteAdddres() const;

private:
	CCommonSocket( boost::asio::io_service& ioService );
	unsigned int	decode_header( const tyDataBuffer& buffer ) const;
	///  구글 프로토콜 버퍼는 4바이트 헤더 + 몸체 가 있기때문에 하나의 패킷을 완성하기 위해서 2번에 걸쳐서 읽어야함. 
	void			readHeader( const boost::system::error_code& err, std::size_t bytes_transferred );
	void			readBody( const boost::system::error_code& err, std::size_t bytes_transferred );
	void			startReadHeader();
	void			startReadBody();

	boost::asio::ip::tcp::socket						_socket;
	tyDataBuffer										_readBuffer;

	/// 패킷이 헤더 + 몸체로 구성된다면 헤더의 크기를 정함, 기본 4바이트로 설정됨
	static const unsigned int							_PACKET_HEADER_SIZE = 4;
	/// 패킷의 몸체 크기를 저장함
	unsigned int										_PACKET_BODY_SIZE;
	std::vector< std::pair< tySigMessage, PVOID > >		_vecSltMessage;
	std::vector< std::pair< tySigDisconnect, PVOID > >		_vecSltDisconnect;
};

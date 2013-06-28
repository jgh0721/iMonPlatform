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
	���� Ŭ���� ( ���� �������� ���۸� ���� ���� Ŭ���� )

	Boost::asio ���̺귯���� �����. 
	
	�� Ŭ������ factory ������ ����Ͽ� ���� �Լ��� createSocket �Լ��� ����Ͽ� ������ �����Ѵ�.

	�� Ŭ������ ��Ʈ��ũ ������ Ŭ������ �����Ͽ� ����ϸ�, �񵿱� ���� / ���� ��常�� ������. 
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
	///  ���� �������� ���۴� 4����Ʈ ��� + ��ü �� �ֱ⶧���� �ϳ��� ��Ŷ�� �ϼ��ϱ� ���ؼ� 2���� ���ļ� �о����. 
	void			readHeader( const boost::system::error_code& err, std::size_t bytes_transferred );
	void			readBody( const boost::system::error_code& err, std::size_t bytes_transferred );
	void			startReadHeader();
	void			startReadBody();

	boost::asio::ip::tcp::socket						_socket;
	tyDataBuffer										_readBuffer;

	/// ��Ŷ�� ��� + ��ü�� �����ȴٸ� ����� ũ�⸦ ����, �⺻ 4����Ʈ�� ������
	static const unsigned int							_PACKET_HEADER_SIZE = 4;
	/// ��Ŷ�� ��ü ũ�⸦ ������
	unsigned int										_PACKET_BODY_SIZE;
	std::vector< std::pair< tySigMessage, PVOID > >		_vecSltMessage;
	std::vector< std::pair< tySigDisconnect, PVOID > >		_vecSltDisconnect;
};

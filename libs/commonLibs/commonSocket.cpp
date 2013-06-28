// commonSocket.cpp : DLL 응용 프로그램을 위해 내보낸 함수를 정의합니다.
//

#include "stdafx.h"
#include "commonSocket.h"

#include "cmnConverter.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnConvert;

CCommonSocket::CCommonSocket( boost::asio::io_service& ioService )
	: _socket( ioService ), _PACKET_BODY_SIZE(0)
{

}

CCommonSocket::~CCommonSocket()
{

}


int CCommonSocket::sendPacket( const tyDataBuffer& dataBuffer )
{
	boost::system::error_code err;

	int writtenByte = boost::asio::write( socket(), boost::asio::buffer( dataBuffer ), err );

	if( err )
	{
		LM_ERROR_TO( "network/commonSocket", L"데이터 전송 실패, 오류 = %1,", 
			CA2U( err.message() ) );
	}

	return writtenByte;
}

unsigned int CCommonSocket::decode_header( const tyDataBuffer& buffer ) const
{
	if( buffer.size() < _PACKET_HEADER_SIZE )
		return 0;

	unsigned msg_size = 0;
	for (unsigned i = 0; i < _PACKET_HEADER_SIZE; ++i)
		msg_size = msg_size * 256 + (static_cast<unsigned>(buffer[i]) & 0xFF);

	return msg_size;
}

void CCommonSocket::receivePacket()
{
	startReadHeader();
}

void CCommonSocket::startReadHeader()
{
	_readBuffer.resize( _PACKET_HEADER_SIZE );
	boost::asio::async_read( socket(), boost::asio::buffer( _readBuffer ), 
		boost::bind( &CCommonSocket::readHeader, shared_from_this(), 
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
}

void CCommonSocket::readHeader( const boost::system::error_code& err, std::size_t bytes_transferred )
{
	if( !err )
	{
		_PACKET_BODY_SIZE = decode_header( _readBuffer );

		//		LM_ERROR_TO( "network/commonSocket", L"패킷 크기 수신 : %1", _PACKET_BODY_SIZE );

		startReadBody();
	}
	else
	{
		if( err.value() != 2 )
		{
			LM_ERROR_TO( "network/commonSocket", L"패킷 크기 수신 실패, 오류 = %1, ipAddress = %2", CA2U( err.message() ), CU82U(getRemoteAdddres()) );
		}

#if _MSC_VER >= 1600
		for( auto it = _vecSltDisconnect.begin(); it != _vecSltDisconnect.end(); ++it )
			(*it->first)( it->second, shared_from_this() );
#else
		for( std::vector< std::pair< tySigDisconnect, PVOID > >::iterator it = _vecSltDisconnect.begin(); it != _vecSltDisconnect.end(); ++it )
			(*it->first)( it->second, shared_from_this() );
#endif

		// 		LM_ERROR_TO( "network/commonSocket", L"%1", L"서버 재연결 시도" );
		// 		getNetwork()->connect();
	}
}

void CCommonSocket::startReadBody()
{
	_readBuffer.resize( _PACKET_HEADER_SIZE + _PACKET_BODY_SIZE );
	boost::asio::mutable_buffers_1 buf = boost::asio::buffer( &_readBuffer[ _PACKET_HEADER_SIZE ], _PACKET_BODY_SIZE );

	boost::asio::async_read( socket(), buf, 
		boost::bind( &CCommonSocket::readBody, shared_from_this(), 
		boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred ) );
}

void CCommonSocket::readBody( const boost::system::error_code& err, std::size_t bytes_transferred )
{
	if( !err )
	{
		for( std::vector< std::pair< tySigMessage, PVOID > >::iterator it = _vecSltMessage.begin(); it != _vecSltMessage.end(); ++it )
			(*it->first)( it->second, shared_from_this(), _readBuffer );
		_PACKET_BODY_SIZE = 0;
		receivePacket();
	}
	else
	{
		LM_ERROR_TO( "network/commonSocket", L"패킷 수신 중 오류 = %1", CA2U( err.message() ) );

		for( std::vector< std::pair< tySigDisconnect, PVOID > >::iterator it = _vecSltDisconnect.begin(); it != _vecSltDisconnect.end(); ++it )
			(*it->first)( it->second, shared_from_this() );

	}
}

void CCommonSocket::registerQueuePacket( tySigMessage sltMessage, const PVOID pParam )
{
#if _MSC_VER >= 1600
	if( sltMessage != nullptr )
		_vecSltMessage.push_back( std::make_pair( sltMessage, pParam ) );
#else
	if( sltMessage != NULL )
		_vecSltMessage.push_back( std::make_pair( sltMessage, pParam ) );
#endif

}

void CCommonSocket::registerDisconnect( tySigDisconnect sltDisconnect, const PVOID pParam )
{
#if _MSC_VER >= 1600
	if( sltDisconnect != nullptr )
		_vecSltDisconnect.push_back( std::make_pair( sltDisconnect, pParam ) );
#else
	if( sltDisconnect != NULL )
		_vecSltDisconnect.push_back( std::make_pair( sltDisconnect, pParam ) );
#endif
}


std::string CCommonSocket::getRemoteAdddres() const
{
	boost::system::error_code err;

	boost::asio::ip::tcp::endpoint& ep = _socket.remote_endpoint( err );
	if( !err )
	{
		std::string address = ep.address().to_string( err );
		if( !err )
			return address;
	}

	return "";
}

//////////////////////////////////////////////////////////////////////////


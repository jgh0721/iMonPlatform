#pragma once

#include <map>
#include <string>
#include <unordered_map>
#include "CPGBTraits.h"
#include "commonSocket.h"
#include "cmnUtils.h"

#ifdef PREFIX_NS_PROTOBUF
	#define PROTOCOL_BUFFER_INCLUDE_NAME <PREFIX_NS_PROTOBUF##.pb.h>

	#include PROTOCOL_BUFFER_INCLUDE_NAME
	typedef CGPTraits< PREFIX_NS_PROTOBUF::msgPacket, PREFIX_NS_PROTOBUF::tagRequestCode, PREFIX_NS_PROTOBUF::tagResponseCode > tyNetworkTraits;

	#define DECLARE_HANDLER( reqCode ) \
		static tyNetworkTraits::tyResult process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF::msgPacket& packet );

	#define DECLARE_PP_HANDLER( reqCode ) \
		static void process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF::msgPacket& inPacket, tyNetworkTraits::tyResult& retPacket );

	#define DEFINE_HANDLER( reqCode ) \
		tyNetworkTraits::tyResult CExchangeNetworkPackets::process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF::msgPacket& packet )

	#define DEFINE_PP_HANDLER( className, reqCode ) \
		void className::process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF::msgPacket& inPacket, tyNetworkTraits::tyResult& retPacket )

	#define REGISTER_HANDLER( networkMgr, reqCode ) \
		registerHandler( ##networkMgr , PREFIX_NS_PROTOBUF::##reqCode , &CExchangeNetworkPackets::process##reqCode , this );

	#define REGISTER_PP_HANDLER( mapper, className, reqCode ) \
		mapper[ PREFIX_NS_PROTOBUF::##reqCode ] = std::make_pair( &className::process##reqCode , pParam );

	PREFIX_NS_PROTOBUF::msgPacket getBasePacket( PREFIX_NS_PROTOBUF::tagRequestCode reqCode, PREFIX_NS_PROTOBUF::tagResponseCode resCode = PREFIX_NS_PROTOBUF::RES_NONE, long msgID = 0 );
#endif

#ifdef PREFIX_NS_PROTOBUF_CLC
#define PROTOCOL_BUFFER_CLC_INCLUDE_NAME <PREFIX_NS_PROTOBUF_CLC##.pb.h>

#include PROTOCOL_BUFFER_CLC_INCLUDE_NAME
	typedef CGPTraits< PREFIX_NS_PROTOBUF_CLC::msgPacket, PREFIX_NS_PROTOBUF_CLC::tagRequestCode, PREFIX_NS_PROTOBUF_CLC::tagResponseCode > tyNetworkCLCTraits;

#define DECLARE_CLC_HANDLER( reqCode ) \
	static tyNetworkCLCTraits::tyResult process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_CLC::msgPacket& packet );

#define DECLARE_CLC_PP_HANDLER( reqCode ) \
	static void process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_CLC::msgPacket& inPacket, tyNetworkCLCTraits::tyResult& retPacket );

#define DEFINE_CLC_HANDLER( reqCode ) \
	tyNetworkCLCTraits::tyResult CExchangeNetworkPackets::process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_CLC::msgPacket& packet )

#define DEFINE_CLC_PP_HANDLER( className, reqCode ) \
	void className::process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_CLC::msgPacket& inPacket, tyNetworkCLCTraits::tyResult& retPacket )

#define REGISTER_CLC_HANDLER( networkMgr, reqCode ) \
	registerHandler( ##networkMgr , PREFIX_NS_PROTOBUF_CLC::##reqCode , &CExchangeNetworkPackets::process##reqCode , this );

#define REGISTER_CLC_PP_HANDLER( mapper, className, reqCode ) \
	mapper[ PREFIX_NS_PROTOBUF_CLC::##reqCode ] = std::make_pair( &className::process##reqCode , pParam );

	PREFIX_NS_PROTOBUF_CLC::msgPacket getBasePacket( PREFIX_NS_PROTOBUF_CLC::tagRequestCode reqCode, PREFIX_NS_PROTOBUF_CLC::tagResponseCode resCode = PREFIX_NS_PROTOBUF_CLC::RES_NONE, long msgID = 0 );
#endif
	
#ifdef PREFIX_NS_PROTOBUF_IPC
	#define PROTOCOL_BUFFER_IPC_INCLUDE_NAME <PREFIX_NS_PROTOBUF_IPC##.pb.h>

	#include PROTOCOL_BUFFER_IPC_INCLUDE_NAME

	typedef CGPTraits< PREFIX_NS_PROTOBUF_IPC::msgPacket, PREFIX_NS_PROTOBUF_IPC::tagRequestCode, PREFIX_NS_PROTOBUF_IPC::tagResponseCode > tyNetworkIPCTraits;

	#define DECLARE_IPC_HANDLER( reqCode ) \
		static tyNetworkIPCTraits::tyResult process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_IPC::msgPacket& packet );

    #define DECLARE_IPC_REQHANDLER( reqCode ) \
        static bool process##reqCode ( const PVOID pParam, const nsCommon::tyMapParams& packetData, nsCommon::tyMapParams& resultData = nsCommon::tyMapParams() );

	#define DEFINE_IPC_HANDLER( reqCode ) \
		tyNetworkIPCTraits::tyResult CExchangeNetworkIPCPackets::process##reqCode ( const PVOID pParam, const PVOID pParamSec, const CCommonSocket* pSocket, const PREFIX_NS_PROTOBUF_IPC::msgPacket& packet )

    #define DEFINE_IPC_REQHANDLER( className, reqCode ) \
        bool className::process##reqCode ( const PVOID pParam, const nsCommon::tyMapParams& packetData, nsCommon::tyMapParams& resultData )

	#define REGISTER_IPC_HANDLER( networkMgr, reqCode ) \
		registerIPCHandler( ##networkMgr , PREFIX_NS_PROTOBUF_IPC::##reqCode , &CExchangeNetworkIPCPackets::process##reqCode , this );

	PREFIX_NS_PROTOBUF_IPC::msgPacket getIPCBasePacket( PREFIX_NS_PROTOBUF_IPC::tagRequestCode reqCode, PREFIX_NS_PROTOBUF_IPC::tagResponseCode resCode = PREFIX_NS_PROTOBUF_IPC::RES_NONE, long msgID = 0 );

#endif

#ifndef DECLARE_REQHANDLER
    #define DECLARE_REQHANDLER( reqCode ) \
        static bool process##reqCode ( const PVOID pParam, const nsCommon::tyMapParams& packetData, nsCommon::tyMapParams& resultData = nsCommon::tyMapParams() );
#endif

#ifndef DEFINE_REQHANDLER
    #define DEFINE_REQHANDLER( className, reqCode ) \
        bool className::process##reqCode ( const PVOID pParam, const nsCommon::tyMapParams& packetData, nsCommon::tyMapParams& resultData )
#endif

#ifndef REQUEST_SERVER
    #define REQUEST_SERVER( className, reqCode, packetData, resultData ) \
        className::process##reqCode ( this, packetData, resultData )
#endif

/*!
	요청처리 플러그인 클래스, LMDB 등 패킷에 대해 별도의 처리에 필요할 때 해당 클래스를 상속받아 이용함
*/
template< typename TyTraits >
class CExchangePacketHandlerPP
{
public:
	CExchangePacketHandlerPP();
	virtual ~CExchangePacketHandlerPP();

	virtual bool InitPlugin( PVOID pParam ) = 0;
	/// 핸들러 이름을 설정함, 해당 이름은 XML 에서 플러그인 이름으로 사용되므로 플러그인 이름과 같아야함. 
	virtual std::wstring GetName() = 0;

	bool IsExistHandler( typename TyTraits::tyReqCode reqCode );
	bool IsExistHandler( typename TyTraits::tyResCode resCode );

	void InvokeHandler( typename TyTraits::tyReqCode reqCode, const PVOID pParamSec, const CCommonSocket* inSocket, const typename TyTraits::tyPacket& inPacket, typename TyTraits::tyResult& retPacket );
	void InvokeHandler( typename TyTraits::tyResCode resCode, const PVOID pParamSec, const CCommonSocket* inSocket, const typename TyTraits::tyPacket& inPacket, typename TyTraits::tyResult& retPacket );

protected:
#if _MSC_VER >= 1600 
	typedef std::unordered_map< typename TyTraits::tyReqCode, std::pair< typename TyTraits::tySigPostPacket, PVOID > >		tyMapRequestToSlot;
	typedef std::unordered_map< typename TyTraits::tyResCode, std::pair< typename TyTraits::tySigPostPacket, PVOID > >		tyMapResponseToSlot;
#else
	typedef std::tr1::unordered_map< typename TyTraits::tyReqCode, std::pair< typename TyTraits::tySigPostPacket, PVOID > >		tyMapRequestToSlot;
	typedef std::tr1::unordered_map< typename TyTraits::tyResCode, std::pair< typename TyTraits::tySigPostPacket, PVOID > >		tyMapResponseToSlot;
#endif

	tyMapRequestToSlot				_mapRequestToSlot;
	tyMapResponseToSlot				_mapResponseToSlot;
};

template< typename TyTraits >
CExchangePacketHandlerPP<TyTraits>::CExchangePacketHandlerPP()
{

}

template< typename TyTraits >
CExchangePacketHandlerPP<TyTraits>::~CExchangePacketHandlerPP()
{

}

template< typename TyTraits >
bool CExchangePacketHandlerPP<TyTraits>::IsExistHandler( typename TyTraits::tyReqCode reqCode )
{
	return _mapRequestToSlot.count( reqCode ) > 0;
}

template< typename TyTraits >
bool CExchangePacketHandlerPP<TyTraits>::IsExistHandler( typename TyTraits::tyResCode resCode )
{
	return _mapResponseToSlot.count( resCode ) > 0;
}

template< typename TyTraits >
void CExchangePacketHandlerPP<TyTraits>::InvokeHandler( typename TyTraits::tyReqCode reqCode, const PVOID pParamSec, const CCommonSocket* inSocket, const typename TyTraits::tyPacket& inPacket, typename TyTraits::tyResult& retPacket )
{
	if( IsExistHandler( reqCode ) == false )
		return ;

	std::pair< typename TyTraits::tySigPostPacket, PVOID > prResult = _mapRequestToSlot[ reqCode ];
	(*prResult.first)( prResult.second, pParamSec, inSocket, inPacket, retPacket );
}

template< typename TyTraits >
void CExchangePacketHandlerPP<TyTraits>::InvokeHandler( typename TyTraits::tyResCode resCode, const PVOID pParamSec, const CCommonSocket* inSocket, const typename TyTraits::tyPacket& inPacket, typename TyTraits::tyResult& retPacket )
{
	if( IsExistHandler( resCode ) == false )
		return ;

	std::pair< typename TyTraits::tySigPostPacket, PVOID > prResult = _mapResponseToSlot[ resCode ];
	(*prResult.first)( prResult.second, pParamSec, inSocket, inPacket, retPacket );
}

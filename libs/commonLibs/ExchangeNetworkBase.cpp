#include "stdafx.h"

#include "ExchangeNetworkBase.h"

//////////////////////////////////////////////////////////////////////////

#ifdef PREFIX_NS_PROTOBUF
PREFIX_NS_PROTOBUF::msgPacket getBasePacket( PREFIX_NS_PROTOBUF::tagRequestCode reqCode, PREFIX_NS_PROTOBUF::tagResponseCode resCode /*= PREFIX_NS_PROTOBUF::RES_NONE*/, long msgID /*= 0 */ )
{
	static volatile LONG _msgID = 0;

	PREFIX_NS_PROTOBUF::msgPacket resultPacket = PREFIX_NS_PROTOBUF::msgPacket::default_instance();
	
	resultPacket.set_identity( resultPacket.identity() );
	resultPacket.set_reqcode( reqCode );
	resultPacket.set_rescode( resCode );
#ifdef PREFIX_NS_PROTOBUF_USE_PRICODE	
	resultPacket.set_pricode( resultPacket.pricode() );
#endif

	if( msgID == 0 )
		resultPacket.set_msgid( InterlockedIncrement( &_msgID ) );
	else
		resultPacket.set_msgid( msgID );

	return resultPacket;
}
#endif

#ifdef PREFIX_NS_PROTOBUF_CLC
PREFIX_NS_PROTOBUF_CLC::msgPacket getBasePacket( PREFIX_NS_PROTOBUF_CLC::tagRequestCode reqCode, PREFIX_NS_PROTOBUF_CLC::tagResponseCode resCode /*= PREFIX_NS_PROTOBUF_CLC::RES_NONE*/, long msgID /*= 0 */ )
{
	static volatile LONG _msgID = 0;

	PREFIX_NS_PROTOBUF_CLC::msgPacket resultPacket = PREFIX_NS_PROTOBUF_CLC::msgPacket::default_instance();

	resultPacket.set_identity( resultPacket.identity() );
	resultPacket.set_reqcode( reqCode );
	resultPacket.set_rescode( resCode );

	if( msgID == 0 )
		resultPacket.set_msgid( InterlockedIncrement( &_msgID ) );
	else
		resultPacket.set_msgid( msgID );

	return resultPacket;
}
#endif

#ifdef PREFIX_NS_PROTOBUF_IPC
PREFIX_NS_PROTOBUF_IPC::msgPacket getIPCBasePacket( PREFIX_NS_PROTOBUF_IPC::tagRequestCode reqCode, PREFIX_NS_PROTOBUF_IPC::tagResponseCode resCode /*= PREFIX_NS_PROTOBUF_IPC::RES_NONE*/, long msgID /*= 0 */ )
{
	static volatile LONG _msgID = 0;

	PREFIX_NS_PROTOBUF_IPC::msgPacket resultPacket = PREFIX_NS_PROTOBUF_IPC::msgPacket::default_instance();

	resultPacket.set_identity( resultPacket.identity() );
	resultPacket.set_reqcode( reqCode );
	resultPacket.set_rescode( resCode );

	if( msgID == 0 )
		resultPacket.set_msgid( InterlockedIncrement( &_msgID ) );
	else
		resultPacket.set_msgid( msgID );

	return resultPacket;
}
#endif

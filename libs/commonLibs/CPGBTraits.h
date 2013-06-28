#pragma once

#include <time.h>

#include <vector>
#include <utility>
#include <unordered_map>
#include <memory>
#include <boost/unordered_map.hpp>

#include "commonSocket.h"
#include "CNetworkCommunicator.h"

template< typename TyPacket, typename TyRequestCode, typename TyResponseCode >
class CGPTraits
{
public:
	typedef TyPacket tyPacket;
	typedef TyRequestCode tyReqCode;
	typedef TyResponseCode tyResCode;

	typedef std::pair< TyPacket, bool > tyResult;
	typedef typename tyResult (*tySigNewPacket)( const PVOID, const PVOID, const CCommonSocket*, const TyPacket& );
	typedef void (*tySigPostPacket)( const PVOID, const PVOID, const CCommonSocket*, const TyPacket&, typename tyResult& );
	typedef void (*tySigConnection)( const PVOID, const SOCK_INDEX );
    typedef void (*tySigPacketMonitor)( const PVOID, const CCommonSocket*, bool isIncoming, __time64_t packetTime, const TyPacket& );

	typedef std::pair< tySpCommonSocket, TyPacket >                     tyPrPacketData;
#if _MSC_VER >= 1600
	typedef std::unordered_map< long, TyPacket >						tyMapMsgIdToMsgPacket;

	typedef std::unordered_map< TyRequestCode, std::pair< tySigNewPacket, PVOID > >		tyMapRequestToSlot;
	typedef std::unordered_map< TyResponseCode, std::pair< tySigNewPacket, PVOID > >	tyMapResponseToSlot;
#else
	typedef std::tr1::unordered_map< long, TyPacket >						tyMapMsgIdToMsgPacket;

	typedef std::tr1::unordered_map< TyRequestCode, std::pair< tySigNewPacket, PVOID > >		tyMapRequestToSlot;
	typedef std::tr1::unordered_map< TyResponseCode, std::pair< tySigNewPacket, PVOID > >	tyMapResponseToSlot;
	
#endif
    typedef boost::unordered_map< long, std::pair< __time64_t, tyPacket > >             tyMapMsgIdToMsgPacketEx;
	typedef std::vector< std::pair< tySigConnection, PVOID > >							tyVecConnectionSlot;	
    typedef std::vector< std::pair< tySigPacketMonitor, PVOID > >                       tyVecPacketMonitorSlot;
};

#if _MSC_VER >= 1600 
    template < typename T >
    std::shared_ptr< T > GetNetworkManager( unsigned int maxIncomingQueueCount = 0 )
    {
        static std::shared_ptr< T > spNetworkManager( new T( maxIncomingQueueCount ) );
        if( spNetworkManager == NULLPTR )
            spNetworkManager = std::shared_ptr< T >( new T( maxIncomingQueueCount ) );

        return spNetworkManager;
    }
#else
    template < typename T >
    std::tr1::shared_ptr< T > GetNetworkManager( unsigned int maxIncomingQueueCount = 0 )
    {
        static std::tr1::shared_ptr< T > spNetworkManager( new T( maxIncomingQueueCount ) );
        if( spNetworkManager == NULLPTR )
            spNetworkManager = std::tr1::shared_ptr< T >( new T( maxIncomingQueueCount ) );

        return spNetworkManager;
    }
#endif

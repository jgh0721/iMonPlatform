#pragma once

/*!
    GPBManager V2

    ��������
        #. ������ ���信 ���� �����ߴ� ��Ŷ�� ���� ������ ��� �߰�


*/

#include <string>
#include <queue>
#include <vector>
#include <set>

#include <boost/atomic.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include <boost/lockfree/queue.hpp>

#include "commonSocket.h"
#include "CNetworkCommunicator.h"
#include "CPGBTraits.h"
#include "ExchangeNetworkBase.h"

#include "cmnUtils.h"
#include "cmnWinUtils.h"

#pragma warning( disable: 4996 )
#pragma warning( disable: 4482 )

const std::string gpbmanagerv2_logger = "network/gpbmanager";

typedef std::vector< std::pair< SOCK_INDEX, int > >		tyVecSendResult;

template< typename TyTraits >
class CGPBManagerV2
{
private:
    static const int                                                                        PACKET_HEADER_SIZE = 4;

    /*!
        #. Ŭ���̾�Ʈ���� ��Ŷ ������ ��û�ϸ� ������ ������ ��Ŷ�� ���ؼ� ������ ť�� �־��
        #. �������� �ٻڴٴ� ������ ���� ������ ť���� ��Ŷ ������
    */
    typedef std::pair< tySpCommonSocket, typename TyTraits::tyPacket >       _packet;
    typedef boost::unordered_map< SOCK_INDEX, typename TyTraits::tyMapMsgIdToMsgPacketEx* >   tyMapSocketToResPacket;

    long                                                                                    _globalMessageID;

    boost::shared_ptr< CNetworkCommunicator >                                               _spNetworkManager;
    boost::atomic<bool>                                                                     _isExit;
    nsCommon::CConcurrentQueue< typename TyTraits::tyPrPacketData >                         _incomingQueue;
    nsCommon::CConcurrentQueue< typename TyTraits::tyPrPacketData >                         _outgoingQueue;
    
    boost::thread_group                                                                     _workThreadGroup;

    typename TyTraits::tyMapRequestToSlot                                                   _mapRequestToSlot;
    typename TyTraits::tyMapResponseToSlot                                                  _mapResponseToSlot;
    typename TyTraits::tyVecConnectionSlot                                                  _vecDisconnectSlot;
    typename TyTraits::tyVecConnectionSlot                                                  _vecNewconnectSlot;
    typename TyTraits::tyVecPacketMonitorSlot                                               _vecPacketMonitorSlot;

    CRITICAL_SECTION                                                                        _lckSockets;
    CRITICAL_SECTION                                                                        _lckQueue;
    CRITICAL_SECTION                                                                        _lckSendQueue;
    tyMapSocketToResPacket                                                                  _mapSocketToResPacket;

    std::set< tySpCommonSocket >                                                            _setConnectionedSockets;
    /*!
        SERVER_BUSY ���, ������ SERVER_BUSY ���� �����ڵ带 �����ϸ� ���� �������� �� �� ��Ŷ�� �������Ѵ�.
    */
    boost::atomic<bool>                                                                     _isBusy;
    volatile bool                                                                           _isUseWaitingWhenServerBusy;
    typename TyTraits::tyResCode                                                            _resCodeServerBusy;
    boost::unordered_map< SOCK_INDEX, typename TyTraits::tyMapMsgIdToMsgPacket* >           _mapSocketToSendedPacket;
    unsigned int                                                                            _nSendPacketIntervalsWhenBusySecs;

public:
    CGPBManagerV2( int nDispatcherThreadCount = 0 );
    ~CGPBManagerV2();

    /// �������� ��Ʈ ��ȣ�� ������ ���� IP �ּҸ� ����, �⺻������ ��� IP �� ���Ͽ� ������
    bool                        setListenPort( unsigned short listenPort, const std::string& ipaddress = "*.*.*.*" );
    /// ������ �ּҿ� ��Ʈ ��ȣ�� ����Ͽ� �ش� ��ǻ�Ϳ� ����
    bool                        connectTo( const std::string& address, unsigned int port );
    bool                        connectTo( const std::string& address, unsigned int port, SOCK_INDEX& socketIndex );

    /// ������ ���� ���ι�ȣ���� ������ ������
    void                        disconnect( SOCK_INDEX socketIndex ) { _spNetworkManager->disconnect( socketIndex ); };
    /// �����ڰ� �����ϴ� ��� ���� ���ι�ȣ���� ������ ������
    void                        disconnectAll() { _spNetworkManager->disconnectAll(); };

    void                        setIsUseServerBusyWaiting( bool isUse ) { _isUseWaitingWhenServerBusy = isUse; }
    bool                        getIsUseServerBusyWaiting() { return _isUseWaitingWhenServerBusy; }

    void                        setSendPacketIntervalsWhenBusy( unsigned int nSendIntervalSecs = 10 ) { _nSendPacketIntervalsWhenBusySecs = nSendIntervalSecs; }
    unsigned int                getSendPacketIntervalsWhenBusy() { return _nSendPacketIntervalsWhenBusySecs; }

    void                        setServerBusyResCode( typename TyTraits::tyResCode resCode ) { _resCodeServerBusy = resCode; }
    typename TyTraits::tyResCode getServerBusyResCode() { return _resCodeServerBusy; }

	/*!
		�ش� �����ڰ� �����ϴ� ��� ���Ͽ��� �ۼ��� ��Ŷ�� �����մϴ�. 

		@return ������ ������ ����Ǿ� �ִ� ���� �����Ͽ� ���� ���ι�ȣ�� ������ ��Ŷ�� �ο��� �޽��� ��ȣ�� ¦�� �̷� ���͸� ��ȯ�մϴ�. 
		�ο��� �ش� �޽��� ��ȣ�� �̿��Ͽ� ������Ŷ�� �˻��մϴ�. �޽��� ��ȣ < 0 �̸� ����
	*/
 	tyVecSendResult             sendPacket( typename TyTraits::tyPacket& packet, bool autoAssignMsgID = true );

	/*! 
		������ ���� ���ι�ȣ�� ����� �������� �ۼ��� ��Ŷ�� �����մϴ�. 

        @return �ش� ��Ŷ�� �ο��� �޽�����ȣ�� ��ȯ�մϴ�. �ش� ��ȣ�� �̿��Ͽ� ������Ŷ�� �˻��մϴ�. 
        @return �޽��� ��ȣ < 0 �̸� ����
	*/
    int                         sendPacket( int socketIndex, typename TyTraits::tyPacket& packet, bool autoAssignMsgID = true );
    int                         sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, bool autoAssignmsgID = true );
    int                         sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, int msgID );

	/*!
		�ش� �޽�����ȣ�� �´� ������Ŷ�� �����Ͽ����� �˻��մϴ�. 	

		ReceivePacket �� RegisterMessageSlot ���� �ݹ��Լ��� ����Ͽ� �켱������ ���޹޵��� ��û�� ��Ŷ�� ������ ������ ������Ŷ�� ���ؼ� Ȯ���մϴ�. 
		�޽��� ������ ���� ������Ŷ�� ���� ���޵Ǿ��ٸ�, ReceivePacket ȣ���� ����� ���� �޽��� �˻��� �����մϴ�. 

		ReceivePacketWait �� �����ð����� ���ο��� ���� ��Ŷ�� ����մϴ�. ������ ����ϴ� ���� ~Wait �Լ��� ȣ���� �Լ��� ���� ������� ����մϴ�. 
	*/
    bool                        receivePacket( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet );
    bool                        receivePacket( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet );
    bool                        receivePacketWait( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet, int timoueMs = 10, int tryout = 1000 );
    bool                        receivePacketWait( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet, int timoueMs = 10, int tryout = 1000 );

	/*!
		����� �޽��� ������ ��Ʈ��ũ ������ Ŭ������ �����ϰ� �ٽ� �����ϱ� ������ ������ �� ����!!!
	*/
	/// �������κ��� �޴� ��Ŷ���� Ư�� ��û�ڵ忡 ���� Ư�� �Լ��� Ǫ������ ���� ��û�մϴ�, REQ_NONE �� ����
	void RegisterMessageSlot( typename TyTraits::tyReqCode requestCode, typename TyTraits::tySigNewPacket fpNewPacketSubscriber, PVOID pParam )
	{ _mapRequestToSlot[ requestCode ] = std::make_pair( fpNewPacketSubscriber, pParam ); };
	/// �������κ��� �޴� ��Ŷ���� Ư�� �����ڵ忡 ���� Ư�� �Լ��� Ǫ������ ���� ��û�մϴ�, RES_NONE �� ����
	void RegisterMessageSlot( typename TyTraits::tyResCode responseCode, typename TyTraits::tySigNewPacket fpNewPacketSubscriber, PVOID pParam )
	{ _mapResponseToSlot[ responseCode ] = std::make_pair( fpNewPacketSubscriber, pParam ); };
 	void RegisterConnectSlot( typename TyTraits::tySigConnection fpConnectSubscriber, PVOID pParam )
	{ _vecNewconnectSlot.push_back( std::make_pair( fpConnectSubscriber, pParam ) ); };
 	void RegisterDisconnectSlot( typename TyTraits::tySigConnection fpDisconnectSubscriber, PVOID pParam )
	{ _vecDisconnectSlot.push_back( std::make_pair( fpDisconnectSubscriber, pParam ) ); }

    void RegisterPacketMonitorSlot( typename TyTraits::tySigPacketMonitor fpPacketMonitorSubscriver, PVOID pParam )
    { _vecPacketMonitorSlot.push_back( std::make_pair( fpPacketMonitorSubscriver, pParam ) ); }

    boost::shared_ptr< CNetworkCommunicator > GetNetworkCommunicator() { return _spNetworkManager; }
private:
    /// ����� �����ϴ� �޽����� ���̸� ���ڵ���. ����� ���ũ�� 4����ũ�� ������
    void                        encode_header( tyDataBuffer& buffer, unsigned size ) const;

    void                        dispatchReceiveMessage();
    void                        dispatchSendMessage();

    static void                 receiveNewConnection( const PVOID pParam, const SOCK_INDEX socketIndex, const tySpCommonSocket& newConnection );
    static void                 queuePacketFromDataBuffer( const PVOID pParam, tySpCommonSocket receivedSocket, const tyDataBuffer& dataBuffer );
    static void                 receiveDisconnection( const PVOID pParam, const SOCK_INDEX socketIndex );
};

template< typename TyTraits >
CGPBManagerV2< TyTraits >::CGPBManagerV2( int nDispatcherThreadCount /* = 0 */ )
    : _isBusy(false), _isExit(false), _nSendPacketIntervalsWhenBusySecs(10), _isUseWaitingWhenServerBusy(false), _globalMessageID(0)
{
    LM_TRACE_TO( gpbmanagerv2_logger, "������ ����", "" );

    InitializeCriticalSectionAndSpinCount( &_lckQueue, 4000 );
    InitializeCriticalSectionAndSpinCount( &_lckSendQueue, 4000 );
    InitializeCriticalSectionAndSpinCount( &_lckSockets, 4000 );

    _spNetworkManager = createNetworkCommunicator();

    _spNetworkManager->registerNewConnectionNotify( &CGPBManagerV2< TyTraits >::receiveNewConnection, this );
    _spNetworkManager->registerDisconnectionNotify( &CGPBManagerV2< TyTraits >::receiveDisconnection, this );

    if( nDispatcherThreadCount <= 0 )
    {
        SYSTEM_INFO si;
        ::GetSystemInfo( &si );
        nDispatcherThreadCount = si.dwNumberOfProcessors + ( si.dwNumberOfProcessors / 2 );
    }
    LM_DEBUG_TO( gpbmanagerv2_logger, "�޽��� �й� ������ ���� = %1", nDispatcherThreadCount );

    for( int idx = 0; idx < nDispatcherThreadCount; ++idx )
        _workThreadGroup.add_thread( 
            new boost::thread( boost::bind( &CGPBManagerV2< TyTraits >::dispatchReceiveMessage, this ) )
            );

    _workThreadGroup.add_thread( new boost::thread( boost::bind( &CGPBManagerV2< TyTraits >::dispatchSendMessage, this ) ) );
    LM_TRACE_TO( gpbmanagerv2_logger, "������ ����", "" );
}

template< typename TyTraits >
CGPBManagerV2< TyTraits >::~CGPBManagerV2()
{
    _isExit = true;
    _incomingQueue.stopAllPop();
    _outgoingQueue.stopAllPop();

    _workThreadGroup.join_all();

    DeleteCriticalSection( &_lckQueue );
    DeleteCriticalSection( &_lckSendQueue );
    DeleteCriticalSection( &_lckSockets );
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::setListenPort( unsigned short listenPort, const std::string& ipaddress /*= "*.*.*.*" */ )
{
    return _spNetworkManager->bind_and_accept( listenPort, ipaddress );
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::connectTo( const std::string& address, unsigned int port )
{
    bool isSuccess = false;

    do 
    {
        SOCK_INDEX sockIndex = _spNetworkManager->connectTo( address, port );
        if( sockIndex > 0 )
            isSuccess = true;
    } while (false);

    return isSuccess;
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::connectTo( const std::string& address, unsigned int port, SOCK_INDEX& socketIndex )
{
    bool isSuccess = false;

    do 
    {
        socketIndex = _spNetworkManager->connectTo( address, port );
        if( socketIndex > 0 )
            isSuccess = true;
    } while (false);

    return isSuccess;
}

template< typename TyTraits >
tyVecSendResult CGPBManagerV2<TyTraits>::sendPacket( typename TyTraits::tyPacket& packet, bool autoAssignMsgID /*= true */ )
{
    tyVecSendResult vecResult;

    CScopedCriticalSection lck( _lckSockets );
    for( std::set< tySpCommonSocket >::iterator it = _setConnectionedSockets.begin(); it != _setConnectionedSockets.end(); ++it )
        vecResult.push_back( std::make_pair( 
        _spNetworkManager->getSocketIndex( (*it) ), sendPacket( (*it), packet, autoAssignMsgID ) 
        ) );

    return vecResult;
}

template< typename TyTraits >
int CGPBManagerV2<TyTraits>::sendPacket( int socketIndex, typename TyTraits::tyPacket& packet, bool autoAssignMsgID /*= true */ )
{
    if( socketIndex <= 0 )
        return -1;

    tySpCommonSocket sendSocket = _spNetworkManager->getSocket( socketIndex );
    if( sendSocket == NULLPTR )
        return -2;

    if( autoAssignMsgID == true )
        packet.set_msgid( ::InterlockedIncrement( &_globalMessageID ) );
    
    _outgoingQueue.push( std::make_pair( sendSocket, packet ) );
    return packet.msgid();
}

template< typename TyTraits >
int CGPBManagerV2<TyTraits>::sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, bool autoAssignmsgID /*= true */ )
{
    if( sendSocket == NULLPTR )
        return -1;

    if( autoAssignmsgID == true )
        packet.set_msgid( ::InterlockedIncrement( &_globalMessageID ) );
    
    _outgoingQueue.push( std::make_pair( sendSocket, packet ) );
    return packet.msgid();
}

template< typename TyTraits >
int CGPBManagerV2<TyTraits>::sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, int msgID )
{
    packet.set_msgid( msgID );
    return sendPacket( sendSocket, packet, false );
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::receivePacket( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet )
{
    bool isReceiveSuccess = false;

    do 
    {
        if( socketIndex <= 0 )
            break;

        if( msgID < 0 )
            break;

        CScopedCriticalSection lck( _lckQueue );
        if( _mapSocketToResPacket.find( socketIndex ) == _mapSocketToResPacket.end() )
            break;

        TyTraits::tyMapMsgIdToMsgPacketEx* mapMsgIdToMsgPacket = _mapSocketToResPacket.find( socketIndex )->second;
        if( mapMsgIdToMsgPacket->find( msgID ) == mapMsgIdToMsgPacket->end() )
            break;
        
        packet = mapMsgIdToMsgPacket->find( msgID )->second.second;
        mapMsgIdToMsgPacket->erase( msgID );

        isReceiveSuccess = true;
    } while (false);

    return isReceiveSuccess;
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::receivePacket( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet )
{
    return receivePacket( prSendResult.first, prSendResult.second, packet );
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::receivePacketWait( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet, int timoueMs /*= 10*/, int tryout /*= 1000 */ )
{
    int tryCount = 0;

    bool isSuccess = false;
    while( isSuccess == false && tryCount++ < tryout )
    {
        isSuccess = receivePacket( socketIndex, msgID, packet );
        SleepEx( timoueMs, FALSE );
    }
    return isSuccess;
}

template< typename TyTraits >
bool CGPBManagerV2<TyTraits>::receivePacketWait( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet, int timoueMs /*= 10*/, int tryout /*= 1000 */ )
{
    return receivePacketWait( prSendResult.first, prSendResult.second, packet, timoueMs, tryout );
}

//////////////////////////////////////////////////////////////////////////

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::encode_header( tyDataBuffer& buffer, unsigned size ) const
{
    assert( buffer.size() >= 4 /* ��� ũ�� */ );

    buffer[0] = static_cast<boost::uint8_t>((size >> 24) & 0xFF);
    buffer[1] = static_cast<boost::uint8_t>((size >> 16) & 0xFF);
    buffer[2] = static_cast<boost::uint8_t>((size >> 8) & 0xFF);
    buffer[3] = static_cast<boost::uint8_t>(size & 0xFF);
}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::dispatchReceiveMessage()
{
    /*!
        ���� ���� �޽����� ������� ó���Ѵ�.  
        
        #. ���Ź��� ��Ŷ�� ��Ŷ ��Ȯ���� Ȯ���Ѵ�. 
        #. ��Ŷ�� ��û�ڵ�/�����ڵ带 �����Ͽ� ó�� �ڵ鷯�� �����Ѵ�.
        #. ó�� �ڵ鷯�� ���������� ó���ǰ� ��ó�� �ڵ鷯�� ��ϵǾ� �ִٸ� ��ó�� �ڵ鷯�� ��Ŷ�� �����Ѵ�.  
        
        �� ���� dispatch �����尡 ���ÿ� �޽����� �������� ������ �� �޽��������� ó�� ������ ������ ����. 
    */

    bool isProcess                              = false;
    TyTraits::tyPrPacketData                    packetData;
    std::pair< TyTraits::tyPacket, bool >       prResult;
    while( _isExit == false )
    {
        _incomingQueue.pop( packetData );
        if( _isExit == true )
            break;

        isProcess = false;

        
        std::pair< typename TyTraits::tySigNewPacket, PVOID > prConnectionToParam;

        if( packetData.second.reqcode() != TyTraits::tyReqCode::REQ_NONE )
        {
            if( _mapRequestToSlot.find( packetData.second.reqcode() ) != _mapRequestToSlot.end() )
            {
                isProcess = true;
                prConnectionToParam = _mapRequestToSlot[ packetData.second.reqcode() ];
            }
        }
        else if( packetData.second.rescode() != TyTraits::tyResCode::RES_NONE )
        {
            if( _mapResponseToSlot.find( packetData.second.rescode() ) != _mapResponseToSlot.end() )
            {
                isProcess = true;
                prConnectionToParam = _mapResponseToSlot[ packetData.second.rescode() ];
            }
        }

        if( isProcess == true )
        {
            prResult = (*prConnectionToParam.first)( (PVOID)prConnectionToParam.second, (PVOID)NULLPTR, packetData.first.get(), packetData.second );
            if( prResult.second == true )
                sendPacket( packetData.first, prResult.first, prResult.first.msgid() );
        }
        else
        {
            SOCK_INDEX socketIndex = _spNetworkManager->getSocketIndex( packetData.first );
            do 
            {
                if( socketIndex <= 0 )
                    break;

                CScopedCriticalSection lck( _lckQueue );
                if( _mapSocketToResPacket.find( socketIndex ) == _mapSocketToResPacket.end() )
                    _mapSocketToResPacket[ socketIndex ] = new TyTraits::tyMapMsgIdToMsgPacketEx();

                TyTraits::tyMapMsgIdToMsgPacketEx* mapMsgIdToMsgPacket = _mapSocketToResPacket.find( socketIndex )->second;
                mapMsgIdToMsgPacket->insert( std::make_pair( packetData.second.msgid(), std::make_pair( _time64(NULL), packetData.second ) ) );

            } while (false);
        }
    }
}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::dispatchSendMessage()
{
    /*!
        #. ť���� ��Ŷ �����͸� ���� �� ���ۿ� ����ȭ��
        #. SERVER_BUSY ����� ��� ������ Ȯ��
            #. ��� ��� : ���� busy = true ���� Ȯ��
                #. busy = true : 
                    #. ������ �ð� ��ŭ ���
                    #. ���� �޽��� �ʿ���  
                #. busy = false : ���� �޽��� �ʿ� �ش� ��Ŷ�� ���� �� ��Ŷ ����
                
            #. ��� �̻�� : ��Ŷ ��� ����
    */

    std::vector< BYTE >                         writeBuffer;
    TyTraits::tyPrPacketData                    packetData;
    while( _isExit == false )
    {
        if( _isUseWaitingWhenServerBusy == true && _isBusy == true && _isExit == false )
        {
            SleepEx( _nSendPacketIntervalsWhenBusySecs * 1000, TRUE );
            continue;
        }

        _outgoingQueue.pop( packetData );
        if( _isExit == true )
            break;

        writeBuffer.resize( PACKET_HEADER_SIZE + packetData.second.ByteSize() );
        encode_header( writeBuffer, packetData.second.ByteSize() );
        packetData.second.SerializeToArray( &writeBuffer[ PACKET_HEADER_SIZE ], packetData.second.ByteSize() );
        
        if( _isUseWaitingWhenServerBusy == true )
        {
            if( _isBusy == true )
                continue;

            SOCK_INDEX socketIndex = _spNetworkManager->getSocketIndex( packetData.first );
            if( socketIndex > 0 )
            {
                CScopedCriticalSection lck( _lckSendQueue );
                if( _mapSocketToSendedPacket.find( socketIndex ) != _mapSocketToSendedPacket.end() )
                {
                    TyTraits::tyMapMsgIdToMsgPacket* mapMsgIdToMsgPacket = _mapSocketToSendedPacket.find( socketIndex )->second;
                    mapMsgIdToMsgPacket->insert( std::make_pair( packetData.second.msgid(), packetData.second ) );
                }
            }
        }

        for( size_t idx = 0; idx < _vecPacketMonitorSlot.size(); ++idx )
            (*_vecPacketMonitorSlot[idx].first)( _vecPacketMonitorSlot[idx].second, packetData.first.get(), false, time(NULL), packetData.second );

        packetData.first->sendPacket( writeBuffer );

    }
}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::receiveNewConnection( const PVOID pParam, const SOCK_INDEX socketIndex, const tySpCommonSocket& newConnection )
{
    static CGPBManagerV2< TyTraits >* pManager = static_cast< CGPBManagerV2< TyTraits >* >( pParam );

    {
        CScopedCriticalSection lck( pManager->_lckSockets );
        pManager->_setConnectionedSockets.insert( newConnection );
    }

    newConnection->registerQueuePacket( &CGPBManagerV2< TyTraits >::queuePacketFromDataBuffer, pManager );

    {
        CScopedCriticalSection lck( pManager->_lckSendQueue );
        if( pManager->_mapSocketToSendedPacket.find( socketIndex ) == pManager->_mapSocketToSendedPacket.end() )
            pManager->_mapSocketToSendedPacket[ socketIndex ] = new TyTraits::tyMapMsgIdToMsgPacket;
    }
}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::queuePacketFromDataBuffer( const PVOID pParam, tySpCommonSocket receivedSocket, const tyDataBuffer& dataBuffer )
{
    /*!
        #. ���Ź��� �����͸� ���� �������� ���� �޽����� ��ȯ
        #. SERVER_BUSY ����� ��� ������ Ȯ��
            #. ��� ��� : ��Ŷ�� resCode �� RES_SERVER_BUSY ���� Ȯ��
                #. ��Ŷ�� resCode �� RES_SERVER_BUSY �̸� �ش� ��Ŷ�� ������, GPBManager �� busy = true �� ������, ������ �����ߴ� ��Ŷ�� ���� �޽��� �ʿ��� ã�Ƽ� ���� ��� ť�� ����
                #. ��Ŷ�� resCode �� RES_SERVER_BUSY �� �ƴϸ� incomingQueue �� ������ �޽����� ����
            #. ��� �̻�� : incomingQueue �� ����
    */

    TyTraits::tyPacket receivedPacket;
    static CGPBManagerV2< TyTraits >* pManager = static_cast< CGPBManagerV2< TyTraits >* >( pParam );

    do 
    {
        if( receivedPacket.ParseFromArray( &dataBuffer[ PACKET_HEADER_SIZE ], (int)dataBuffer.size() - PACKET_HEADER_SIZE ) == false )
            break;

        LM_TRACE_TO( gpbmanagerv2_logger, "��Ʈ��ũ ��Ŷ ����, ��Ŷ ������ = %1", receivedPacket.ShortDebugString() );

        for( size_t idx = 0; idx < pManager->_vecPacketMonitorSlot.size(); ++idx )
            (*pManager->_vecPacketMonitorSlot[idx].first)( pManager->_vecPacketMonitorSlot[idx].second, receivedSocket.get(), true, time(NULL), receivedPacket );
        
        if( pManager->_isUseWaitingWhenServerBusy == true )
        {
            SOCK_INDEX socketIndex = pManager->_spNetworkManager->getSocketIndex( receivedSocket );

            do 
            {
                if( socketIndex <= 0 )
                    break;

                if( receivedPacket.rescode() == pManager->_resCodeServerBusy )
                    pManager->_isBusy = true;
                else
                    pManager->_isBusy = false;

                CScopedCriticalSection lck( pManager->_lckSendQueue );
                TyTraits::tyMapMsgIdToMsgPacket* mapMsgIDToMsgpacket = pManager->_mapSocketToSendedPacket.find( socketIndex )->second;

                if( mapMsgIDToMsgpacket->find( receivedPacket.msgid() ) != mapMsgIDToMsgpacket->end() )
                {
                    if( pManager->_isBusy == true )
                    {
                        pManager->_outgoingQueue.push( std::make_pair( receivedSocket, (*mapMsgIDToMsgpacket)[ receivedPacket.msgid() ] ) );
                    }
                    else
                    {
                        pManager->_incomingQueue.push( std::make_pair( receivedSocket, receivedPacket ) );
                    }
                    mapMsgIDToMsgpacket->erase( receivedPacket.msgid() );
                }

            } while (false);
        }
        else
        {
            pManager->_incomingQueue.push( std::make_pair( receivedSocket, receivedPacket ) );
        }

    } while (false);

}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::receiveDisconnection( const PVOID pParam, const SOCK_INDEX socketIndex )
{
    static CGPBManagerV2< TyTraits >* pManager = static_cast< CGPBManagerV2< TyTraits >* >( pParam );
    LM_TRACE_TO( gpbmanagerv2_logger, "���� ���� �뺸 ����, ���� ���� ��ȣ = %1", socketIndex );

    /*!
        #. ���� ����� ���� ��Ͽ��� ���� ����
        #. ���� �޽��� �ʿ��� ������ ���� ���Ͼ� ���� ������ ����
        #. ���� �޽��� �ʿ��� ������ ���� ���Ͽ� ���� ������ ����
    */

    {
        CScopedCriticalSection lck( pManager->_lckSockets );
        pManager->_setConnectionedSockets.erase( pManager->_spNetworkManager->getSocket( socketIndex ) );
    }

    {
        // ���� �޽��� �ʿ��� ������ ���� ���Ͽ� ���� ������ ����
        CScopedCriticalSection lck( pManager->_lckSendQueue );
        if( pManager->_mapSocketToSendedPacket.find( socketIndex ) != pManager->_mapSocketToSendedPacket.end() )
        {
            delete pManager->_mapSocketToSendedPacket.find( socketIndex )->second;
            pManager->_mapSocketToSendedPacket.erase( socketIndex );
        }
    }

}

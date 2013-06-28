#pragma once

/*!
    GPBManager V2

    개선사항
        #. 서버의 응답에 따라 전송했던 패킷에 대한 재전송 기능 추가


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
        #. 클라이언트에서 패킷 전송을 요청하면 전송이 성공한 패킷에 대해서 별도의 큐에 넣어둠
        #. 서버에서 바쁘다는 응답이 오면 별도의 큐에서 패킷 재전송
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
        SERVER_BUSY 기능, 서버가 SERVER_BUSY 등의 응답코드를 전송하면 일정 간격으로 쉰 후 패킷을 재전송한다.
    */
    boost::atomic<bool>                                                                     _isBusy;
    volatile bool                                                                           _isUseWaitingWhenServerBusy;
    typename TyTraits::tyResCode                                                            _resCodeServerBusy;
    boost::unordered_map< SOCK_INDEX, typename TyTraits::tyMapMsgIdToMsgPacket* >           _mapSocketToSendedPacket;
    unsigned int                                                                            _nSendPacketIntervalsWhenBusySecs;

public:
    CGPBManagerV2( int nDispatcherThreadCount = 0 );
    ~CGPBManagerV2();

    /// 연결대기할 포트 번호와 응답을 받을 IP 주소를 적음, 기본적으로 모든 IP 에 대하여 응답함
    bool                        setListenPort( unsigned short listenPort, const std::string& ipaddress = "*.*.*.*" );
    /// 지정한 주소와 포트 번호를 사용하여 해당 컴퓨터에 연결
    bool                        connectTo( const std::string& address, unsigned int port );
    bool                        connectTo( const std::string& address, unsigned int port, SOCK_INDEX& socketIndex );

    /// 지정한 소켓 색인번호와의 연결을 해제함
    void                        disconnect( SOCK_INDEX socketIndex ) { _spNetworkManager->disconnect( socketIndex ); };
    /// 관리자가 관리하는 모든 소켓 색인번호와의 연결을 해제함
    void                        disconnectAll() { _spNetworkManager->disconnectAll(); };

    void                        setIsUseServerBusyWaiting( bool isUse ) { _isUseWaitingWhenServerBusy = isUse; }
    bool                        getIsUseServerBusyWaiting() { return _isUseWaitingWhenServerBusy; }

    void                        setSendPacketIntervalsWhenBusy( unsigned int nSendIntervalSecs = 10 ) { _nSendPacketIntervalsWhenBusySecs = nSendIntervalSecs; }
    unsigned int                getSendPacketIntervalsWhenBusy() { return _nSendPacketIntervalsWhenBusySecs; }

    void                        setServerBusyResCode( typename TyTraits::tyResCode resCode ) { _resCodeServerBusy = resCode; }
    typename TyTraits::tyResCode getServerBusyResCode() { return _resCodeServerBusy; }

	/*!
		해당 관리자가 관리하는 모든 소켓에게 작성한 패킷을 전송합니다. 

		@return 소켓이 여러개 연결되어 있는 것을 가정하여 소켓 색인번호와 전송한 패킷에 부여된 메시지 번호가 짝을 이룬 벡터를 반환합니다. 
		부여된 해당 메시지 번호를 이용하여 응답패킷을 검색합니다. 메시지 번호 < 0 이면 오류
	*/
 	tyVecSendResult             sendPacket( typename TyTraits::tyPacket& packet, bool autoAssignMsgID = true );

	/*! 
		지정한 소켓 색인번호에 연결된 소켓으로 작성한 패킷을 전송합니다. 

        @return 해당 패킷에 부여된 메시지번호를 반환합니다. 해당 번호를 이용하여 응답패킷을 검색합니다. 
        @return 메시지 번호 < 0 이면 오류
	*/
    int                         sendPacket( int socketIndex, typename TyTraits::tyPacket& packet, bool autoAssignMsgID = true );
    int                         sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, bool autoAssignmsgID = true );
    int                         sendPacket( tySpCommonSocket sendSocket, typename TyTraits::tyPacket& packet, int msgID );

	/*!
		해당 메시지번호에 맞는 응답패킷이 도착하였는지 검색합니다. 	

		ReceivePacket 은 RegisterMessageSlot 으로 콜백함수를 등록하여 우선적으로 전달받도록 요청한 패킷을 제외한 나머지 응답패킷에 대해서 확인합니다. 
		메시지 슬롯을 통해 응답패킷이 직접 전달되었다면, ReceivePacket 호출을 사용한 응답 메시지 검색은 실패합니다. 

		ReceivePacketWait 은 일정시간동안 내부에서 응답 패킷을 대기합니다. 응답을 대기하는 동안 ~Wait 함수를 호출한 함수가 속한 스레드는 대기합니다. 
	*/
    bool                        receivePacket( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet );
    bool                        receivePacket( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet );
    bool                        receivePacketWait( SOCK_INDEX socketIndex, int msgID, typename TyTraits::tyPacket& packet, int timoueMs = 10, int tryout = 1000 );
    bool                        receivePacketWait( const std::pair< SOCK_INDEX, int >& prSendResult, typename TyTraits::tyPacket& packet, int timoueMs = 10, int tryout = 1000 );

	/*!
		등록한 메시지 슬롯은 네트워크 관리자 클래스를 삭제하고 다시 생성하기 전에는 변경할 수 없음!!!
	*/
	/// 소켓으로부터 받는 패킷에서 특정 요청코드에 대해 특정 함수로 푸시해줄 것을 요청합니다, REQ_NONE 은 제외
	void RegisterMessageSlot( typename TyTraits::tyReqCode requestCode, typename TyTraits::tySigNewPacket fpNewPacketSubscriber, PVOID pParam )
	{ _mapRequestToSlot[ requestCode ] = std::make_pair( fpNewPacketSubscriber, pParam ); };
	/// 소켓으로부터 받는 패킷에서 특정 응답코드에 대해 특정 함수로 푸시해줄 것을 요청합니다, RES_NONE 은 제외
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
    /// 헤더에 전송하는 메시지의 길이를 인코드함. 현재는 헤더크기 4바이크만 지원함
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
    LM_TRACE_TO( gpbmanagerv2_logger, "생성자 시작", "" );

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
    LM_DEBUG_TO( gpbmanagerv2_logger, "메시지 분배 스레드 수량 = %1", nDispatcherThreadCount );

    for( int idx = 0; idx < nDispatcherThreadCount; ++idx )
        _workThreadGroup.add_thread( 
            new boost::thread( boost::bind( &CGPBManagerV2< TyTraits >::dispatchReceiveMessage, this ) )
            );

    _workThreadGroup.add_thread( new boost::thread( boost::bind( &CGPBManagerV2< TyTraits >::dispatchSendMessage, this ) ) );
    LM_TRACE_TO( gpbmanagerv2_logger, "생성자 종료", "" );
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
    assert( buffer.size() >= 4 /* 헤더 크기 */ );

    buffer[0] = static_cast<boost::uint8_t>((size >> 24) & 0xFF);
    buffer[1] = static_cast<boost::uint8_t>((size >> 16) & 0xFF);
    buffer[2] = static_cast<boost::uint8_t>((size >> 8) & 0xFF);
    buffer[3] = static_cast<boost::uint8_t>(size & 0xFF);
}

template< typename TyTraits >
void CGPBManagerV2<TyTraits>::dispatchReceiveMessage()
{
    /*!
        수신 받은 메시지를 순서대로 처리한다.  
        
        #. 수신받은 패킷의 패킷 정확성을 확인한다. 
        #. 패킷의 요청코드/응답코드를 조사하여 처리 핸들러로 전달한다.
        #. 처리 핸들러가 정상적으로 처리되고 후처리 핸들러가 등록되어 있다면 후처리 핸들러로 패킷을 전달한다.  
        
        ※ 여러 dispatch 스레드가 동시에 메시지를 꺼내가기 때문에 각 메시지간에는 처리 순서의 보장은 없다. 
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
        #. 큐에서 패킷 데이터를 꺼낸 후 버퍼에 직렬화함
        #. SERVER_BUSY 기능을 사용 중인지 확인
            #. 기능 사용 : 현재 busy = true 인지 확인
                #. busy = true : 
                    #. 설정한 시간 만큼 대기
                    #. 전송 메시지 맵에서  
                #. busy = false : 전송 메시지 맵에 해당 패킷을 넣은 후 패킷 전송
                
            #. 기능 미사용 : 패킷 즉시 전송
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
        #. 수신받은 데이터를 구글 프로토콜 버퍼 메시지로 변환
        #. SERVER_BUSY 기능을 사용 중인지 확인
            #. 기능 사용 : 패킷의 resCode 가 RES_SERVER_BUSY 인지 확인
                #. 패킷의 resCode 가 RES_SERVER_BUSY 이면 해당 패킷은 버리며, GPBManager 는 busy = true 로 설정됨, 기존에 전송했던 패킷을 전송 메시지 맵에서 찾아서 전송 대기 큐에 넣음
                #. 패킷의 resCode 가 RES_SERVER_BUSY 가 아니면 incomingQueue 에 수신한 메시지를 넣음
            #. 기능 미사용 : incomingQueue 에 넣음
    */

    TyTraits::tyPacket receivedPacket;
    static CGPBManagerV2< TyTraits >* pManager = static_cast< CGPBManagerV2< TyTraits >* >( pParam );

    do 
    {
        if( receivedPacket.ParseFromArray( &dataBuffer[ PACKET_HEADER_SIZE ], (int)dataBuffer.size() - PACKET_HEADER_SIZE ) == false )
            break;

        LM_TRACE_TO( gpbmanagerv2_logger, "네트워크 패킷 도착, 패킷 데이터 = %1", receivedPacket.ShortDebugString() );

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
    LM_TRACE_TO( gpbmanagerv2_logger, "연결 끊김 통보 받음, 소켓 색인 번호 = %1", socketIndex );

    /*!
        #. 현재 연결된 소켓 목록에서 소켓 제거
        #. 응답 메시지 맵에서 연결이 끊긴 소켓애 대한 데이터 삭제
        #. 전송 메시지 맵에서 연결이 끊긴 소켓에 대한 데이터 삭제
    */

    {
        CScopedCriticalSection lck( pManager->_lckSockets );
        pManager->_setConnectionedSockets.erase( pManager->_spNetworkManager->getSocket( socketIndex ) );
    }

    {
        // 전송 메시지 맵에서 연결이 끊긴 소켓에 대한 데이터 삭제
        CScopedCriticalSection lck( pManager->_lckSendQueue );
        if( pManager->_mapSocketToSendedPacket.find( socketIndex ) != pManager->_mapSocketToSendedPacket.end() )
        {
            delete pManager->_mapSocketToSendedPacket.find( socketIndex )->second;
            pManager->_mapSocketToSendedPacket.erase( socketIndex );
        }
    }

}

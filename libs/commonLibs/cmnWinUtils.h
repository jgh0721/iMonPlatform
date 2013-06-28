#ifndef CMNWINUTILS_H
#define CMNWINUTILS_H

#include <string>
#include <queue>
#include <list>
#include <vector>
#include <unordered_map>

#include <boost/thread.hpp>
#include <boost/thread/mutex.hpp>

#include <Windows.h>
#include <WinSock2.h>
#include <UrlMon.h>
#pragma comment(lib, "urlmon" )

#ifndef _INC_COMUTIL
#include <comutil.h>
#endif

namespace nsCommon
{
    class CScopedCriticalSection
    {
    public:
        explicit CScopedCriticalSection( CRITICAL_SECTION& cs );
        ~CScopedCriticalSection();

    private:
        CRITICAL_SECTION&		m_cs;
    };

    class CCriticalSectionEx
    {
    public:
        CCriticalSectionEx() { InitializeCriticalSectionAndSpinCount( &m_cs, 4000 ); };
        ~CCriticalSectionEx() { DeleteCriticalSection( &m_cs ); };

        void Lock() { EnterCriticalSection( &m_cs ); };
        void Unlock() { LeaveCriticalSection( &m_cs ); };

    private:
        CRITICAL_SECTION        m_cs;
    };

    template< typename T >
    class CLocker
    {
        T& _obj;
    public:
        CLocker( T& obj ) : _obj( obj )
        {
            _obj.Lock();
        }

        ~CLocker()
        {
            _obj.Unlock();
        }
    };

    template< typename T >
    class CSharedQueue
    {
        CRITICAL_SECTION                _cs;
        boost::condition_variable       _cv;
        std::queue<T>                   _queue;

    public:
        CSharedQueue() { ::InitializeCriticalSectionAndSpinCount( &_cs, 4000 ); } ;
        ~CSharedQueue() {  ::DeleteCriticalSection( &_cs ); };

        void Push( const T& item ) 
        {
            CScopedCriticalSection lck( _cs );
            _queue.push( item );
            _cv.notify_one();
        };

#if _MSC_VER >= 1600 
        void Push( T&& item )
        {
            CScopedCriticalSection lck( _cs );
            _queue.emplace( item );
            _cv.notify_one();
        }
#endif

        bool Try_and_pop( T& poppedItem )
        {
            CScopedCriticalSection lck( _cs );
            if( _queue.empty() == true )
                return false;
#if _MSC_VER >= 1600 
            poppedItem = std::move( _queue.front() );
#else
            poppedItem = _queue.front();
#endif
            _queue.pop();
            return true;
        }

        void Wait_and_pop( T& poppedItem )
        {
            CScopedCriticalSection lck( _cs );
            while( _queue.empty() ) { ::SleepEx(1, FALSE);}
#if _MSC_VER >= 1600 
            poppedItem = std::move( _queue.front() );
#else
            poppedItem = _queue.front();
#endif
            _queue.pop();
        }

        bool IsEmpty() const 
        {
            CScopedCriticalSection lck( _cs );
            return _queue.empty();
        }

        size_t Size() const 
        {
            CScopedCriticalSection lck( _cs );
            return _queue.size();
        }

    };

    /*!
        CCmnObject 를 상속받은 객체들을 CCmnObjectHolder 를 통해 CCmnObjectPool 로 관리

        http://www.codeproject.com/Articles/8108/Template-based-Generic-Pool-using-C
    */
    class CCmnObject
    {
    public:
        CCmnObject();
        ~CCmnObject();

        virtual void Init() {};
        virtual void Release() {};
        virtual bool IsUsable() { return true; }

        virtual bool MakeUsable()
        {
            if( !IsUsable() ) { Init(); }
            return true;
        }
    };

    template< typename T >
    class CCmnObjectHolder
    {
    public:
        CCmnObjectHolder() : _pObj( NULLPTR ), _nTimeStamp(-1) {};
        ~CCmnObjectHolder() { ReleasePtr< T* >( _pObj ); };

        void InitObject() 
        {
            if( _pObj == NULLPTR )
            {
                _pObj = new T();
                _pObj->Init();
            }
        }

        T* GetObj() { return _pObj; };
        void SetObj( T* object = NULLPTR ) { _pObj = object; };
        
        long GetTimeStamp() { return _nTimeStamp; };
        void SetTimeStamp( long nTime ) { _nTimeStamp = nTime; };

        void Set( T* pObj, long nTime ) { _pObj = pObj; _nTimeStamp = nTime; };
    private:
        T*      _pObj;
        long    _nTimeStamp; 
    };

    template< typename T >
    class CCmnObjectPool
    {
        typedef CCmnObjectHolder< T >       TyObjectHolder;
        typedef std::list< TyObjectHolder > TyObjectHolderList;

        static CCriticalSectionEx           _csPool;
        CCriticalSectionEx                  _csData;
    public:
        CCmnObjectPool()
        { };

        ~CCmnObjectPool()
        { };


        void                                InitPool( unsigned int nMaxPoolSize, long unsigned int nWaitTime = 3 )
        {
            _nMaxPoolSize = nMaxPoolSize;
            _nWaitTime = nWaitTime;
        }

    private:
        T*                                  createObject()
        {

            return new T();
        }

        T*                                  findObject()
        {
            return new T();
        }

        unsigned int                        _nMaxPoolSize;
        unsigned int                        _nWaitTime;
        TyObjectHolderList                  _lstReservedObj;
        TyObjectHolderList                  _lstFreeObj;

    };

    template< typename T >
    class CConcurrentQueue : public boost::noncopyable
    {
    public:
        CConcurrentQueue() : _isStop(false) {}

        void pop( T& val )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            while( _queue.empty() )
                _cvCond.wait( lck );

            if( _isStop == true )
                return;

            val = _queue.front();
            _queue.pop();
        }

        void push( const T& item )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            _queue.push( item );
            lck.unlock();
            _cvCond.notify_one();
        }

#if _MSC_VER >= 1600 
        void push( T&& item )
        {
            boost::unique_lock< boost::mutex > lck( _mutex );
            _queue.push( item );
            lck.unlock();
            _cvCond.notify_one();
        }
#endif

        void stopAllPop() 
        { 
            _isStop = true; 
            _cvCond.notify_all();
        }

    private:
        volatile bool               _isStop;
        std::queue< T >             _queue;
        boost::mutex                _mutex;
        boost::condition_variable   _cvCond;
    };

    class CBenchTimer
    {
    public:
        CBenchTimer();
        ~CBenchTimer();

        void        step();
        // 이전에 호출한 step 과 step 사이의 경과 시간, 단위는 ms, step 를 한번밖에 호출하지 않았다면, 처음 부터의 경과시간을 구한다. 
        float       getElapsedTimeFromPrev();
        // 맨 처음 클래스를 생성한 시간부터의 경과 시간, 단위는 ms
        float       getElapsedTimeFromStart();

    private:
        float       _checkTimeRun;
        __int64     _checkTimeStart;
        __int64     _checkTimeFreq;
        __int64     _checkTimePrev;
        __int64     _checkTimeCurrent;
    };
    
    template< typename T >
    class CRoundRobinAllocator
    {
    public:
        CRoundRobinAllocator( size_t maxPoolSize = 256 )
            : _lCurrentIndex(0), _poolSize( maxPoolSize )
        {
            ::InitializeCriticalSectionAndSpinCount( &_cs, 4000 );
        }

        ~CRoundRobinAllocator()
        {
            ::DeleteCriticalSection( &_cs );
        }

        void CreatePool()
        {
            CScopedCriticalSection lck( _cs );

            for( size_t idx = 0; idx < _poolSize; ++idx )
            {
                _vecPool.push_back( new T() );
                _mapUsingTable[ idx ] = false;
            }
        }

        void DeletePool()
        {
            CScopedCriticalSection lck( _cs );

            for( size_t idx = 0; idx < _vecPool.size(); ++idx )
                delete _vecPool[ idx ];
        }

        T*  CheckOut()
        {
            CScopedCriticalSection lck( _cs );

            if( _lCurrentIndex >= _poolSize )
                _lCurrentIndex = 0;

            if( _mapUsingTable[ _lCurrentIndex ] == true )
                return NULLPTR;

            _mapUsingTable[ _lCurrentIndex ] = true;
            return _vecPool[ _lCurrentIndex++ ];
        }

        void CheckIn( T* object )
        {
            CScopedCriticalSection lck( _cs );
            for( size_t idx = 0; idx < _poolSize; ++idx )
            {
                if( _vecPool[ idx ] == object )
                {
                    _mapUsingTable[ idx ] = false;
                    break;
                }
            }
        }

    private:
        size_t                                  _poolSize;
        CRITICAL_SECTION                        _cs;
        std::vector< T* >                       _vecPool;
		
#if _MSC_VER >= 1600
		std::unordered_map< size_t, bool >      _mapUsingTable;
#else
		std::tr1::unordered_map< size_t, bool >      _mapUsingTable;
#endif      
        
		size_t                                  _lCurrentIndex;
    };

    #pragma pack(push,8)
    typedef struct tagTHREADNAME_INFO
    {
        DWORD dwType; // Must be 0x1000.
        LPCSTR szName; // Pointer to name (in user addr space).
        DWORD dwThreadID; // Thread ID (-1=caller thread).
        DWORD dwFlags; // Reserved for future use, must be zero.
    } THREADNAME_INFO;
    #pragma pack(pop)
#ifdef _MSC_VER
    void SetThreadName( DWORD dwThreadID, LPCSTR pszThreadName );
#endif

    void SetServiceToAutoRestart( const std::wstring& serviceName, int restartDelayMs = 1000 * 60, int resetPeriodSec = 60 );
    //************************************
    // 트레이아이콘에 강제종료 등으로 인해 남겨진 아이콘들 정리
    // Method:    sweepNotificationIcon
    // FullName:  sweepNotificationIcon
    // Access:    public 
    // @return:   void
    // Qualifier:
    //************************************
    void sweepNotificationIcon();
    /*!
       해당 PID 의 프로세스가 64비트 프로세스인지 반환, 32비트 윈도라면 무조건 false
       Method:    is64BitProcess
       FullName:  is64BitProcess
       Access:    public 
       @param: DWORD dwProcessID
       @return:   bool
       Qualifier:
    */
    bool					is64BitProcess( DWORD dwProcessID );

    // 파일의 버전 정보 및 회사 정보를 구할수 있다.
    enum tagInformationType { COMMENTS, INTERNALNAME, PRODUCTNAME,
        COMPANYNAME,LEGALCOPYRIGHT, PRODUCTVERSION,
        FILEDESCRIPTION, LEGALTRADEMARKS, PRIVATEBUILD,
        FILEVERSION, ORIGINALFILENAME, SPECIALBUILD };

    std::wstring GetFileInfomation( const std::wstring& filePath, const tagInformationType selectInfo = PRODUCTVERSION );
    std::wstring trim_left( const std::wstring& str );
    std::wstring trim_right( const std::wstring& str );

    std::wstring getOSDisplayName();
    std::wstring getOSVer();

    /*!
       인터넷에 연결을 시도하여 실제로 인터넷에 연결된 로컬 컴퓨터의 IP 를 반환, 랜카드가 여러개 있거나 할 때 유용
       Method:    getInternetConnectedAddress
       FullName:  getInternetConnectedAddress
       Access:    public 
       @param: const std::wstring & tryInternetIP
       @return:   std::wstring
       Qualifier:
    */
    std::wstring			getInternetConnectedAddress( const std::wstring& tryInternetIP = L"www.msn.co.kr" );

    typedef struct tag_LANCARD_ITEM
    {
        std::wstring	strIP;
        std::wstring	strGateWayIP;
        std::wstring	strMAC;
        bool			isEthernet;
        std::wstring	strName;

        tag_LANCARD_ITEM() : isEthernet(false) {}
    } LANCARD_ITEM, *PLANCARD_ITEM;

    typedef std::vector<LANCARD_ITEM>	vecLancard;
    typedef vecLancard::iterator		vecLancardIter;

    DWORD _tinet_addr(const TCHAR *cp);
    /** 라우트 정보를 이용하여서 자신의 인터넷 연결 IP 주소를 찾는다. * */
    bool GetInternetConnectLanAddress( const std::wstring& strConnectIPAddress, std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress = false );
    /** 호스트에서 인터넷에 연결되어 있는 IP 주소를 가져온다. */
    bool GetInternetConnectLanInfo( std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress = false );

    /*!
       넘겨준 IP 에 해당하는 MAC 주소 값을 반환. IPv4, IPv6 모두 대응함
       Method:    getMACFromIP
       FullName:  getMACFromIP
       Access:    public 
       @param: const std::wstring & ipaddress
       @param: ADDRESS_FAMILY family
       @return:   std::wstring
       Qualifier:
    */
    #if _WIN32_WINNT > 0x0500
    std::wstring			getMACFromIP( const std::wstring& ipaddress, ADDRESS_FAMILY family = AF_INET );
    #endif

    std::wstring			getMACFromHDR( const PBYTE pEtherHder );

    /*!
        로컬 컴퓨터의 IP 를 반환
        Method:    getIPAddress
        FullName:  getIPAddress
        Access:    public 
        @return:   std::wstring
        Qualifier:
    */
    std::wstring			getIPAddress();
#ifdef _AFX
    std::wstring            QueryOSStringFromWMI();
    std::wstring            QueryOSStringFromRegistry();
    // 해당 운영체제를 나타내는 문자열 반환
    std::wstring            QueryOSStringFromREG();
    std::wstring            QueryBiosSerialFromWMI();
    std::wstring            QueryComputerUUIDFromWMI();
    std::wstring            QueryOSStringFromWMINonArch();
    std::wstring            QueryBaseBoardSerialFromWMI();
#endif


    std::wstring                    getAllMACAddress();

    bool isTrusZoneVTN();
    bool                            IsProtectedFileByOS( const std::wstring& fileName );
	
	// SE_DEBUG_NAME 권한 필요
	bool GetProcessOwner( HANDLE hProcess_i, TCHAR* csOwner_o );
	BOOL SetRegistyStartProgram(BOOL bAutoExec, LPCWSTR lpValueName, LPCWSTR lpExeFileName);

    #ifdef _AFX
    // 초단위의 숫자를 넘겨서 경과시간을 %H %M %S  를 사용한 문자열로 서식화, 
    CString formatElapsedTime( unsigned int elapsedTimeSec, const CString& fmt = _T("%H 시간 %M 분 %S 초") );
    /*!
        HTTP, HTTPS, FTP 로부터 파일을 내려받습니다. 

        @param strRemoteAddr 원격지의 주소
        @param strRemoteFile 내려받을 원격지의 파일이름
        @param strLocalPath 내려받은 파일을 저장할 로컬 경로 ( 파일이름은 변경없이 그대로 저장됨 )

        포트가 일반적인 포트가 아닐 경우 해당 서버의 프로토콜 형식을 직접 지정할 수 있음
        downloadType = 0( AUTO ), 1 = FTP, 2 = HTTPS, 3 = HTTP, 

        @return bool 파일 내려받기 성공여부 반환

        http://www.msn.co.kr/index.html 을 내려받는다면
        strRemoteAddr = http://www.msn.co.kr
        strRemoteFile = /index.html
        strLocalPath = C:/ 
    */
    bool DownloadFile( CString strRemoteAddr, CString strRemoteFile, CString strLocalPath, int downloadType = 2 );
    bool InCompressUsing7ZIP(CString strPath, CString strTargetFile, CString strinCompressFileName, CString str7zipPath);
	#endif
    std::wstring            getInternetExplorerVersion();

    bool                    DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strPassword = L"dkdlahsdbwj!@#" );

    DWORD                   getFileSize( const std::wstring& sFilePath );
    std::wstring            getFileExtension( std::wstring sFilePath );

    bool                    createProcessAsAnotherSession( DWORD dwWinlogonPID, DWORD dwSessionID, std::wstring fileName );
    HANDLE					createProcessToAnotherSession( std::wstring fileName, std::wstring sDescProcess = L"", DWORD dwSessionID = -1, bool ignoreSessionID = false );
    DWORD					getProcessPIDFromSession( const std::wstring& processName, DWORD dwSessionID, bool ignoreSessionID = false );
    DWORD                   GetActiveSession();
    std::wstring            GetProcessPathFromPID( unsigned int dwPID, bool isImagePath = false );

    bool					is64BitOS();

    bool                    StartServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs = 5 );
    bool                    StopServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs = 5, DWORD dwCode = 0x00000001 );

    std::vector< HWND >     getWindHandle( DWORD dwProcessID );

    BOOL                    EnablePrivilege(LPCTSTR szPrivilege);
    std::string             GetNewGUID();

    /*! 
        찾아보기 대화상자를 출력하여 특정 폴더를 선택 후 선택된 폴더 경로를 반환한다.

        @return 선택된 폴더 경로
        @return 빈 문자열, 사용자가 대화상자를 취소했을 때
    */
    std::wstring SelectBrowseFolder( const std::wstring& titleName = L"" );

    class CWindowList
    {
    public:
	    CWindowList() {};
	    ~CWindowList() {};

	    std::vector< std::pair< HWND, std::wstring > > GetWindowList();
	    HWND isExistWindow( DWORD processID, std::wstring windowText );

	    // 0 = not found
	    // 1 = exists
	    // 2 = visible
	    // 4 = enabled
	    // 8 = active
	    // 16 = minimized
	    // 32 = maximized
	    int getWindowState( HWND hWnd );

    private:
	    static BOOL CALLBACK enumChildWindowProc( HWND hWnd, LPARAM lParam );

	    std::vector< std::pair< HWND, std::wstring > > _vecWindowList;
    };

/*!
	eventType
	#define EVENTLOG_SUCCESS                0x0000
	#define EVENTLOG_ERROR_TYPE             0x0001
	#define EVENTLOG_WARNING_TYPE           0x0002
	#define EVENTLOG_INFORMATION_TYPE       0x0004
	#define EVENTLOG_AUDIT_SUCCESS          0x0008
	#define EVENTLOG_AUDIT_FAILURE          0x0010
*/
	bool WriteEventLog( const std::wstring& programName, const std::wstring& logMessage, WORD eventType = EVENTLOG_INFORMATION_TYPE, WORD eventCatecory = 0, DWORD eventID = 0 );

	void SetFroegroundWindowForce( HWND hWnd );
	
    class CDownloaderStatusCallback : public IBindStatusCallback
    {
    public:

        virtual HRESULT STDMETHODCALLTYPE OnStartBinding( /* [in] */ DWORD dwReserved, /* [in] */ IBinding *pib ) 
        { return S_OK; }

        virtual HRESULT STDMETHODCALLTYPE GetPriority( /* [out] */ LONG *pnPriority) 
        { return S_OK; }

        virtual HRESULT STDMETHODCALLTYPE OnLowResource( /* [in] */ DWORD reserved) 
        { return S_OK; }

        virtual HRESULT STDMETHODCALLTYPE OnProgress( /* [in] */ ULONG ulProgress, /* [in] */ ULONG ulProgressMax, /* [in] */ ULONG ulStatusCode, /* [in] */ LPCWSTR szStatusText)
        { 
            SetEvent(itsProgressEvent);
            return S_OK; 
        }

        virtual HRESULT STDMETHODCALLTYPE OnStopBinding( /* [in] */ HRESULT hresult, /* [unique][in] */ LPCWSTR szError ) 
        { return S_OK; }

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE GetBindInfo( /* [out] */ DWORD *grfBINDF, /* [unique][out][in] */ BINDINFO *pbindinfo) 
        { return S_OK; }

        virtual /* [local] */ HRESULT STDMETHODCALLTYPE OnDataAvailable( /* [in] */ DWORD grfBSCF, /* [in] */ DWORD dwSize, /* [in] */ FORMATETC *pformatetc, /* [in] */ STGMEDIUM *pstgmed ) 
        { return S_OK; }

        virtual HRESULT STDMETHODCALLTYPE OnObjectAvailable( /* [in] */ REFIID riid, /* [iid_is][in] */ IUnknown *punk) 
        { return S_OK; }

        STDMETHOD_(ULONG, AddRef)() { return 0; }
        STDMETHOD_(ULONG, Release)() { return 0; }

        STDMETHOD(QueryInterface)( /* [in] */ REFIID riid, /* [iid_is][out] */ void __RPC_FAR *__RPC_FAR *ppvObject ) 
        { return E_NOTIMPL; }

    public:
        CDownloaderStatusCallback( HANDLE progressEvent ) { itsProgressEvent = progressEvent; }

    private:
        HANDLE itsProgressEvent;
    };

    class CDownloaderByAPI
    {
    public:
        CDownloaderByAPI();
        ~CDownloaderByAPI();

        HRESULT             DownloadToFile( const std::wstring& remoteURL, const std::wstring& localPath );

    private:
        void                workerThread();

        std::wstring        remoteURL_;
        std::wstring        localPath_;

        HRESULT             hResult_;

        HANDLE              hOnProgressEvent;
        HANDLE              hOnCompleteEvent;
        HANDLE              hDownloadThread;
    };

} // namespace nsCommon

/*!
	코드의 수행시간을 측정하기위한 매크로

	QueryPerformanceCounter 를 이용하여 측정함

	_checkTimeRun 변수에는 MS 단위의 실행시간이 float 형태의 값으로 들어있음, 값을 구할 수 없으면 0.0 이 초기값으로 들어감

	참조 : http://maytrees.tistory.com/81
*/

#define CHECK_TIME_START \
	__int64 _checkTimeStart =0, _checkTimeFreq = 0, _checkTimeEnd = 0; \
	float _checkTimeRun = 0.0f; \
	BOOL _checkTimeCondition = FALSE; \
	if( (_checkTimeCondition = QueryPerformanceFrequency( (_LARGE_INTEGER*)&_checkTimeFreq ) ) ) \
	{		QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeStart );	};

	
#define CHECK_TIME_END \
	if( _checkTimeCondition != FALSE ) \
	{ \
		QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeEnd ); \
		_checkTimeRun = (float)((double)(_checkTimeEnd - _checkTimeStart)/_checkTimeFreq*1000); \
	};

#endif // CMNWINUTILS_H

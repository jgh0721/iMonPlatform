#include "StdAfx.h"

#include <tchar.h>
#include <WinSvc.h>
#include <Winver.h>
#include <WinIoCtl.h>
#include <IPHlpApi.h>
#include <WtsApi32.h>
#include <Tlhelp32.h>
#include <Userenv.h>
#include <Shlobj.h>
#include <Shlwapi.h>
#ifndef PSAPI_VERSION
#define PSAPI_VERSION 1 
#endif
#include <Psapi.h>
#include <Sfc.h>
#include <WinInet.h>

#include "cmnWinUtils.h"
#include "cmnConverter.h"
#include "cmnFormatter.hpp"

#pragma comment( lib, "Version.lib" )
#pragma comment( lib, "Iphlpapi.lib" )
#pragma comment( lib, "Wtsapi32.lib" )
#pragma comment( lib, "Userenv.lib" )
#pragma comment( lib, "shlwapi.lib" )
#pragma comment( lib, "Advapi32.lib" )
#pragma comment( lib, "shell32.lib" )
#pragma comment( lib, "psapi.lib" )
#pragma comment( lib, "sfc" )
#pragma comment( lib, "wininet" )

#ifdef _AFX
#include <afxinet.h>
#include "CWMI.h"
#include <atlsecurity.h>
#endif

#include <Sddl.h>

#pragma warning( disable: 4996 )

using namespace nsCommon::nsCmnConvert;

namespace nsCommon
{
    CScopedCriticalSection::CScopedCriticalSection( CRITICAL_SECTION& cs )
        : m_cs( cs )
    {
        EnterCriticalSection( &m_cs );
    }

    CScopedCriticalSection::~CScopedCriticalSection()
    {
        LeaveCriticalSection( &m_cs );
    }

    //////////////////////////////////////////////////////////////////////////

    CBenchTimer::CBenchTimer()
        : _checkTimeStart(0), _checkTimePrev(0), _checkTimeFreq(0), _checkTimeCurrent(0), _checkTimeRun(0.0)
    {
        ::QueryPerformanceFrequency( (_LARGE_INTEGER*)&_checkTimeFreq );
        if( _checkTimeFreq > 0 )
            ::QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeStart );
    }

    CBenchTimer::~CBenchTimer()
    {

    }

    void CBenchTimer::step()
    {
        if( _checkTimeCurrent > 0 )
            _checkTimePrev = _checkTimeCurrent;
        ::QueryPerformanceCounter( (_LARGE_INTEGER*)&_checkTimeCurrent );
    }

    float CBenchTimer::getElapsedTimeFromPrev()
    {
        if( _checkTimePrev == 0 )
            return getElapsedTimeFromStart();

        return _checkTimeRun = (float)( (double)( _checkTimeCurrent - _checkTimePrev ) / _checkTimeFreq * 1000 );
    }

    float CBenchTimer::getElapsedTimeFromStart()
    {
        return _checkTimeRun = (float)( (double)( _checkTimeCurrent - _checkTimeStart ) / _checkTimeFreq * 1000 );
    }

#ifdef _MSC_VER
    void SetThreadName( DWORD dwThreadID, LPCSTR pszThreadName )
    {
        const DWORD MS_VC_EXCEPTION=0x406D1388;

        THREADNAME_INFO info;
        info.dwType = 0x1000;
        info.szName = pszThreadName;
        info.dwThreadID = dwThreadID;
        info.dwFlags = 0;

        __try
        {
            RaiseException( MS_VC_EXCEPTION, 0, sizeof(info)/sizeof(DWORD), (const ULONG_PTR*)&info );
        }
        __except(EXCEPTION_CONTINUE_EXECUTION)
        {
        }

    }
#endif

    void SetServiceToAutoRestart( const std::wstring& serviceName, int restartDelayMs /*= 1000 * 60*/, int resetPeriodSec /*= 60 */ )
    {
        SERVICE_FAILURE_ACTIONS sfa;
        SC_HANDLE hSCM = NULL;
        SC_HANDLE hService = NULL;

        do 
        {
            hSCM = OpenSCManager( NULL, NULL, NULL );
            if( hSCM == NULL )
                break;

            hService = OpenService( hSCM, serviceName.c_str(), SC_MANAGER_ALL_ACCESS  );
            if( hService == NULL )
                break;

            SC_ACTION sca;
            sca.Type = SC_ACTION_RESTART;
            sca.Delay = restartDelayMs;

            sfa.dwResetPeriod = resetPeriodSec;
            sfa.lpRebootMsg = NULL;
            sfa.lpCommand = NULL;
            sfa.lpsaActions = &sca;
            sfa.cActions = 1;

            ChangeServiceConfig2( hService, SERVICE_CONFIG_FAILURE_ACTIONS, &sfa );

        } while (false);

        if( hService != NULL )
            CloseServiceHandle( hService );
        hService = NULL;

        if( hSCM != NULL )
            CloseServiceHandle( hSCM );
        hSCM = NULL;

    }

    void sweepNotificationIcon()
    {
        HWND h = FindWindow( L"Shell_TrayWnd", NULL);

        if ( h )
        {
            if ( (h = FindWindowEx(h, NULL, L"TrayNotifyWnd", NULL)) != NULL )
            {
                if( FindWindowEx(h, NULL, L"ToolbarWindow32", NULL) != NULL )
                {
                    RECT r;
                    long x;

                    GetWindowRect(h, &r);

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for(int i = 0; i < loop; i++)
                    {
                        for(x = r.left; x < r.right; x+=16)
                        {
                            SendMessage(h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left);
                        }
                    }

                }

                if ( FindWindowEx(h, NULL, L"SysPager", NULL) != NULL )
                {
                    h = FindWindowEx(h, NULL, L"SysPager", NULL);
                    h = FindWindowEx(h, NULL, L"ToolbarWindow32", NULL);

                    RECT r;
                    long x;

                    GetWindowRect(h, &r);

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for(int i = 0; i < loop; i++)
                    {
                        for(x = r.left; x < r.right; x+=16)
                        {
                            SendMessage(h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left);
                        }
                    }
                }
                else
                {
                    h = FindWindowEx(h, NULL, L"ToolbarWindow32", NULL);

                    RECT r;
                    long x;

                    GetWindowRect(h, &r);

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for(int i = 0; i < loop; i++)
                    {
                        for(x = r.left; x < r.right; x+=16)
                        {
                            SendMessage(h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left);
                        }
                    }

                }
            }

            //숨겨진 아이콘 표시 잔상 제거
            h = FindWindow(L"NotifyIconOverflowWindow", NULL);

            if(h)
            {
                if( (h = FindWindowEx(h, NULL, L"ToolbarWindow32", NULL)) != NULL )
                {
                    RECT r;
                    long x;

                    GetWindowRect(h, &r);

                    int height = 16;
                    int loop = (r.bottom - r.top) / height;

                    for(int i = 0; i < loop; i++)
                    {
                        for(x = r.left; x < r.right; x+=16)
                        {
                            SendMessage(h, WM_MOUSEMOVE, 0, (height * i) * 0x10000 + x - r.left);
                        }
                    }

                }

            }
        }
    }

    bool is64BitProcess( DWORD dwProcessID )
    {
	    /*!
		    해당 프로세스ID 가 64비트인지 32비트인지 판별
		    #. 실행중인 윈도가 32비트 윈도인지 확인 
		    #. 실행중인 윈도가 32비트 윈도라면 무조건 false
		    #. 실행중인 윈도가 64비트 윈도인지 확인
		    #. 실행중인 PID 를 isWoW64process 함수를 통해 확인 

	    */
	    SYSTEM_INFO si;
	    ZeroMemory( &si, sizeof(SYSTEM_INFO) );

	    typedef void (WINAPI *LPFN_GETNATIVESYSTEMINFO)( LPSYSTEM_INFO );
	    LPFN_GETNATIVESYSTEMINFO fnGetNativeSystemInfo = NULL;
	    fnGetNativeSystemInfo = (LPFN_GETNATIVESYSTEMINFO)GetProcAddress( GetModuleHandle( TEXT("kernel32") ), "GetNativeSystemInfo" );
	    if( fnGetNativeSystemInfo == NULL )
		    return false;
	
	    fnGetNativeSystemInfo( &si );

	    if( si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL )
		    return false;

	    typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

	    LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;
	
	    fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( GetModuleHandle( TEXT("kernel32") ), "IsWow64Process" );
	    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION, FALSE, dwProcessID );
	    if( hProcess == NULL )
		    return false;
	
	    BOOL isWow64Bit = TRUE;

	    // WOW64 는 64비트 윈도에서의 32비트 환경이므로 결과값이 true 이면 32비트 프로세스임
	    fnIsWow64Process( hProcess, &isWow64Bit );

	    return isWow64Bit != FALSE ? false : true;
    }

    std::wstring GetFileInfomation( const std::wstring& filePath, const tagInformationType selectInfo )
    {
        if( filePath.empty() == true )
            return L"";

        std::wstring fileInfo;

        struct LANGCODEPAGE
        {
            WORD wLanguage;
            WORD wCodePage;
        };
        LANGCODEPAGE* lpLang = NULL;

        UINT uiTranslate = 0;
        UINT dwResultLenth = 0;  

        TCHAR* pszResult = NULL;
        TCHAR* pszVer  = NULL;
        TCHAR  szSub[MAX_PATH];

        INT iVerSize = ::GetFileVersionInfoSize( filePath.c_str(), NULL );

        if( iVerSize == 0 )
            return fileInfo;

        pszVer = (TCHAR*)malloc( sizeof(TCHAR) * iVerSize );
        memset( pszVer, 0, sizeof( TCHAR ) * iVerSize );

        try
        {
            if( ::GetFileVersionInfo( filePath.c_str(), 0, iVerSize, OUT pszVer ) == FALSE )
            {
                free( pszVer );
                return fileInfo;
            }

            if( ::VerQueryValue( pszVer, L"\\VarFileInfo\\Translation", OUT (LPVOID *)&lpLang, OUT &uiTranslate )  == FALSE )
            {
                free( pszVer );
                return fileInfo;
            }

            switch ( selectInfo )
            {
            case COMMENTS : 
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\Comments", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  INTERNALNAME :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\InternalName", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  PRODUCTNAME :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductName", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  COMPANYNAME :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\CompanyName", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  LEGALCOPYRIGHT :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\LegalCopyright", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  PRODUCTVERSION :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductVersion", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  FILEDESCRIPTION :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileDescription", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  LEGALTRADEMARKS :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\LegalTrademarks", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  PRIVATEBUILD :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\PrivateBuild", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  FILEVERSION :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\FileVersion", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  ORIGINALFILENAME :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\OriginalFilename", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            case  SPECIALBUILD :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\SpecialBuild", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            default :
                wsprintf( szSub, L"\\StringFileInfo\\%04x%04x\\ProductName", lpLang[0].wLanguage, lpLang[0].wCodePage );
                break;
            }

            if( ::VerQueryValue( pszVer, szSub, OUT (LPVOID*)&pszResult, OUT &dwResultLenth ) == FALSE )
            {
                free( pszVer );
                return fileInfo;
            }

            if (pszResult != NULL)
                fileInfo = pszResult;

            free( pszVer );
        }
        catch ( ... )
        {
            free( pszVer );
            return fileInfo;
        }

        return trim_right( fileInfo );
    }

    std::wstring trim_left( const std::wstring& str )
    {
        size_t n = str.find_first_not_of( L" \t\v\n" );
        return n == std::wstring::npos ? str : str.substr( n, str.length() );
    }

    std::wstring trim_right( const std::wstring& str )
    {
        size_t n = str.find_last_not_of( L" \t\v\n" );
        return n == std::wstring::npos ? str : str.substr( 0, n + 1 );
    }


    std::wstring getOSDisplayName()
    {
        std::wstring osDPName;

        const int BUFSIZE = 256;

        typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
        typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

        OSVERSIONINFOEX osvi;
        SYSTEM_INFO si;
        PGNSI pGNSI;
        PGPI pGPI;
        DWORD dwType = 0;

        ZeroMemory(&si, sizeof(SYSTEM_INFO));
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if( GetVersionEx((OSVERSIONINFO*) &osvi) == FALSE )
            return L"Unknown OS";

        // Call GetNativeSystemInfo if supported or GetSystemInfo otherwise.

        pGNSI = (PGNSI) GetProcAddress( GetModuleHandle(TEXT("kernel32.dll")), "GetNativeSystemInfo");
        if( NULL != pGNSI )
            pGNSI(&si);
        else 
            GetSystemInfo(&si);

        if( ( osvi.dwPlatformId != VER_PLATFORM_WIN32_NT ) || 
            ( osvi.dwMajorVersion <= 4 ) )
            return L"Windows 9x Family";

        osDPName += L"Microsoft ";

        if( osvi.dwMajorVersion == 6 )
        {
            switch( osvi.dwMinorVersion )
            {
            case 0:
                if( osvi.wProductType == VER_NT_WORKSTATION )
                    osDPName += L"Windows Vista ";
                else
                    osDPName += L"Windows Server 2008 ";
                break;
            case 1:
                if( osvi.wProductType == VER_NT_WORKSTATION )
                    osDPName += L"Windows 7 ";
                else
                    osDPName += L"Windows Server 2008 R2 ";
                break;
            case 2:
                if( osvi.wProductType == VER_NT_WORKSTATION )
                    osDPName += L"Windows 8 ";
                else
                    osDPName += L"Windows Server 2012";
                break;
            }

            pGPI = (PGPI) GetProcAddress( GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo" );

            pGPI( osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType );

            switch( dwType )
            {
            case PRODUCT_ULTIMATE:
                osDPName += L"Ultimate Edition";
                break;
            case 0x00000030: // PRODUCT_PROFESSIONAL
                osDPName += L"Professional";
                break;
            case PRODUCT_HOME_PREMIUM:
                osDPName += L"Home Premium Edition";
                break;
            case PRODUCT_HOME_BASIC:
                osDPName += L"Home Basic Edition";
                break;
            case PRODUCT_ENTERPRISE:
                osDPName += L"Enterprise Edition";
                break;
            case PRODUCT_BUSINESS:
                osDPName += L"Business Edition";
                break;
            case PRODUCT_STARTER:
                osDPName += L"Starter Edition";
                break;
            case PRODUCT_CLUSTER_SERVER:
                osDPName += L"Cluster Server Edition";
                break;
            case PRODUCT_DATACENTER_SERVER:
                osDPName += L"Datacenter Edition";
                break;
            case PRODUCT_DATACENTER_SERVER_CORE:
                osDPName += L"Datacenter Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER:
                osDPName += L"Enterprise Edition";
                break;
            case PRODUCT_ENTERPRISE_SERVER_CORE:
                osDPName += L"Enterprise Edition (core installation)";
                break;
            case PRODUCT_ENTERPRISE_SERVER_IA64:
                osDPName += L"Enterprise Edition for Itanium-based Systems";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER:
                osDPName += L"Small Business Server";
                break;
            case PRODUCT_SMALLBUSINESS_SERVER_PREMIUM:
                osDPName += L"Small Business Server Premium Edition";
                break;
            case PRODUCT_STANDARD_SERVER:
                osDPName += L"Standard Edition";
                break;
            case PRODUCT_STANDARD_SERVER_CORE:
                osDPName += L"Standard Edition (core installation)";
                break;
            case PRODUCT_WEB_SERVER:
                osDPName += L"Web Server Edition";
                break;
            }
        }
        else if( ( osvi.dwMajorVersion == 5 ) && 
            ( osvi.dwMinorVersion == 2 ) )
        {
#if _WIN32_WINNT == 0x0500
#define SM_SERVERR2 89
#endif
            if( GetSystemMetrics(SM_SERVERR2) )
                osDPName += L"Windows Server 2003 R2, ";
            else if ( osvi.wSuiteMask & VER_SUITE_STORAGE_SERVER )
                osDPName += L"Windows Storage Server 2003";
            else if ( osvi.wSuiteMask & 0x00008000 /* VER_SUITE_WH_SERVER */ )
                osDPName += L"Windows Home Server";
            else if( osvi.wProductType == VER_NT_WORKSTATION &&
                si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64)
                osDPName += L"Windows XP Professional x64 Edition";
            else 
                osDPName += L"Windows Server 2003, ";

            // Test for the server type.
            if ( osvi.wProductType != VER_NT_WORKSTATION )
            {
                if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter Edition for Itanium-based Systems";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise Edition for Itanium-based Systems";
                }
                else if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
                {
                    if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter x64 Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise x64 Edition";
                    else
                        osDPName += L"Standard x64 Edition";
                }
                else
                {
                    if ( osvi.wSuiteMask & VER_SUITE_COMPUTE_SERVER )
                        osDPName += L"Compute Cluster Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                        osDPName += L"Datacenter Edition";
                    else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                        osDPName += L"Enterprise Edition";
                    else if ( osvi.wSuiteMask & VER_SUITE_BLADE )
                        osDPName += L"Web Edition";
                    else 
                        osDPName += L"Standard Edition";
                }
            }
        }
        else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 1 )
        {
            osDPName += L"Windows XP ";
            if( osvi.wSuiteMask & VER_SUITE_PERSONAL )
                osDPName += L"Home Edition";
            else 
                osDPName += L"Professional";
        }
        else if ( osvi.dwMajorVersion == 5 && osvi.dwMinorVersion == 0 )
        {
            osDPName += L"Windows 2000 ";

            if ( osvi.wProductType == VER_NT_WORKSTATION )
            {
                osDPName += L"Professional";
            }
            else 
            {
                if( osvi.wSuiteMask & VER_SUITE_DATACENTER )
                    osDPName += L"Datacenter Server";
                else if( osvi.wSuiteMask & VER_SUITE_ENTERPRISE )
                    osDPName += L"Advanced Server";
                else 
                    osDPName += L"Server";
            }
        }

        if( wcslen(osvi.szCSDVersion) > 0 )
        {
            osDPName += L" ";
            osDPName += osvi.szCSDVersion;
        }

        osDPName += format( L" (build %d", osvi.dwBuildNumber );

        if ( osvi.dwMajorVersion >= 6 )
        {
            if ( si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64 )
                osDPName += L", 64-bit";
            else if (si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_INTEL )
                osDPName += L", 32-bit";
        }

        return osDPName;
    }

    std::wstring getOSVer()
    {
        OSVERSIONINFOEX osvi;
        ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));

        osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
        if( GetVersionEx((OSVERSIONINFO*) &osvi) == FALSE )
            return L"0.0";

        return format( L"%d.%d", osvi.dwMajorVersion, osvi.dwMinorVersion );
    }


    std::wstring getInternetConnectedAddress( const std::wstring& tryInternetIP /*= L"www.msn.com" */ )
    {
        std::wstring ipaddress;

        unsigned long ulTryIP = inet_addr( CU2A( tryInternetIP ).c_str() );
        DWORD iIndex = 0;
        std::vector< std::pair< std::wstring, std::wstring > > vecIPAddress;

        DWORD dwErr = 0, dwAdapterInfoSize = 0;
        PIP_ADAPTER_INFO	pAdapterInfo = NULL, pAdapt = NULL;
        PIP_ADDR_STRING		pAddrStr = NULL;

        do 
        {
            if( GetBestInterface( ulTryIP, &iIndex ) != NO_ERROR )
                break;

            dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize );
            if( dwErr != ERROR_BUFFER_OVERFLOW )
                break;

            if( ( pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
                break;

            if( ( dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize ) ) != 0 )
                break;

            for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
            {
                if( pAdapt->Index != iIndex )
                    continue;

                switch ( pAdapt->Type )
                {
                case MIB_IF_TYPE_ETHERNET:
                case MIB_IF_TYPE_PPP:
                case IF_TYPE_IEEE80211:
                    if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                    {
                        DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                        dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                        for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                        {
                            if( strlen( pAddrStr->IpAddress.String ) <= 0 )
                                continue;

                            dwIp = inet_addr( pAddrStr->IpAddress.String );
                            dwMask = inet_addr( pAddrStr->IpMask.String );
                            dwNetwork = dwIp & dwMask;
                            dwGwNetwork = dwGwIp & dwMask;

                            if( dwGwNetwork != dwNetwork )
                                continue;

                            vecIPAddress.push_back( 
                                std::make_pair( 
                                format( L"%s", CA2U( pAddrStr->IpAddress.String ).c_str() ),
                                CA2U( pAdapt->AdapterName ).c_str() )
                                );
                        }
                    }
                    break;
                }
            }

            for( std::vector< std::pair< std::wstring, std::wstring > >::iterator iter = vecIPAddress.begin(); iter != vecIPAddress.end(); ++iter)
            {
                if( _wcsicmp( iter->first, L"0.0.0.0" ) == 0 )
                    continue;

                if( vecIPAddress.size() > 1 )
                {
                    if( StrStrIW( iter->second.c_str(), L"vmware" ) == NULL )
                        continue;

                    if( StrStrIW( iter->second.c_str(), L"virtual" ) == NULL )
                        continue;

                    ipaddress = iter->first;
                }
                else
                {
                    ipaddress = iter->first;
                }
            }

        } while (false);

        if( pAdapterInfo != NULL )
            ::GlobalFree( pAdapterInfo );

        return ipaddress;
    }

    DWORD _tinet_addr(const TCHAR *cp)
    {
#ifdef UNICODE
        char IP[16];
        int Ret = 0;
        Ret = WideCharToMultiByte(CP_ACP, 0, cp, (int)_tcslen(cp), IP, 15, NULL, NULL);
        IP[Ret] = 0;

        return inet_addr(IP);
#endif

#ifndef UNICODE
        return inet_addr(cp);
#endif
    }

    bool GetInternetConnectLanAddress( const std::wstring& strConnectIPAddress, std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress )
    {
        vecLancard vecLan;
        bool isResult = false;
        DWORD	iAddr = _tinet_addr( strConnectIPAddress.c_str() );
        DWORD	iIndex = 0;

        GetBestInterface( iAddr, &iIndex );

        DWORD dwErr, dwAdapterInfoSize = 0;
        PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
        PIP_ADDR_STRING		pAddrStr;

        if( ( dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize ) ) != 0 )
        {
            if( dwErr != ERROR_BUFFER_OVERFLOW )
            {
                return isResult;
            }
        }

        // Allocate memory from sizing information
        if( ( pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
        {
            return isResult;
        }

        // Get actual adapter information
        if( ( dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize ) ) != 0 )
        {
            GlobalFree( pAdapterInfo );
            return isResult;
        }

        for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
        {
            if( pAdapt->Index != iIndex )
                continue;

            switch ( pAdapt->Type )
            {
            case MIB_IF_TYPE_ETHERNET:
            case MIB_IF_TYPE_PPP:
            case IF_TYPE_IEEE80211:
                if( ignoreGatewayAddress == true )
                {
                    for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                    {
                        LANCARD_ITEM item;
                        item.strName = format( L"%S", pAdapt->Description );

                        if( strlen(pAddrStr->IpAddress.String) > 0 )
                        {
                            std::wstring strCurrentIP, strCurrentMAC;

                            strCurrentIP = format(_T("%S"), pAddrStr->IpAddress.String);
                            strCurrentMAC = format( _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
                                pAdapt->Address[0], 
                                pAdapt->Address[1],
                                pAdapt->Address[2],
                                pAdapt->Address[3],
                                pAdapt->Address[4],
                                pAdapt->Address[5] );

                            isResult = true;

                            item.strIP = strCurrentIP.c_str();
                            item.strMAC = strCurrentMAC.c_str();
                        }

                        vecLan.push_back(item);
                    }
                }
                else
                {
                    if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                    {
                        DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                        dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                        for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                        {
                            LANCARD_ITEM item;
                            item.strName = format( L"%S", pAdapt->Description );

                            if( strlen(pAddrStr->IpAddress.String) > 0 )
                            {
                                dwIp = inet_addr( pAddrStr->IpAddress.String );
                                dwMask = inet_addr( pAddrStr->IpMask.String );
                                dwNetwork = dwIp & dwMask;
                                dwGwNetwork = dwGwIp & dwMask;

                                if( dwGwNetwork == dwNetwork )
                                {
                                    std::wstring strCurrentIP, strCurrentMAC;

                                    strCurrentIP = format(_T("%S"), pAddrStr->IpAddress.String);
                                    strCurrentMAC = format( _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
                                        pAdapt->Address[0], 
                                        pAdapt->Address[1],
                                        pAdapt->Address[2],
                                        pAdapt->Address[3],
                                        pAdapt->Address[4],
                                        pAdapt->Address[5] );

                                    isResult = true;

                                    item.strIP = strCurrentIP.c_str();
                                    item.strMAC = strCurrentMAC.c_str();
                                }
                            }

                            vecLan.push_back(item);

                        }
                    }
                }

                break;
            default:
                break;
            }

            if( isResult == true ) 
                break;
        }

        GlobalFree( pAdapterInfo );

        if (!vecLan.empty())
        {
            std::wstring strCurrentIP, strCurrentMAC;
            for (vecLancardIter iter = vecLan.begin(); iter != vecLan.end(); ++iter)
            {
                if( iter->strIP != _T("0.0.0.0"))
                {
                    if (vecLan.size() > 1)
                    {
                        if( ::StrStrI( iter->strName.c_str(), L"vmware" ) == NULLPTR )
                        {
                            strCurrentIP = iter->strIP;
                            strCurrentMAC = iter->strMAC;
                        }
                    }
                    else
                    {
                        strCurrentIP = iter->strIP;
                        strCurrentMAC = iter->strMAC;
                    }
                }
            }
            strMAC = strCurrentMAC;
            strIP = strCurrentIP;
            isResult = true;
        }

        return isResult;
    }

    bool GetInternetConnectLanInfo( std::wstring& strMAC, std::wstring& strIP, bool ignoreGatewayAddress )
    {
        ULONG nBufferLength = 0;
        PBYTE pBuffer = NULL;

        bool isResult = false;

        if( ERROR_BUFFER_OVERFLOW == ::GetAdaptersInfo( NULL, &nBufferLength ) )
        {
            pBuffer = new BYTE[ nBufferLength ];
            ZeroMemory( pBuffer, nBufferLength );

            PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);

            if( GetAdaptersInfo( pAdapterInfo, &nBufferLength ) == ERROR_SUCCESS )
            {
                vecLancard vecLan;
                vecLan.clear();
                while (pAdapterInfo != NULL)
                {
                    std::wstring strDescription = format( L"%S", pAdapterInfo->Description);
                    DWORD dwGwIp = inet_addr( pAdapterInfo->GatewayList.IpAddress.String );

                    if( ignoreGatewayAddress == true )
                        dwGwIp = 1;

                    if( ::StrStrI( strDescription.c_str(), L"vmware") == NULLPTR && dwGwIp != 0)
                    {
                        LANCARD_ITEM item;
                        item.strGateWayIP = format( L"%S", pAdapterInfo->GatewayList.IpAddress.String);

                        if (pAdapterInfo->Type == MIB_IF_TYPE_ETHERNET)
                        {
                            if( ::StrStrI( strDescription.c_str(), L"wireless") == NULLPTR )
                                item.isEthernet = true;
                            else
                                item.isEthernet = false;
                        }
                        else
                            item.isEthernet = false;

                        item.strIP = format( L"%S", pAdapterInfo->IpAddressList.IpAddress.String );

                        std::wstring strMACAddress = format( _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
                            pAdapterInfo->Address[0], 
                            pAdapterInfo->Address[1],
                            pAdapterInfo->Address[2],
                            pAdapterInfo->Address[3],
                            pAdapterInfo->Address[4],
                            pAdapterInfo->Address[5] );
                        item.strMAC = strMACAddress;

                        vecLan.push_back(item);
                    }

                    pAdapterInfo = pAdapterInfo->Next;
                }

                if (!vecLan.empty())
                {
                    std::wstring strCurrentIP, strCurrentMAC;
                    for( vecLancardIter iter = vecLan.begin(); iter != vecLan.end(); ++iter)
                    {
                        if (iter->strIP != _T("0.0.0.0"))
                        {
                            strCurrentIP = iter->strIP;
                            strCurrentMAC = iter->strMAC;
                            if (iter->isEthernet)
                                break;
                        }
                    }
                    strMAC = strCurrentMAC;
                    strIP = strCurrentIP;
                    isResult = true;
                }
            }

            delete [] pBuffer;
        }

        return isResult;
    }

#if _WIN32_WINNT > 0x500
    std::wstring getMACFromIP( const std::wstring& ipaddress, ADDRESS_FAMILY family /*= AF_INET */ )
    {
        std::wstring macaddress;

        DWORD dwRet;
        PIP_ADAPTER_ADDRESSES pAdapters = NULL;
        PIP_ADAPTER_ADDRESSES pCurrent = NULL;
        PIP_ADAPTER_UNICAST_ADDRESS pCurrentAddress = NULL;

        unsigned long ulBufLen = 0;

        dwRet = GetAdaptersAddresses( family, 0, NULL, pAdapters, &ulBufLen );

        if( dwRet == ERROR_NO_DATA )
            return macaddress;

        if( dwRet == ERROR_BUFFER_OVERFLOW )
        {
            pAdapters = (PIP_ADAPTER_ADDRESSES)malloc( ulBufLen );
            if( pAdapters == NULL )
                return macaddress;
        }

        dwRet = GetAdaptersAddresses( family, 0, NULL, pAdapters, &ulBufLen );
        if( dwRet != ERROR_SUCCESS )
            return macaddress;

        for( pCurrent = pAdapters; pCurrent != NULL; pCurrent = pCurrent->Next )
        {
            for( pCurrentAddress = pCurrent->FirstUnicastAddress; pCurrentAddress != NULL; pCurrentAddress = pCurrentAddress->Next )
            {
                LPSOCKADDR pAddr = (LPSOCKADDR)pCurrentAddress->Address.lpSockaddr;
                if( pAddr == NULL )
                    continue;

                WCHAR wszAddress[ 128 ] = {0,};
                DWORD chAddress = _countof( wszAddress );

                dwRet = WSAAddressToString( 
                    pCurrentAddress->Address.lpSockaddr,
                    pCurrentAddress->Address.iSockaddrLength,
                    NULL,
                    wszAddress,
                    &chAddress );

                if( dwRet != 0 )
                    continue;

                if( _wcsicmp( wszAddress, ipaddress ) == 0 )
                {
                    for( DWORD i = 0; i < pCurrent->PhysicalAddressLength; i++) 
                    {
                        if( i == ( pCurrent->PhysicalAddressLength - 1 ) )
                            macaddress += format( L"%.2X", (int) pCurrent->PhysicalAddress[ i ] );
                        else
                            macaddress += format( L"%.2X-", (int) pCurrent->PhysicalAddress[ i ] );
                    }
                }
            }
        }

        if( pAdapters != NULL )
            free( (void *)pAdapters );

        return macaddress;
    }
#endif

    std::wstring getMACFromHDR( const PBYTE pEtherHder )
    {
        return format( L"%.2X%.2X%.2X%.2X%.2X%.2X", pEtherHder[0], pEtherHder[1], pEtherHder[2], pEtherHder[3], pEtherHder[4], pEtherHder[5] );
    }


    std::wstring getIPAddress()
    {
        ULONG nBufferLength = 0;
        PBYTE pBuffer = NULL;

        std::wstring ipaddress;
        std::vector< std::pair< std::wstring, std::wstring > > vecIPAddress;

        if( ERROR_BUFFER_OVERFLOW == ::GetAdaptersInfo( NULL, &nBufferLength ) )
        {
            pBuffer = new BYTE[ nBufferLength ];
            ZeroMemory( pBuffer, nBufferLength );

            PIP_ADAPTER_INFO pAdapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(pBuffer);

            if( GetAdaptersInfo( pAdapterInfo, &nBufferLength ) == ERROR_SUCCESS )
            {

                while (pAdapterInfo != NULL)
                {
                    DWORD dwGwIp;
                    dwGwIp = inet_addr( pAdapterInfo->GatewayList.IpAddress.String );

                    if (dwGwIp != 0)
                    {
                        std::wstring strGateWayIP = CA2U( pAdapterInfo->GatewayList.IpAddress.String );
                        std::wstring strIP = CA2U( pAdapterInfo->IpAddressList.IpAddress.String );

                        std::wstring strMACAddress = format( _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
                            pAdapterInfo->Address[0], 
                            pAdapterInfo->Address[1],
                            pAdapterInfo->Address[2],
                            pAdapterInfo->Address[3],
                            pAdapterInfo->Address[4],
                            pAdapterInfo->Address[5] );

                        vecIPAddress.push_back( std::make_pair( strIP, strMACAddress ) );
                    }

                    pAdapterInfo = pAdapterInfo->Next;
                }

                for( std::vector< std::pair< std::wstring, std::wstring > >::iterator iter = vecIPAddress.begin(); iter != vecIPAddress.end(); ++iter)
                {
                    if( _wcsicmp( iter->first, L"0.0.0.0" ) == 0 )
                        continue;

                    if( vecIPAddress.size() > 1 )
                    {
                        if( StrStrIW( iter->second.c_str(), L"vmware" ) == NULL )
                            continue;

                        if( StrStrIW( iter->second.c_str(), L"virtual" ) == NULL )
                            continue;

                        ipaddress = iter->first;
                        break;
                    }
                    else
                    {
                        ipaddress = iter->first;
                    }
                }
            }

            delete [] pBuffer;
        }

        return ipaddress;
    }

#ifdef _AFX
    std::wstring QueryOSStringFromWMI()
    {
        //const LPTSTR UNKNOWN_OS = _T("Unknown OS");

        std::wstring wsResult = QueryOSStringFromRegistry();
        std::wstring sOSArchitecture;
        sOSArchitecture.clear();

        CoInitializeEx(0, COINIT_MULTITHREADED);

        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( wbemLocator.ConnectServer( wbemService ) == false )
                return wsResult;

            CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( CComBSTR(L"Win32_OperatingSystem"), wbemClsEnumerator ) ))
                return wsResult;

            wbemClsEnumerator.SetProperty();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                CComVariant vtItem;
                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"Caption", vtItem)) ) 
                {
                    wsResult = ConvertToStringW(vtItem, L",").c_str();
                }

                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"OSArchitecture", vtItem)) ) 
                {
                    sOSArchitecture = ConvertToStringW(vtItem, L",").c_str();
                }
            }

        }

        CoUninitialize();

        if( sOSArchitecture.empty() == false )
            wsResult = wsResult + L" " + sOSArchitecture;

        return wsResult;
    }

    std::wstring QueryOSStringFromRegistry()
    {
        const LPTSTR UNKNOWN_OS = _T("Unknown OS");

        std::wstring wsResult = UNKNOWN_OS;

        CRegKey reg;
        if( reg.Open( HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion"), 
            is64BitOS() == false ? KEY_READ : KEY_READ | KEY_WOW64_64KEY ) != ERROR_SUCCESS )
            return wsResult;

        TCHAR pszValueData[MAX_PATH] = {0,};
        ULONG ulValueData = MAX_PATH;
        if( reg.QueryStringValue(_T("ProductName"), pszValueData, &ulValueData) == ERROR_SUCCESS ) 
            wsResult = pszValueData;

        return wsResult;
    }

    std::wstring QueryOSStringFromREG()
    {
        OSVERSIONINFO osi;
        ZeroMemory(&osi, sizeof(osi));
        osi.dwOSVersionInfoSize = sizeof(osi);

        GetVersionEx(&osi);

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows NT 4.0");

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows 2000");

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 1 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows XP");

        if( osi.dwMajorVersion == 5 && osi.dwMinorVersion == 2 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows 2003");

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows Vista");

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 1 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows 7");

        if( osi.dwMajorVersion == 6 && osi.dwMinorVersion == 2 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT )
            return _T("Windows 8");

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T("Windows 95");

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 10 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T("Windows 98");

        if( osi.dwMajorVersion == 4 && osi.dwMinorVersion == 90 && osi.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS )
            return _T("Windows Me");

        return _T("Unknown OS");
    }

    std::wstring QueryBiosSerialFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx(0, COINIT_MULTITHREADED);

        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( wbemLocator.ConnectServer( wbemService ) == false )
                return wsResult;

            CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( CComBSTR(L"Win32_Bios"), wbemClsEnumerator ) ))
                return wsResult;

            wbemClsEnumerator.SetProperty();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                CComVariant vtItem;
                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"SerialNumber", vtItem)) ) 
                {
                    wsResult = ConvertToStringW(vtItem, L",").c_str();
                    break;
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryComputerUUIDFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx(0, COINIT_MULTITHREADED);

        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( wbemLocator.ConnectServer( wbemService ) == false )
                return wsResult;

            CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( CComBSTR(L"Win32_ComputerSystemProduct"), wbemClsEnumerator ) ))
                return wsResult;

            wbemClsEnumerator.SetProperty();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                CComVariant vtItem;
                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"UUID", vtItem)) ) 
                {
                    wsResult = ConvertToStringW(vtItem, L",").c_str();
                    break;
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryOSStringFromWMINonArch()
    {
        std::wstring wsResult = QueryOSStringFromRegistry();

        CoInitializeEx(0, COINIT_MULTITHREADED);

        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( wbemLocator.ConnectServer( wbemService ) == false )
                return wsResult;

            CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( CComBSTR(L"Win32_OperatingSystem"), wbemClsEnumerator ) ))
                return wsResult;

            wbemClsEnumerator.SetProperty();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                CComVariant vtItem;
                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"Caption", vtItem)) ) 
                {
                    wsResult = ConvertToStringW(vtItem, L",").c_str();
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }

    std::wstring QueryBaseBoardSerialFromWMI()
    {
        std::wstring wsResult = L"";

        CoInitializeEx(0, COINIT_MULTITHREADED);

        {
            CWbemLocator wbemLocator;
            CWbemService wbemService;

            if( wbemLocator.ConnectServer( wbemService ) == false )
                return wsResult;

            CWbemClassEnumerator wbemClsEnumerator;
            if( FAILED( wbemService.GetInstanceEnumerator( CComBSTR(L"Win32_BaseBoard"), wbemClsEnumerator ) ))
                return wsResult;

            wbemClsEnumerator.SetProperty();
            if( FAILED( wbemClsEnumerator.RefreshEnum() ) )
                return wsResult;

            for( DWORD nIDX = 0; nIDX < wbemClsEnumerator.GetCount(); ++nIDX )
            {
                CComVariant vtItem;
                if( SUCCEEDED(wbemClsEnumerator.GetItem(nIDX, L"SerialNumber", vtItem)) ) 
                {
                    wsResult = ConvertToStringW(vtItem, L",").c_str();
                    break;
                }
            }

        }

        CoUninitialize();

        return wsResult;
    }
#endif

    std::wstring getAllMACAddress()
    {
        std::wstring sAllMac;
        sAllMac.clear();

        DWORD dwErr, dwAdapterInfoSize = 0;
        PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;
        PIP_ADDR_STRING		pAddrStr;

        if( ( dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize ) ) != 0 )
        {
            if( dwErr != ERROR_BUFFER_OVERFLOW )
            {
                return sAllMac;
            }
        }

        // Allocate memory from sizing information
        if( ( pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
        {
            return sAllMac;
        }

        // Get actual adapter information
        if( ( dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize ) ) != 0 )
        {
            GlobalFree( pAdapterInfo );
            return sAllMac;
        }

        for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
        {
            switch ( pAdapt->Type )
            {
            case MIB_IF_TYPE_ETHERNET:
            case MIB_IF_TYPE_PPP:
            case IF_TYPE_IEEE80211:
                {
                    std::wstring strDescription = format( L"%S", pAdapt->Description );

                    if( StrStrI( strDescription.c_str(), L"vm" ) != NULLPTR
                        || StrStrI( strDescription.c_str(), L"virtual" ) != NULLPTR )
                        break;

                    if( strlen( pAdapt->GatewayList.IpAddress.String ) > 0 )
                    {
                        DWORD	dwGwIp, dwMask, dwIp, dwGwNetwork, dwNetwork;

                        dwGwIp = inet_addr( pAdapt->GatewayList.IpAddress.String );

                        for( pAddrStr = &(pAdapt->IpAddressList); pAddrStr; pAddrStr = pAddrStr->Next )
                        {
                            if( strlen(pAddrStr->IpAddress.String) > 0 )
                            {
                                dwIp = inet_addr( pAddrStr->IpAddress.String );
                                dwMask = inet_addr( pAddrStr->IpMask.String );
                                dwNetwork = dwIp & dwMask;
                                dwGwNetwork = dwGwIp & dwMask;

                                if( dwGwNetwork == dwNetwork )
                                {
                                    std::wstring strCurrentIP, strCurrentMAC;

                                    strCurrentIP = format(_T("%S"), pAddrStr->IpAddress.String);
                                    strCurrentMAC = format( _T("%02x:%02x:%02x:%02x:%02x:%02x"), 
                                        pAdapt->Address[0], 
                                        pAdapt->Address[1],
                                        pAdapt->Address[2],
                                        pAdapt->Address[3],
                                        pAdapt->Address[4],
                                        pAdapt->Address[5] );

                                    if( sAllMac.empty() == false )
                                        sAllMac += L" ";

                                    sAllMac += strCurrentMAC;
                                }
                            }
                        }
                    }
                }
                break;
            default:
                break;
            }
        }

        if( pAdapterInfo != NULL )
            GlobalFree( pAdapterInfo );

        return sAllMac;
    }

    bool isTrusZoneVTN()
    {
        bool isTrusZone = false;

        DWORD dwErr, dwAdapterInfoSize = 0;
        PIP_ADAPTER_INFO	pAdapterInfo, pAdapt;

        if( ( dwErr = GetAdaptersInfo( NULL, &dwAdapterInfoSize ) ) != 0 )
        {
            if( dwErr != ERROR_BUFFER_OVERFLOW )
            {
                return isTrusZone;
            }
        }

        // Allocate memory from sizing information
        if( ( pAdapterInfo = (PIP_ADAPTER_INFO) GlobalAlloc( GPTR, dwAdapterInfoSize )) == NULL )
            return isTrusZone;

        // Get actual adapter information
        if( ( dwErr = GetAdaptersInfo( pAdapterInfo, &dwAdapterInfoSize ) ) != 0 )
        {
            GlobalFree( pAdapterInfo );
            return isTrusZone;
        }

        for( pAdapt = pAdapterInfo; pAdapt; pAdapt = pAdapt->Next )
        {
            switch ( pAdapt->Type )
            {
            case MIB_IF_TYPE_ETHERNET:
            case MIB_IF_TYPE_PPP:
            case IF_TYPE_IEEE80211:
                {
                    std::wstring strDescription = format( L"%S", pAdapt->Description );

                    if( strDescription.find( L"TrusZone VTN driver" ) != std::wstring::npos )
                    {
                        isTrusZone = true;
                        break;
                    }
                }
                break;
            default:
                break;
            }
        }

        if( pAdapterInfo != NULL )
            GlobalFree( pAdapterInfo );

        return isTrusZone;
    }

#ifdef _AFX
    bool DownloadFile( CString strRemoteAddr, CString strRemoteFile, CString strLocalPath, int downloadType /*= -1 */ )
    {
        bool isSuccess = false;

        CFile fpLocalPath;
        CInternetSession session;
        CFtpConnection* pFtpConn = NULL;
        CHttpConnection* pHttpConn = NULL;
        CHttpFile* pFile = NULL;
        CInternetFile* pFtpFile = NULL;
        CString strDebug;

        do 
        {
            if ( strRemoteAddr.IsEmpty() || strRemoteFile.IsEmpty() || strLocalPath.IsEmpty() )
                break;

            session.SetOption( INTERNET_OPTION_CONNECT_TIMEOUT | INTERNET_OPTION_CONNECTED_STATE, 3000 );
            session.SetOption( INTERNET_OPTION_CONNECT_BACKOFF, 1000 );
            session.SetOption( INTERNET_OPTION_CONNECT_RETRIES, 3 );
            session.SetOption( INTERNET_OPTION_SECURITY_FLAGS, SECURITY_FLAG_IGNORE_CERT_CN_INVALID );
            session.SetOption( INTERNET_OPTION_SECURITY_FLAGS, SECURITY_FLAG_IGNORE_CERT_DATE_INVALID );

            INTERNET_PORT nPort = 80;
            CString strServer, strObject;
            DWORD dwServiceType = AFX_INET_SERVICE_HTTP;
            if( strRemoteFile.Left(1).Compare( L"/" ) != 0 )
                strRemoteFile = L"/" + strRemoteFile;

            // 주소에서 서비스 프로토콜 종류 검출
            if( downloadType == 0 )
            {
                if( strRemoteAddr.Find( L"://" ) == -1 )
                    dwServiceType = AFX_INET_SERVICE_HTTP;
            }
            else if( downloadType <= 3 && downloadType >= 0 )   // downloadType = 1 과 2 는 AFX_INET_SERVICE_FTP, AFX_INET_SERVICE_HTTP 에 대응함
                dwServiceType = downloadType == 2 ? AFX_INET_SERVICE_HTTPS : downloadType;
            else
                break;

            if( AfxParseURL( strRemoteAddr + strRemoteFile, dwServiceType, strServer, strObject, nPort ) == 0 )
                dwServiceType = downloadType == 2 ? AFX_INET_SERVICE_HTTPS : downloadType;

            try
            {
                DWORD dwRet = 0;
                const DWORD BUFFER_SIZE = 8192;
                BYTE byBuffer[ BUFFER_SIZE ] = {0,};
                DWORD dwFlags = INTERNET_FLAG_EXISTING_CONNECT | INTERNET_FLAG_RELOAD | INTERNET_FLAG_DONT_CACHE | INTERNET_FLAG_TRANSFER_BINARY;
                DWORD dwReadAmount = 0;

                switch( dwServiceType )
                {
                case AFX_INET_SERVICE_HTTP:
                case AFX_INET_SERVICE_HTTPS:
                    if( dwServiceType == AFX_INET_SERVICE_HTTP )
                        pHttpConn = session.GetHttpConnection( strServer, nPort );
                    else if( dwServiceType == AFX_INET_SERVICE_HTTPS )
                        pHttpConn = session.GetHttpConnection( strServer, 
                        INTERNET_FLAG_SECURE | 
                        INTERNET_FLAG_IGNORE_CERT_CN_INVALID | 
                        INTERNET_FLAG_IGNORE_CERT_DATE_INVALID, nPort );

                    if( dwServiceType == AFX_INET_SERVICE_HTTPS )
                        dwFlags |= INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID | INTERNET_FLAG_IGNORE_CERT_DATE_INVALID;

                    pFile = pHttpConn->OpenRequest( CHttpConnection::HTTP_VERB_GET, strRemoteFile, NULL, 1, NULL, NULL, dwFlags );
                    if( pFile == NULL )
                        break;

                    pFile->SetReadBufferSize( BUFFER_SIZE );

                    if( pFile->SendRequest() == FALSE )
                        break;

                    if( pFile->QueryInfoStatusCode(dwRet) == FALSE )
                        break;

                    if( dwRet != HTTP_STATUS_OK )
                        break;

                    if( fpLocalPath.Open( strLocalPath, CFile::modeCreate | CFile::modeWrite, NULL ) == FALSE )
                        break;

                    while( ( dwReadAmount = pFile->Read( byBuffer, BUFFER_SIZE ) ) > 0 )
                        fpLocalPath.Write( byBuffer, dwReadAmount );

                    isSuccess = true;
                    break;
                case AFX_INET_SERVICE_FTP:
                    pFtpConn = session.GetFtpConnection(strRemoteAddr);

                    if( fpLocalPath.Open(strLocalPath, CFile::modeWrite | CFile::modeCreate, NULL ) == FALSE )
                        break;

                    pFtpFile = pFtpConn->OpenFile(strRemoteFile);
                    if( pFtpFile == NULL )
                        break;

                    while( ( dwReadAmount = pFtpFile->Read( byBuffer, BUFFER_SIZE ) ) > 0 )
                        fpLocalPath.Write( byBuffer, dwReadAmount );

                    isSuccess = true;
                    break;
                };
            }
            catch (CException* e)
            {
                UNREFERENCED_PARAMETER(e);
                fpLocalPath.Close();
                DeleteFile( strLocalPath );
            }

        } while (false);

        if( pFile != NULL )
        {    pFile->Close(); delete pFile; pFile = NULL; }

        if( pFtpFile != NULL )
        {    pFtpFile->Close(); delete pFtpFile; pFtpFile = NULL; }

        if( pHttpConn != NULL )
        {    pHttpConn->Close(); delete pHttpConn; pHttpConn = NULL; }

        if( pFtpConn != NULL )
        {    pFtpConn->Close(); delete pFtpConn; pFtpConn = NULL; }

        return isSuccess;
    }

    bool InCompressUsing7ZIP(CString strPath, CString strTargetFile, CString strinCompressFileName, CString str7zipPath)
    {
	    bool isResult = false;

	    CString str7zFile;
	    str7zFile.Format(_T("\"%s%s\""),str7zipPath,_T("7za.exe"));
	    CString strParam;
	    strParam.Format(_T(" a -y  \"%s\"  \"%s\""),strinCompressFileName,strTargetFile);

	    CString strtmp;
	    strtmp.Format(_T("%s%s"),str7zFile,strParam);

	    TCHAR* wsz7zFile = new TCHAR[ strtmp.GetLength() + 1];
	    wcscpy( wsz7zFile, strtmp.GetString() );

	    TCHAR* wszParam = new TCHAR[ strParam.GetLength() + 1];
	    wcscpy( wszParam, strParam.GetString() );

	    STARTUPINFO si;
	    PROCESS_INFORMATION pi;

	    ZeroMemory( &si, sizeof(si) );
	    si.cb = sizeof(si);
	    ZeroMemory( &pi, sizeof(pi) );
	    si.dwFlags       = STARTF_USESHOWWINDOW; 
	    si.wShowWindow = SW_HIDE;

	    if (CreateProcess( NULL,   // No module name (use command line). 
		    wsz7zFile, // Command line. 
		    NULL,             // Process handle not inheritable. 
		    NULL,             // Thread handle not inheritable. 
		    FALSE,            // Set handle inheritance to FALSE. 
		    0,                // No creation flags. 
		    NULL,             // Use parent's environment block. 
		    NULL,             // Use parent's starting directory. 
		    &si,              // Pointer to STARTUPINFO structure.
		    &pi              // Pointer to PROCESS_INFORMATION structure.
		    ))
	    {
		    isResult = true;

	    }
	    else
	    {
		    delete [] wsz7zFile;
		    delete [] wszParam;
		
		    return isResult;

	    }

	    WaitForSingleObject( pi.hProcess, INFINITE );

	    if( wsz7zFile )
		    delete [] wsz7zFile;
	    if( wszParam )
		    delete [] wszParam;

	    CloseHandle( pi.hProcess );
	    CloseHandle( pi.hThread );

	    /*
	    SHELLEXECUTEINFO shi;
	    ZeroMemory(&shi, sizeof(SHELLEXECUTEINFO));
	    shi.cbSize		= sizeof(SHELLEXECUTEINFO);
	    shi.fMask		= SEE_MASK_FLAG_NO_UI;
	    shi.hwnd		= NULL;
	    shi.lpVerb		= NULL;
	    shi.lpFile		= str7zFile.GetString();
	    shi.lpParameters = strParam.GetString();
	    shi.nShow		= SW_HIDE;
	    shi.lpDirectory	= NULL;

	    bool isSuccess = ShellExecuteEx(&shi) != FALSE ? true : false;

	    */

	    return isResult;

    }
#endif
    std::wstring	getInternetExplorerVersion()
    {
        WCHAR ieVersion[ 64 ] = {0,};
        DWORD dwVersion = _countof( ieVersion ) * sizeof(WCHAR);
        HKEY hKEY = NULL;

        do 
        {
            LONG lRet = ERROR_SUCCESS;

            lRet = RegOpenKeyEx( HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Internet Explorer", 0, KEY_READ, &hKEY );
            if( lRet != ERROR_SUCCESS )
                break;

            lRet = RegQueryValueEx( hKEY, L"svcVersion", NULL, NULL, (LPBYTE)ieVersion, &dwVersion );
            if( lRet == ERROR_SUCCESS )
                break;

            memset( ieVersion, '\0', sizeof( WCHAR ) * _countof( ieVersion ) );

            lRet = RegQueryValueEx( hKEY, L"Version", NULL, NULL, (LPBYTE)ieVersion, &dwVersion );
            if( lRet == ERROR_SUCCESS )
                break;

        } while (false);

        if( hKEY != NULL )
            RegCloseKey( hKEY );

        return ieVersion;
    }

    bool DeCompressUsing7ZIP( const std::wstring& strPath, const std::wstring& strFileName, const std::wstring& str7zipPath, const std::wstring& strPassword /* = L"dkdlahsdbwj!@#" */ )
    {
	    bool isResult = false;

        std::wstring str7zFile;
        std::wstring strParam;

        if( *str7zipPath.rbegin() == L'\\' || *str7zipPath.rbegin() == L'/' )
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1%2\"", str7zipPath, L"7za.exe" );
        else
            str7zFile = nsCmnFormatter::tsFormat( L"\"%1\\%2\"", str7zipPath, L"7za.exe" );
        
        if( strPassword.empty() == false )
            strParam = nsCmnFormatter::tsFormat( L" x -y -p%1 -o\"%2\" \"%2\\%3", strPassword, strPath, strFileName );
        else
            strParam = nsCmnFormatter::tsFormat( L" x -y -o\"%1\" \"%1\\%2", strPath, strFileName );

        std::wstring strtmp = str7zFile + strParam;

	    WCHAR* wsz7zFile = new WCHAR[ strtmp.size() + 1];
        strtmp = nsCommon::string_replace_all( strtmp, L"/", L"\\" );
	    wcscpy( wsz7zFile, strtmp.c_str() );

	    STARTUPINFO si;
	    PROCESS_INFORMATION pi;

	    ZeroMemory( &si, sizeof(si) );
	    si.cb = sizeof(si);
	    ZeroMemory( &pi, sizeof(pi) );
	    si.dwFlags       = STARTF_USESHOWWINDOW; 
	    si.wShowWindow = SW_HIDE;

        do 
        {
            if( CreateProcess( NULL,   // No module name (use command line). 
                wsz7zFile, // Command line. 
                NULL,             // Process handle not inheritable. 
                NULL,             // Thread handle not inheritable. 
                FALSE,            // Set handle inheritance to FALSE. 
                0,                // No creation flags. 
                NULL,             // Use parent's environment block. 
                NULL,             // Use parent's starting directory. 
                &si,              // Pointer to STARTUPINFO structure.
                &pi              // Pointer to PROCESS_INFORMATION structure.
                ) == FALSE )
            {
                break;
            }

            WaitForSingleObject( pi.hProcess, INFINITE );

            CloseHandle( pi.hProcess );
            CloseHandle( pi.hThread );

            isResult = true;
        } while (false);

        if( wsz7zFile )
            delete [] wsz7zFile;

	    return isResult;
    }

    DWORD getFileSize( const std::wstring& sFilePath )
    {
        WIN32_FIND_DATA W32FindData;
        HANDLE hFile=::FindFirstFile( sFilePath.c_str(), &W32FindData );
        if (hFile == INVALID_HANDLE_VALUE)
            return 0;

        FindClose(hFile);

        return W32FindData.nFileSizeLow;
    }

    std::wstring getFileExtension( std::wstring sFilePath )
    {
        TCHAR szFileExt[MAX_PATH] = {0, };
        _wsplitpath( sFilePath.c_str(), NULL, NULL, NULL, szFileExt );
        return szFileExt;
    }

    bool createProcessAsAnotherSession(DWORD dwWinlogonPID, DWORD dwSessionId, std::wstring strProgram)
    {
        BOOL isSuccess = FALSE;

        HANDLE hProcess = NULL;
        HANDLE hToken = NULL;
        HANDLE hTokenDup = NULL;

        do 
        {
            hProcess = ::OpenProcess( MAXIMUM_ALLOWED, TRUE, dwWinlogonPID );
            if( hProcess == NULL )
                break;

            isSuccess = OpenProcessToken( hProcess, 
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE, &hToken );
            if( isSuccess == FALSE )
                break;

            LUID luid;
            if( !LookupPrivilegeValue(NULL,SE_DEBUG_NAME, &luid) )
                break;

            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if( !DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup) )
                break;

            SetTokenInformation(hTokenDup, TokenSessionId, (LPVOID)&dwSessionId, sizeof(DWORD));
            if( !AdjustTokenPrivileges(hTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL,NULL) )
                break;

            // CreateProcessAsUser 준비
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.lpDesktop = _T("winsta0\\default");

            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
            LPVOID pEnv = NULL;
            if( CreateEnvironmentBlock(&pEnv, hTokenDup, TRUE) )
                dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
            else
                pEnv = NULL;

            LPTSTR lpszPath = _tcsdup(strProgram.c_str());
            if( lpszPath == NULL )
                break;

            wchar_t wszCurrentDirectory[ MAX_PATH ] = {0,};
            wcsncpy( wszCurrentDirectory, strProgram.c_str(), MAX_PATH - 1 );
            PathRemoveFileSpec( wszCurrentDirectory );

            isSuccess = ::CreateProcessAsUser(
                hTokenDup,
                lpszPath,
                NULL,
                NULL,
                NULL,
                FALSE,
                dwCreationFlags,
                pEnv,
                wszCurrentDirectory,
                &si,
                &pi);

            free(lpszPath);

            if( isSuccess != FALSE )
                ::WaitForInputIdle( pi.hProcess, 1000 * 15 );

            if( (isSuccess) && (pi.hProcess != INVALID_HANDLE_VALUE) )
                CloseHandle(pi.hProcess);
            if( (isSuccess) && (pi.hThread != INVALID_HANDLE_VALUE) )
                CloseHandle(pi.hThread);

            isSuccess = true;
        } while (false);

        if( hProcess != NULL )
            CloseHandle(hProcess);
        if( hToken )
            CloseHandle(hToken);
        if( hTokenDup )
            CloseHandle(hTokenDup);

        return isSuccess != FALSE;
    }

#if _WIN32_WINNT > 0x0500
    HANDLE createProcessToAnotherSession( std::wstring fileName, std::wstring sDescProcess /*= L""*/, DWORD dwSessionID /*= -1*/, bool ignoreSessionID /*= false*/ )
    {
        //DWORD dwActiveClientSessionID = WTSGetActiveConsoleSessionId();
		DWORD dwActiveClientSessionID = dwSessionID == -1 ? GetActiveSession() : dwSessionID;
		DWORD explorerPID = -1;

		if( sDescProcess.empty() == false )
			explorerPID = getProcessPIDFromSession( sDescProcess, dwActiveClientSessionID, ignoreSessionID );
		else
	        explorerPID = getProcessPIDFromSession( L"explorer.exe", dwActiveClientSessionID, ignoreSessionID );

        if( explorerPID <= 0 )
            return NULL;

        BOOL isSuccess = false;

        HANDLE hProcess = NULL;
        HANDLE hToken = NULL;
        HANDLE hTokenDup = NULL;
        HANDLE hCreatedProcess = NULL;

        do 
        {
            hProcess = ::OpenProcess( MAXIMUM_ALLOWED, TRUE, explorerPID );
            if( hProcess == NULL )
                break;

            isSuccess = OpenProcessToken( hProcess, 
                TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE, &hToken );
            if( isSuccess == FALSE )
                break;

            LUID luid;
            if( !LookupPrivilegeValue(NULL,SE_DEBUG_NAME, &luid) )
                break;

            TOKEN_PRIVILEGES tp;
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if( !DuplicateTokenEx(hToken, MAXIMUM_ALLOWED, NULL, SecurityIdentification, TokenPrimary, &hTokenDup) )
                break;

            SetTokenInformation(hTokenDup, TokenSessionId, (LPVOID)&dwActiveClientSessionID, sizeof(DWORD));
            if( !AdjustTokenPrivileges(hTokenDup, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL,NULL) )
                break;

            // CreateProcessAsUser 준비
            STARTUPINFO si;
            ZeroMemory(&si, sizeof(STARTUPINFO));
            si.cb = sizeof(STARTUPINFO);
            si.lpDesktop = _T("winsta0\\default");
            PROCESS_INFORMATION pi;
            ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
            DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS | CREATE_NEW_CONSOLE;
            LPVOID pEnv = NULL;

            if( CreateEnvironmentBlock(&pEnv, hTokenDup, TRUE) )
                dwCreationFlags |= CREATE_UNICODE_ENVIRONMENT;
            else
                pEnv = NULL;

            LPTSTR lpszPath = _tcsdup(fileName.c_str());
            if( lpszPath == NULL )
                break;

            //         LPTSTR lpwszCurrentDirectory = _tcsdup( lpszPath );
            //         PathRemoveFileSpec( lpwszCurrentDirectory );

            isSuccess = ::CreateProcessAsUser(
                hTokenDup,
                NULL,
                lpszPath,
                NULL,
                NULL,
                FALSE,
                dwCreationFlags,
                pEnv,
                NULL,
                &si,
                &pi);

            free( lpszPath );
            //        free( lpwszCurrentDirectory );

            if( (isSuccess) && (pi.hProcess != INVALID_HANDLE_VALUE) )
                hCreatedProcess = pi.hProcess;

            if( (isSuccess) && (pi.hThread != INVALID_HANDLE_VALUE) )
                CloseHandle(pi.hThread);

            isSuccess = true;
        } while (false);

        if( hProcess != NULL )
            CloseHandle(hProcess);
        if( hToken )
            CloseHandle(hToken);
        if( hTokenDup )
            CloseHandle(hTokenDup);

        return hCreatedProcess;
    }
#endif

    DWORD getProcessPIDFromSession( const std::wstring& processName, DWORD dwSessionID, bool ignoreSessionID /*= false*/ )
    {
        HANDLE hSnapShot = CreateToolhelp32Snapshot( TH32CS_SNAPPROCESS, 0 );
        DWORD processID = 0;

        do 
        {
            if( hSnapShot == INVALID_HANDLE_VALUE )
                break;

            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);

            if( Process32First( hSnapShot, &pe32 ) == FALSE )
                break;

            do 
            {
                DWORD processSessionID = 0;

				if( ignoreSessionID == false )
				{
					ProcessIdToSessionId( pe32.th32ProcessID, &processSessionID );
					if( processSessionID != dwSessionID )
						continue;
				}

                if( _wcsicmp( processName, pe32.szExeFile ) != 0 )
                    continue;

                processID = pe32.th32ProcessID;
                break;

            } while ( Process32Next( hSnapShot, &pe32 ) != FALSE );

        } while (false);

        if( hSnapShot != INVALID_HANDLE_VALUE )
            CloseHandle( hSnapShot );

        return processID;
    }

    DWORD GetActiveSession()
    {
        DWORD dwSessionId = 0;

        PWTS_SESSION_INFO pSessionInfo = 0;
        DWORD dwCount = 0;

        WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount);

        int dataSize = sizeof(WTS_SESSION_INFO);

        for (DWORD i = 0; i < dwCount; ++i)
        {
            WTS_SESSION_INFO si = pSessionInfo[i];
            if( WTSActive == si.State )
            { 
                dwSessionId = si.SessionId;
                break;
            }
        }

        WTSFreeMemory(pSessionInfo);

        return dwSessionId;
    }

    std::wstring GetProcessPathFromPID( unsigned int dwPID, bool isImagePath )
    {
        typedef DWORD (WINAPI *fnGetProcessImageFileNameW)( HANDLE, LPTSTR, DWORD );

        HANDLE hProcess = ::OpenProcess( PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, dwPID );
        if( hProcess != NULL )
        {
            TCHAR szPath[ 10240 ] = {0,};
            DWORD dwGetModuleFileResult = 0;
            if( ( dwGetModuleFileResult = GetModuleFileNameEx( hProcess, NULL, szPath, 10240 ) ) != 0 && isImagePath == false )
            {
                CloseHandle( hProcess );
                return szPath;
            }

            if( isImagePath == true || std::wstring( szPath ).empty() == true || dwGetModuleFileResult == 0 )
            {
                std::tr1::unordered_map< std::wstring, std::wstring > _mapVolumePathToDosPath;

                WCHAR chDrive[3] = L"A:";
                WCHAR wszBuffer[ MAX_PATH ] = {0,};

                for( int driveLetter = 0; driveLetter < 26; ++driveLetter )
                {
                    chDrive[0] = L'A' + driveLetter;
                    int errorCode = QueryDosDevice( chDrive, wszBuffer, MAX_PATH );
                    if( errorCode == 0 )
                        continue;

                    _mapVolumePathToDosPath.insert( std::make_pair( wszBuffer, chDrive ) );
                }

                HMODULE hModule = NULL;

                hModule = ::LoadLibrary( L"Psapi.dll" );

                if( hModule == NULL )
                    return L"";

                fnGetProcessImageFileNameW pfnGetProcessImageFileName;
                pfnGetProcessImageFileName = (fnGetProcessImageFileNameW)GetProcAddress( hModule, "GetProcessImageFileNameW" );

                if( pfnGetProcessImageFileName == NULL )
                {
                    CloseHandle( hProcess );
                    ::FreeLibrary( hModule );
                    return L"";
                }

                if( pfnGetProcessImageFileName( hProcess, szPath, 10240 ) == NULL )
                {
                    CloseHandle( hProcess );
                    ::FreeLibrary( hModule );
                    return L"";
                }

                std::wstring sPath = szPath;

                for( std::tr1::unordered_map< std::wstring, std::wstring >::iterator it = _mapVolumePathToDosPath.begin(); it != _mapVolumePathToDosPath.end(); ++it )
                    sPath = string_replace_all( sPath, it->first, it->second );

                CloseHandle( hProcess );
                ::FreeLibrary( hModule );

                return sPath;

            }

            CloseHandle( hProcess );
        }

        return L"";
    }

    bool is64BitOS()
    {
        //Windows 2000 이하의 제품군은 64bit 가 존재 하지 않으므로 무조건 32bit 동작
        OSVERSIONINFO osi;
        ZeroMemory(&osi, sizeof(osi));
        osi.dwOSVersionInfoSize = sizeof(osi);

        GetVersionEx(&osi);

        if(osi.dwMajorVersion == 4)
            return false;

        if(osi.dwMajorVersion == 5 && osi.dwMinorVersion == 0 && osi.dwPlatformId == VER_PLATFORM_WIN32_NT)
            return false;

        BOOL bIsWoW64 = FALSE;

        typedef BOOL (WINAPI *LPFN_ISWOW64PROCESS) (HANDLE, PBOOL);

        LPFN_ISWOW64PROCESS fnIsWow64Process = NULL;

        HMODULE hModule = GetModuleHandle(TEXT("kernel32"));

        fnIsWow64Process = (LPFN_ISWOW64PROCESS)GetProcAddress( hModule, "IsWow64Process") ;

        if (NULL != fnIsWow64Process)
        {
            if (!fnIsWow64Process(GetCurrentProcess(),&bIsWoW64))
            {
                if( hModule )
                    FreeLibrary( hModule );
                return false;
            }
        }

        if( hModule )
            FreeLibrary( hModule );

        return bIsWoW64 != FALSE ? true : false;
    }


    bool StartServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs /* = 5 */ )
    {
        bool			IsResult = false;
        SC_HANDLE		hManager = NULL;
        SC_HANDLE		hService = NULL;

        hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( hManager == NULL )
            return IsResult;

        hService = ::OpenService( hManager, pszServiceName, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP );
        if( hService == NULL )
            goto CLEANUP;

        if( ::StartService( hService, 0, NULL ) != FALSE )
        {
            SERVICE_STATUS ss;
            for( UINT nSec = 0; nSec < nWaitSecs; ++nSec )
            {
                ::QueryServiceStatus( hService, &ss );
                if( ss.dwCurrentState == SERVICE_RUNNING )
                {
                    IsResult = true;
                    break;
                }
                Sleep( 1000 );
            }
        }

CLEANUP:
        if( hManager )
            CloseServiceHandle( hManager );
        if( hService )
            CloseServiceHandle( hService );

        return IsResult;
    }

    bool StopServiceFromName( LPCTSTR pszServiceName, UINT nWaitSecs /* = 5 */, DWORD dwCode /* = SERVICE_CONTROL_STOP*/ )
    {
        bool			IsResult = false;
        SC_HANDLE		hManager = NULL;
        SC_HANDLE		hService = NULL;

        hManager = ::OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
        if( hManager == NULL )
            return IsResult;

        hService = ::OpenService( hManager, pszServiceName, SERVICE_QUERY_STATUS | SERVICE_START | SERVICE_STOP );
        if( hService == NULL )
            goto CLEANUP;

        SERVICE_STATUS ss;
        ::QueryServiceStatus( hService, &ss );
        if( ss.dwCurrentState != SERVICE_STOPPED )
        {
            if( ::ControlService( hService, dwCode, &ss ) != FALSE )
            {
                for( UINT nSec = 0; nSec < nWaitSecs; ++nSec )
                {
                    ::QueryServiceStatus( hService, &ss );
                    if( ss.dwCurrentState == SERVICE_STOPPED )
                    {
                        IsResult = true;
                        break;
                    }
                    Sleep( 1000 );
                }
            }
            else
            {
                IsResult = ::GetLastError() == ERROR_SERVICE_NOT_ACTIVE ? true : false;
            }
        }
        else
        {
            IsResult = true;
        }

CLEANUP:
        if( hManager )
            CloseServiceHandle( hManager );
        if( hService )
            CloseServiceHandle( hService );

        return IsResult;
    }

    std::vector< HWND > getWindHandle( DWORD dwProcessID )
    {
        std::vector<HWND> vecHwnd;
        vecHwnd.clear();

        HWND tempHwnd = FindWindow(NULL,NULL);

        while( tempHwnd != NULL )
        {
            ULONG idProc;
            GetWindowThreadProcessId( tempHwnd, &idProc );

            if( dwProcessID == idProc )
            {
                vecHwnd.push_back(tempHwnd);
            }

            tempHwnd = GetWindow(tempHwnd, GW_HWNDNEXT);
        }
        return vecHwnd;
    }

    BOOL EnablePrivilege(LPCTSTR szPrivilege)
    {
        BOOL bResult = FALSE;
        HANDLE hToken = NULL;
        TOKEN_PRIVILEGES tpOld, tpCurrent;

        if( !OpenProcessToken(::GetCurrentProcess(), TOKEN_QUERY | TOKEN_ADJUST_PRIVILEGES, &hToken) )
            return bResult;

        tpCurrent.PrivilegeCount = 1;
        tpCurrent.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

        if( ::LookupPrivilegeValue(NULL, szPrivilege, &tpCurrent.Privileges[0].Luid) )
        {
            DWORD dwOld = sizeof(TOKEN_PRIVILEGES);
            if( ::AdjustTokenPrivileges(hToken, FALSE, &tpCurrent, dwOld, &tpOld, &dwOld) )
                bResult = TRUE;
            else
                bResult = FALSE;
        }
        else
            bResult = FALSE;

        CloseHandle(hToken);
        return bResult;
    }

    std::string GetNewGUID()
    {
        GUID guid;
        std::string formattedGUID;
        if( SUCCEEDED( CoCreateGuid( &guid ) ) )
        {
            formattedGUID = 
                format( "%.8X-%.4X-%.4X-%.2X%.2X-%.2X%.2X%.2X%.2X%.2X%.2X", 
                guid.Data1,    guid.Data2,    guid.Data3,  guid.Data4[0],
                guid.Data4[1], guid.Data4[2], guid.Data4[3], guid.Data4[4],
                guid.Data4[5], guid.Data4[6], guid.Data4[7]
            );
        }

        return formattedGUID;
    }


    std::wstring SelectBrowseFolder( const std::wstring& titleName /* = L"" */ )
    {
        std::wstring retVal;
        TCHAR pszFolderName[ MAX_PATH ] = {0,};

        BROWSEINFO bi;
        ZeroMemory( &bi, sizeof( BROWSEINFO ) );

        bi.hwndOwner = NULL;
        bi.pidlRoot = NULL;
        bi.pszDisplayName = pszFolderName;
        bi.lpszTitle = titleName.c_str();
        bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_DONTGOBELOWDOMAIN | BIF_EDITBOX ;

        PIDLIST_ABSOLUTE pidl = SHBrowseForFolder( &bi );

        ZeroMemory( pszFolderName, sizeof( TCHAR ) * MAX_PATH );

        if( SHGetPathFromIDList( pidl, pszFolderName ) != FALSE )
            retVal = pszFolderName;

        return retVal;
    }





    std::vector< std::pair< HWND, std::wstring > > CWindowList::GetWindowList()
    {
        _vecWindowList.clear();

        EnumChildWindows( GetDesktopWindow(), (WNDENUMPROC)CWindowList::enumChildWindowProc, (LPARAM)this );

        return _vecWindowList;
    }

    BOOL CALLBACK CWindowList::enumChildWindowProc( HWND hWnd, LPARAM lParam )
    {
        CWindowList* pWindwList = reinterpret_cast< CWindowList* >( lParam );

        WCHAR	szBuffer[ 1024 + 1 ] = L"";

        GetWindowText( hWnd, szBuffer, 1024 );

        pWindwList->_vecWindowList.push_back( std::make_pair( hWnd, (std::wstring)szBuffer ) );

        return TRUE;
    }

    HWND CWindowList::isExistWindow( DWORD processID, std::wstring windowText )
    {
        DWORD dwPID = 0;

        for( std::vector< std::pair< HWND, std::wstring > >::iterator It = _vecWindowList.begin(); It != _vecWindowList.end(); ++It )
        {
            GetWindowThreadProcessId( It->first, &dwPID );

            if( dwPID != processID )
                continue;

            if( wcsstr( It->second.c_str(), windowText.c_str() ) != NULL )
                return It->first;
        }

        return NULL;
    }

    int CWindowList::getWindowState( HWND hWnd )
    {
        if( hWnd == NULL )
            return 0;

        int nState = 1;

        // Is it visible?
        if (IsWindowVisible(hWnd))
            nState |= 2;

        // Is it enabled?
        if (IsWindowEnabled(hWnd))
            nState |= 4;

        // Is it active?
        if (GetForegroundWindow() == hWnd)
            nState |= 8;

        // Is it minimized?
        if (IsIconic(hWnd))
            nState |= 16;

        // Is it maximized?
        if (IsZoomed(hWnd))
            nState |= 32;

        return nState;
    }

	bool WriteEventLog( const std::wstring& programName, const std::wstring& logMessage, WORD eventType, WORD eventCatecory, DWORD eventID )
	{
		bool isWriteSuccess = false;

		HANDLE hEvent = NULL;
		PSID currentSID = NULL;

		hEvent = RegisterEventSource( NULL, programName.c_str() );

		if( hEvent == NULL )
			return isWriteSuccess;

#ifdef _AFX
		ATL::CAccessToken accessToken;
		ATL::CSid currentUserSid;
		if( accessToken.GetProcessToken( TOKEN_READ | TOKEN_QUERY ) && accessToken.GetUser( &currentUserSid ) )
			ConvertStringSidToSid( currentUserSid.Sid(), &currentSID );
#endif
		LPCWSTR pInsertStrings[ 1 ] = { NULL };
		pInsertStrings[ 0 ] = (LPCWSTR)logMessage.c_str();

		if( ReportEvent( hEvent, eventType, eventCatecory, eventID, currentSID, 1, 0, pInsertStrings, NULL ) == TRUE )
			isWriteSuccess = true;
		
		if( hEvent != NULL )
			DeregisterEventSource( hEvent );

		if( currentSID != NULL )
			LocalFree( currentSID );

		return isWriteSuccess;
	}

    bool IsProtectedFileByOS( const std::wstring& fileName )
    {
        return SfcIsFileProtected( NULL, fileName.c_str() ) != FALSE ? true : false;
    }

	bool GetProcessOwner( HANDLE hProcess_i, TCHAR* csOwner_o )
	{
		HANDLE hProcessToken = NULL;
		if( !::OpenProcessToken( hProcess_i, TOKEN_READ, &hProcessToken ) || !hProcessToken )
			return false;

		DWORD dwProcessTokenInfoAllocSize = 0;
		::GetTokenInformation(hProcessToken, TokenUser, NULL, 0, &dwProcessTokenInfoAllocSize);

		if( ::GetLastError() == ERROR_INSUFFICIENT_BUFFER )
		{
			PTOKEN_USER pUserToken = reinterpret_cast<PTOKEN_USER>( new BYTE[dwProcessTokenInfoAllocSize] );
			if (pUserToken != NULL)
			{
				if( ::GetTokenInformation( hProcessToken, TokenUser, pUserToken, dwProcessTokenInfoAllocSize, &dwProcessTokenInfoAllocSize ) == TRUE )
				{
					SID_NAME_USE snuSIDNameUse;
					TCHAR szUser[MAX_PATH] = { 0 };
					DWORD dwUserNameLength = MAX_PATH;
					TCHAR szDomain[MAX_PATH] = { 0 };
					DWORD dwDomainNameLength = MAX_PATH;

					if ( ::LookupAccountSid( NULL, pUserToken->User.Sid, szUser, &dwUserNameLength,
						szDomain, &dwDomainNameLength, &snuSIDNameUse ) )
					{
						wcscpy( csOwner_o, szUser );

						CloseHandle( hProcessToken );
						delete [] pUserToken;

						return true;
					}
				}

				delete [] pUserToken;
			}
		}

		CloseHandle( hProcessToken );

		return false;
	}

	BOOL SetRegistyStartProgram(BOOL bAutoExec, LPCWSTR lpValueName, LPCWSTR lpExeFileName)
	{
		HKEY hKey;
		LONG lRes;
		if(bAutoExec)
		{
			if(lpValueName == NULL || lpExeFileName == NULL)
				return FALSE;

			if(RegOpenKeyEx( 
				HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run",
				0L,
				KEY_WRITE,
				&hKey
				) != ERROR_SUCCESS)
				return FALSE;    

			lRes = RegSetValueEx(hKey,
				lpValueName, 
				0,      
				REG_SZ,    
				(BYTE*)lpExeFileName,
				wcslen(lpExeFileName) * 2 + 1);

			RegCloseKey(hKey);

			if(lRes != ERROR_SUCCESS) 
				return FALSE;
		}
		else 
		{
			if(RegOpenKeyEx(
				HKEY_LOCAL_MACHINE,
				L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Run", 
				0, KEY_ALL_ACCESS, 
				&hKey) != ERROR_SUCCESS)
				return FALSE;

			lRes = RegDeleteValue(hKey, lpValueName);      

			RegCloseKey(hKey);

			if(lRes != ERROR_SUCCESS) 
				return FALSE;
		}

		return TRUE;
	}

	void SetFroegroundWindowForce( HWND hWnd )
	{
		HWND hFHwnd;
		DWORD dwID, dwFid;

		hFHwnd = GetForegroundWindow();
		if( hFHwnd == hWnd )
			return;

		dwFid = GetWindowThreadProcessId( hFHwnd, NULL );
		dwID = GetWindowThreadProcessId( hWnd, NULL );
		if( AttachThreadInput( dwFid, dwID, TRUE) )
		{
			SetForegroundWindow( hWnd );
			BringWindowToTop( hWnd );
			SetFocus( hWnd );
			AttachThreadInput( dwFid, dwID, FALSE );
		}
	}

    CDownloaderByAPI::CDownloaderByAPI()
        : hDownloadThread( NULL ), hOnCompleteEvent( NULL ),hOnProgressEvent( NULL ), hResult_( S_OK )
    {
        hOnCompleteEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
        hOnProgressEvent = ::CreateEvent( NULL, FALSE, FALSE, NULL );
    }

    CDownloaderByAPI::~CDownloaderByAPI()
    {
        if( hOnProgressEvent != NULL )
            CloseHandle( hOnProgressEvent );
        if( hOnCompleteEvent != NULL )
            CloseHandle( hOnCompleteEvent );
    }

    HRESULT CDownloaderByAPI::DownloadToFile( const std::wstring& remoteURL, const std::wstring& localPath )
    {
        remoteURL_ = remoteURL;
        localPath_ = localPath;

        boost::thread t( boost::bind( &CDownloaderByAPI::workerThread, this ) );

        HANDLE hWaitEvent[2] = { NULL, };
        hWaitEvent[0] = hOnProgressEvent;
        hWaitEvent[1] = hOnCompleteEvent;

        while( true )
        {
            DWORD dwWaitState = ::WaitForMultipleObjects( 2, hWaitEvent, FALSE, 1000 * 60 * 10 );
            if( dwWaitState == WAIT_OBJECT_0 ) // onProgress
                continue;;
            if( dwWaitState == (WAIT_OBJECT_0 + 1) )
            {
                DeleteUrlCacheEntryW( remoteURL_.c_str() );
                continue;
            }
            if( dwWaitState == WAIT_TIMEOUT )
            {
                t.interrupt();
                break;
            }
        }

        return hResult_;
    }

    void CDownloaderByAPI::workerThread()
    {
        CDownloaderStatusCallback callback( hOnProgressEvent );

        hResult_ = URLDownloadToFileW( NULL, remoteURL_.c_str(), localPath_.c_str(), 0, &callback );

        SetEvent( hOnCompleteEvent );
    }

} // namepsace nsCommon


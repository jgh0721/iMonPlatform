#include "stdafx.h"

#ifdef NSCMNWMI_H
#include <cmnWMI.h>
using namespace nsCommon::nsCmnWMI;
#endif

#include <WinIoCtl.h>
#include <IPHlpApi.h>

#pragma comment( lib, "Iphlpapi.lib" )

#include "cmnPCInfo.h"

using namespace nsCommon;
using namespace nsCommon::nsCmnConvert;

namespace nsCommon
{
    namespace nsPCInfo
    {
        std::string GetCPUBrandString()
        {
            char CPUString[0x20] = {0,};
            char CPUBrandString[0x40] = {0,};
            int CPUInfo[4] = {-1};
            unsigned nExIds, i;

            // __cpuid with an InfoType argument of 0 returns the number of
            // valid Ids in CPUInfo[0] and the CPU identification string in
            // the other three array elements. The CPU identification string is
            // not in linear order. The code below arranges the information 
            // in a human readable form.
            __cpuid(CPUInfo, 0);
            memset(CPUString, 0, sizeof(CPUString));
            *((int*)CPUString) = CPUInfo[1];
            *((int*)(CPUString+4)) = CPUInfo[3];
            *((int*)(CPUString+8)) = CPUInfo[2];

            // Calling __cpuid with 0x80000000 as the InfoType argument
            // gets the number of valid extended IDs.
            __cpuid(CPUInfo, 0x80000000);
            nExIds = CPUInfo[0];
            memset(CPUBrandString, 0, sizeof(CPUBrandString));

            // Get the information associated with each extended ID.
            for (i=0x80000000; i<=nExIds; ++i)
            {
                __cpuid(CPUInfo, i);

                // Interpret CPU brand string and cache information.
                if  (i == 0x80000002)
                    memcpy(CPUBrandString, CPUInfo, sizeof(CPUInfo));
                else if  (i == 0x80000003)
                    memcpy(CPUBrandString + 16, CPUInfo, sizeof(CPUInfo));
                else if  (i == 0x80000004)
                    memcpy(CPUBrandString + 32, CPUInfo, sizeof(CPUInfo));
            }

            if( nExIds >= 0x80000004 )
                return CPUBrandString;
            else
                return CPUString;
        }

        unsigned __int64 GetTotalPhysicalMEMbytes()
        {
            MEMORYSTATUSEX msex;
            memset( &msex, '\0', sizeof(msex) );
            msex.dwLength       = sizeof(msex);

            ::GlobalMemoryStatusEx( &msex );

            return msex.ullTotalPhys;
        }

        std::string GetHDDSerial( unsigned nHDDIndex /*= 0 */ )
        {
            std::string strHDDSerial;

            //  GETVERSIONOUTPARAMS contains the data returned from the 
            //  Get Driver Version function.
            typedef struct _GETVERSIONOUTPARAMS
            {
                BYTE bVersion;      // Binary driver version.
                BYTE bRevision;     // Binary driver revision.
                BYTE bReserved;     // Not used.
                BYTE bIDEDeviceMap; // Bit map of IDE devices.
                DWORD fCapabilities; // Bit mask of driver capabilities.
                DWORD dwReserved[4]; // For future use.
            } GETVERSIONOUTPARAMS, *PGETVERSIONOUTPARAMS, *LPGETVERSIONOUTPARAMS;

            const DWORD DFP_GET_VERSION = 0x00074080;
            const DWORD DFP_RECEIVE_DRIVE_DATA = 0x0007c088;

            HANDLE hPhysicalDriveIOCTL = INVALID_HANDLE_VALUE;

            do 
            {
                hPhysicalDriveIOCTL = ::CreateFile( 
                    format( L"\\\\.\\PhysicalDrive%d", nHDDIndex ).c_str(), 
                    GENERIC_READ | GENERIC_WRITE, 
                    FILE_SHARE_READ | FILE_SHARE_WRITE, 
                    NULL, 
                    OPEN_EXISTING, 
                    0, 
                    NULL );

                if( hPhysicalDriveIOCTL == INVALID_HANDLE_VALUE )
                    break;

                DWORD cbBytesReturned = 0;
                GETVERSIONOUTPARAMS versionParams;
                ZeroMemory( &versionParams, sizeof( GETVERSIONOUTPARAMS ) );

                if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_GET_VERSION, 
                    NULL, 0, &versionParams, sizeof( GETVERSIONOUTPARAMS ), 
                    &cbBytesReturned, NULL ) == FALSE )
                    break;

                if( versionParams.bIDEDeviceMap > 0 )
                {
                    BYTE bIDCmd = 0;
                    //  Valid values for the bCommandReg member of IDEREGS.
                    const BYTE IDE_ATAPI_IDENTIFY = 0xA1;
                    const BYTE IDE_ATA_IDENTIFY = 0xEC;

                    // Now, get the ID sector for all IDE devices in the system.
                    // If the device is ATAPI use the IDE_ATAPI_IDENTIFY command,
                    // otherwise use the IDE_ATA_IDENTIFY command
                    bIDCmd = (versionParams.bIDEDeviceMap >> nHDDIndex & 0x10) ? IDE_ATAPI_IDENTIFY : IDE_ATA_IDENTIFY;
                    SENDCMDINPARAMS scip;
                    BYTE IdOutCmd [sizeof (SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1] = {0,};
                    ZeroMemory( &scip, sizeof( SENDCMDINPARAMS ) );

                    // Set up data structures for IDENTIFY command.
                    scip.cBufferSize = IDENTIFY_BUFFER_SIZE;
                    scip.irDriveRegs.bFeaturesReg = 0;
                    scip.irDriveRegs.bSectorCountReg = 1;
                    scip.irDriveRegs.bSectorNumberReg = 1;
                    scip.irDriveRegs.bCylLowReg = 0;
                    scip.irDriveRegs.bCylHighReg = 0;

                    // Compute the drive number.
                    scip.irDriveRegs.bDriveHeadReg = 0xA0 | (( nHDDIndex & 1 ) << 4);

                    // The command can either be IDE identify or ATAPI identify.
                    scip.irDriveRegs.bCommandReg = bIDCmd;
                    scip.bDriveNumber = nHDDIndex;
                    scip.cBufferSize = IDENTIFY_BUFFER_SIZE;

                    if( DeviceIoControl( hPhysicalDriveIOCTL, DFP_RECEIVE_DRIVE_DATA,
                        (LPVOID)&scip,
                        sizeof(SENDCMDINPARAMS) - 1,
                        (LPVOID)&IdOutCmd[0],
                        sizeof(SENDCMDOUTPARAMS) + IDENTIFY_BUFFER_SIZE - 1,
                        &cbBytesReturned, NULL ) == FALSE )
                        break;

                    DWORD diskData[ 256 ] = {0,};
                    USHORT *pIdSector = (USHORT *)((PSENDCMDOUTPARAMS)IdOutCmd)->bBuffer;

                    for( int idx = 0; idx < _countof(diskData); ++idx )
                        diskData[ idx ] = pIdSector[ idx ];

                    char szHDDSerial[1024] = {0,};
                    int index = 0;
                    int position = 0;

                    //  each integer has two characters stored in it backwards
                    for (index = 10; index <= 19; index++)
                    {
                        //  get high byte for 1st character
                        szHDDSerial [position] = (char) (diskData [index] / 256);
                        position++;

                        //  get low byte for 2nd character
                        szHDDSerial [position] = (char) (diskData [index] % 256);
                        position++;
                    }

                    //  end the string 
                    szHDDSerial [position] = '\0';

                    //  cut off the trailing blanks
                    for (index = position - 1; index > 0 && ' ' == szHDDSerial [index]; index--)
                        szHDDSerial [index] = '\0';

                    strHDDSerial = szHDDSerial; 
                }

            } while (false);

            if( hPhysicalDriveIOCTL != INVALID_HANDLE_VALUE )
                CloseHandle( hPhysicalDriveIOCTL );
            hPhysicalDriveIOCTL = INVALID_HANDLE_VALUE;

            return strHDDSerial;
        }

        std::wstring GetVolumeSerial( const std::wstring& volumePath /*= L"C:\\" */ )
        {
            wchar_t nameBuffer[ MAX_PATH + 1 ] = {0,};
            DWORD volumeSerial = 0;

            GetVolumeInformation
                (
                volumePath.c_str(),  
                (LPTSTR)nameBuffer, // volume label 버퍼
                MAX_PATH,                // volume label을 저장할 버퍼의 크기
                &volumeSerial,      // volume의 일련번호
                NULL,
                NULL,
                NULL,
                0                 // 파일 시스템 이름을 저장할 버퍼의 크기
                );

            return format( L"%d", volumeSerial );
        }

        //////////////////////////////////////////////////////////////////////////

        std::vector< NETWORK_INFO > GetNetworkInfos( ULONG defaultAddressFamily /* = AF_INET */ )
        {
            std::vector< NETWORK_INFO > vecResult;

            DWORD dwRetVal = 0;
            PIP_ADAPTER_ADDRESSES pAddresses = NULL;
            PIP_ADAPTER_ADDRESSES pCurrAddress = NULL;

            ULONG nOutBufferLen = 8192;
            
            do 
            {
                pAddresses = (PIP_ADAPTER_ADDRESSES)malloc( nOutBufferLen );
                if( pAddresses == NULLPTR )
                    break;
                
                dwRetVal = GetAdaptersAddresses( defaultAddressFamily, 0 
#if _WIN32_WINNT >= NTDDI_WINXPSP1
                    | GAA_FLAG_INCLUDE_PREFIX 
#endif
#if _WIN32_WINNT >= NTDDI_VISTA
                    | GAA_FLAG_INCLUDE_WINS_INFO  | GAA_FLAG_INCLUDE_GATEWAYS | GAA_FLAG_INCLUDE_ALL_INTERFACES 
#endif
                    , NULL, pAddresses, &nOutBufferLen );


                if( dwRetVal == ERROR_SUCCESS )
                    break;

                if( dwRetVal == ERROR_BUFFER_OVERFLOW )
                {
                    free( pAddresses );
                    pAddresses = NULL;
                }

                nOutBufferLen += nOutBufferLen;
            } while ( dwRetVal == ERROR_BUFFER_OVERFLOW );

            do 
            {
                if( dwRetVal != ERROR_SUCCESS )
                    break;

                pCurrAddress = pAddresses;
                while( pCurrAddress )
                {
                    NETWORK_INFO networkInfo;

                    networkInfo.ipv4IfIndex     = pCurrAddress->IfIndex;
                    networkInfo.IfType          = pCurrAddress->IfType;
                    networkInfo.ipv6IfIndex     = pCurrAddress->Ipv6IfIndex;
                    networkInfo.adapterName     = pCurrAddress->AdapterName;
                    networkInfo.description     = pCurrAddress->Description; 
                    networkInfo.friendlyName    = pCurrAddress->FriendlyName;

                    PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurrAddress->FirstUnicastAddress;
                    while( pUnicast )
                    {
                        // networkInfo.vecUnicastIP.push_back( 
                        ADDRESS_FAMILY af = pUnicast->Address.lpSockaddr->sa_family;
                        if( af == AF_INET )
                        {
                            networkInfo.vecUnicastIPv4.push_back(
                                inet_ntoa( ((sockaddr_in*)(pUnicast->Address.lpSockaddr))->sin_addr )
                                );
                        }
                        else if( af == AF_INET6 )
                        {
                            char ipv6[ 128 ] = {0,};
                            DWORD nBufferLen = 128;
                            WSAAddressToStringA( pUnicast->Address.lpSockaddr, pUnicast->Address.iSockaddrLength, NULL, ipv6, &nBufferLen );
                            networkInfo.vecUnicastIPv6.push_back( ipv6 );
                        }
                        
                        pUnicast = pUnicast->Next;
                    }
                    
                    PIP_ADAPTER_DNS_SERVER_ADDRESS pDnServer = pCurrAddress->FirstDnsServerAddress;
                    while( pDnServer )
                    {
                        char dnsServer[ 128 ] = {0,};
                        DWORD nBufferLen = 128;
                        WSAAddressToStringA( pDnServer->Address.lpSockaddr, pDnServer->Address.iSockaddrLength, NULL, dnsServer, &nBufferLen );
                        if( pDnServer->Address.lpSockaddr->sa_family == AF_INET )
                            networkInfo.vecDNSServerIPv4.push_back( dnsServer );
                        else if( pDnServer->Address.lpSockaddr->sa_family == AF_INET6 )
                            networkInfo.vecDNSServerIPv6.push_back( dnsServer );

                        pDnServer = pDnServer->Next;
                    }

                    if( pCurrAddress->PhysicalAddressLength != 0 )
                    {
                        for( int idx = 0; idx < pCurrAddress->PhysicalAddressLength; ++idx )
                        {
                            if( idx == ( pCurrAddress->PhysicalAddressLength - 1 ) )
                                networkInfo.macAddress += format( "%.2X", (int)pCurrAddress->PhysicalAddress[idx] );
                            else
                                networkInfo.macAddress += format( "%.2X-", (int)pCurrAddress->PhysicalAddress[idx] );
                        }
                    }

                    pCurrAddress = pCurrAddress->Next;
                    vecResult.push_back( networkInfo );
                }
            } while (false);

            return vecResult;
        }

        std::vector< std::wstring > GetDNSServerIPList()
        {
            std::vector< std::wstring > vecDNSServerIP;

            FIXED_INFO* pFixedInfo = (FIXED_INFO*)malloc( sizeof( FIXED_INFO ) );
            DWORD dwRetValue = 0;
            ULONG ulOutBufLen = sizeof( FIXED_INFO );

            do 
            {
                if( pFixedInfo == NULL )
                    break;

                if( GetNetworkParams( pFixedInfo, &ulOutBufLen ) == ERROR_BUFFER_OVERFLOW )
                {
                    if( pFixedInfo != NULL )
                        free( pFixedInfo );
                    pFixedInfo = NULL;
                    pFixedInfo = (FIXED_INFO *)malloc( ulOutBufLen );
                    if( pFixedInfo == NULL )
                        break;
                }

                dwRetValue = GetNetworkParams( pFixedInfo, &ulOutBufLen );
                if( dwRetValue != NO_ERROR )
                    break;

                IP_ADDR_STRING *pIPAddress = &pFixedInfo->DnsServerList;
                while( pIPAddress )
                {
                    vecDNSServerIP.push_back( CA2U(pIPAddress->IpAddress.String).c_str() );
                    pIPAddress = pIPAddress->Next;
                }

            } while (false);

            if( pFixedInfo != NULL )
                free( pFixedInfo );

            return vecDNSServerIP;
        }

        std::vector< std::wstring > GetGatewayIPList()
        {
            std::vector< std::wstring > vecGatewayIP;

            PIP_ADAPTER_INFO pAdapter = NULL;
            PIP_ADAPTER_INFO pAdapterInfo = NULL;

            do 
            {
                DWORD dwRetVal = 0;

                ULONG ulOutBufLen = sizeof (IP_ADAPTER_INFO);
                pAdapterInfo = (IP_ADAPTER_INFO *) malloc(sizeof (IP_ADAPTER_INFO));
                if( pAdapterInfo == NULL )
                    break;

                if( GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW ) 
                {
                    free( pAdapterInfo );
                    pAdapterInfo = NULL;
                    pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
                    if( pAdapterInfo == NULL ) 
                        break;
                }

                dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
                if( dwRetVal != NO_ERROR )
                    break;

                pAdapter = pAdapterInfo;

                while (pAdapter) 
                {
                    IP_ADDR_STRING* pIPAddrString = &pAdapter->GatewayList;
                    while( pIPAddrString != NULL )
                    {
                        if( _stricmp( pIPAddrString->IpAddress.String, "0.0.0.0" ) != 0 )
                            vecGatewayIP.push_back( CA2U( pIPAddrString->IpAddress.String ).c_str() );
                        pIPAddrString = pIPAddrString->Next;
                    }

                    pAdapter = pAdapter->Next;
                }

            } while (false);

            if (pAdapterInfo)
                free(pAdapterInfo);

            return vecGatewayIP;
        }

        //////////////////////////////////////////////////////////////////////////

        std::wstring GetLocalComputerName()
        {
            TCHAR wszComputerName[ MAX_COMPUTERNAME_LENGTH + 2 ] = {0,};
            DWORD dwComputerBufferSize = _countof( wszComputerName );

            if( GetComputerName( wszComputerName, &dwComputerBufferSize ) == FALSE )
                return L"";

            return wszComputerName;
        }

#ifdef NSCMNWMI_H
        std::wstring GetWMIInfoToStr( const std::wstring& className, const std::wstring& propertyName )
        {
            std::wstring retVal;

            do 
            {
                CWbemLocator wbemLocator;
                CWbemService wbemService;

                if( wbemLocator.ConnectServer( wbemService ) == false )
                    break;
                
                CWbemClassEnumerator wbemClasEnumerator;
                if( FAILED( wbemService.GetInstanceEnumerator( className, wbemClasEnumerator ) ) )
                    break;

                wbemClasEnumerator.SetObjectProperty( TyVecWbemProperty( 1, propertyName ) );
                if( FAILED( wbemClasEnumerator.RefreshEnum() ) )
                    break;

                VARIANT vtData;
                VariantInit( &vtData );
               
                // 항상 첫 번째 인스턴스의 값 가져옴
                if( SUCCEEDED( wbemClasEnumerator.GetItem( 0, propertyName, vtData ) ) )
                {
                    if( vtData.vt == VT_BSTR )
                    {
                        retVal = V_BSTR( &vtData );
                    }
                    else 
                    {
                        VARIANT vtDest;
                        VariantInit( &vtDest );
                        if( SUCCEEDED( VariantChangeType( &vtData, &vtDest, VARIANT_ALPHABOOL | VARIANT_NOUSEROVERRIDE, VT_BSTR ) ) )
                            retVal = V_BSTR( &vtDest );
                        VariantClear( &vtDest );
                    }
                }

                VariantClear( &vtData );

            } while (false);
            
            return retVal;
        }
#endif

    }
}


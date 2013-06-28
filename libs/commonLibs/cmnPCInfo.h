#pragma once

#include <stdio.h>
#include <string.h>
#include <intrin.h>

#include <string>
#include <vector>

#include "cmnUtils.h"
#include "cmnConverter.h"

namespace nsCommon
{
    namespace nsPCInfo
    {
        typedef struct tagNETWORK_INFO
        {
            unsigned int                            ipv4IfIndex;    // ipv4 를 사용할 수 없는 인터페이스라면(ipv6전용등) 해당 인덱스는 0을 사용한다
            unsigned int                            ipv6IfIndex;    // ipv6 를 사용할 수 없는 인터페이스라면 해당 인덱스는 0 을 사용한다
            unsigned int                            IfType;         // IF_TYPE_ETHERNET_CSMACD, IF_TYPE_SOFTWARE_LOOPBACK, IF_TYPE_IEEE80211 ...
            std::string                             adapterName;
            std::wstring                            description;
            std::wstring                            friendlyName;

            std::string                             macAddress;

            std::vector< std::string >              vecUnicastIPv4;
            std::vector< std::string >              vecUnicastIPv6;

            std::vector< std::string >              vecDNSServerIPv4;
            std::vector< std::string >              vecDNSServerIPv6;

            std::vector< std::wstring >             vecGatewayIP;

        } NETWORK_INFO, *PNETWORK_INFO;

        // CPU 정보
        std::string                     GetCPUBrandString();
        
        // 메모리 정보
        unsigned __int64                GetTotalPhysicalMEMbytes();

        // HDD 정보
        std::string                     GetHDDSerial( unsigned nHDDIndex = 0 );
        
        // 볼륨 정보
        std::wstring                    GetVolumeSerial( const std::wstring& volumePath = L"C:\\" );

        // 네트워크 정보, 2 = AF_INET, 23 = AF_INET6, 0 = AF_UNSPEC
        std::vector< NETWORK_INFO >     GetNetworkInfos( ULONG defaultAddressFamily = 0 );
        /*!
           인터넷 연결에 필요한 DNS 서버의 IP 목록을 가져온다
        // Method:    getDNSServerIPList
        // FullName:  getDNSServerIPList
        // Access:    public 
        // Returns:   std::vector< std::wstring >
        // Qualifier:
        */
        std::vector< std::wstring >     GetDNSServerIPList();
        std::vector< std::wstring >     GetGatewayIPList();

        // 기타 정보
        /*!
            로컬 컴퓨터의 NetBIOS 컴퓨터 이름을 반환
            Method:    getLocalComputerName
            FullName:  getLocalComputerName
            Access:    public 
            @return:   std::wstring
            Qualifier:
        */
        std::wstring                    GetLocalComputerName();
        
#ifdef NSCMNWMI_H
        std::wstring                    GetWMIInfoToStr( const std::wstring& className, const std::wstring& propertyName );
#endif
    }
}

/*!
    http://www.codeproject.com/Tips/308855/Identify-Unknown-Hardware-Using-Hardware-IDs
    http://zextor.tistory.com/m/post/view/id/2670193
*/


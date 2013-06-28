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
            unsigned int                            ipv4IfIndex;    // ipv4 �� ����� �� ���� �������̽����(ipv6�����) �ش� �ε����� 0�� ����Ѵ�
            unsigned int                            ipv6IfIndex;    // ipv6 �� ����� �� ���� �������̽���� �ش� �ε����� 0 �� ����Ѵ�
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

        // CPU ����
        std::string                     GetCPUBrandString();
        
        // �޸� ����
        unsigned __int64                GetTotalPhysicalMEMbytes();

        // HDD ����
        std::string                     GetHDDSerial( unsigned nHDDIndex = 0 );
        
        // ���� ����
        std::wstring                    GetVolumeSerial( const std::wstring& volumePath = L"C:\\" );

        // ��Ʈ��ũ ����, 2 = AF_INET, 23 = AF_INET6, 0 = AF_UNSPEC
        std::vector< NETWORK_INFO >     GetNetworkInfos( ULONG defaultAddressFamily = 0 );
        /*!
           ���ͳ� ���ῡ �ʿ��� DNS ������ IP ����� �����´�
        // Method:    getDNSServerIPList
        // FullName:  getDNSServerIPList
        // Access:    public 
        // Returns:   std::vector< std::wstring >
        // Qualifier:
        */
        std::vector< std::wstring >     GetDNSServerIPList();
        std::vector< std::wstring >     GetGatewayIPList();

        // ��Ÿ ����
        /*!
            ���� ��ǻ���� NetBIOS ��ǻ�� �̸��� ��ȯ
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


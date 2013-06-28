#include "StdAfx.h"

#include <vector>

#include <Shlwapi.h>
#pragma comment( lib, "Shlwapi" )

#include "cmnPath.h"
#include "cmnUtils.h"

namespace nsCommon
{
    std::wstring nsCmnPath::GetCurrentPath()
    {
        wchar_t wszBuffer[ MAX_PATH ] = {0,};
        
        ::GetModuleFileNameW( NULL, wszBuffer, MAX_PATH );
        ::PathRemoveFileSpecW( wszBuffer );
        
        return CanonicalizePath(wszBuffer);
    }

    std::wstring nsCmnPath::CanonicalizePath( const std::wstring& strPath )
    {
        std::vector< wchar_t > vecBuffer;
        vecBuffer.resize( strPath.size() + 1 );

        if( ::PathCanonicalizeW( &vecBuffer[0], strPath.c_str() ) == FALSE )
            return strPath;

        return &vecBuffer[0];
    }

    std::wstring nsCmnPath::GetFilePath( const std::wstring& filePath, bool isAppendSep /*= true */ )
    {
        std::wstring retPath = nsCommon::string_replace_all( filePath, L"/", L"\\" );

        



        return retPath;
    }

};

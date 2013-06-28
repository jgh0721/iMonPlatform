#include "StdAfx.h"

#include <locale>

#include "cmnUtils.h"
#include "cmnConverter.h"

using namespace nsCommon::nsCmnConvert;

namespace nsCommon
{
    const std::locale locEnUs( "English" );
    const std::locale locKoKr( "Korean" );

    /* 유용한 std::string 에 대한 formatting 함수 */
    __inline std::string format_arg_list(const char *fmt, va_list args)
    {
        if (!fmt) return "";
        int   result = -1, length = 512;
        char *buffer = 0;
        while (result == -1)    {
            if (buffer)
                delete [] buffer;
            buffer = new char [length + 1];
            memset(buffer, 0, (length + 1) * sizeof(char) );

            // remove deprecate warning
            //result = _vsnprintf(buffer, length, fmt, args);

            result = _vsnprintf_s(buffer, length, _TRUNCATE, fmt, args);
            length *= 2;
        }
        std::string s(buffer);
        delete [] buffer;
        return s;
    }

    __inline std::wstring format_arg_list(const wchar_t *fmt, va_list args)
    {
        if (!fmt) return L"";
        int   result = -1, length = 512;
        wchar_t *buffer = 0;
        while (result == -1)    {
            if (buffer)
                delete [] buffer;
            buffer = new wchar_t [length + 1];
            memset(buffer, 0, (length + 1) * sizeof(wchar_t) );

            // remove deprecate warning
            //result = _vsnprintf(buffer, length, fmt, args);

            result = _vsnwprintf_s(buffer, length, _TRUNCATE, fmt, args);
            length *= 2;
        }
        std::wstring s(buffer);
        delete [] buffer;
        return s;
    }

    std::string format(const char *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        std::string s = format_arg_list(fmt, args);
        va_end(args);
        return s;
    }

    std::wstring format(const wchar_t *fmt, ...)
    {
        va_list args;
        va_start(args, fmt);
        std::wstring s = format_arg_list(fmt, args);
        va_end(args);
        return s;
    }

    bool IsAlphabet( const std::string& strString )
    {
        for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
            if( isalpha( *iterChar, locEnUs ) == true )
                return true;
        return false;
    }

    bool IsAlphabet( const std::wstring& strString )
    {
        for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
            if( isalpha( *iterChar, locEnUs ) == true )
                return true;
        return false;
    }

    bool IsNumber( const std::string& strString )
    {
        for( std::string::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
            if( isdigit( *iterChar, locEnUs ) == false )
                return false;

        return true;
    }

    bool IsNumber( const std::wstring& strString )
    {
        for( std::wstring::const_iterator iterChar = strString.begin(); iterChar != strString.end(); ++iterChar )
            if( isdigit( *iterChar, locEnUs ) == false )
                return false;
        return true;
    }

    std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace )
    {
        std::string result = src;    
        std::string::size_type pos = 0;    
        std::string::size_type offset = 0;    
        std::string::size_type pattern_len = pattern.size();    
        std::string::size_type replace_len = replace.size();    

        while ( ( pos = result.find( pattern, offset ) ) != std::string::npos )    
        {    
            result.replace( result.begin() + pos,     
                result.begin() + pos + pattern_len,     
                replace );    
            offset = pos + replace_len;    
        }    
        return result;    
    }

    std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace )
    {
        std::wstring result = src;    
        std::wstring::size_type pos = 0;    
        std::wstring::size_type offset = 0;    
        std::wstring::size_type pattern_len = pattern.size();    
        std::wstring::size_type replace_len = replace.size();    

        while ( ( pos = result.find( pattern, offset ) ) != std::wstring::npos )    
        {    
            result.replace( result.begin() + pos,     
                result.begin() + pos + pattern_len,     
                replace );    
            offset = pos + replace_len;    
        }    
        return result;    

    }

    std::vector< std::wstring > convertMultiSZToVector( const WCHAR* pwszText )
    {
        std::vector< std::wstring > vecRet;

        size_t index = 0;
        size_t len = wcslen( pwszText );
        while( len > 0 )
        {
            vecRet.push_back( &pwszText[ index ] );
            index += len + 1;
            len = wcslen( &pwszText[ index ] );
        }

        return vecRet;
    }

    int _wcsicmp( const std::wstring& lhs, const std::wstring& rhs )
    {
        return ::_wcsicmp( lhs.c_str(), rhs.c_str() );
    }

#ifdef _AFX
    int _wcsicmp( const CString& lhs, const CString& rhs )
    {
        return ::_wcsicmp( lhs.GetString(), rhs.GetString() );
    }
#endif

    int _wtoi( const std::wstring& lhs )
    {
        return ::_wtoi( lhs.c_str() );
    }

    __int64 _wtoi64( const std::wstring& lhs )
    {
        return ::_wtoi64( lhs.c_str() );
    }

    int u8sicmp( const std::string& lhs, const std::string& rhs )
    {
        return ::_wcsicmp( nsCmnConvert::CU82U( lhs ), nsCmnConvert::CU82U( rhs ) );
    }

    int u8toi( const std::string& lhs )
    {
        return ::_wtoi( nsCmnConvert::CU82U( lhs ) );
    }

    __int64 u8toi64( const std::string& lhs )
    {
        return ::_wtoi64( nsCmnConvert::CU82U( lhs ) );
    }

    bool CreateDirectoryRecursively( const wchar_t* wszDirectory )
    {
        bool isSuccess = false;
        const unsigned int MAX_FOLDER_DEPTH = 122;

        do 
        {
            size_t nLen = wcslen( wszDirectory );
            if( nLen < (_MAX_DRIVE + 1) || nLen > (MAX_PATH - 1) ) 
                break;

            wchar_t szPath[MAX_PATH] = {0,};
            INT_PTR offset = _MAX_DRIVE;

            for(int i = 0; i < MAX_FOLDER_DEPTH; i++)
            {
                if( (UINT_PTR)offset >= nLen ) 
                    break;

                const wchar_t * pos = wszDirectory + offset;
                const wchar_t * dst = wcschr(pos, L'\\');

                if( dst == NULL ) 
                {
                    wcsncpy_s(szPath, MAX_PATH, wszDirectory, nLen);
                    i = MAX_FOLDER_DEPTH;
                }
                else 
                {
                    INT_PTR cnt = dst - wszDirectory;
                    wcsncpy_s(szPath, MAX_PATH, wszDirectory, cnt);

                    offset = cnt + 1;
                }

                isSuccess = ::CreateDirectoryW( szPath, NULL ) != FALSE ? true : false;
                if( isSuccess == false && ::GetLastError() == ERROR_ALREADY_EXISTS )
                    isSuccess = true;
            }
            
        } while (false);

        return isSuccess;
    }

    std::string getCPUBrandString()
    {
        char CPUString[0x20];
        char CPUBrandString[0x40];
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

        // Display all the information in user-friendly format.

        printf_s("\n\nCPU String: %s\n", CPUString);

        if( nExIds >= 0x80000004 )
            return CPUBrandString;
        else
            return  CPUString;
    }

    unsigned long getULONGFromIPAddress( BYTE lsb, BYTE lsbBy1, BYTE lsbBy2, BYTE lsbBy3 )
    {
        unsigned long ipaddress = 0;

        ipaddress = lsb | ( lsbBy1 << 8 );
        ipaddress = ipaddress | ( lsbBy2 << 16 );
        ipaddress = ipaddress | ( lsbBy3 << 24 );

        return ipaddress;
    }

    std::string getIPAddressFromULONG( unsigned long ulIPaddress )
    {
        if( ulIPaddress <= 0 )
            "";

        in_addr in;

        in.S_un.S_addr = ulIPaddress;
        return inet_ntoa( in );
    }

    std::wstring getMajorVersion( const std::wstring& sVersion, const std::wstring delimiter )
    {
        if( sVersion.find( delimiter ) == -1 )
            return L"";

        std::wstring sMajorVer;
        sMajorVer.clear();

        sMajorVer = sVersion.substr( 0, sVersion.find( delimiter ) );

        return sMajorVer;
    }

    std::wstring GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter )
    {
        bool isFoundQuote = false;

        size_t delimLength = delimiter.size();

        std::wstring::size_type startPos = strMessage.find( delimiter );
        std::wstring::size_type endPos = std::wstring::npos;

        if( startPos == std::wstring::npos )
            return L"";

        startPos += delimLength;

        if( strMessage[ startPos ] == L'\"' )
        {
            isFoundQuote = true;
            startPos++;
        }

        endPos = strMessage.find( isFoundQuote == false ? L" " : L"\"", startPos + 1 );
        if( endPos == std::wstring::npos )
            return strMessage.substr( startPos );

        return strMessage.substr( startPos, endPos - startPos );
    }

    bool isSelectY( const std::string& u8Text )
    {
        if( u8sicmp( u8Text, "Y" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "T" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "Yes" ) == 0 )
            return true;

        if( u8sicmp( u8Text, "True" ) == 0 )
            return true;

        return false;
    }

    bool isSelectY( const std::wstring& text )
    {
        if( _wcsicmp( text, L"Y" ) == 0 )
            return true;

        if( _wcsicmp( text, L"T" ) == 0 )
            return true;

        if( _wcsicmp( text, L"Yes" ) == 0 )
            return true;

        if( _wcsicmp( text, L"True" ) == 0 )
            return true;

        return false;
    }

#ifdef QSTRING_H
    bool isSelectY( const QString& text )
    {
        return isSelectY( text.toStdWString() );
    }
#endif

};

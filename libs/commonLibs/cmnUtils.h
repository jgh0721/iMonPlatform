#ifndef CMNUTILS_H
#define CMNUTILS_H

#include <string>
#include <string.h>
#include <functional>
#include <stdarg.h>
#include <algorithm>
#include <cctype>
#include <set>
#include <vector>

#include <unordered_map>

#if _MSC_VER == 1500 
    #define NULLPTR NULL
#else
    #define NULLPTR nullptr
#endif

namespace nsCommon
{
    /// 매개변수들 UTF-8 형태로 관리함
    #if _MSC_VER >= 1600
        typedef std::unordered_map< std::string, std::string >        tyMapParams;
    #else
        typedef std::tr1::unordered_map< std::string, std::string >        tyMapParams;
    #endif

    /*!
        구글 프로토콜 버퍼의 키-값 쌍의 목록을 tyMapParams 로 변환
    */
    template< typename T >
    nsCommon::tyMapParams convertKVPairListToMap( const T& kvPairList )
    {
        nsCommon::tyMapParams mapParams;
        for( auto& it = kvPairList.begin(); it != kvPairList.end(); ++it )
            mapParams[ it->key() ] = it->value();

        return mapParams;
    }

    template< typename T, typename K, typename V >
    void setKVPairWithValue( T kvPair, const K& key, const V& value )
    {
        kvPair->set_key( key );
        kvPair->set_value( value );
    }

    const std::string		SELECT_Y									= "Y";
    const std::string		SELECT_N									= "N";

    bool isSelectY( const std::string& u8Text );
    bool isSelectY( const std::wstring& text );
#ifdef QSTRING_H
    bool isSelectY( const QString& text );
#endif
    #define IF_FALSE_BREAK( var, expr ) if( ( var = (expr) ) == false ) break;

    const unsigned int MAX_TIME_BUFFER_SIZE = 128;
    extern const std::locale locEnUs;
    extern const std::locale locKoKr;

    std::string format(const char *fmt, ...);
    std::string format_arg_list(const char *fmt, va_list args);
    std::wstring format(const wchar_t *fmt, ...);
    std::wstring format_arg_list(const wchar_t *fmt, va_list args);

    bool IsAlphabet( const std::string& strString );
    bool IsAlphabet( const std::wstring& strString );

    bool IsNumber( const std::string& strString );
    bool IsNumber( const std::wstring& strString );

    std::string string_replace_all( const std::string& src, const std::string& pattern, const std::string& replace );
    std::wstring string_replace_all( const std::wstring& src, const std::wstring& pattern, const std::wstring& replace );

    std::vector< std::wstring >     convertMultiSZToVector( const WCHAR* pwszText );

    template <typename T> inline 
        void DeletePtr( T& ptr ) 
    { 
        if( ptr != NULLPTR )
        {
            delete ptr; 
            ptr = NULLPTR; 
        }
    }

    template <typename T> inline
        void DeletePtrA( T& ptr )
    {
        if( ptr != NULLPTR )
        {
            delete [] ptr;
            ptr = NULLPTR;
        }
    }

    template <typename T> inline
        void ReleasePtr( T& ptr )
    {
        if( ptr != NULLPTR )
        {
            ptr->Release();
            delete ptr;
            ptr = NULLPTR;
        }
    }

    template< typename T > inline
        void DeleteMapContainerPointerValue( T& container )
    {
        for( typename T::iterator it = container.begin(); it != container.end(); ++it )
            delete (*it).second;
    }

    int						_wcsicmp( const std::wstring& lhs, const std::wstring& rhs );
#ifdef _AFX
    int						_wcsicmp( const CString& lhs, const CString& rhs );
#endif

    int						_wtoi( const std::wstring& lhs );
    __int64					_wtoi64( const std::wstring& lhs );

    int						u8sicmp( const std::string& lhs, const std::string& rhs );
    int						u8toi( const std::string& lhs );
    __int64					u8toi64( const std::string& lhs );

    class eq_nocaseW
        : public std::binary_function< const std::wstring, const std::wstring, bool >
    {
    public:
        bool operator()( const std::wstring& lsh, const std::wstring& rsh ) const
        {
            return _wcsicmp( lsh, rsh ) == 0;
        }
    };

    class lt_nocaseW
        : public std::binary_function< const std::wstring, const std::wstring, bool>
    {
    public:
        bool operator()( const std::wstring& x, const std::wstring& y ) const
        {
            return _wcsicmp( x, y ) < 0 ;
        }
    };

    class lt_nocaseA
        : public std::binary_function< char*, char*, bool>
    {
    public:
        bool operator()( std::string x, std::string y ) const
        {
            return _stricmp( x.c_str(), y.c_str() ) < 0;
        }
    };
    
    bool CreateDirectoryRecursively( const wchar_t* wszDirectory );
    std::string		getCPUBrandString();
    // IP 주소로부터 해당하는 U_INT 숫자를 반환, 네트워크 바이트 순서로 반환됨
    unsigned long	getULONGFromIPAddress( BYTE lsb, BYTE lsbBy1, BYTE lsbBy2, BYTE lsbBy3 );
    // 네트워크 바이트 순서로 된 ULONG 을 받아 문자열된 IP 문자열 반환
    std::string		getIPAddressFromULONG( unsigned long ulIPaddress );

    std::wstring getMajorVersion( const std::wstring& sVersion, const std::wstring delimiter = L"." );

    // 문자열에서 공백으로 구분된 후 특정 delmiter 를 제거한 단어를 반환한다. 
    // /MessageText:"fsdfsdfds" /MessageCode:232 
    // delimiter = /MessageText: 라면 "fsdfsdfds" 반환
    // delimiter = /MessageCode: 라면 "232" 반환
    std::wstring GetWordsUsingDelimiter( const std::wstring& strMessage, const std::wstring& delimiter );

    typedef std::set< std::wstring, nsCommon::lt_nocaseW >              TySetTextNoCase;
} // namespace nsCmnCommon

#endif // CMNUTILS_H

#pragma once

#include <string>

#ifndef _INC_COMUTIL
#include <comutil.h>
#endif

#include "cmnUtils.h"

/*!
    문자열 인코딩 변환 클래스

    ----
    2013-01-09
        namespace 밑으로 이동
        
*/
namespace nsCommon
{
    namespace nsCmnConvert
    {
        int getActualUTF8Length( const char* szUTF8 );
        int convertUTF8toUTF16( const char* pszUTF8, wchar_t* pwszUTF16 );
        int convertUTF16toUTF8( wchar_t* pwszUTF16, char* pszUTF8 );
        int getBytesUTF16toUTF8( wchar_t* pwszUTF16 );

        errno_t CopyAnsiString( char** pszDest, const char* pszSrc, size_t srcLength = -1 );
        errno_t CopyWideString( wchar_t** pszDest, const wchar_t* pszSrc, size_t srcLength = -1 );

        void ConvertUpperString( std::string& strText );
        void ConvertUpperString( std::wstring& strText );

#if defined(_INC_COMUTIL) && defined(_AFX)
        std::string ConvertToStringA( const VARIANT& vtItem, const std::string& strSep = "," );
        std::wstring ConvertToStringW( const VARIANT& vtItem, const std::wstring& strSep = L"," );
#endif
        class CA2U
        {
        public:
            CA2U( const char* szText );
            CA2U( const std::string& strText );
            CA2U( const CA2U& converter );
            ~CA2U();

            CA2U&   operator=( const char* szText );
            CA2U&   operator=( const std::string& strText );
            CA2U&   operator=( const CA2U& converter );

            operator const wchar_t*() const { return _szText != NULLPTR ? _szText : L"(NULL)"; };
            const wchar_t*          c_str() const { return this->operator const wchar_t *(); }
            const std::wstring      toStdString() { return _szText != NULLPTR ? _szText : L"(NULL)"; };

#if _MSC_VER >= 1600
            CA2U( CA2U&& converter );
            CA2U&   operator=( CA2U&& converter );
#endif

#ifdef QSTRING_H
            CA2U( const QString& strText );
            const QString           toQtString() { return QString::fromUtf16( _szText != NULLPTR ? (const ushort*)_szText : (const ushort*)L"(NULL)" ); };
#endif

#ifdef _AFX
            CA2U( const CStringA& strText );
            const CStringW          toMFCString() { return CStringW( _szText != NULLPTR ? _szText : L"(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            wchar_t*                _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU2A
        {
        public:
            CU2A( const wchar_t* szText );
            CU2A( const std::wstring& strText );
            CU2A( const CU2A& converter );
            ~CU2A();

            CU2A&   operator=( const wchar_t* szText );
            CU2A&   operator=( const std::wstring& strText );
            CU2A&   operator=( const CU2A& converter );

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*             c_str() const { return this->operator const char *(); }
            const std::string       toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU2A( CU2A&& converter );
            CU2A&   operator=( CU2A&& converter );
#endif

#ifdef QSTRING_H
            CU2A( const QString& strText );
            const QString           toQtString() { return QString::fromLocal8Bit( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU2A( const CStringW& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const wchar_t* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU82U
        {
        public:
            CU82U( const char* szText );
            CU82U( const std::string& strText );
            CU82U( const CU82U& converter );
            ~CU82U();

            CU82U&   operator=( const char* szText );
            CU82U&   operator=( const std::string& strText );
            CU82U&   operator=( const CU82U& converter );

            operator const wchar_t*() const { return _szText != NULLPTR ? _szText : L"(NULL)"; };
            const wchar_t*          c_str() const { return this->operator const wchar_t *(); }
            const std::wstring      toStdString() { return _szText != NULLPTR ? _szText : L"(NULL)"; };

#if _MSC_VER >= 1600
            CU82U( CU82U&& converter );
            CU82U&   operator=( CU82U&& converter );
#endif

#ifdef QSTRING_H
            CU82U( const QString& strText );
            const QString           toQtString() { return QString::fromUtf16( _szText != NULLPTR ? (const ushort*)_szText : (const ushort*)L"(NULL)" ); };
#endif

#ifdef _AFX
            CU82U( const CStringA& strText );
            const CStringW          toMFCString() { return CStringW( _szText != NULLPTR ? _szText : L"(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            wchar_t*                _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU2U8
        {
        public:
            CU2U8( const wchar_t* szText );
            CU2U8( const std::wstring& strText );
            CU2U8( const CU2U8& converter );
            ~CU2U8();

            CU2U8&   operator=( const wchar_t* szText );
            CU2U8&   operator=( const std::wstring& strText );
            CU2U8&   operator=( const CU2U8& converter );

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*             c_str() const { return this->operator const char *(); }
            const std::string       toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU2U8( CU2U8&& converter );
            CU2U8&   operator=( CU2U8&& converter );
#endif

#ifdef QSTRING_H
            CU2U8( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU2U8( const CStringW& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const wchar_t* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CA2U8
        {
        public:
            CA2U8( const char* szText );
            CA2U8( const std::string& strText );
            CA2U8( const CA2U8& converter );
            ~CA2U8();

            CA2U8&   operator=( const char* szText );
            CA2U8&   operator=( const std::string& strText );
            CA2U8&   operator=( const CA2U8& converter );

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*            c_str() const { return this->operator const char *(); }
            const std::string      toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CA2U8( CA2U8&& converter );
            CA2U8&   operator=( CA2U8&& converter );
#endif

#ifdef QSTRING_H
            CA2U8( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CA2U8( const CStringA& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            char*                   _szText;
        };

        //////////////////////////////////////////////////////////////////////////

        class CU82A
        {
        public:
            CU82A( const char* szText );
            CU82A( const std::string& strText );
            CU82A( const CU82A& converter );
            ~CU82A();

            CU82A&   operator=( const char* szText );
            CU82A&   operator=( const std::string& strText );
            CU82A&   operator=( const CU82A& converter );

            operator const char*() const { return _szText != NULLPTR ? _szText : "(NULL)"; };
            const char*            c_str() const { return this->operator const char *(); }
            const std::string      toStdString() { return _szText != NULLPTR ? _szText : "(NULL)"; };

#if _MSC_VER >= 1600
            CU82A( CU82A&& converter );
            CU82A&   operator=( CU82A&& converter );
#endif

#ifdef QSTRING_H
            CU82A( const QString& strText );
            const QString           toQtString() { return QString::fromUtf8( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif

#ifdef _AFX
            CU82A( const CStringA& strText );
            const CStringA          toMFCString() { return CStringA( _szText != NULLPTR ? _szText : "(NULL)" ); };
#endif
        private:
            size_t                  convert( const char* szText );

            char*                   _szText;
        };

    }; // namespace nsCmnConvert

}; // namespace nsCmnCommon

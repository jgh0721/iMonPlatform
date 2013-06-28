#pragma once

/*!
    Type-Safe Formatter

    코드 및 구조 참조 : http://code.google.com/p/stringf/

    제작 : DevTester, http://jgh0721.homeip.net

    형식 문법

    탈출 문자열
        \t
        \n
        \\
        \r
        \n
        \"
        \'

    위치 지정자
        %0                                  특수 문법, 추가 인자를 받지 않고 데이터를 출력하는 문법
        %0{TID}                             => tsFormat 를 호출한 스레드의 ID 를 출력
        %0{PID}                             => tsFormat 를 호출한 프로세스의 ID 를 출력

        %0{FILE}                            => tsLogFormat 함수를 사용하여 prFileName 에 넘긴 문자열을 출력
        %0{FUNC}                            => tsLogFormat 함수를 사용하여 prFuncName 에 넘긴 문자열을 출력
        %0{LINE}                            => tsLogFormat 함수를 사용하여 lineNumber 에 넘긴 문자열을 출력

        %0{DATE}                            => tsFormat 를 호출했을 때의 time_t 를 내부적으로 사용하여 날짜 출력
        %0{TIME}                            => tsFormat 를 호출한을 때의 time_t 를 내부적으로 사용하여 시간 출력

        %0{LOGGER}                          => 할당된 로거 이름 출력
        %0{LOGLEVEL}                        => 출력하는 로그의 현재 등급을 출력

        %1 ~ %20        : 인자의 위치를 지정함

        예). 
                        %1                  => 1번째 인자를 문자열 형태로 출력함
                        %1{U}               => 인자를 대문자, 소문자로 모두 표현할 수 있을 때 대문자로 표현하도록 함, 기본값 = 소문자
                                            => bool 값, 정수의 16진수 표현, 문자 값(printf 의 %c) 등
                        %1{+}               => 인자가 숫자일 때 앞에 항상 부호를 표시합니다. 
                        %1{DT}              => time_t 형식의 숫자를 받아서 날짜, 시간 문자열로 출력, time_t 형식은 시스템에 따라 32비트 또는 64비트 부호있는 정수로 정의되어 있으며 tsFormat_Integer 에서 서식화함
                                            => DT 다음에 : %Y-%m-%d 등의 strftime 형식의 문자열로 출력하거나 생략하면 기본값을 사용해 출력됨
                        %1{DT:%Y-%m-%d}     => time_t 형식의 숫자 입력

                        %1{ERR}             => 인자를 GetLastError 반환 값으로 인식한 후 해당 오류에 대한 설명 문자열을 출력
                        %1{HERR}            => 인자를 HRESULT 값으로 인식한 후 해당 오류에 대한 설명 문자열을 출력

        %%              : % 문자 출력


    기본 지원하는 형식
    bool                        : true, false 문자열로 출력
    int, unsigned int           : 정수, 문자열로 출력
    long long, unsigned long long   : 64 비트 정수, 문자열로 출력
    float, double               : float, double 부동소수점 출력
    char*, wchar_t*             : 문자열 출력, 반환받는 std::string, std::wstring 과 인코딩이 다르면 자동 변환
    std::string, std::wstring   : 문자열 출력, 반환받는 std::string, std::wstring 과 인코딩이 다르면 자동 변환
    CStringA, CStringW          : 문자열 출력, 반환받는 std::string, std::wstring 과 인코딩이 다르면 자동 변환
    QString                     : 문자열 출력,

    ※ UTF-8 문자열은 인자로 받거나 출력할 수 없으므로, 반드시 ANSI, 또는 UTF-16 형태의 문자열로 변환해서 넣을 것!

    함수 호출 그래프
    사용자는 tsFormat, tsFormatBuffer 를 호출
        tsFormat, tsFormatBuffer 는 tsFormatInternal 호출
        tsFormatInternal 는 doFormat 호출
        doFormat 는 tsParse 호출하여 형식 문자열을 분석 후 boost::apply_visitor 를 호출하여 각 형식에 맞는 formatValue 가 호출되도록 한다. 

    TODO :
        실수형에 대한 서식화 완성
        알 수 없는 형식에 대한 지원 추가

    History :
        2012-12-10
            %0 특수문법 TID, PID 지원 추가
            출력 형식 ERR 지원 추가
            부호없는 정수에 대해 문자열로 변환이 이루어지지 않던 문제 수정
            float, double 형에 대한 간단한 문자열 출력 지원
            알 수 없는 형식에 대하여 "Unknown Type" 이라는 문자열이 찍히도록 추가함
            tsFormatBuffer 기본 구현 완성
        2013-01-07
            로거등에서 활용할 수 있도록 개선
        2013-01-14
            로거 이름을 출력할 수 있도록 추가
        2013-01-22
            로거 스펙을 fmtSpec 으로 통합
            LOG LEVEL 를 출력할 수 있도록 추가
        2013-01-30
            cmnConverter 네임스페이스를 클래스들을 인자로 넘겼을 때 자동으로 변환되어 "Unknown Type" 이라는 문자열이 출력되지 않도록 함. 
        2013-05-03
            HRESULT 반환 값에 대한 설명 문자열 생성 추가
*/

#include <string>
#include <crtdefs.h>

#include <stdexcept>
#include <iosfwd>
#include <iomanip>
#include <sstream>
#include <locale>
#include <algorithm>
#include <iterator>
#include <xutility>
#include <utility>

#define FORMAT_MAX_ARGS                 20
#ifdef BOOST_MPL_LIMIT_VECTOR_SIZE
#undef BOOST_MPL_LIMIT_VECTOR_SIZE
#define BOOST_MPL_LIMIT_VECTOR_SIZE                 FORMAT_MAX_ARGS
#endif

#ifdef BOOST_MPL_LIMIT_LIST_SIZE
#undef BOOST_MPL_LIMIT_LIST_SIZE
#define BOOST_MPL_LIMIT_LIST_SIZE                   FORMAT_MAX_ARGS
#endif

#include <boost/variant.hpp>
#include <boost/type_traits.hpp>
#include <boost/call_traits.hpp>
#include <boost/utility/enable_if.hpp>
#include <boost/integer.hpp>
#include <boost/concept_check.hpp>
#include <boost/type_traits/is_same.hpp>
#include <boost/tuple/tuple.hpp>

#include <boost/mpl/bool.hpp>
#include <boost/mpl/inserter.hpp>
#include <boost/mpl/sort.hpp>
#include <boost/mpl/unique.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/front_inserter.hpp>

#include <boost/preprocessor.hpp>

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtTypes
        {
            const int                   DEFAULT_BUFFER_SIZE             = 256;

            const char* const           DEFAULT_DATE_FORMAT_A           = "%Y-%m-%d";
            const char* const           DEFAULT_TIME_FORMAT_A           = "%H:%M:%S";
            const char* const           DEFAULT_DATE_TIME_FORMAT_A      = "%Y-%m-%d %H:%M:%S";

            const wchar_t* const        DEFAULT_DATE_FORMAT_W           = L"%Y-%m-%d";
            const wchar_t* const        DEFAULT_TIME_FORMAT_W           = L"%H:%M:%S";
            const wchar_t* const        DEFAULT_DATE_TIME_FORMAT_W      = L"%Y-%m-%d %H:%M:%S";

            // FileNameA, FileNameW, FuncNameA, FuncNameW, LoggerNameA, LoggerNameW, LogLevelA, LogLevelW, lineNumber
            typedef boost::tuple< const char*, const wchar_t*, const char*, const wchar_t*, const char*, const wchar_t*, const char*, const wchar_t*, unsigned int >       TyTpLoggerType;

        };  // namespace nsCmnFmtTypes

        namespace nsCmnFmtDetail
        {
            struct formatSpec
            {
                long                                    argIndex;

                union
                {
                    struct
                    {
                        unsigned short                  upperLetter:1;      // 대문자 출력
                        unsigned short                  plus:1;             // 숫자 부호 출력
                        unsigned short                  datetime:1;         // 날짜,시간 출력
                        unsigned short                  errorText:1;        // GetLastError 값, 문자열로 변환 출력 필요
                        unsigned short                  errorHRESULT:1;     // HRESULT 값, 문자열로 변환 출력 필요

                        unsigned short                  loggerFuncName:1;   // 로그, 함수 이름
                        unsigned short                  loggerFileName:1;   // 로그, 파일 이름
                        unsigned short                  loggerLineNumber:1; // 로그, 줄 번호 
                        unsigned short                  loggerName:1;       // 로거 이름
                        unsigned short                  loggerLogLevel:1;   // 로그, 레벨
                    };

                    unsigned short                      flags;
                };

                formatSpec() : argIndex(0), flags(0)
                {};
            };

        }; // namespace nsCmnFmtDetail

    }; // namespace nsCmnFormatter

}; // namespace nsCommon

#include "cmnFormatter_Utils.hpp"
#include "cmnFormatter_Parse.hpp"
#include "cmnFormatter_Renderder.hpp"
#include "cmnFormatter_Format.hpp"

#include "cmnFormatter_Integer.hpp"
#include "cmnFormatter_Float.hpp"
#include "cmnFormatter_String.hpp"
#include "cmnFormatter_Converter.hpp"

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct logger_type_tag
            { };

            template<>
            struct type_tag< nsCmnFmtTypes::TyTpLoggerType >
            {
                typedef logger_type_tag                type;
            };

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, logger_type_tag, 
                typename boost::enable_if< boost::is_same< typename Renderer::charType, char > >::type * = 0 )
            {
                // FileNameA, FileNameW, FuncNameA, FuncNameW, LoggerNameA, LoggerNameW, LogLevelA, LogLevelW, lineNumber
                if( fmtSpec.loggerFileName ) 
                    return renderer.render( tpLoggerType.get<0>(), -1 );
                else if( fmtSpec.loggerFuncName )
                    return renderer.render( tpLoggerType.get<2>(), -1 );
                else if( fmtSpec.loggerName )
                    return renderer.render( tpLoggerType.get<4>(), -1 );
                else if( fmtSpec.loggerLogLevel )
                    return renderer.render( tpLoggerType.get<6>(), -1 );
                else if( fmtSpec.loggerLineNumber )
                {
                    memset( fmtBuffer, '\0', sizeof( char ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    size_t numberCount = formatNumber< Renderer, unsigned int >( fmtBuffer, fmtSpec, tpLoggerType.get<8>() );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }

                return renderer.render( "Unknown Logger Type", -1 );
            }

            template< typename Renderer >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, logger_type_tag, 
                typename boost::enable_if< boost::is_same< typename Renderer::charType, wchar_t > >::type * = 0 )
            {
                // FileNameA, FileNameW, FuncNameA, FuncNameW, LoggerNameA, LoggerNameW, LogLevelA, LogLevelW, lineNumber
                if( fmtSpec.loggerFileName ) 
                    return renderer.render( tpLoggerType.get<1>(), -1 );
                else if( fmtSpec.loggerFuncName )
                    return renderer.render( tpLoggerType.get<3>(), -1 );
                else if( fmtSpec.loggerName )
                    return renderer.render( tpLoggerType.get<5>(), -1 );
                else if( fmtSpec.loggerLogLevel )
                    return renderer.render( tpLoggerType.get<7>(), -1 );
                else if( fmtSpec.loggerLineNumber )
                {
                    memset( fmtBuffer, '\0', sizeof( wchar_t ) * nsCmnFmtTypes::DEFAULT_BUFFER_SIZE );
                    size_t numberCount = formatNumber< Renderer, unsigned int >( fmtBuffer, fmtSpec, tpLoggerType.get<8>() );
                    return renderer.render( &fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE - numberCount - 1 ], -1 );
                }

                return renderer.render( "Unknown Logger Type", -1 );
            }

            template< typename Renderer, typename T >
            size_t formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const T& value, unknown_type_tag )
            {
                return renderer.render( "Unknown Type", -1 );
            }
        }; // namespace nsCmnFmtDetail

    }; // namespace nsCmnFormatter

}; // namespace nsCommon

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {

#define FORMAT_EXPAND(...)          __VA_ARGS__
#define FORMAT_TYPE(z, n, unused)   BOOST_PP_COMMA_IF(n) typename boost::call_traits<T ## n>::param_type
#define FORMAT_DETAIL_FUNC(z, n, unused)    \
            template< typename Dest BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
            size_t format_internal( nsCmnFmtDetail::clsRenderer<Dest>& dest, const typename nsCmnFmtDetail::clsRenderer<Dest>::charType* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, T, arg ) ) \
            {                               \
                using namespace ::boost::mpl;   \
                typename nsCmnFmtDetail::clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,}; \
                BOOST_PP_EXPAND( FORMAT_EXPAND BOOST_PP_IF( n, (typedef vector ## n<BOOST_PP_REPEAT_ ## z(n, FORMAT_TYPE, 0)> template_types), (typedef vector1<int> template_types ) ) ); \
                typedef typename boost::mpl::sort< template_types, type_less, boost::mpl::front_inserter< vector0<> > >::type sorted_types;                                               \
                typedef typename boost::mpl::unique< sorted_types, boost::is_same< boost::mpl::_1, boost::mpl::_2 >, boost::mpl::front_inserter< boost::mpl::vector0<> > >::type unique_types;                                              \
                typedef boost::make_variant_over< typename unique_types::type > variant;                                                                                                                            \
                typename variant::type args[n] = { BOOST_PP_ENUM_PARAMS_Z(z, n, arg) };                                                                                                                             \
                return doFormat( dest, format, fmtBuffer, args, n ); \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_DETAIL_FUNC, 0 )

#undef FORMAT_DETAIL_FUNC

#define FORMAT_DETAIL_FUNC(z, n, unused)    \
            template< typename Dest BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) >     \
            size_t format_internal_logger( nsCmnFmtDetail::clsRenderer<Dest>& dest, const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, const typename nsCmnFmtDetail::clsRenderer<Dest>::charType* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, T, arg ) )  \
            {                               \
                using namespace ::boost::mpl;   \
                typename nsCmnFmtDetail::clsRenderer<Dest>::charType fmtBuffer[ nsCmnFmtTypes::DEFAULT_BUFFER_SIZE ] = {0,}; \
                BOOST_PP_EXPAND( FORMAT_EXPAND BOOST_PP_IF( n, (typedef boost::mpl::vector<BOOST_PP_REPEAT_ ## z(n,FORMAT_TYPE, 0), typename boost::call_traits< nsCmnFmtTypes::TyTpLoggerType >::param_type, typename boost::call_traits< const unsigned int >::param_type > template_types), (typedef vector1<int> template_types ) ) ); \
                typedef typename boost::mpl::sort< template_types, type_less, boost::mpl::front_inserter< vector0<> > >::type sorted_types;                                               \
                typedef typename boost::mpl::unique< sorted_types, boost::is_same< boost::mpl::_1, boost::mpl::_2 >, boost::mpl::front_inserter< boost::mpl::vector0<> > >::type unique_types;                                              \
                typedef boost::make_variant_over< typename unique_types::type > variant;                                                                                                                            \
                typename variant::type args[ n + 1 ] = { BOOST_PP_ENUM_PARAMS_Z(z, n, arg), tpLoggerType };   \
                return doFormat( dest, format, fmtBuffer, args, n + 1 ); \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_DETAIL_FUNC, 0 )

#undef FORMAT_EXPAND
#undef FORMAT_TYPE
#undef FORMAT_DETAIL_FUNC

        }; // namespace nsCmnFmtDetail

        // 외부에서 사용할 함수들 선언
#define FORMAT_FUNCS(z, n, unused) \
        template< typename Dest, typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( Dest& dest, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer<Dest> out( dest );  \
            return format_internal( out, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) ); \
        }   \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( CharT*& buffer, size_t& bufferSize, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer< CharT* > out( buffer, bufferSize );              \
            return format_internal( out, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) ); \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        std::basic_string< CharT > tsFormat( const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg) ) \
        {                       \
            std::basic_string< CharT > s;    \
            tsFormatBuffer( s, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            return s;           \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t tsFormatBuffer( const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, CharT*& buffer, size_t& bufferSize, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ) ) \
        {                       \
            nsCmnFmtDetail::clsRenderer< CharT* > out( buffer, bufferSize );             \
            return format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
        }                       \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        std::basic_string< CharT > tsFormat( const nsCmnFmtTypes::TyTpLoggerType& tpLoggerType, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg) ) \
        {                       \
            std::basic_string< CharT > s;    \
            nsCmnFmtDetail::clsRenderer< std::basic_string< CharT > > out(s);                                       \
            format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            return s;           \
        }                       

        BOOST_PP_REPEAT( BOOST_PP_ADD( FORMAT_MAX_ARGS, 1 ), FORMAT_FUNCS, 0 )

#undef FORMAT_FUNCS

    }; // namespace nsCmnFormatter

}; // namespace nsCommon

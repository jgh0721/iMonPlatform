#pragma once

/*!
    Type-Safe Formatter

    �ڵ� �� ���� ���� : http://code.google.com/p/stringf/

    ���� : DevTester, http://jgh0721.homeip.net

    ���� ����

    Ż�� ���ڿ�
        \t
        \n
        \\
        \r
        \n
        \"
        \'

    ��ġ ������
        %0                                  Ư�� ����, �߰� ���ڸ� ���� �ʰ� �����͸� ����ϴ� ����
        %0{TID}                             => tsFormat �� ȣ���� �������� ID �� ���
        %0{PID}                             => tsFormat �� ȣ���� ���μ����� ID �� ���

        %0{FILE}                            => tsLogFormat �Լ��� ����Ͽ� prFileName �� �ѱ� ���ڿ��� ���
        %0{FUNC}                            => tsLogFormat �Լ��� ����Ͽ� prFuncName �� �ѱ� ���ڿ��� ���
        %0{LINE}                            => tsLogFormat �Լ��� ����Ͽ� lineNumber �� �ѱ� ���ڿ��� ���

        %0{DATE}                            => tsFormat �� ȣ������ ���� time_t �� ���������� ����Ͽ� ��¥ ���
        %0{TIME}                            => tsFormat �� ȣ������ ���� time_t �� ���������� ����Ͽ� �ð� ���

        %0{LOGGER}                          => �Ҵ�� �ΰ� �̸� ���
        %0{LOGLEVEL}                        => ����ϴ� �α��� ���� ����� ���

        %1 ~ %20        : ������ ��ġ�� ������

        ��). 
                        %1                  => 1��° ���ڸ� ���ڿ� ���·� �����
                        %1{U}               => ���ڸ� �빮��, �ҹ��ڷ� ��� ǥ���� �� ���� �� �빮�ڷ� ǥ���ϵ��� ��, �⺻�� = �ҹ���
                                            => bool ��, ������ 16���� ǥ��, ���� ��(printf �� %c) ��
                        %1{+}               => ���ڰ� ������ �� �տ� �׻� ��ȣ�� ǥ���մϴ�. 
                        %1{DT}              => time_t ������ ���ڸ� �޾Ƽ� ��¥, �ð� ���ڿ��� ���, time_t ������ �ý��ۿ� ���� 32��Ʈ �Ǵ� 64��Ʈ ��ȣ�ִ� ������ ���ǵǾ� ������ tsFormat_Integer ���� ����ȭ��
                                            => DT ������ : %Y-%m-%d ���� strftime ������ ���ڿ��� ����ϰų� �����ϸ� �⺻���� ����� ��µ�
                        %1{DT:%Y-%m-%d}     => time_t ������ ���� �Է�

                        %1{ERR}             => ���ڸ� GetLastError ��ȯ ������ �ν��� �� �ش� ������ ���� ���� ���ڿ��� ���
                        %1{HERR}            => ���ڸ� HRESULT ������ �ν��� �� �ش� ������ ���� ���� ���ڿ��� ���

        %%              : % ���� ���


    �⺻ �����ϴ� ����
    bool                        : true, false ���ڿ��� ���
    int, unsigned int           : ����, ���ڿ��� ���
    long long, unsigned long long   : 64 ��Ʈ ����, ���ڿ��� ���
    float, double               : float, double �ε��Ҽ��� ���
    char*, wchar_t*             : ���ڿ� ���, ��ȯ�޴� std::string, std::wstring �� ���ڵ��� �ٸ��� �ڵ� ��ȯ
    std::string, std::wstring   : ���ڿ� ���, ��ȯ�޴� std::string, std::wstring �� ���ڵ��� �ٸ��� �ڵ� ��ȯ
    CStringA, CStringW          : ���ڿ� ���, ��ȯ�޴� std::string, std::wstring �� ���ڵ��� �ٸ��� �ڵ� ��ȯ
    QString                     : ���ڿ� ���,

    �� UTF-8 ���ڿ��� ���ڷ� �ްų� ����� �� �����Ƿ�, �ݵ�� ANSI, �Ǵ� UTF-16 ������ ���ڿ��� ��ȯ�ؼ� ���� ��!

    �Լ� ȣ�� �׷���
    ����ڴ� tsFormat, tsFormatBuffer �� ȣ��
        tsFormat, tsFormatBuffer �� tsFormatInternal ȣ��
        tsFormatInternal �� doFormat ȣ��
        doFormat �� tsParse ȣ���Ͽ� ���� ���ڿ��� �м� �� boost::apply_visitor �� ȣ���Ͽ� �� ���Ŀ� �´� formatValue �� ȣ��ǵ��� �Ѵ�. 

    TODO :
        �Ǽ����� ���� ����ȭ �ϼ�
        �� �� ���� ���Ŀ� ���� ���� �߰�

    History :
        2012-12-10
            %0 Ư������ TID, PID ���� �߰�
            ��� ���� ERR ���� �߰�
            ��ȣ���� ������ ���� ���ڿ��� ��ȯ�� �̷������ �ʴ� ���� ����
            float, double ���� ���� ������ ���ڿ� ��� ����
            �� �� ���� ���Ŀ� ���Ͽ� "Unknown Type" �̶�� ���ڿ��� �������� �߰���
            tsFormatBuffer �⺻ ���� �ϼ�
        2013-01-07
            �ΰŵ�� Ȱ���� �� �ֵ��� ����
        2013-01-14
            �ΰ� �̸��� ����� �� �ֵ��� �߰�
        2013-01-22
            �ΰ� ������ fmtSpec ���� ����
            LOG LEVEL �� ����� �� �ֵ��� �߰�
        2013-01-30
            cmnConverter ���ӽ����̽��� Ŭ�������� ���ڷ� �Ѱ��� �� �ڵ����� ��ȯ�Ǿ� "Unknown Type" �̶�� ���ڿ��� ��µ��� �ʵ��� ��. 
        2013-05-03
            HRESULT ��ȯ ���� ���� ���� ���ڿ� ���� �߰�
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
                        unsigned short                  upperLetter:1;      // �빮�� ���
                        unsigned short                  plus:1;             // ���� ��ȣ ���
                        unsigned short                  datetime:1;         // ��¥,�ð� ���
                        unsigned short                  errorText:1;        // GetLastError ��, ���ڿ��� ��ȯ ��� �ʿ�
                        unsigned short                  errorHRESULT:1;     // HRESULT ��, ���ڿ��� ��ȯ ��� �ʿ�

                        unsigned short                  loggerFuncName:1;   // �α�, �Լ� �̸�
                        unsigned short                  loggerFileName:1;   // �α�, ���� �̸�
                        unsigned short                  loggerLineNumber:1; // �α�, �� ��ȣ 
                        unsigned short                  loggerName:1;       // �ΰ� �̸�
                        unsigned short                  loggerLogLevel:1;   // �α�, ����
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

        // �ܺο��� ����� �Լ��� ����
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

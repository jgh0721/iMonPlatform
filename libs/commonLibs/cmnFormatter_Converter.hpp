#pragma once

#include "cmnConverter.h"

namespace nsCommon
{
    namespace nsCmnFormatter
    {
        namespace nsCmnFmtDetail
        {
            struct encoding_converter_type_tag
            { };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CA2U >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU2A >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU82U >
            {
                typedef encoding_converter_type_tag             type;
            };

            template<>
            struct type_tag< nsCommon::nsCmnConvert::CU82A >
            {
                typedef encoding_converter_type_tag             type;
            };

            template< typename Renderer, typename CharT >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, CharT const& value, encoding_converter_type_tag )
            {
                return renderer.render( value.c_str(), -1 );
            }

            // �⺻������ �������� �����ʹ� UTF-8 ���ڿ��� �������� �ʱ� ������ ANSI �Ǵ� UNICODE ���·� ��ȯ���Ѽ� ����Ѵ�.
            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CA2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                return renderer.render( nsCommon::nsCmnConvert::CU82A( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CA2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                return renderer.render( nsCommon::nsCmnConvert::CU82U( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CU2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, char >::value >::type * = 0 )
            {
                return renderer.render( nsCommon::nsCmnConvert::CU82A( value.c_str() ).c_str(), -1 );
            }

            template< typename Renderer >
            long formatValue( Renderer& renderer, const formatSpec& fmtSpec, typename Renderer::charType* fmtBuffer, const nsCommon::nsCmnConvert::CU2U8& value,
                typename boost::enable_if_c< boost::is_same< typename Renderer::charType, wchar_t >::value >::type * = 0 )
            {
                return renderer.render( nsCommon::nsCmnConvert::CU82U( value.c_str() ).c_str(), -1 );
            }

        } // nsCmnFmtDetail
    } // nsCmnFormatter
} // nsCommon

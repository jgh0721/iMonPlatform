#pragma once

#include <map>
#include <string>
#include <utility>
#include <vector>

#include "cmnUtils.h"
#include "cmnFormatter.hpp"
#include "cmnWinUtils.h"

#include <boost/preprocessor.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>
#include <boost/tuple/tuple.hpp>
#include <boost/pool/pool.hpp>
#include <boost/pool/object_pool.hpp>
#include <boost/pool/singleton_pool.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/lockfree/queue.hpp>

/*!
    �α� ���̺귯��

    �ΰ� �̸� 
        �ΰ� �̸��� "/" �����ڸ� �̿��Ͽ� ���������� ������ �� ������ ��� �ΰŴ� "root" �̸��� ������ �ΰ��� ���� �ΰ��̴�. 
        ���� �ΰŴ� ���� �ΰ��� ������ �ڵ������� �°��Ͽ� ���󰣴�. 
        �� �ΰź��� ������ ������ �� �ִ�. 
        �ΰ� �̸��� ���̴� �ִ� 128 ���ڷ� ���ѵȴ�. 

    �ΰ� ȯ�漳��
        �ΰ��� ȯ���� InitLogger �� �Բ� �⺻���� �����Ǵ� "root" �� �������� ��� ���� �ΰſ� ��������� ����ȴ�. 
            ��). root/a/b/c �� ���� �� a/b �� �ΰŸ� �����ϸ� a/b/c �� �ΰ��� ȯ�漳���� �Բ� �����ȴ�. 

    �ΰ� ���� ���� ��Ģ
        ���/�����̸�_��¥[���ι�ȣ].Ȯ����

*/

 
/*!
        �α� ���ڿ��� PreFix ���ڿ��� Contents ���ڿ��� �� �κ����� ��������. 
        PreFix ���ڿ� : ��� �α� ���ڿ��� �տ� �ڵ����� �ٰ� �Ǵ� ���ڿ�
        Contents ���ڿ� : ����ڰ� ����Ϸ��� ���ڿ�
        
    ������ ����� :
        ���ø����̼��� ���� �����ϴ� �κп� InitLogger() �� �����Ͽ� �α� ���̺귯���� �ʱ�ȭ��

        ���ø����̼��� ���� �Ǵ� �κп� StopLogger() �� �����Ͽ� �α� ���̺귯���� ������

        �����Ǵ� �α� ���� ���� : �⺻��, �α� ����
            �α� ���� ���( isSplitByLogger = false ), ���\_defaultLogFileName_�α׻�����¥-[����].Ȯ����
            �α� �и� ���( isSplitByLogger = true ), ���\_defaultLogFileName_�ΰ��̸�_�α׻�����¥-[����].Ȯ����

    �߸��� ��� ��). 

        1. LM_TRACE((  ��¼�� ��¼��...  )) 
            => ����ó�� ��ȣ 2���� ��� ��ũ�θ� ����ϸ� �ȵ�. ���ù����� ������ ������ �߻���. 
        2. LM_TRACE( L"��¼�� ��¼��" )
            => �ݵ�� ó������ ����ȭ ���ڿ���, �ι�°���� ���ڵ��� �;���. 
                ==> LM_TRACE( L"%1", L"��¼�� ��¼��" ) �� ���� �����ʿ�
        3. LM_TRACE( L"%s", "fsdfsd" )
            => ����� �ΰŴ� %s ���� ���� C/C++ ����ȭ ������ �������� �����Ƿ� �ش� ��ũ�δ� "%s" ��� ���ڿ��� �α׷� ����ϰ� �ȴ�!!!
*/
 
/*!
    2013-05-27 
*/

namespace nsCommon
{
    namespace nsCmnLogger
    {
        enum CLOG_LEVEL
        {
            LL_NONE = 0,
            LL_CRITICAL,
            LL_ERROR,
            LL_WARNING,
            LL_DEBUG,
            LL_TRACE
        };

        enum CLOG_TRANSFER
        {
            LT_NONE = 0,
            LT_STDERR = 1,
            LT_STREAM = 2,
            LT_FILE = 4,
            LT_EVENTLOG = 8,
            LT_CONSOLE = 16,
            LT_DEBUGGER = 32,
            LT_CALLBACK = 64,
            LT_STDOUT = 128
        };

        const unsigned int MAX_LOGGERNAME_SIZE = 128;
        const std::string DEFAULT_ROOT_LOGGER = "root";

        typedef struct tagLoggerConfiguration
        {
			std::wstring		loggerNameW;
			std::string			loggerNameA;

            unsigned int        logTransferMethods;
            CLOG_LEVEL          logLevel;

            bool                isSynchronousOp;
            bool                isSplitWhenDate;
            unsigned int        fileSizeLimit;          // �α� ���� ũ�� ����, 0 = ������, MB ����

            std::wstring        fileName;
            std::wstring        filePath;
            std::wstring        fileExt;

            struct tm           currentFileDate;
            unsigned int        currentFileIndex;       // �ΰ� ���������� ���
            std::wstring        currentFullFileName;    // �ΰ� ���������� ���

            std::string         prefixLogA;             // ��� ��� ���ڿ� �� �κп� �׻� ��µǴ� ����
            std::wstring        prefixLogW;

            bool                isAppendNewline;

            unsigned int        maxLogFileDateCount;    // �ִ� ��ĥġ�� �α� ������ ������ ������ ����, �⺻�� 7��

            bool                isSplitByLogger;        // �� �ΰź��� ������ �α������� ������ �� ������, �� �ΰ� �̸����� ������ �α׸� �����ϵ��� �ϸ� �α� ���� �̸��� �ΰ� �̸��� �߰��� ��ϵ�

            // �α��� ��ġ ���
            bool                isUseBatchFileWrite;
            unsigned int        nBatchFileWriteIntervalsSec;

            tagLoggerConfiguration() : 
                logTransferMethods( LT_DEBUGGER ), logLevel( LL_DEBUG ),
                isSynchronousOp( false ), isSplitWhenDate( true ), fileSizeLimit(0),
                currentFileIndex(0),
                isAppendNewline(true),
                maxLogFileDateCount(7), isSplitByLogger(false),
                isUseBatchFileWrite(false), nBatchFileWriteIntervalsSec(10)
            {};

        } LoggerConfiguration;

        typedef struct tagLogContext
        {
			CLOG_LEVEL		logLevel;
            char*           logDataA;
            wchar_t*        logDataW;
            size_t			logDataSizeA;
            size_t			logDataSizeW;
            bool            isUnicodeData;

            /* 
                LoggerConfiguration �� ���� �����ʹ� CLogger Ŭ�������� �����ϰ� �̰��� ��ũ�� �ɸ���. 
                ���� �̰����� �ش� �����ʹ� ���� �����ϸ� �ȵ�
            */
            LoggerConfiguration*
                            loggerConfiguration;

            tagLogContext() : logLevel( LL_NONE ), logDataA( NULLPTR ), logDataW( NULLPTR ), isUnicodeData(false), logDataSizeA(0), logDataSizeW(0) {};
            ~tagLogContext() 
            { 
                DeletePtrA< char* >( logDataA ); DeletePtrA< wchar_t* >( logDataW ); 
            };

        } LOG_CONTEXT, *PLOG_CONTEXT;

		typedef void (*pfnLogCallbackW)	( PVOID pParam, CLOG_LEVEL logLevel, const wchar_t* wszData );
		typedef void (*pfnLogCallbackA)	( PVOID pParam,	CLOG_LEVEL logLevel, const char* szData );
        
		typedef std::map< pfnLogCallbackA, PVOID >	       TyMapCallbackA;
		typedef std::map< pfnLogCallbackW, PVOID >	       TyMapCallbackW;

		typedef std::pair< TyMapCallbackA*, CRITICAL_SECTION* >	TyPrCallbackAToMutex;
		typedef std::pair< TyMapCallbackW*, CRITICAL_SECTION* >	TyPrCallbackWToMutex;

		typedef std::pair< FILE*, CRITICAL_SECTION* >       TyPrFileToMutex;

        class CLogFactory;

        class CLogUtil
        {
        public:
            static LOG_CONTEXT*             CheckOutLOGContext();
            static void                     CheckInLOGContext( LOG_CONTEXT* object );

			static TyPrCallbackAToMutex       GetCallback( const std::string& fileName );
			static TyPrCallbackWToMutex       GetCallback( const std::wstring& fileName );

			static TyPrCallbackAToMutex       MakeCallback( const std::string& fileName );
			static TyPrCallbackWToMutex       MakeCallback( const std::wstring& fileName );

            static bool                     IsExistLOGFile();
            static TyPrFileToMutex          GetFile( LoggerConfiguration* lc );
            static TyPrFileToMutex          GetFile( const std::wstring& fileName );
            static TyPrFileToMutex          MakeLOGFile( LoggerConfiguration* lc );

            static void                     WriteData( LOG_CONTEXT* logContext );
            static const char*              GetLoglevelTextA( CLOG_LEVEL logLevel );
            static const wchar_t*           GetLoglevelTextW( CLOG_LEVEL logLevel );
        private:
            static void                     CheckFileExist( LoggerConfiguration* lc );
            static bool                     CheckFileSizeAndDate( const wchar_t* wszFullFileName, LoggerConfiguration* lc );
            static void                     CheckOldFileExist( LoggerConfiguration* lc );

            friend class                    CLogFactory;
            static CRITICAL_SECTION                                         _csFile;
            static boost::unordered_map< std::wstring, TyPrFileToMutex >    _mapFileNameToDest;
            static CRoundRobinAllocator<LOG_CONTEXT>                        _rbAllocator;

			static CRITICAL_SECTION													_csCallback;
			static boost::unordered_map< std::string, TyPrCallbackAToMutex >		_mapFileNameToCallbackA;
			static boost::unordered_map< std::wstring, TyPrCallbackWToMutex >		_mapFileNameToCallbackW;
        };

        class CLogger
        {
        public:
            ~CLogger();

			bool				AddCallback( pfnLogCallbackA pfnCallback, PVOID pParam );
			bool				AddCallback( pfnLogCallbackW pfnCallback, PVOID pParam );
			void				RemoveCallback( pfnLogCallbackA pfnCallback );
			void				RemoveCallback( pfnLogCallbackW pfnCallback );

            void                SetLoggerConfiguration( const LoggerConfiguration& loggerConfiguration );
            LoggerConfiguration GetLoggerConfiguration() { return _loggerConfiguration; };

            void                AppendChildLogger( CLogger* logger )
            { assert( logger != NULLPTR ); _vecChilds.push_back( logger ); };

            bool                IsLogging( CLOG_LEVEL wantLevel ) { return true; };

            /*!
                prefix �����Ϳ� ���� ���ڿ��� ������ �� �Է��� ���ڿ��� ���� contents �� �����Ͽ� ���� �ѹ��� �����
            */
#define FORMAT_FUNCS(z, n, unused) \
            template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
            size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ), \
                    typename boost::enable_if_c< boost::is_same< CharT, char >::value >::type * = 0 ) \
            {                      \
                size_t lRet = 0;     \
                nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber ); \
                LOG_CONTEXT* pLC = retrieveLOGContext( false, &_loggerConfiguration ); \
				pLC->logLevel = currentLogLevel; \
                if( _loggerConfiguration.prefixLogA.empty() == false ) \
                { \
                    nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA );    \
                    lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, _loggerConfiguration.prefixLogA.c_str() BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
                } \
                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataA, pLC->logDataSizeA, pLC->logDataSizeA - lRet );    \
                lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
                appendNewLineToLOGDATA( pLC, lRet, false ); \
                writeData( pLC ); \
                return lRet;        \
            }

            BOOST_PP_REPEAT( BOOST_PP_ADD( 15, 1 ), FORMAT_FUNCS, 0 )

#undef FORMAT_FUNCS

#define FORMAT_FUNCS(z, n, unused) \
        template< typename CharT BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, typename T) > \
        size_t Log( CLOG_LEVEL currentLogLevel, const std::pair< const char*, const wchar_t* >& prFileName, const std::pair< const char*, const wchar_t* >& prFuncName, unsigned int lineNumber, const CharT* format BOOST_PP_ENUM_TRAILING_BINARY_PARAMS_Z(z, n, const T, & arg ), \
        typename boost::enable_if_c< boost::is_same< CharT, wchar_t >::value >::type * = 0 ) \
        {                      \
            size_t lRet = 0;     \
            nsCommon::nsCmnFormatter::nsCmnFmtTypes::TyTpLoggerType tpLoggerType( prFileName.first, prFileName.second, prFuncName.first, prFuncName.second, _loggerNameA, _loggerNameW, CLogUtil::GetLoglevelTextA(currentLogLevel), CLogUtil::GetLoglevelTextW(currentLogLevel), lineNumber ); \
            LOG_CONTEXT* pLC = retrieveLOGContext( true, &_loggerConfiguration ); \
			pLC->logLevel = currentLogLevel; \
            if( _loggerConfiguration.prefixLogW.empty() == false ) \
            { \
                nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW );    \
                lRet = nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, _loggerConfiguration.prefixLogW.c_str() BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            } \
            nsCommon::nsCmnFormatter::nsCmnFmtDetail::clsRenderer< CharT* > out( pLC->logDataW, pLC->logDataSizeW, pLC->logDataSizeW - lRet );    \
            lRet += nsCommon::nsCmnFormatter::nsCmnFmtDetail::format_internal_logger( out, tpLoggerType, format BOOST_PP_ENUM_TRAILING_PARAMS_Z(z, n, arg) );     \
            appendNewLineToLOGDATA( pLC, lRet, true ); \
            writeData( pLC ); \
            return lRet;        \
        }

        BOOST_PP_REPEAT( BOOST_PP_ADD( 15, 1 ), FORMAT_FUNCS, 0 )

#undef FORMAT_FUNCS
        private:
            CLogger( const char* loggerNameA, const wchar_t* loggerNameW );
            CLogger( const char* loggerNameA, const wchar_t* loggerNameW, const LoggerConfiguration& loggerConfiguration );
            LOG_CONTEXT*        retrieveLOGContext( bool isUnicodeData, LoggerConfiguration* lc );
            void                appendNewLineToLOGDATA( LOG_CONTEXT* lc, size_t& dataSize, bool isUnicodeData );
            void                writeData( LOG_CONTEXT* lc );

            friend class        CLogFactory;

            char                _loggerNameA[ MAX_LOGGERNAME_SIZE ];
            wchar_t             _loggerNameW[ MAX_LOGGERNAME_SIZE ];

            std::vector< CLogger* > _vecChilds;
            LoggerConfiguration _loggerConfiguration;
        };

        class CLogFactory
        {
        public:
            // nBatchWriteLOGIntervalsSec = 0 �̸� ť���� �����͸� ������ ��� ���, �׿ܴ� �ش� �ʸ��� ���
            static void InitLogger( const LoggerConfiguration& loggerConfiguration );
            static void StopLogger();

            static CLogger&             GetLogger( const std::string& loggerName = DEFAULT_ROOT_LOGGER );
            static void                 PushLOG_CONTEXT( LOG_CONTEXT* logContext )
            {   _queue.Push( logContext ); };

            static volatile bool                                    _isInit;
        private:
            friend class                                            CLogUtil;
            static void                 workerThread();
            static void                 workerFileThread( unsigned int nBatchWriteLOGIntervalsSec );
            static volatile bool                                    _isExit;
            static CRITICAL_SECTION                                 _csFactory;
            static LoggerConfiguration                              _defaultConfiguration;
            static boost::unordered_map< std::string, CLogger* >    _mapNameToLogger;
            static boost::thread                                    _workerThread;
            static boost::thread                                    _workerFileThread;
            static CSharedQueue< LOG_CONTEXT* >                     _queue;
            static boost::unordered_map< TyPrFileToMutex, CSharedQueue< std::wstring >* >
                                                                    _fileWriteQueue;
        };

#define LM_INSTANCE nsCommon::nsCmnLogger::CLogFactory

#define LM_TRACE( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_TRACE ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_TRACE, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_TRACE_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_TRACE ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_TRACE, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_DEBUG( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_DEBUG ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_DEBUG, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_DEBUG_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_DEBUG ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_DEBUG, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_ERROR( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_ERROR ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_ERROR, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_ERROR_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_ERROR ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_ERROR, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_WARNING( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_WARNING ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_WARNING, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_WARNING_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_WARNING ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_WARNING, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_CRITICAL( format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( nsCommon::nsCmnLogger::DEFAULT_ROOT_LOGGER ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_CRITICAL ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_CRITICAL, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)

#define LM_CRITICAL_TO( loggerName, format, ...) \
    do { \
        if( nsCommon::nsCmnLogger::CLogFactory::_isInit == false ) break; \
        nsCommon::nsCmnLogger::CLogger& logger = LM_INSTANCE::GetLogger( loggerName ); \
        if( logger.IsLogging( nsCommon::nsCmnLogger::LL_CRITICAL ) == false ) \
            break;  \
        logger.Log( nsCommon::nsCmnLogger::LL_CRITICAL, std::make_pair( __FILE__, __FILEW__ ), std::make_pair( __FUNCTION__, __FUNCTIONW__ ), __LINE__, format, __VA_ARGS__ );   \
    } while(0)


    } // namespace nsCmnLogger

} // namespace nsCommon


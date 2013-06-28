#ifndef CMNPATHS_H
#define CMNPATHS_H

#include <string>

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>

namespace nsCommon
{
    namespace nsCmnPath
    {
        std::wstring GetCurrentPath();
        std::wstring CanonicalizePath( const std::wstring& strPath );

        // 경로명 + 파일명 에서 경로명 부분만 추출합니다.
        std::wstring GetFilePath( const std::wstring& filePath, bool isAppendSep = true );

    } // namespace nsCmnPath

} // namespace nsCommon

#endif // CMNPATHS_H

#pragma once

#include <QtCore>
#include <string>

/*!
    서버 고유 설정


*/

/*!
    프로세스 정의

    파일 등록
        => 어드민에서 파일을 선택, SHA256 해시 추출
        => 서버로 전송
        => 파일이름을 GUID 로 매핑시킴, 생성할 토렌토 파일이름으로 사용할 GUID 생성
        => 생성한 GUID 로 폴더 생성 후 이곳에 어드민이 전송한 파일 옮긴 후 TORRENT 생성

*/


/*!
    서버 테이블 설정

    TB_FILE

        GUID                TEXT
        ORIGINAL_NAME       TEXT

    TB_OS
        
        GUID                TEXT PRIMARY KEY

    TB_CLIENT               // iMonPlatform 의 자체 업그레이드를 위함

        GUID                TEXT PRIMARY KEY

        VERSION             TEXT
*/
class CSQLComputer
{
public:
    __int64     id;

    QString     uniqueid;
    QString     hw_cpu_name;
    QString     hw_board_name;
    QString     hw_disk_sn;
    QString     sw_os_name;
    QString     etc_netbios_name;
    QString     network_ipv4;
    QString     network_mac;

    // 키=값| 형식으로 저장
    QString     hw_extra_data;
    QString     sw_extra_data;
};

QX_REGISTER_PRIMARY_KEY( CSQLComputer, __int64 );
QX_REGISTER_HPP( CSQLComputer, qx::trait::no_base_class_defined, 1 );

class CSQLFile
{
public:
    QString     id;
};


/*!
    서버 설정
        서버 옵션
            서버 서비스가 동작하는데 필요한 값
            SQLite 를 통해 로컬 DB 에 저장된다. 
        관리 정책
            제품이 동작하는데 필요한 정책
            SQLite 를 통해 로컹 DB 에 저장된다. 


    TB_SETTINGS
    TB_POLICY
*/
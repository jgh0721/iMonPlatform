#pragma once

#include <QtCore>
#include <string>

/*!
    ���� ���� ����


*/

/*!
    ���μ��� ����

    ���� ���
        => ���ο��� ������ ����, SHA256 �ؽ� ����
        => ������ ����
        => �����̸��� GUID �� ���ν�Ŵ, ������ �䷻�� �����̸����� ����� GUID ����
        => ������ GUID �� ���� ���� �� �̰��� ������ ������ ���� �ű� �� TORRENT ����

*/


/*!
    ���� ���̺� ����

    TB_FILE

        GUID                TEXT
        ORIGINAL_NAME       TEXT

    TB_OS
        
        GUID                TEXT PRIMARY KEY

    TB_CLIENT               // iMonPlatform �� ��ü ���׷��̵带 ����

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

    // Ű=��| �������� ����
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
    ���� ����
        ���� �ɼ�
            ���� ���񽺰� �����ϴµ� �ʿ��� ��
            SQLite �� ���� ���� DB �� ����ȴ�. 
        ���� ��å
            ��ǰ�� �����ϴµ� �ʿ��� ��å
            SQLite �� ���� ���� DB �� ����ȴ�. 


    TB_SETTINGS
    TB_POLICY
*/
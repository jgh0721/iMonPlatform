#pragma once

/*!
    iMonPlatform �� Ŭ���̾�Ʈ�� �������� ���������� ���Ǵ� ����

    Ŭ���̾�Ʈ���� �����ϴ� ��ũ��Ʈ = QtLua ��ũ��Ʈ 

*/

#include <string>
#include <QtCore>

typedef enum tagDeployType
{
    DEPLOY_NORMAL, DEPLOY_RUN_SCRIPT, DEPLOY_SETUP_SW
} eDeployType;

typedef enum tagScriptType
{
    SCRIPT_BEFORE, SCRIPT_EXECUTE, SCRIPT_AFTER_NORMAL, SCRIPT_AFTER_ERROR
} eScriptType;

typedef QHash< eScriptType, QVector< QString > >        TyMapTypeWithScripts;

typedef struct tagDeployItem
{
    QString                     itemName;
    eDeployType                 itemType;
    TyMapTypeWithScripts        itemScripts;

} DEPLOY_ITEM, *PDEPLOY_ITEM;

/*!
    
*/
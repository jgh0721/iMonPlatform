#pragma once

/*!
    iMonPlatform 의 클라이언트와 서버에서 공통적으로 사용되는 구성

    클라이언트에서 수행하는 스크립트 = QtLua 스크립트 

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
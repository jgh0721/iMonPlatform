#pragma once

#pragma warning( disable: 4308 )

#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <WinSock2.h>
#pragma comment( lib, "ws2_32")

#include <QtCore>
#include <boost/serialization/export.hpp>

#define QX_NO_QT_GUI
#define _QX_SERIALIZE_BINARY_ENABLED
#define _QX_SERIALIZE_XML_ENABLED
#define _QX_SERIALIZE_TEXT_ENABLED
#define _QX_STATIC_BUILD

#include <QxOrm.h>
#include <QxMemLeak.h>

#ifdef _DEBUG
    #pragma comment( lib, "QxOrmd" )
#else
    #pragma comment( lib, "QxOrm" )
#endif

#pragma comment( lib, "oci" )
#pragma comment( lib, "libmysql" )


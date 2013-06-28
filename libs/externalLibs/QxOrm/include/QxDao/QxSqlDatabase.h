/****************************************************************************
**
** http://www.qxorm.com/
** Copyright (C) 2013 Lionel Marty (contact@qxorm.com)
**
** This file is part of the QxOrm library
**
** This software is provided 'as-is', without any express or implied
** warranty. In no event will the authors be held liable for any
** damages arising from the use of this software
**
** Commercial Usage
** Licensees holding valid commercial QxOrm licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Lionel Marty
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file 'license.gpl3.txt' included in the
** packaging of this file. Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met : http://www.gnu.org/copyleft/gpl.html
**
** If you are unsure which license is appropriate for your use, or
** if you have questions regarding the use of this file, please contact :
** contact@qxorm.com
**
****************************************************************************/

#ifndef _QX_SQL_DATABASE_H_
#define _QX_SQL_DATABASE_H_

#ifdef _MSC_VER
#pragma once
#endif

/*!
 * \file QxSqlDatabase.h
 * \author Lionel Marty
 * \ingroup QxDao
 * \brief Define all parameters to connect to database and retrieve a valid connection by thread
 */

#include <QtCore/qhash.h>
#include <QtCore/qmutex.h>
#include <QtCore/qthread.h>
#include <QtCore/quuid.h>

#include <QtSql/qsqldatabase.h>
#include <QtSql/qsqlquery.h>
#include <QtSql/qsqlerror.h>

#include <QxSingleton/QxSingleton.h>

#include <QxDao/QxSqlGenerator/IxSqlGenerator.h>

#define QX_CONSTRUCT_QX_SQL_DATABASE() \
m_iPort(-1), m_bTraceSqlQuery(true), m_bTraceSqlRecord(false), \
m_ePlaceHolderStyle(ph_style_2_point_name), m_bSessionThrowable(false), \
m_bSessionAutoTransaction(true), m_bValidatorThrowable(false), \
m_bAutoReplaceSqlAliasIntoQuery(true), m_bVerifyOffsetRelation(false), \
m_bAddAutoIncrementIdToUpdateQuery(true)

namespace qx {

/*!
 * \ingroup QxDao
 * \brief qx::QxSqlDatabase : define all parameters to connect to database and retrieve a valid connection by thread (this class is a singleton and is thread-safe)
 */
class QX_DLL_EXPORT QxSqlDatabase : public QxSingleton<QxSqlDatabase>
{

   friend class QxSingleton<QxSqlDatabase>;

public:

   enum ph_style { ph_style_question_mark, ph_style_2_point_name, ph_style_at_name };

private:

   QHash<Qt::HANDLE, QString> m_lstDbByThread;              //!< Collection of databases connexions by thread id
   QMutex m_oDbMutex;                                       //!< Mutex => 'QxSqlDatabase' is thread-safe
   QString m_sDriverName;                                   //!< Driver name to connect to database
   QString m_sConnectOptions;                               //!< Connect options to database
   QString m_sDatabaseName;                                 //!< Database name
   QString m_sUserName;                                     //!< Connection's user name
   QString m_sPassword;                                     //!< Connection's password
   QString m_sHostName;                                     //!< Connection's host name
   int m_iPort;                                             //!< Connection's port number
   bool m_bTraceSqlQuery;                                   //!< Trace each sql query executed
   bool m_bTraceSqlRecord;                                  //!< Trace each sql record
   ph_style m_ePlaceHolderStyle;                            //!< Place holder style to build sql query
   bool m_bSessionThrowable;                                //!< An exception of type qx::dao::sql_error is thrown when a SQL error is appended to qx::QxSession object
   bool m_bSessionAutoTransaction;                          //!< A transaction is automatically beginned when a qx::QxSession object is instantiated
   bool m_bValidatorThrowable;                              //!< An exception of type qx::validator_error is thrown when invalid values are detected inserting or updating an element into database
   qx::dao::detail::IxSqlGenerator_ptr m_pSqlGenerator;     //!< SQL generator to build SQL query specific for each database
   bool m_bAutoReplaceSqlAliasIntoQuery;                    //!< Replace all sql alias into sql query automatically
   bool m_bVerifyOffsetRelation;                            //!< Only for debug purpose : assert if invalid offset detected fetching a relation
   bool m_bAddAutoIncrementIdToUpdateQuery;                 //!< For Microsoft SqlServer database compatibility : add or not auto-increment id to SQL update query

private:

   QxSqlDatabase() : QxSingleton<QxSqlDatabase>("qx::QxSqlDatabase"), QX_CONSTRUCT_QX_SQL_DATABASE() { ; }
   virtual ~QxSqlDatabase() { ; }

public:

   QString getDriverName() const                   { return m_sDriverName; }
   QString getConnectOptions() const               { return m_sConnectOptions; }
   QString getDatabaseName() const                 { return m_sDatabaseName; }
   QString getUserName() const                     { return m_sUserName; }
   QString getPassword() const                     { return m_sPassword; }
   QString getHostName() const                     { return m_sHostName; }
   int getPort() const                             { return m_iPort; }
   bool getTraceSqlQuery() const                   { return m_bTraceSqlQuery; }
   bool getTraceSqlRecord() const                  { return m_bTraceSqlRecord; }
   ph_style getSqlPlaceHolderStyle() const         { return m_ePlaceHolderStyle; }
   bool getSessionThrowable() const                { return m_bSessionThrowable; }
   bool getSessionAutoTransaction() const          { return m_bSessionAutoTransaction; }
   bool getValidatorThrowable() const              { return m_bValidatorThrowable; }
   bool getAutoReplaceSqlAliasIntoQuery() const    { return m_bAutoReplaceSqlAliasIntoQuery; }
   bool getVerifyOffsetRelation() const            { return m_bVerifyOffsetRelation; }
   bool getAddAutoIncrementIdToUpdateQuery() const { return m_bAddAutoIncrementIdToUpdateQuery; }

   void setDriverName(const QString & s)                          { m_sDriverName = s; getSqlGenerator(); }
   void setConnectOptions(const QString & s)                      { m_sConnectOptions = s; }
   void setDatabaseName(const QString & s)                        { m_sDatabaseName = s; }
   void setUserName(const QString & s)                            { m_sUserName = s; }
   void setPassword(const QString & s)                            { m_sPassword = s; }
   void setHostName(const QString & s)                            { m_sHostName = s; }
   void setPort(int i)                                            { m_iPort = i; }
   void setTraceSqlQuery(bool b)                                  { m_bTraceSqlQuery = b; }
   void setTraceSqlRecord(bool b)                                 { m_bTraceSqlRecord = b; }
   void setSqlPlaceHolderStyle(ph_style e)                        { m_ePlaceHolderStyle = e; }
   void setSessionThrowable(bool b)                               { m_bSessionThrowable = b; }
   void setSessionAutoTransaction(bool b)                         { m_bSessionAutoTransaction = b; }
   void setValidatorThrowable(bool b)                             { m_bValidatorThrowable = b; }
   void setSqlGenerator(qx::dao::detail::IxSqlGenerator_ptr p)    { m_pSqlGenerator = p; if (p) { p->init(); } }
   void setAutoReplaceSqlAliasIntoQuery(bool b)                   { m_bAutoReplaceSqlAliasIntoQuery = b; }
   void setVerifyOffsetRelation(bool b)                           { m_bVerifyOffsetRelation = b; }
   void setAddAutoIncrementIdToUpdateQuery(bool b)                { m_bAddAutoIncrementIdToUpdateQuery = b; }

   static QSqlDatabase getDatabase();
   static QSqlDatabase getDatabase(QSqlError & dbError);
   static QSqlDatabase getDatabaseCloned();

   qx::dao::detail::IxSqlGenerator * getSqlGenerator();

private:

   QSqlDatabase getDatabaseByCurrThreadId(QSqlError & dbError);
   QSqlDatabase createDatabase(QSqlError & dbError);

   void displayLastError(const QSqlDatabase & db, const QString & sDesc) const;
   QString formatLastError(const QSqlDatabase & db) const;

   bool isValid() const { return (! m_sDriverName.isEmpty() && ! m_sDatabaseName.isEmpty()); }

};

} // namespace qx

QX_DLL_EXPORT_QX_SINGLETON_HPP(qx::QxSqlDatabase)

#endif // _QX_SQL_DATABASE_H_

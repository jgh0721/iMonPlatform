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

#ifndef _QX_SQL_RELATION_MANY_TO_MANY_H_
#define _QX_SQL_RELATION_MANY_TO_MANY_H_

#ifdef _MSC_VER
#pragma once
#endif

/*!
 * \file QxSqlRelation_ManyToMany.h
 * \author Lionel Marty
 * \ingroup QxDao
 * \brief Manage a relationship many-to-many defined between 2 classes (or between 2 tables in database)
 */

#include <QxDao/QxSqlRelation.h>

namespace qx {

/*!
 * \ingroup QxDao
 * \brief qx::QxSqlRelation_ManyToMany<DataType, Owner> : manage a relationship many-to-many defined between 2 classes (or between 2 tables in database)
 */
template <class DataType, class Owner>
class QxSqlRelation_ManyToMany : public QxSqlRelation<DataType, Owner>
{

private:

   typedef typename QxSqlRelation<DataType, Owner>::type_owner type_owner;
   typedef typename QxSqlRelation<DataType, Owner>::type_data type_data;
   typedef typename QxSqlRelation<DataType, Owner>::type_container type_container;
   typedef typename QxSqlRelation<DataType, Owner>::type_generic_container type_generic_container;
   typedef typename QxSqlRelation<DataType, Owner>::type_item type_item;
   typedef typename type_generic_container::type_iterator type_iterator;

   enum { is_data_container = QxSqlRelation<DataType, Owner>::is_data_container };

protected:

   QString m_sExtraTable;           //!< Extra table that holds the relationship
   QString m_sForeignKeyOwner;      //!< SQL query foreign key for owner
   QString m_sForeignKeyDataType;   //!< SQL query foreign key for data type

public:

   QxSqlRelation_ManyToMany(IxDataMember * p, const QString & sExtraTable, const QString & sForeignKeyOwner, const QString & sForeignKeyDataType) : QxSqlRelation<DataType, Owner>(p), m_sExtraTable(sExtraTable), m_sForeignKeyOwner(sForeignKeyOwner), m_sForeignKeyDataType(sForeignKeyDataType) { this->m_eRelationType = qx::IxSqlRelation::many_to_many; this->verifyParameters(); }
   virtual ~QxSqlRelation_ManyToMany() { BOOST_STATIC_ASSERT(is_data_container); }

   virtual QString getDescription() const                                     { return "relation many-to-many"; }
   virtual QString getExtraTable() const                                      { return m_sExtraTable; }
   virtual bool getCartesianProduct() const                                   { return true; }
   virtual void createTable(QxSqlRelationParams & params) const               { Q_UNUSED(params); }
   virtual void lazySelect(QxSqlRelationParams & params) const                { Q_UNUSED(params); }
   virtual void lazyFrom(QxSqlRelationParams & params) const                  { Q_UNUSED(params); }
   virtual void eagerFrom(QxSqlRelationParams & params) const                 { Q_UNUSED(params); }
   virtual void lazyJoin(QxSqlRelationParams & params) const                  { Q_UNUSED(params); }
   virtual void lazyWhere(QxSqlRelationParams & params) const                 { Q_UNUSED(params); }
   virtual void eagerWhere(QxSqlRelationParams & params) const                { Q_UNUSED(params); }
   virtual void lazyWhereSoftDelete(QxSqlRelationParams & params) const       { Q_UNUSED(params); }
   virtual void lazyFetch_ResolveInput(QxSqlRelationParams & params) const    { Q_UNUSED(params); }
   virtual void eagerFetch_ResolveInput(QxSqlRelationParams & params) const   { Q_UNUSED(params); }
   virtual void lazyFetch_ResolveOutput(QxSqlRelationParams & params) const   { Q_UNUSED(params); }
   virtual void lazyInsert(QxSqlRelationParams & params) const                { Q_UNUSED(params); }
   virtual void lazyInsert_Values(QxSqlRelationParams & params) const         { Q_UNUSED(params); }
   virtual void lazyUpdate(QxSqlRelationParams & params) const                { Q_UNUSED(params); }
   virtual void lazyInsert_ResolveInput(QxSqlRelationParams & params) const   { Q_UNUSED(params); }
   virtual void lazyUpdate_ResolveInput(QxSqlRelationParams & params) const   { Q_UNUSED(params); }
   virtual QSqlError onBeforeSave(QxSqlRelationParams & params) const         { Q_UNUSED(params); return QSqlError(); }

   virtual QVariant getIdFromQuery(bool bEager, QxSqlRelationParams & params) const
   {
      QString sId; IxDataMember * pId = this->getDataId(); if (! bEager || ! pId) { return QVariant(); }
      for (int i = 0; i < pId->getNameCount(); i++) { sId += params.query().value(params.offset() + i).toString() + "|"; }
      return QVariant(sId);
   }

   virtual void updateOffset(bool bEager, QxSqlRelationParams & params) const
   {
      if (! bEager) { return; }
      long lOffsetNew = params.offset() + this->getDataCount();
      lOffsetNew += (this->getDataId() ? this->getDataId()->getNameCount() : 0);
      lOffsetNew += (this->m_oSoftDelete.isEmpty() ? 0 : 1);
      params.setOffset(lOffsetNew);
   }

   virtual void eagerSelect(QxSqlRelationParams & params) const
   {
      long l1(0);
      QString & sql = params.sql();
      IxDataMember * p = NULL; IxDataMember * pId = this->getDataId(); qAssert(pId);
      QString tableAlias = this->tableAlias(params);
      if (pId) { sql += (pId->getSqlTablePointNameAsAlias(tableAlias) + ", "); }
      while ((p = this->nextData(l1))) { sql += (p->getSqlTablePointNameAsAlias(tableAlias) + ", "); }
      if (! this->m_oSoftDelete.isEmpty()) { sql += (this->m_oSoftDelete.buildSqlTablePointName(tableAlias) + ", "); }
   }

   virtual void eagerJoin(QxSqlRelationParams & params) const
   {
      QString & sql = params.sql();
      IxDataMember * pIdOwner = this->getDataIdOwner(); qAssert(pIdOwner);
      IxDataMember * pIdData = this->getDataId(); qAssert(pIdData);
      QString table = this->table(); QString tableAlias = this->tableAlias(params);
      QString tableOwner = this->tableAliasOwner(params);
      if (! pIdOwner || ! pIdData) { return; }
      QStringList lstForeignKeyOwner = m_sForeignKeyOwner.split("|");
      QStringList lstForeignKeyDataType = m_sForeignKeyDataType.split("|");
      qAssert(pIdOwner->getNameCount() == lstForeignKeyOwner.count());
      qAssert(pIdData->getNameCount() == lstForeignKeyDataType.count());
      QString sExtraTableAlias = m_sExtraTable + "_" + QString::number(params.index());
      sql += this->getSqlJoin(params.joinType()) + m_sExtraTable + " " + sExtraTableAlias + " ON ";
      for (int i = 0; i < pIdOwner->getNameCount(); i++)
      { sql += pIdOwner->getSqlAlias(tableOwner, true, i) + " = " + sExtraTableAlias + "." + lstForeignKeyOwner.at(i) + " AND "; }
      sql = sql.left(sql.count() - 5); // Remove last " AND "
      sql += this->getSqlJoin(params.joinType()) + table + " " + tableAlias + " ON ";
      params.builder().addSqlQueryAlias(table, tableAlias);
      for (int i = 0; i < pIdData->getNameCount(); i++)
      { sql += sExtraTableAlias + "." + lstForeignKeyDataType.at(i) + " = " + pIdData->getSqlAlias(tableAlias, true, i) + " AND "; }
      sql = sql.left(sql.count() - 5); // Remove last " AND "
   }

   virtual void eagerWhereSoftDelete(QxSqlRelationParams & params) const
   {
      if (this->m_oSoftDelete.isEmpty()) { return; }
      QString & sql = params.sql();
      QString tableAlias = this->tableAlias(params);
      sql += qx::IxSqlQueryBuilder::addSqlCondition(sql);
      sql += this->m_oSoftDelete.buildSqlQueryToFetch(tableAlias);
   }

   virtual void * eagerFetch_ResolveOutput(QxSqlRelationParams & params) const
   {
      if (! this->verifyOffset(params, true)) { return NULL; }
      QSqlQuery & query = params.query();
      IxDataMember * p = NULL; IxDataMember * pId = this->getDataId(); qAssert(pId);
      long lIndex = 0; long lOffsetId = (pId ? pId->getNameCount() : 0); bool bValidId(false);
      long lOffsetOld = params.offset(); this->updateOffset(true, params);
      for (int i = 0; i < pId->getNameCount(); i++)
      { QVariant v = query.value(lOffsetOld + i); bValidId = (bValidId || qx::trait::is_valid_primary_key(v)); }
      if (! bValidId) { return NULL; }
      type_item item = this->createItem();
      type_data & item_val = item.value_qx();
      for (int i = 0; i < pId->getNameCount(); i++)
      { QVariant v = query.value(lOffsetOld + i); qx::cvt::from_variant(v, item.key(), "", i); }
      for (int i = 0; i < pId->getNameCount(); i++)
      { QVariant v = query.value(lOffsetOld + i); pId->fromVariant((& item_val), v, "", i); }
      while ((p = this->nextData(lIndex)))
      { p->fromVariant((& item_val), query.value(lIndex + lOffsetOld + lOffsetId - 1)); }
      type_generic_container::insertItem(this->getContainer(params), item);
      return (& item_val);
   }

   virtual QSqlError onAfterSave(QxSqlRelationParams & params) const
   {
      if (this->isNullData(params)) { return this->deleteFromExtraTable(params); }
      QSqlError daoError = qx::dao::save(this->getContainer(params), (& params.database()));
      if (daoError.isValid()) { return daoError; }
      daoError = this->deleteFromExtraTable(params);
      if (daoError.isValid()) { return daoError; }
      daoError = this->insertIntoExtraTable(params);
      if (daoError.isValid()) { return daoError; }
      return QSqlError();
   }

   virtual QString createExtraTable() const
   {
      IxDataMember * pIdOwner = this->getDataIdOwner(); qAssert(pIdOwner);
      IxDataMember * pIdData = this->getDataId(); qAssert(pIdData);
      if (! pIdOwner || ! pIdData) { return ""; }

      bool bOldPKOwner = pIdOwner->getIsPrimaryKey(); pIdOwner->setIsPrimaryKey(false);
      bool bOldPKData = ((pIdOwner == pIdData) ? bOldPKOwner : pIdData->getIsPrimaryKey()); pIdData->setIsPrimaryKey(false);
      bool bOldAIOwner = pIdOwner->getAutoIncrement(); pIdOwner->setAutoIncrement(false);
      bool bOldAIData = ((pIdOwner == pIdData) ? bOldAIOwner : pIdData->getAutoIncrement()); pIdData->setAutoIncrement(false);

      QString sql = "CREATE TABLE IF NOT EXISTS " + m_sExtraTable + " (";
      sql += pIdOwner->getSqlNameAndTypeAndParams(", ", m_sForeignKeyOwner) + ", "; qAssert(! pIdOwner->getSqlType().isEmpty());
      sql += pIdData->getSqlNameAndTypeAndParams(", ", m_sForeignKeyDataType) + ", "; qAssert(! pIdData->getSqlType().isEmpty());
      sql = sql.left(sql.count() - 2); // Remove last ", "
      sql += ")";

      pIdOwner->setIsPrimaryKey(bOldPKOwner); pIdData->setIsPrimaryKey(bOldPKData);
      pIdOwner->setAutoIncrement(bOldAIOwner); pIdData->setAutoIncrement(bOldAIData);
      if (this->traceSqlQuery()) { qDebug("[QxOrm] create extra-table (relation many-to-many) : %s", qPrintable(sql)); }
      return sql;
   }

private:

   inline void verifyParameters()
   { qAssert(! m_sExtraTable.isEmpty() && ! m_sForeignKeyOwner.isEmpty() && ! m_sForeignKeyDataType.isEmpty() && (m_sForeignKeyOwner != m_sForeignKeyDataType)); }

   QSqlError deleteFromExtraTable(QxSqlRelationParams & params) const
   {
      IxDataMember * pIdOwner = this->getDataIdOwner(); qAssert(pIdOwner);
      QString sql = "DELETE FROM " + m_sExtraTable + " WHERE ";
      QStringList lstForeignKeyOwner = m_sForeignKeyOwner.split("|");
      qAssert(pIdOwner->getNameCount() == lstForeignKeyOwner.count());
      for (int i = 0; i < pIdOwner->getNameCount(); i++)
      { sql += m_sExtraTable + "." + lstForeignKeyOwner.at(i) + " = " + pIdOwner->getSqlPlaceHolder("", i) + " AND "; }
      sql = sql.left(sql.count() - 5); // Remove last " AND "
      if (this->traceSqlQuery()) { qDebug("[QxOrm] sql query (extra-table) : %s", qPrintable(sql)); }

      QSqlQuery queryDelete(params.database());
      queryDelete.prepare(sql);
      pIdOwner->setSqlPlaceHolder(queryDelete, params.owner());
      if (! queryDelete.exec()) { return queryDelete.lastError(); }
      return QSqlError();
   }

   QSqlError insertIntoExtraTable(QxSqlRelationParams & params) const
   {
      IxDataMember * pIdOwner = this->getDataIdOwner(); qAssert(pIdOwner);
      IxDataMember * pIdData = this->getDataId(); qAssert(pIdData);
      if (! pIdOwner || ! pIdData) { return QSqlError(); }
      QStringList lstForeignKeyOwner = m_sForeignKeyOwner.split("|");
      QStringList lstForeignKeyDataType = m_sForeignKeyDataType.split("|");
      qAssert(pIdOwner->getNameCount() == lstForeignKeyOwner.count());
      qAssert(pIdData->getNameCount() == lstForeignKeyDataType.count());

      QString sql = "INSERT INTO " + m_sExtraTable + " (";
      sql += pIdOwner->getSqlName(", ", m_sForeignKeyOwner) + ", " + pIdData->getSqlName(", ", m_sForeignKeyDataType);
      sql += ") VALUES (";
      sql += pIdOwner->getSqlPlaceHolder("", -1, ", ", m_sForeignKeyOwner) + ", " + pIdData->getSqlPlaceHolder("", -1, ", ", m_sForeignKeyDataType) + ")";
      if (this->traceSqlQuery()) { qDebug("[QxOrm] sql query (extra-table) : %s", qPrintable(sql)); }

      type_item item;
      type_container & container = this->getContainer(params);
      type_iterator itr = type_generic_container::begin(container, item);
      type_iterator itr_end = type_generic_container::end(container);
      QSqlQuery queryInsert(params.database());
      queryInsert.prepare(sql);

      while (itr != itr_end)
      {
         pIdOwner->setSqlPlaceHolder(queryInsert, params.owner(), "", m_sForeignKeyOwner);
         pIdData->setSqlPlaceHolder(queryInsert, (& item.value_qx()), "", m_sForeignKeyDataType);
         if (! queryInsert.exec()) { return queryInsert.lastError(); }
         itr = type_generic_container::next(container, itr, item);
      }

      return QSqlError();
   }

};

} // namespace qx

#endif // _QX_SQL_RELATION_MANY_TO_MANY_H_

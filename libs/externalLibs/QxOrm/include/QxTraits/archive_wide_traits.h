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

#ifndef _QX_ARCHIVE_WIDE_TRAITS_H_
#define _QX_ARCHIVE_WIDE_TRAITS_H_

#ifdef _MSC_VER
#pragma once
#endif

/*!
 * \file archive_wide_traits.h
 * \author Lionel Marty
 * \ingroup QxTraits
 * \brief qx::trait::is_archive_wide<T>::value : define if a boost::archive type uses wide string character and stream (for example std::wstring) or not (for example std::string)
 */

#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

#include <boost/mpl/if.hpp>
#include <boost/mpl/logical.hpp>

#include <QtCore/qstring.h>

#include <QxSerialize/boost/QxSerializeInclude.h>

namespace qx {
namespace trait {

/*!
 * \ingroup QxTraits
 * \brief qx::trait::is_archive_wide<T>::value : define if a boost::archive type uses wide string character and stream (for example std::wstring) or not (for example std::string)
 */
template <typename T> struct is_archive_wide { enum { value = false }; };

#if _QX_SERIALIZE_WIDE_BINARY
template <> struct is_archive_wide<boost::archive::binary_wiarchive> { enum { value = true }; };
template <> struct is_archive_wide<boost::archive::binary_woarchive> { enum { value = true }; };
#endif // _QX_SERIALIZE_WIDE_BINARY

#if _QX_SERIALIZE_WIDE_TEXT
template <> struct is_archive_wide<boost::archive::text_wiarchive>   { enum { value = true }; };
template <> struct is_archive_wide<boost::archive::text_woarchive>   { enum { value = true }; };
#endif // _QX_SERIALIZE_WIDE_TEXT

#if _QX_SERIALIZE_WIDE_XML
template <> struct is_archive_wide<boost::archive::xml_wiarchive>    { enum { value = true }; };
template <> struct is_archive_wide<boost::archive::xml_woarchive>    { enum { value = true }; };
#endif // _QX_SERIALIZE_WIDE_XML

template <typename T>
class archive_wide_traits
{

public:

   enum { is_wide = qx::trait::is_archive_wide<T>::value };

   typedef typename boost::mpl::if_c<is_wide, wchar_t, char>::type type_char;
   typedef typename boost::mpl::if_c<is_wide, std::wstring, std::string>::type type_string;

   typedef typename boost::mpl::if_c<is_wide, std::wistream, std::istream>::type type_istream;
   typedef typename boost::mpl::if_c<is_wide, std::wostream, std::ostream>::type type_ostream;

   typedef typename boost::mpl::if_c<is_wide, std::wstringstream, std::stringstream>::type type_stringstream;
   typedef typename boost::mpl::if_c<is_wide, std::wistringstream, std::istringstream>::type type_istringstream;
   typedef typename boost::mpl::if_c<is_wide, std::wostringstream, std::ostringstream>::type type_ostringstream;

   typedef typename boost::mpl::if_c<is_wide, std::wfstream, std::fstream>::type type_fstream;
   typedef typename boost::mpl::if_c<is_wide, std::wifstream, std::ifstream>::type type_ifstream;
   typedef typename boost::mpl::if_c<is_wide, std::wofstream, std::ofstream>::type type_ofstream;

   static inline QString toQString(const type_string & str)                   { return cvtQString<is_wide, 0>::toQString(str); }
   static inline void fromQString(const QString & str, type_string & result)  { cvtQString<is_wide, 0>::fromQString(str, result); }

   static inline QByteArray toQByteArray(const type_string & str, type_string * owner) { return cvtQByteArray<is_wide, 0>::toQByteArray(str, owner); }
   static inline void fromQByteArray(const QByteArray & data, type_string & result)    { cvtQByteArray<is_wide, 0>::fromQByteArray(data, result); }

private:

   template <bool isWide /* = false */, int dummy>
   struct cvtQString
   {
      static inline QString toQString(const std::string & str)                   { return QString::fromStdString(str); }
      static inline void fromQString(const QString & str, std::string & result)  { result = str.toStdString(); }
   };

   template <int dummy>
   struct cvtQString<true, dummy>
   {
      static inline QString toQString(const std::wstring & str)                     { return QString::fromStdWString(str); }
      static inline void fromQString(const QString & str, std::wstring & result)    { result = str.toStdWString(); }
   };

   template <bool isWide /* = false */, int dummy>
   struct cvtQByteArray
   {
      static inline QByteArray toQByteArray(const std::string & str, std::string * owner)
      { if (owner) { (* owner) = str; }; return (owner ? QByteArray::fromRawData(owner->data(), owner->size()) : QByteArray(str.data(), str.size())); }
      static inline void fromQByteArray(const QByteArray & data, std::string & result)
      { result.clear(); result.append(data.constData(), data.size()); }
   };

   template <int dummy>
   struct cvtQByteArray<true, dummy>
   {
      static inline QByteArray toQByteArray(const std::wstring & str, std::wstring * owner)
      { Q_UNUSED(owner); return QString::fromStdWString(str).toUtf8(); }
      static inline void fromQByteArray(const QByteArray & data, std::wstring & result)
      { result = QString::fromUtf8(data.constData(), data.size()).toStdWString(); }
   };

};

} // namespace trait
} // namespace qx

#endif // _QX_ARCHIVE_WIDE_TRAITS_H_

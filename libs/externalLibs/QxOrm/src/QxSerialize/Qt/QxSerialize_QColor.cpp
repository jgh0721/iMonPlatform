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

#include <QxPrecompiled.h>

#if _QX_ENABLE_QT_GUI_DEPENDENCY

#include <QxSerialize/Qt/QxSerialize_QColor.h>

#include <QxMemLeak.h>

namespace boost {
namespace serialization {

template <class Archive>
inline void qx_save(Archive & ar, const QColor & t, const unsigned int file_version)
{
   Q_UNUSED(file_version);
   QColor clr = t.toRgb();
   int iR(clr.red()), iG(clr.green()), iB(clr.blue()), iA(t.alpha());

   ar << boost::serialization::make_nvp("R", iR);
   ar << boost::serialization::make_nvp("G", iG);
   ar << boost::serialization::make_nvp("B", iB);
   ar << boost::serialization::make_nvp("A", iA);
}

template <class Archive>
inline void qx_load(Archive & ar, QColor & t, const unsigned int file_version)
{
   Q_UNUSED(file_version);
   int iR(0), iG(0), iB(0), iA(0);

   ar >> boost::serialization::make_nvp("R", iR);
   ar >> boost::serialization::make_nvp("G", iG);
   ar >> boost::serialization::make_nvp("B", iB);
   ar >> boost::serialization::make_nvp("A", iA);

   t.setRed(iR);
   t.setGreen(iG);
   t.setBlue(iB);
   t.setAlpha(iA);
}

} // namespace boost
} // namespace serialization

QX_SERIALIZE_FAST_COMPIL_SAVE_LOAD_CPP(QColor)

#endif // _QX_ENABLE_QT_GUI_DEPENDENCY

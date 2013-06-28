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

#if _QX_ENABLE_QT_NETWORK_DEPENDENCY
#ifndef _IX_SERVICE_PARAMETER_H_
#define _IX_SERVICE_PARAMETER_H_

#ifdef _MSC_VER
#pragma once
#endif

/*!
 * \file IxParameter.h
 * \author Lionel Marty
 * \ingroup QxService
 * \brief Common interface for all parameters transfered by QxService module of QxOrm library
 */

#include <QxRegister/QxRegisterInternalHelper.h>

namespace qx {
namespace service {

/*!
 * \ingroup QxService
 * \brief qx::service::IxParameter : common interface for all parameters transfered by QxService module of QxOrm library
 *
 * <a href="http://www.qxorm.com/qxorm_en/tutorial_2.html" target="_blank">Click here to access to a tutorial to explain how to work with QxService module.</a>
 */
class QX_DLL_EXPORT IxParameter
{

public:

   IxParameter();
   virtual ~IxParameter();

};

typedef boost::shared_ptr<IxParameter> IxParameter_ptr;

} // namespace service
} // namespace qx

QX_REGISTER_INTERNAL_HELPER_HPP(QX_DLL_EXPORT, qx::service::IxParameter, 0)

#endif // _IX_SERVICE_PARAMETER_H_
#endif // _QX_ENABLE_QT_NETWORK_DEPENDENCY

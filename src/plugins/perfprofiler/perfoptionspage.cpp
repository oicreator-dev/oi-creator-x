/****************************************************************************
**
** Copyright (C) 2018 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of Qt Creator.
**
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3 as published by the Free Software
** Foundation with exceptions as appearing in the file LICENSE.GPL3-EXCEPT
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-3.0.html.
**
****************************************************************************/

#include "perfconfigwidget.h"
#include "perfoptionspage.h"
#include "perfprofilerconstants.h"
#include "perfprofilerplugin.h"

#include <debugger/analyzer/analyzericons.h>

namespace PerfProfiler {
namespace Internal {

PerfOptionsPage::PerfOptionsPage(QObject *parent) : Core::IOptionsPage(parent)
{
    setId(Constants::PerfSettingsId);
    setDisplayName(QCoreApplication::translate("PerfProfiler::PerfOptionsPage", "CPU Usage"));
    setCategory("T.Analyzer");
    setDisplayCategory(QCoreApplication::translate("Analyzer", "Analyzer"));
    setCategoryIcon(Analyzer::Icons::SETTINGSCATEGORY_ANALYZER);
}

QWidget *PerfOptionsPage::widget()
{
    if (!m_widget)
        m_widget = new PerfConfigWidget(PerfProfilerPlugin::globalSettings());
    return m_widget;
}

void PerfOptionsPage::apply()
{
    PerfProfilerPlugin::globalSettings()->writeGlobalSettings();
}

void PerfOptionsPage::finish()
{
    delete m_widget;
}

} // namespace Internal
} // namespace PerfProfiler

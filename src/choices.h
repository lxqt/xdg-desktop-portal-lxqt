/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
 *
 * Copyright: 2016-2018 Red Hat Inc
 * Copyright: 2016-2018 Jan Grulich <jgrulich@redhat.com>
 * Copyright: 2021~ LXQt team
 * Authors:
 *   Palo Kisa <palo.kisa@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */

#pragma once

#include <QList>
#include <QMap>
#include <QString>
#include <QVariant>

class QCheckBox;
class QComboBox;
class QDBusArgument;
class QWidget;

namespace LXQt
{
    struct Choice {
        QString id;
        QString value;
    };
    using Choices = QList<Choice>;

    struct Option {
        QString id;
        QString label;
        Choices choices;
        QString initialChoiceId;
    };
    using OptionList = QList<Option>;

    QDBusArgument &operator<<(QDBusArgument &arg, const Choice &choice);
    const QDBusArgument &operator>>(const QDBusArgument &arg, Choice &choice);
    QDBusArgument &operator<<(QDBusArgument &arg, const Option &option);
    const QDBusArgument &operator>>(const QDBusArgument &arg, Option &option);

    void registerChoiceMetaTypes();

    QWidget *CreateChoiceControls(const OptionList &optionList, QMap<QString, QCheckBox *> &checkboxes, QMap<QString, QComboBox *> &comboboxes);

    QVariant EvaluateSelectedChoices(const QMap<QString, QCheckBox *> &checkboxes, const QMap<QString, QComboBox *> &comboboxes);
}

Q_DECLARE_METATYPE(LXQt::Choice)
Q_DECLARE_METATYPE(LXQt::Choices)
Q_DECLARE_METATYPE(LXQt::Option)
Q_DECLARE_METATYPE(LXQt::OptionList)

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

#include "choices.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDBusArgument>
#include <QDBusMetaType>
#include <QGridLayout>
#include <QLabel>

namespace LXQt
{
    QDBusArgument &operator<<(QDBusArgument &arg, const Choice &choice)
    {
        arg.beginStructure();
        arg << choice.id << choice.value;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, Choice &choice)
    {
        QString id;
        QString value;
        arg.beginStructure();
        arg >> id >> value;
        choice.id = id;
        choice.value = value;
        arg.endStructure();
        return arg;
    }

    QDBusArgument &operator<<(QDBusArgument &arg, const Option &option)
    {
        arg.beginStructure();
        arg << option.id << option.label << option.choices << option.initialChoiceId;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, Option &option)
    {
        QString id;
        QString label;
        Choices choices;
        QString initialChoiceId;
        arg.beginStructure();
        arg >> id >> label >> choices >> initialChoiceId;
        option.id = id;
        option.label = label;
        option.choices = choices;
        option.initialChoiceId = initialChoiceId;
        arg.endStructure();
        return arg;
    }

    void registerChoiceMetaTypes()
    {
        qDBusRegisterMetaType<Choice>();
        qDBusRegisterMetaType<Choices>();
        qDBusRegisterMetaType<Option>();
        qDBusRegisterMetaType<OptionList>();
    }

    QWidget *CreateChoiceControls(const OptionList &optionList,
            QMap<QString, QCheckBox *> &checkboxes,
            QMap<QString, QComboBox *> &comboboxes)
    {
        if (optionList.empty()) {
            return nullptr;
        }

        QWidget *optionsWidget = new QWidget;
        QGridLayout *layout = new QGridLayout(optionsWidget);
        // set stretch for (unused) column 2 so controls only take the space they actually need
        layout->setColumnStretch(2, 1);
        optionsWidget->setLayout(layout);

        for (const Option &option : optionList) {
            const int nextRow = layout->rowCount();
            // empty list of choices -> boolean choice according to the spec
            if (option.choices.empty()) {
                QCheckBox *checkbox = new QCheckBox(option.label, optionsWidget);
                checkbox->setChecked(option.initialChoiceId == QStringLiteral("true"));
                layout->addWidget(checkbox, nextRow, 1);
                checkboxes.insert(option.id, checkbox);
            } else {
                QComboBox *combobox = new QComboBox(optionsWidget);
                for (const Choice &choice : option.choices) {
                    combobox->addItem(choice.value, choice.id);
                    // select this entry if initialChoiceId matches
                    if (choice.id == option.initialChoiceId) {
                        combobox->setCurrentIndex(combobox->count() - 1);
                    }
                }
                QString labelText = option.label;
                if (!labelText.endsWith(QChar::fromLatin1(':'))) {
                    labelText += QChar::fromLatin1(':');
                }
                QLabel *label = new QLabel(labelText, optionsWidget);
                label->setBuddy(combobox);
                layout->addWidget(label, nextRow, 0, Qt::AlignRight);
                layout->addWidget(combobox, nextRow, 1);
                comboboxes.insert(option.id, combobox);
            }
        }

        return optionsWidget;
    }

    QVariant EvaluateSelectedChoices(const QMap<QString, QCheckBox *> &checkboxes, const QMap<QString, QComboBox *> &comboboxes)
    {
        Choices selectedChoices;
        const auto checkboxKeys = checkboxes.keys();
        for (const QString &id : checkboxKeys) {
            Choice choice;
            choice.id = id;
            choice.value = checkboxes.value(id)->isChecked() ? QStringLiteral("true") : QStringLiteral("false");
            selectedChoices << choice;
        }
        const auto comboboxKeys = comboboxes.keys();
        for (const QString &id : comboboxKeys) {
            Choice choice;
            choice.id = id;
            choice.value = comboboxes.value(id)->currentData().toString();
            selectedChoices << choice;
        }

        return QVariant::fromValue<Choices>(selectedChoices);
    }
}

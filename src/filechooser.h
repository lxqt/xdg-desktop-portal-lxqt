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

#include <QDBusAbstractAdaptor>

class QCheckBox;
class QComboBox;
class QDBusObjectPath;

namespace LXQt
{
    class FileChooserPortal : public QDBusAbstractAdaptor
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.FileChooser")
    public:
        // Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
        struct Filter {
            uint type;
            QString filterString;
        };
        using Filters = QList<Filter>;

        struct FilterList {
            QString userVisibleName;
            Filters filters;
        };
        using FilterListList = QList<FilterList>;

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

        explicit FileChooserPortal(QObject *parent);
        ~FileChooserPortal();

    public Q_SLOTS:
        uint OpenFile(const QDBusObjectPath &handle,
                const QString &app_id,
                const QString &parent_window,
                const QString &title,
                const QVariantMap &options,
                QVariantMap &results);

        uint SaveFile(const QDBusObjectPath &handle,
                const QString &app_id,
                const QString &parent_window,
                const QString &title,
                const QVariantMap &options,
                QVariantMap &results);

    private:
        static QWidget *CreateChoiceControls(const OptionList &optionList, QMap<QString, QCheckBox *> &checkboxes, QMap<QString, QComboBox *> &comboboxes);

        static QVariant EvaluateSelectedChoices(const QMap<QString, QCheckBox *> &checkboxes, const QMap<QString, QComboBox *> &comboboxes);

        static QString ExtractAcceptLabel(const QVariantMap &options);

        static void ExtractFilters(const QVariantMap &options,
                QStringList &nameFilters,
                QMap<QString, FilterList> &allFilters,
                QString &selectedNameFilter);

        static QStringList NameFiltersForMimeType(const QString &mimeType);

    private:
        QMap<QString, QUrl> mLastVisitedDirs;
    };
}

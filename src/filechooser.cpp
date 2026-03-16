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
#include "filechooser.h"
#include "utils.h"
#include "filedialoghelper.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDialogButtonBox>
#include <QFile>
#include <QLayout>
#include <QLoggingCategory>
#include <QMimeDatabase>
#include <QUrl>
#include <QDBusObjectPath>
#include <libfm-qt6/filedialog.h>

// Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Filter)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Filters)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::FilterListList)

namespace LXQt
{
    Q_LOGGING_CATEGORY(XdgDesktopPortalLxqtFileChooser, "xdp-lxqt-file-chooser")


    QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Filter &filter)
    {
        arg.beginStructure();
        arg << filter.type << filter.filterString;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Filter &filter)
    {
        uint type;
        QString filterString;
        arg.beginStructure();
        arg >> type >> filterString;
        filter.type = type;
        filter.filterString = filterString;
        arg.endStructure();

        return arg;
    }

    QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::FilterList &filterList)
    {
        arg.beginStructure();
        arg << filterList.userVisibleName << filterList.filters;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::FilterList &filterList)
    {
        QString userVisibleName;
        FileChooserPortal::Filters filters;
        arg.beginStructure();
        arg >> userVisibleName >> filters;
        filterList.userVisibleName = userVisibleName;
        filterList.filters = filters;
        arg.endStructure();

        return arg;
    }

    FileChooserPortal::FileChooserPortal(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
        qDBusRegisterMetaType<Filter>();
        qDBusRegisterMetaType<Filters>();
        qDBusRegisterMetaType<FilterList>();
        qDBusRegisterMetaType<FilterListList>();
        registerChoiceMetaTypes();
    }

    FileChooserPortal::~FileChooserPortal()
    {
    }

    // The portal may send us null terminated strings. Make sure to strip the extranous \0
    // in favor of the implicit \0.
    // QByteArrays are implicitly terminated already.
    static QUrl decodeFileName(const QByteArray &name)
    {
        QByteArray decodedName = name;
        while (decodedName.endsWith('\0')) {
            decodedName.chop(1);
        }
        QString str = QFile::decodeName(decodedName);
        if (!str.isEmpty()) {
            return QUrl::fromLocalFile(str);
        }
        return QUrl();
    }

    uint FileChooserPortal::OpenFile(const QDBusObjectPath &handle,
            const QString &app_id,
            const QString &parent_window,
            const QString &title,
            const QVariantMap &options,
            QVariantMap &results)
    {
        Q_UNUSED(app_id);

        qCDebug(XdgDesktopPortalLxqtFileChooser) << "OpenFile called with parameters:";
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    handle: " << handle.path();
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    parent_window: " << parent_window;
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    title: " << title;
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    options: " << options;

        bool directory = false;
        bool modalDialog = true;
        bool multipleFiles = false;
        QUrl currentFolder;
        QStringList nameFilters;
        QString selectedNameFilter;
        // mapping between filter strings and actual filters
        QMap<QString, FilterList> allFilters;

        const QString acceptLabel = ExtractAcceptLabel(options);

        if (options.contains(QStringLiteral("modal"))) {
            modalDialog = options.value(QStringLiteral("modal")).toBool();
        }

        if (options.contains(QStringLiteral("multiple"))) {
            multipleFiles = options.value(QStringLiteral("multiple")).toBool();
        }

        if (options.contains(QStringLiteral("directory"))) {
            directory = options.value(QStringLiteral("directory")).toBool();
        }

        if (options.contains(QStringLiteral("current_folder"))) {
            currentFolder = decodeFileName(options.value(QStringLiteral("current_folder")).toByteArray());
        }

        ExtractFilters(options, nameFilters, allFilters, selectedNameFilter);

        // for handling of options - choices
        std::unique_ptr<QWidget> optionsWidget;
        // to store IDs for choices along with corresponding comboboxes/checkboxes
        QMap<QString, QCheckBox *> checkboxes;
        QMap<QString, QComboBox *> comboboxes;

        if (options.contains(QStringLiteral("choices"))) {
            OptionList optionList = qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
            optionsWidget.reset(CreateChoiceControls(optionList, checkboxes, comboboxes));
        }

        auto fileDialog = FileDialogHelper::createFileDialogHelper();
        Utils::setParentWindow(&fileDialog->dialog(), parent_window);
        fileDialog->setWindowTitle(title);
        fileDialog->setModal(modalDialog);
        fileDialog->setFileMode(directory ? QFileDialog::Directory : (multipleFiles ? QFileDialog::ExistingFiles : QFileDialog::ExistingFile));
        if (!acceptLabel.isEmpty())
            fileDialog->setLabelText(QFileDialog::Accept, acceptLabel);

        if (currentFolder.isValid()) {
            fileDialog->setDirectory(currentFolder);
        } else if (mLastVisitedDirs.count(parent_window) > 0) {
            fileDialog->setDirectory(mLastVisitedDirs[parent_window]);
        }

        if (!nameFilters.isEmpty()) {
            fileDialog->setNameFilters(nameFilters);
            fileDialog->selectNameFilter(selectedNameFilter);
        }

        bool bHasOptions = false;
        if (optionsWidget) {
            if (auto layout = fileDialog->dialog().layout()) {
                layout->addWidget(optionsWidget.release());
                bHasOptions = true;
            }
        }

        if (fileDialog->execResult() == QDialog::Accepted) {
            QStringList files;
            for (const auto & url : fileDialog->selectedFiles()) {
                files << url.toDisplayString();
            }

            if (files.isEmpty()) {
                qCDebug(XdgDesktopPortalLxqtFileChooser) << "Failed to open file: no local file selected";
                return 2;
            }

            results.insert(QStringLiteral("uris"), files);
            results.insert(QStringLiteral("writable"), true);

            if (bHasOptions) {
                QVariant choices = EvaluateSelectedChoices(checkboxes, comboboxes);
                results.insert(QStringLiteral("choices"), choices);
            }

            // try to map current filter back to one of the predefined ones
            QString selectedFilter = fileDialog->selectedNameFilter();
            if (allFilters.contains(selectedFilter)) {
                results.insert(QStringLiteral("current_filter"), QVariant::fromValue<FilterList>(allFilters.value(selectedFilter)));
            }

            mLastVisitedDirs[parent_window] = fileDialog->directory();

            return 0;
        }

        return 1;
    }

    uint FileChooserPortal::SaveFile(const QDBusObjectPath &handle,
            const QString &app_id,
            const QString &parent_window,
            const QString &title,
            const QVariantMap &options,
            QVariantMap &results)
    {
        Q_UNUSED(app_id);

        qCDebug(XdgDesktopPortalLxqtFileChooser) << "SaveFile called with parameters:";
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    handle: " << handle.path();
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    parent_window: " << parent_window;
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    title: " << title;
        qCDebug(XdgDesktopPortalLxqtFileChooser) << "    options: " << options;

        bool modalDialog = true;
        QString currentName;
        QUrl currentFolder;
        QUrl currentFile;
        QStringList nameFilters;
        QString selectedNameFilter;
        // mapping between filter strings and actual filters
        QMap<QString, FilterList> allFilters;

        if (options.contains(QStringLiteral("modal"))) {
            modalDialog = options.value(QStringLiteral("modal")).toBool();
        }

        const QString acceptLabel = ExtractAcceptLabel(options);

        if (options.contains(QStringLiteral("current_name"))) {
            currentName = options.value(QStringLiteral("current_name")).toString();
        }

        if (options.contains(QStringLiteral("current_folder"))) {
            currentFolder = decodeFileName(options.value(QStringLiteral("current_folder")).toByteArray());
        }

        if (options.contains(QStringLiteral("current_file"))) {
            currentFile = decodeFileName(options.value(QStringLiteral("current_file")).toByteArray());
        }

        ExtractFilters(options, nameFilters, allFilters, selectedNameFilter);

        // for handling of options - choices
        std::unique_ptr<QWidget> optionsWidget;
        // to store IDs for choices along with corresponding comboboxes/checkboxes
        QMap<QString, QCheckBox *> checkboxes;
        QMap<QString, QComboBox *> comboboxes;

        if (options.contains(QStringLiteral("choices"))) {
            OptionList optionList = qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
            optionsWidget.reset(CreateChoiceControls(optionList, checkboxes, comboboxes));
        }

        auto fileDialog = FileDialogHelper::createFileDialogHelper();
        Utils::setParentWindow(&fileDialog->dialog(), parent_window);
        fileDialog->setWindowTitle(title);
        fileDialog->setModal(modalDialog);
        fileDialog->setFileMode(QFileDialog::AnyFile);
        fileDialog->setAcceptMode(QFileDialog::AcceptSave);
        if (!acceptLabel.isEmpty())
            fileDialog->setLabelText(QFileDialog::Accept, acceptLabel);

        if (currentFolder.isValid()) {
            fileDialog->setDirectory(currentFolder);
        } else if (mLastVisitedDirs.count(parent_window) > 0) {
            fileDialog->setDirectory(mLastVisitedDirs[parent_window]);
        }

        if (currentFile.isValid()) {
            fileDialog->selectFile(currentFile);
        } else if (!currentName.isEmpty()) {
            // Fm::FileDialog::directory() returns url w/o trailing slash, so QUrl treats it as file instead of a directory
            // => we need to workaround it to get correct file with QUrl::resolved()
            const QUrl dir = fileDialog->directory();
            QString dir_name = dir.fileName();
            if (!dir_name.isEmpty())
            {
                dir_name += QLatin1Char('/');
                currentName.prepend(dir_name);
            }
            QUrl relative_file;
            relative_file.setPath(currentName);
            fileDialog->selectFile(dir.resolved(relative_file));
        }

        if (!nameFilters.isEmpty()) {
            fileDialog->setNameFilters(nameFilters);
            fileDialog->selectNameFilter(selectedNameFilter);
        }

        bool bHasOptions = false;
        if (optionsWidget) {
            if (auto layout = fileDialog->dialog().layout()) {
                layout->addWidget(optionsWidget.release());
                bHasOptions = true;
            }
        }

        if (fileDialog->execResult() == QDialog::Accepted) {
            QStringList files;
            for (const auto & url : fileDialog->selectedFiles()) {
                files << url.toDisplayString();
                break;
            }
            results.insert(QStringLiteral("uris"), files);

            if (bHasOptions) {
                QVariant choices = EvaluateSelectedChoices(checkboxes, comboboxes);
                results.insert(QStringLiteral("choices"), choices);
            }

            // try to map current filter back to one of the predefined ones
            QString selectedFilter = fileDialog->selectedNameFilter();
            if (allFilters.contains(selectedFilter)) {
                results.insert(QStringLiteral("current_filter"), QVariant::fromValue<FilterList>(allFilters.value(selectedFilter)));
            }

            mLastVisitedDirs[parent_window] = fileDialog->directory();

            return 0;
        }

        return 1;
    }

    QString FileChooserPortal::ExtractAcceptLabel(const QVariantMap &options)
    {
        QString acceptLabel;
        if (options.contains(QStringLiteral("accept_label"))) {
            acceptLabel = options.value(QStringLiteral("accept_label")).toString();
            Utils::convertGtkMnemonic(acceptLabel);
        }
        return acceptLabel;
    }

    void FileChooserPortal::ExtractFilters(const QVariantMap &options,
            QStringList &nameFilters,
            QMap<QString, FilterList> &allFilters,
            QString &selectedNameFilter)
    {
        if (options.contains(QStringLiteral("filters"))) {
            const FilterListList filterListList = qdbus_cast<FilterListList>(options.value(QStringLiteral("filters")));
            for (const FilterList &filterList : filterListList) {
                QStringList filterStrings;
                for (const Filter &filterStruct : filterList.filters) {
                    if (filterStruct.type == 0) {
                        filterStrings << filterStruct.filterString;
                    } else {
                        filterStrings << NameFiltersForMimeType(filterStruct.filterString);
                    }
                }

                if (!filterStrings.isEmpty()) {
                    const QString filterString = filterStrings.join(QLatin1Char(' '));
                    const QString nameFilter = QStringLiteral("%2 (%1)").arg(filterString, filterList.userVisibleName);
                    nameFilters << nameFilter;
                    allFilters[nameFilter] = filterList;
                }
            }
        }

        if (options.contains(QStringLiteral("current_filter"))) {
            FilterList filterList = qdbus_cast<FilterList>(options.value(QStringLiteral("current_filter")));
            if (filterList.filters.size() == 1) {
                QStringList filterStrings;
                Filter filterStruct = filterList.filters.at(0);
                if (filterStruct.type == 0) {
                    filterStrings << filterStruct.filterString;
                } else {
                    filterStrings << NameFiltersForMimeType(filterStruct.filterString);
                }

                if (!filterStrings.isEmpty()) {
                    // make the relevant entry the first one in the list of filters,
                    // since that is the one that gets preselected by KFileWidget::setFilter
                    const QString filterString = filterStrings.join(QLatin1Char(' '));
                    const QString nameFilter = QStringLiteral("%2 (%1)").arg(filterString, filterList.userVisibleName);
                    nameFilters.removeAll(nameFilter);
                    nameFilters.push_front(nameFilter);
                    selectedNameFilter = nameFilter;
                }
            } else {
                qCDebug(XdgDesktopPortalLxqtFileChooser) << "Ignoring 'current_filter' parameter with 0 or multiple filters specified.";
            }
        }
    }

    QStringList FileChooserPortal::NameFiltersForMimeType(const QString &mimeType)
    {
        QMimeDatabase db;
        QMimeType mime(db.mimeTypeForName(mimeType));

        if (mime.isValid()) {
            if (mime.isDefault()) {
                return QStringList("*");
            }
            return mime.globPatterns();
        }
        return QStringList();
    }
}

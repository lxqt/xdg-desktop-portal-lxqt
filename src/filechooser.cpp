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

#include "filechooser.h"
#include "utils.h"
#include "filedialoghelper.h"

#include <QDBusArgument>
#include <QDBusMetaType>
#include <QDialogButtonBox>
#include <QFile>
#include <QGridLayout>
#include <QLabel>
#include <QLoggingCategory>
#include <QMimeDatabase>
#include <QUrl>
#include <QCheckBox>
#include <QComboBox>
#include <QDBusObjectPath>
#include <libfm-qt6/filedialog.h>

// Keep in sync with qflatpakfiledialog from flatpak-platform-plugin
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Filter)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Filters)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::FilterList)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::FilterListList)
// used for options - choices
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Choice)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Choices)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::Option)
Q_DECLARE_METATYPE(LXQt::FileChooserPortal::OptionList)

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

    QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Choice &choice)
    {
        arg.beginStructure();
        arg << choice.id << choice.value;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Choice &choice)
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

    QDBusArgument &operator<<(QDBusArgument &arg, const FileChooserPortal::Option &option)
    {
        arg.beginStructure();
        arg << option.id << option.label << option.choices << option.initialChoiceId;
        arg.endStructure();
        return arg;
    }

    const QDBusArgument &operator>>(const QDBusArgument &arg, FileChooserPortal::Option &option)
    {
        QString id;
        QString label;
        FileChooserPortal::Choices choices;
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

    FileChooserPortal::FileChooserPortal(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
        qDBusRegisterMetaType<Filter>();
        qDBusRegisterMetaType<Filters>();
        qDBusRegisterMetaType<FilterList>();
        qDBusRegisterMetaType<FilterListList>();
        qDBusRegisterMetaType<Choice>();
        qDBusRegisterMetaType<Choices>();
        qDBusRegisterMetaType<Option>();
        qDBusRegisterMetaType<OptionList>();
    }

    FileChooserPortal::~FileChooserPortal()
    {
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
            currentFolder = QUrl::fromLocalFile(options.value(QStringLiteral("current_folder")).toString());
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
            currentFolder = QUrl::fromLocalFile(options.value(QStringLiteral("current_folder")).toString());
        }

        if (options.contains(QStringLiteral("current_file"))) {
            currentFile = QUrl::fromLocalFile(options.value(QStringLiteral("current_file")).toString());
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

    QWidget *FileChooserPortal::CreateChoiceControls(const FileChooserPortal::OptionList &optionList,
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

    QVariant FileChooserPortal::EvaluateSelectedChoices(const QMap<QString, QCheckBox *> &checkboxes, const QMap<QString, QComboBox *> &comboboxes)
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

    QString FileChooserPortal::ExtractAcceptLabel(const QVariantMap &options)
    {
        QString acceptLabel;
        if (options.contains(QStringLiteral("accept_label"))) {
            acceptLabel = options.value(QStringLiteral("accept_label")).toString();
            // 'accept_label' allows mnemonic underlines, but Qt uses '&' character, so replace/escape accordingly
            // to keep literal '&'s and transform mnemonic underlines to the Qt equivalent using '&' for mnemonic
            acceptLabel.replace(QChar::fromLatin1('&'), QStringLiteral("&&"));
            const int mnemonic_pos = acceptLabel.indexOf(QChar::fromLatin1('_'));
            if (mnemonic_pos != -1) {
                acceptLabel.replace(mnemonic_pos, 1, QChar::fromLatin1('&'));
            }
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

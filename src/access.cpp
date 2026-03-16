/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
 *
 * Copyright: 2026~ LXQt team
 * Authors:
 *   Basil Crow <me@basilcrow.com>
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

#include "access.h"
#include "choices.h"
#include "utils.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDBusArgument>
#include <QDBusObjectPath>
#include <QDialog>
#include <QDialogButtonBox>
#include <QEventLoop>
#include <QIcon>
#include <QLabel>
#include <QLoggingCategory>
#include <QPushButton>
#include <QVBoxLayout>

namespace LXQt
{
    Q_LOGGING_CATEGORY(XdgDesktopPortalLxqtAccess, "xdp-lxqt-access")

    AccessPortal::AccessPortal(QObject *parent)
        : QDBusAbstractAdaptor(parent)
    {
        registerChoiceMetaTypes();
    }

    uint AccessPortal::AccessDialog(const QDBusObjectPath &handle,
            const QString &app_id,
            const QString &parent_window,
            const QString &title,
            const QString &subtitle,
            const QString &body,
            const QVariantMap &options,
            QVariantMap &results)
    {
        Q_UNUSED(app_id);

        qCDebug(XdgDesktopPortalLxqtAccess) << "AccessDialog called with parameters:";
        qCDebug(XdgDesktopPortalLxqtAccess) << "    handle: " << handle.path();
        qCDebug(XdgDesktopPortalLxqtAccess) << "    parent_window: " << parent_window;
        qCDebug(XdgDesktopPortalLxqtAccess) << "    title: " << title;
        qCDebug(XdgDesktopPortalLxqtAccess) << "    subtitle: " << subtitle;
        qCDebug(XdgDesktopPortalLxqtAccess) << "    body: " << body;
        qCDebug(XdgDesktopPortalLxqtAccess) << "    options: " << options;

        bool modalDialog = true;
        if (options.contains(QStringLiteral("modal"))) {
            modalDialog = options.value(QStringLiteral("modal")).toBool();
        }

        QString grantLabel = tr("Grant Access");
        if (options.contains(QStringLiteral("grant_label"))) {
            grantLabel = options.value(QStringLiteral("grant_label")).toString();
            Utils::convertGtkMnemonic(grantLabel);
        }

        QString denyLabel = tr("Deny Access");
        if (options.contains(QStringLiteral("deny_label"))) {
            denyLabel = options.value(QStringLiteral("deny_label")).toString();
            Utils::convertGtkMnemonic(denyLabel);
        }

        // for handling of options - choices
        QMap<QString, QCheckBox *> checkboxes;
        QMap<QString, QComboBox *> comboboxes;
        std::unique_ptr<QWidget> choiceControls;
        bool hasChoices = false;

        if (options.contains(QStringLiteral("choices"))) {
            OptionList optionList =
                qdbus_cast<OptionList>(options.value(QStringLiteral("choices")));
            choiceControls.reset(CreateChoiceControls(optionList, checkboxes, comboboxes));
            hasChoices = choiceControls != nullptr;
        }

        QDialog dialog;
        dialog.setWindowTitle(title);
        dialog.setWindowModality(modalDialog ? Qt::ApplicationModal : Qt::NonModal);
        Utils::setParentWindow(&dialog, parent_window);

        QVBoxLayout *layout = new QVBoxLayout(&dialog);

        // Icon
        if (options.contains(QStringLiteral("icon"))) {
            const QString iconName = options.value(QStringLiteral("icon")).toString();
            const QIcon icon = QIcon::fromTheme(iconName);
            if (!icon.isNull()) {
                QLabel *iconLabel = new QLabel(&dialog);
                iconLabel->setPixmap(icon.pixmap(48, 48));
                iconLabel->setAlignment(Qt::AlignCenter);
                layout->addWidget(iconLabel);
            }
        }

        // Subtitle (bold/larger)
        if (!subtitle.isEmpty()) {
            QLabel *subtitleLabel = new QLabel(&dialog);
            QFont boldFont = subtitleLabel->font();
            boldFont.setBold(true);
            boldFont.setPointSizeF(boldFont.pointSizeF() * 1.2);
            subtitleLabel->setFont(boldFont);
            subtitleLabel->setTextFormat(Qt::PlainText);
            subtitleLabel->setText(subtitle);
            subtitleLabel->setWordWrap(true);
            layout->addWidget(subtitleLabel);
        }

        // Body text
        if (!body.isEmpty()) {
            QLabel *bodyLabel = new QLabel(&dialog);
            bodyLabel->setTextFormat(Qt::PlainText);
            bodyLabel->setText(body);
            bodyLabel->setWordWrap(true);
            layout->addWidget(bodyLabel);
        }

        // Choice controls
        if (hasChoices) {
            layout->addWidget(choiceControls.release());
        }

        // Buttons
        QDialogButtonBox *buttonBox = new QDialogButtonBox(&dialog);
        QPushButton *grantButton = buttonBox->addButton(grantLabel, QDialogButtonBox::AcceptRole);
        buttonBox->addButton(denyLabel, QDialogButtonBox::RejectRole);
        grantButton->setDefault(true);
        QObject::connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        QObject::connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
        layout->addWidget(buttonBox);

        QEventLoop loop;
        QObject::connect(&dialog, &QDialog::finished, &loop, &QEventLoop::quit);
        dialog.open();
        loop.exec();

        if (dialog.result() == QDialog::Accepted) {
            if (hasChoices) {
                QVariant choices = EvaluateSelectedChoices(checkboxes, comboboxes);
                results.insert(QStringLiteral("choices"), choices);
            }
            return 0;
        }

        return 1;
    }
}

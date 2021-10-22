/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
 *
 * Copyright: 2016 Red Hat Inc
 * Copyright: 2016 Jan Grulich <jgrulich@redhat.com>
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

#include <QApplication>
#include <QDBusConnection>
#include <QLoggingCategory>

#include "desktopportal.h"

Q_LOGGING_CATEGORY(XdgDesktopPortalLxqt, "xdp-lxqt")

int main(int argc, char *argv[])
{
    QCoreApplication::setAttribute(Qt::AA_DisableSessionManager);
    QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    QApplication a{argc, argv};
    a.setApplicationName(QStringLiteral("xdg-desktop-portal-lxqt"));
    a.setQuitOnLastWindowClosed(false);

    QDBusConnection sessionBus = QDBusConnection::sessionBus();

    if (sessionBus.registerService(QStringLiteral("org.freedesktop.impl.portal.desktop.lxqt"))) {
        const auto desktopPortal = new LXQt::DesktopPortal{&a};
        if (sessionBus.registerObject(QStringLiteral("/org/freedesktop/portal/desktop"), desktopPortal, QDBusConnection::ExportAdaptors)) {
            qCDebug(XdgDesktopPortalLxqt) << "Desktop portal registered successfully";
        } else {
            qCDebug(XdgDesktopPortalLxqt) << "Failed to register desktop portal";
        }
    } else {
        qCDebug(XdgDesktopPortalLxqt) << "Failed to register org.freedesktop.impl.portal.desktop.lxqt service";
        return 1;
    }

    return a.exec();
}

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

#pragma once

#include <QDBusAbstractAdaptor>

class QDBusObjectPath;

namespace LXQt
{
    class AccessPortal : public QDBusAbstractAdaptor
    {
        Q_OBJECT
        Q_CLASSINFO("D-Bus Interface", "org.freedesktop.impl.portal.Access")
    public:
        explicit AccessPortal(QObject *parent);

    public Q_SLOTS:
        uint AccessDialog(const QDBusObjectPath &handle,
                const QString &app_id,
                const QString &parent_window,
                const QString &title,
                const QString &subtitle,
                const QString &body,
                const QVariantMap &options,
                QVariantMap &results);
    };
}

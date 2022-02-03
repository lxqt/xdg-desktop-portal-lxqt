/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt-project.org
 *
 * Copyright: 2022~ LXQt team
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

#include <libfm-qt/filedialoghelper.h>
#include <libfm-qt/filedialog.h>
#include <memory>
#include <QFileDialog>

namespace Fm
{
    class FileDialog;
}

namespace LXQt
{
    class FileDialogHelper : public Fm::FileDialogHelper
    {
    public:
        static std::unique_ptr<FileDialogHelper> createFileDialogHelper();

    private:
        FileDialogHelper() = default;

    public:
        inline Fm::FileDialog & dialog() { return Fm::FileDialogHelper::dialog(); }
        inline void setFileMode(QFileDialog::FileMode mode) { options()->setFileMode(static_cast<QFileDialogOptions::FileMode>(mode)); }
        inline void setWindowTitle(const QString & title) { options()->setWindowTitle(title); }
        inline void setModal(bool modal) { dialog().setModal(modal); }
        inline void setLabelText(QFileDialog::DialogLabel label, const QString &text) { options()->setLabelText(static_cast<QFileDialogOptions::DialogLabel>(label), text); };
        inline void setMimeTypeFilters(const QStringList &filters) { options()->setMimeTypeFilters(filters); }
        inline void setNameFilters(const QStringList &filters) { options()->setNameFilters(filters); }
        inline void setAcceptMode(QFileDialog::AcceptMode mode) { options()->setAcceptMode(static_cast<QFileDialogOptions::AcceptMode>(mode)); }
        int execResult();

    };
}

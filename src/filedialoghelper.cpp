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

#include "filedialoghelper.h"
#include <libfm-qt6/libfmqt.h>
#include <QCoreApplication>
#include <QWindow>

namespace LXQt
{
    /*static*/ std::unique_ptr<FileDialogHelper> FileDialogHelper::createFileDialogHelper()
    {
        static std::unique_ptr<Fm::LibFmQt> libfmQtContext_;
        if(!libfmQtContext_) {
            // initialize libfm-qt only once
            libfmQtContext_ = std::unique_ptr<Fm::LibFmQt>{new Fm::LibFmQt()};
            // add translations
            QCoreApplication::installTranslator(libfmQtContext_.get()->translator());
        }
        auto d = std::unique_ptr<FileDialogHelper>{new FileDialogHelper{}};
        d->setOptions(QFileDialogOptions::create());
        return d;
    }

    int FileDialogHelper::execResult()
    {
        show(dialog().windowFlags(), dialog().windowModality(), dialog().windowHandle() ? dialog().windowHandle()->transientParent() : nullptr);
        Fm::FileDialogHelper::exec();
        return dialog().result();
    }

}

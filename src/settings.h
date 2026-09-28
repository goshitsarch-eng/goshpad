// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <QString>

class Settings
{
public:
    struct Values {
        QString colorScheme = QStringLiteral("system");
        bool wordWrap = false;
        bool showStatusBar = true;
        QString fontFamily = QStringLiteral("monospace");
        int fontSize = 14;
    };

    static Values load();
    static void save(const Values &values);
};

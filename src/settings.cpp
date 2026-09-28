// SPDX-License-Identifier: GPL-3.0-or-later

#include "settings.h"

#include <KConfigGroup>
#include <KSharedConfig>

namespace {

KSharedConfig::Ptr config()
{
    return KSharedConfig::openConfig(QStringLiteral("com.goshapps.GoshPadrc"));
}

} // namespace

Settings::Values Settings::load()
{
    const KConfigGroup group(config(), QStringLiteral("Editor"));
    Values values;
    const QString scheme = group.readEntry("color_scheme", values.colorScheme);
    if (scheme == QLatin1String("light") || scheme == QLatin1String("dark") || scheme == QLatin1String("system")) {
        values.colorScheme = scheme;
    }
    values.wordWrap = group.readEntry("word_wrap", values.wordWrap);
    values.showStatusBar = group.readEntry("show_status_bar", values.showStatusBar);
    const QString family = group.readEntry("font_family", values.fontFamily);
    if (!family.trimmed().isEmpty()) {
        values.fontFamily = family;
    }
    const int size = group.readEntry("font_size", values.fontSize);
    if (size >= 1 && size <= 512) {
        values.fontSize = size;
    }
    return values;
}

void Settings::save(const Values &values)
{
    KConfigGroup group(config(), QStringLiteral("Editor"));
    group.writeEntry("color_scheme", values.colorScheme);
    group.writeEntry("word_wrap", values.wordWrap);
    group.writeEntry("show_status_bar", values.showStatusBar);
    group.writeEntry("font_family", values.fontFamily);
    group.writeEntry("font_size", values.fontSize);
    group.sync();
}

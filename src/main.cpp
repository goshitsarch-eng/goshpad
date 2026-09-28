// SPDX-License-Identifier: GPL-3.0-or-later

#include "controller.h"

#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QGuiApplication>
#include <QIcon>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include <QQuickStyle>
#include <QUrl>

#include <KAboutData>
#include <KDBusService>
#include <KLocalizedQmlContext>
#include <KLocalizedString>

namespace {

QString firstLaunchPath(int argc, char *argv[])
{
    const QString cwd = QDir::currentPath();
    for (int index = 1; index < argc; ++index) {
        const QString argument = QString::fromLocal8Bit(argv[index]);
        if (argument.isEmpty() || argument.startsWith(u'-')) {
            continue;
        }
        const QFileInfo info(argument);
        return info.isAbsolute() ? info.absoluteFilePath() : QFileInfo(QDir(cwd).filePath(argument)).absoluteFilePath();
    }
    return {};
}

} // namespace

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    QQuickStyle::setStyle(QStringLiteral("org.kde.desktop"));

    KLocalizedString::setApplicationDomain("goshpad");

    KAboutData about(QStringLiteral("goshpad"),
                     i18n("GoshPad"),
                     QStringLiteral(GOSHPAD_VERSION),
                     i18n("A plain-text editor in the style of classic Microsoft Notepad"),
                     KAboutLicense::GPL_V3,
                     i18n("© 2026 Gosh and GoshPad contributors"));
    about.setHomepage(QStringLiteral("https://github.com/goshitsarch-eng/goshpad"));
    about.setBugAddress(QByteArrayLiteral("https://github.com/goshitsarch-eng/goshpad/issues"));
    about.setOrganizationDomain(QByteArrayLiteral("goshapps.com"));
    about.setDesktopFileName(QStringLiteral("com.goshapps.GoshPad"));
    KAboutData::setApplicationData(about);

    const QIcon icon(QStringLiteral(GOSHPAD_ICON_PATH));
    if (!icon.isNull()) {
        QGuiApplication::setWindowIcon(icon);
    }

    Controller controller;
    controller.startup(firstLaunchPath(argc, argv));

    KDBusService service(KDBusService::Unique | KDBusService::NoExitOnFailure);
    QObject::connect(&service, &KDBusService::activateRequested, &controller, [&controller](const QStringList &arguments, const QString &workingDirectory) {
        controller.activate(arguments, workingDirectory);
    });
    QObject::connect(&service, &KDBusService::openRequested, &controller, [&controller](const QList<QUrl> &urls) {
        controller.activateUrls(urls);
    });

    QQmlApplicationEngine engine;
    KLocalization::setupLocalizedContext(&engine);
    engine.rootContext()->setContextProperty(QStringLiteral("controller"), &controller);
    QObject::connect(&engine, &QQmlApplicationEngine::objectCreationFailed, &app, []() {
        qCritical().noquote() << QStringLiteral("GoshPad failed to load its QML interface.");
    });
    engine.loadFromModule("com.goshapps.goshpad", "Main");
    if (engine.rootObjects().isEmpty()) {
        qCritical() << "GoshPad failed to create its window.";
        return 1;
    }
    return app.exec();
}

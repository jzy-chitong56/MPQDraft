/*
    MPQDraft Qt GUI - GUI Entry Point

    This is the Qt-based GUI for MPQDraft, replacing the old MFC GUI.
    It provides a modern, cross-platform interface while maintaining
    compatibility with the existing plugin system.
*/

#include "main_gui.h"
#include <QApplication>
#include <QMessageBox>
#include <QIcon>
#include <QTranslator>
#include <QLocale>
#include <QLibraryInfo>
#include <QDir>
#include <QFileInfo>
#include <QStandardPaths>
#include <QFile>
#include <QSettings>
#include <QDebug>
#include "mainwindow.h"

// Core Qt GUI initialization - used by both standalone and integrated builds
static int runQtGuiCore(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set application metadata
    // Use same organization name as original MFC app for registry compatibility on Windows
    QApplication::setOrganizationName("Team MoPaQ");
    QApplication::setApplicationName("MPQDraft");
    QApplication::setApplicationVersion("1.0");

    // ------------------------------------------------------------------------
    // Determine preferred language
    // ------------------------------------------------------------------------
    QSettings settings;
    const QString overrideLang = settings.value("language/override", QString()).toString();

    QStringList preferredLangs;
    if (!overrideLang.isEmpty()) {
        preferredLangs << overrideLang;
    }
    const QString sysFull = QLocale::system().name();           // e.g. "zh_CN"
    const QString sysShort = sysFull.left(2);                   // e.g. "zh"
    if (!sysFull.isEmpty())    preferredLangs << sysFull;
    if (!sysShort.isEmpty() && !preferredLangs.contains(sysShort)) preferredLangs << sysShort;

    // Always fall back to English if nothing else matches
    preferredLangs << QStringLiteral("en");

    // Dedup while preserving order
    QStringList seen;
    for (const QString &l : preferredLangs) {
        if (!l.isEmpty() && !seen.contains(l)) seen << l;
    }
    preferredLangs = seen;

    qDebug() << "MPQDraft translation search list:" << preferredLangs;

    QStringList searchPaths = {
        QApplication::applicationDirPath(),
        QApplication::applicationDirPath() + "/translations",
        QCoreApplication::applicationDirPath() + "/../translations",
        ":/translations"
    };

    // Also search Qt's own translations directory
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    const QString qtTransDir = QLibraryInfo::path(QLibraryInfo::TranslationsPath);
#else
    const QString qtTransDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
#endif

    // ------------------------------------------------------------------------
    // Load Qt's built-in translations (QWizard buttons, standard dialogs, etc.)
    // Try both "qtbase_" (Qt 5.15+ split) and "qt_" (legacy) prefixes.
    // ------------------------------------------------------------------------
    QTranslator qtTranslator;
    for (const QString &lang : preferredLangs) {
        if (lang == "en") break;   // English is built-in
        for (const QString &prefix : {QStringLiteral("qtbase_"), QStringLiteral("qt_")}) {
            const QString baseName = prefix + lang;
            for (const QString &path : searchPaths) {
                if (qtTranslator.load(baseName, path)) {
                    app.installTranslator(&qtTranslator);
                    qDebug() << "MPQDraft: loaded Qt translation" << baseName << "from" << path;
                    goto qt_done;
                }
            }
        }
        // Also try Qt's own directory
        for (const QString &prefix : {QStringLiteral("qtbase_"), QStringLiteral("qt_")}) {
            const QString baseName = prefix + lang;
            if (qtTranslator.load(baseName, qtTransDir)) {
                app.installTranslator(&qtTranslator);
                qDebug() << "MPQDraft: loaded Qt translation" << baseName << "from" << qtTransDir;
                goto qt_done;
            }
        }
    }
qt_done:

    // ------------------------------------------------------------------------
    // Load MPQDraft translations
    // ------------------------------------------------------------------------
    QTranslator appTranslator;
    bool translatorLoaded = false;
    for (const QString &lang : preferredLangs) {
        if (lang == "en") {
            qDebug() << "MPQDraft: using English (source) language";
            translatorLoaded = true;
            break;
        }
        const QString baseName = QString("mpqdraft_%1").arg(lang);
        for (const QString &path : searchPaths) {
            if (appTranslator.load(baseName, path)) {
                app.installTranslator(&appTranslator);
                qDebug() << "MPQDraft: loaded translation" << baseName << "from" << path;
                translatorLoaded = true;
                break;
            }
        }
        if (translatorLoaded) break;
    }
    if (!translatorLoaded) {
        qWarning() << "MPQDraft: no translation loaded, falling back to English source text";
    }

    // Set the application icon (window and taskbar)
    QIcon appIcon(":/icons/mpqdraft.ico");
    app.setWindowIcon(appIcon);

    // Create and show the main window
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

// Entry point for integrated build (called from MPQDraft.cpp)
int runQtGui(int argc, char *argv[])
{
    return runQtGuiCore(argc, argv);
}

// Standalone entry point for Qt-only builds (CMake/Linux development)
#ifndef MPQDRAFT_INTEGRATED_BUILD
int main(int argc, char *argv[])
{
    return runQtGuiCore(argc, argv);
}
#endif

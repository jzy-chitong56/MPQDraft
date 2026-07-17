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

    // Load Qt's built-in translations for standard dialogs (e.g., QFileDialog)
    QTranslator qtTranslator;
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    if (qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::path(QLibraryInfo::TranslationsPath))) {
#else
    if (qtTranslator.load(QLocale(), "qt", "_", QLibraryInfo::location(QLibraryInfo::TranslationsPath))) {
#endif
        app.installTranslator(&qtTranslator);
    }

    // ------------------------------------------------------------------------
    // Load MPQDraft translations
    //
    // Resolution order:
    //   1. User override saved in QSettings (e.g. "zh_CN", "ko_KR", "en")
    //   2. System locale (full name like "zh_CN", then short "zh")
    //   3. Several filesystem locations + the embedded :/translations resource
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

    // App translator must live for the entire QApplication lifetime
    QTranslator appTranslator;
    bool translatorLoaded = false;
    for (const QString &lang : preferredLangs) {
        if (lang == "en") {
            // English is the source language, no .qm needed
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

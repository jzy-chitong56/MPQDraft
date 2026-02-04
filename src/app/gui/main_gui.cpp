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

    // Load MPQDraft translations
    QTranslator appTranslator;
    // Try to load from several locations:
    // 1. Next to the executable (for deployed apps)
    // 2. In a translations subdirectory next to the executable
    // 3. In the build directory (for development)
    QString translationFile = QString("mpqdraft_%1").arg(QLocale().name().left(2));
    QStringList searchPaths = {
        QApplication::applicationDirPath(),
        QApplication::applicationDirPath() + "/translations",
        ":/translations"  // Embedded in resources (if added later)
    };

    for (const QString &path : searchPaths) {
        if (appTranslator.load(translationFile, path)) {
            app.installTranslator(&appTranslator);
            break;
        }
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

/*
    MainWindow - Main menu for MPQDraft

    Provides two main options:
    1. Load MPQs and Patch - Opens the patch wizard
    2. Create Self-Executing MPQ - Opens the SEMPQ wizard
*/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPushButton>
#include <QActionGroup>

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void onPatchClicked();
    void onSEMPQClicked();
    void onLanguageChanged(QAction *action);

private:
    void setupUI();
    void setupMenuBar();

    // UI components
    QPushButton *patchButton;
    QPushButton *sempqButton;

    // Language menu state
    QActionGroup *languageGroup;
};

#endif // MAINWINDOW_H

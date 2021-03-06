/// \file MainWindow.h
/// \class MainWindow
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _MAIN_WINDOW_H_
#define _MAIN_WINDOW_H_

//UI include
#include "ui_MainWindow.h"

//Qt includes
#include <QMainWindow>

//Project includes
#include "ModuleTabWidget.h"

namespace Ui {
    class MainWindow;
}

/// Main class of the project
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    /// Get the unique instance of the main window
    static MainWindow* GetInstance();
    void SetFullScreen(bool pFullScreen);

protected:
    /// Protected constructor for the singleton class
    explicit MainWindow(QWidget *pParent = 0);
    /// Destructor called when the main window is closed
    ~MainWindow();
    void keyPressEvent(QKeyEvent *pEvent);

private:
    /// Unique instance of the main window
    static MainWindow* mInstance;
    /// User interface instance
    Ui::MainWindow *mUi;
    /// Widget that handle multiple tabs
    ModuleTabWidget* mModuleTabWidget;

    bool mFullScreen;
    bool mWasMaximized;

private slots:
    /// Method called when the user change from one module to another
    void ModuleChanged(int pIndex);

    //Menu
    void LoadMainModule();
    /// Show information about the program
    void ShowAbout();
};

#endif

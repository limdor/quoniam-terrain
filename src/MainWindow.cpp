//Definition include
#include "MainWindow.h"

//System includes
#include <time.h>

//Qt includes
#include <QMessageBox>

//Project includes
#include "Debug.h"
#include "MainModuleController.h"

MainWindow* MainWindow::mInstance = NULL;

MainWindow* MainWindow::GetInstance()
{
    if(mInstance == NULL)
    {
        mInstance = new MainWindow();
    }
    return mInstance;
}

void MainWindow::SetFullScreen(bool pFullScreen)
{
    if(mFullScreen != pFullScreen)
    {
        mFullScreen = pFullScreen;
        if(mFullScreen)
        {
            mWasMaximized = isMaximized();
            showFullScreen();
        }
        else
        {
            if(mWasMaximized)
            {
                showMaximized();
            }
            else
            {
                showNormal();
            }
        }
    }
}

MainWindow::MainWindow(QWidget *pParent) :
    QMainWindow(pParent),
    mUi(new Ui::MainWindow)
{
    mUi->setupUi(this);

    srand( time(NULL) );

    mModuleTabWidget = new ModuleTabWidget(this);
    setCentralWidget(mModuleTabWidget);
    connect(mModuleTabWidget, SIGNAL(currentChanged(int)), this, SLOT(ModuleChanged(int)));

    LoadMainModule();

    mFullScreen = false;
}

MainWindow::~MainWindow()
{
    delete mUi;
    mModuleTabWidget->RemoveAllModules();
    delete mModuleTabWidget;
}

void MainWindow::keyPressEvent(QKeyEvent *pEvent)
{
    if( pEvent->key() == Qt::Key_F11 )
    {
        SetFullScreen(!mFullScreen);
    }
}

void MainWindow::ModuleChanged(int pIndex)
{
    Debug::SetConsole(NULL);
    menuBar()->clear();
    if(pIndex >= 0)
    {
        // We load the menus of the current module
        ModuleController* currentModule = dynamic_cast<ModuleController*>(mModuleTabWidget->currentWidget());

        currentModule->ActiveModule();

        QVector<QMenu*> menus = currentModule->GetModuleMenus();
        for( int i = 0; i < menus.size(); i++ )
        {
            menuBar()->addMenu(menus.at(i));
        }
    }

    // We load the menus of the main application
    QMenu* menuLoadModule = menuBar()->addMenu("Window");

    QAction* actionMainModule = new QAction("New tab", this);
    actionMainModule->setShortcut(Qt::CTRL + Qt::Key_T);
    menuLoadModule->addAction(actionMainModule);
    connect(actionMainModule, SIGNAL(triggered()), this, SLOT(LoadMainModule()));

    QMenu* menuHelp = menuBar()->addMenu("&Help");

    QAction* actionAbout = new QAction("&About", this);
    actionAbout->setShortcut(Qt::Key_F1);
    menuHelp->addAction(actionAbout);
    connect(actionAbout, SIGNAL(triggered()), this, SLOT(ShowAbout()));
}

/*****************
 * Private slots *
 *****************/
void MainWindow::LoadMainModule()
{
    mModuleTabWidget->LoadModule(new MainModuleController(mModuleTabWidget), "Main module", "MainModule");
}

void MainWindow::ShowAbout()
{
    QMessageBox::about(this, tr("About Quoniam v%1").arg(APP_VERSION),
                       tr("<b>Quoniam</b> is an application of viewpoint selection and "
                          "object exploration developed by the University of Girona."
                          "<br><br>More information: xavierb@imae.udg.edu"));
}

#include "ModuleController.h"

ModuleController::ModuleController(QWidget *pParent) :
    QWidget(pParent)
{

}

ModuleController::~ModuleController()
{
    for(int i = 0; i < mMenus.size(); i++)
    {
        delete mMenus[i];
    }
}

QVector<QMenu*> ModuleController::GetModuleMenus() const
{
    return mMenus;
}

void ModuleController::CreateModuleMenus()
{

}

void ModuleController::ActiveModule()
{

}

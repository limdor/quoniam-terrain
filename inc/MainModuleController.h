/// \file MainModuleController.h
/// \class MainModuleController
/// \author Xavier Bonaventura
/// \author Copyright: (c) Universitat de Girona

#ifndef _MAIN_MODULE_CONTROLLER_H_
#define _MAIN_MODULE_CONTROLLER_H_

//UI include
#include "ui_MainModule.h"

//Qt includes
#include <QSignalMapper>

//Dependency includes
#include "VisibilityChannelHistogram.h"

//Project includes
#include "GLCanvas.h"
#include "ModuleController.h"
#include "NBestViews.h"
#include "ObscuranceMap.h"

namespace Ui {
    class MainModule;
}

/// Controller for the view of the main module
class MainModuleController : public ModuleController
{
    Q_OBJECT
public:
    explicit MainModuleController(QWidget *pParent = 0);
    ~MainModuleController();

    void CreateModuleMenus();
    void ActiveModule();

protected:
    void keyPressEvent(QKeyEvent *pEvent);
    void mouseMoveEvent(QMouseEvent *pEvent);
    void wheelEvent(QWheelEvent *pEvent);
    void mousePressEvent(QMouseEvent *pEvent);

private:
    /// Scene related methods
    void LoadScene(const QString &pFileName);
    void ComputeObscurances();

    /// Mesh of viewpoints related methods
    void LoadViewpoints();
    void LoadViewpointsFromFile(const QString &pFileName);
    void LoadViewpointsFromSphere(float pRadius, float pAngle, int pSubdivision);
    void ChangeNumberOfViewpoints(int pNumberOfViewpoints);
    void SaveViewpointMeasuresInformation(const QString &pFileName);
    int NextViewpoint();
    void SetViewpoint(int pViewpoint);
    void ShowViewpointInformation(int pViewpoint);

    /// Altres
    void SetFullScreen(bool pFullScreen);
    QVector<bool> GetPolygonalVisibility( int pViewpoint, int pVisibilityCriteria );
    void ReloadVisibilityTexture(int pViewpoint, int pVisibilityCriteria);
    void UpdateRenderingGUI();
    void UpdateOthersGUI();
    QString GetScreenshotName(int pViewpoint);

    QMenu* mMenuVisualization;
    QAction* mActionExport;
    QAction* mActionViewpointsSphere;


    Ui::MainModule* mUi;

    GLCanvas *mOpenGLCanvas;
    Scene *mScene;
    QVector<unsigned int> mMaxAreaPolygon;
    ViewpointsMesh *mViewpointsMesh;
    VisibilityChannelHistogram* mHistogram;
    NBestViews* mNBestViews;

    bool mUpdateView;
    bool mFullScreen;

    int mCurrentViewpoint;

    GLuint mPolygonalTexture;
    GLuint mPolygonalVisibilityTexture;
    ObscuranceMap* mObscurancesGenerator;
    bool mObscurancesComputed;
    GLuint mObscurancesTexture;
    QVector<float> mObscurancesPerPolygon;
    GLuint mObscurancesPerPolygonTexture;
    unsigned int mPolygonalTexturesSize;

    bool mViewPolygonalVisibility;

    PolygonalI1* mPolygonalI1;
    PolygonalI2* mPolygonalI2;
    PolygonalI3* mPolygonalI3;
    ProjectedLocalMeasurePVO* mProjectedI1;
    ProjectedLocalMeasurePVO* mProjectedI2;
    ProjectedLocalMeasurePVO* mProjectedI3;

    QVector<Measure*> mPolygonalMeasures;
    QVector<Measure*> mViewpointMeasures;

    QVector<QSlider*> mViewpointMeasuresSliders;

    /// Posició del mouse
    QPoint mLastMousePosition;

    QString mScenePath;

private slots:
    //Menu
    void OpenModel();
    void ExportInformation();
    void WillDrawViewpointsSphere(bool pDraw);

    //Right panel
    void on_measureInViewpointSphereList_currentIndexChanged(int pValue);
    void on_polygonalInformationComboBox_currentIndexChanged(int pIndex);
    void on_alphaSlider_valueChanged(int pValue);
    void on_alphaSpinBox_valueChanged(double pValue);
    void on_powerSlider_valueChanged(int pValue);
    void on_powerSpinBox_valueChanged(double pValue);
    void on_luminanceSlider_valueChanged(int pValue);
    void on_luminanceSpinBox_valueChanged(double pValue);
    void on_weightSlider_valueChanged(int pValue);
    void on_weightSpinBox_valueChanged(double pValue);
    void on_takeScreenshotsButton_clicked();
    void on_takeScreenshotsFromAllViewpointsButton_clicked();

    void on_nBestViewsComputeButton_clicked();
    void on_nBestViewsComputeAllButton_clicked();

    void SliderChanged(int pMeasure, int pValue);

    void on_loadViewpointsSphereButton_clicked();
    void on_loadFreeViewpointsMeshButton_clicked();
    void on_viewVisiblePolygonsCheckBox_clicked(bool pChecked);

    void on_computeObscurancesAndAmbientOcclusionButton_clicked();
    void on_computeCameraDistanceButton_clicked();
    void on_computeCameraAngleButton_clicked();
    void on_rgbCheckBox_clicked(bool pChecked);
    void on_polygonalInformationCheckBox_clicked(bool pChecked);
    void on_texturedCheckBox_clicked(bool pChecked);
};

#endif

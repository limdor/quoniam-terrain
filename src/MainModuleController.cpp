//Definition include
#include "MainModuleController.h"

//Qt includes
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressDialog>
#include <QTextStream>
#include <QTime>
#include <QXmlStreamWriter>

//Dependency includes
#include "glm/mat4x4.hpp"
#include "glm/vec3.hpp"
#include "glm/geometric.hpp"
#include "glm/trigonometric.hpp"
#include "glm/exponential.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtx/rotate_vector.hpp"

//Project includes
#include "Debug.h"
#include "HistogramBuilder.h"
#include "MainWindow.h"
#include "OrthographicCamera.h"
#include "ProjectedLocalMeasurePVO.h"
#include "SceneLoader.h"
#include "Tools.h"
#include "ViewpointMeasureSlider.h"

MainModuleController::MainModuleController(QWidget *pParent) :
    ModuleController(pParent), mUi(new Ui::MainModule)
{
    mUi->setupUi(this);

    //Configuració de la consola
    Debug::SetConsole(mUi->console);

    //Inicialització del GLCanvas
    mOpenGLCanvas = mUi->Canvas;
    mOpenGLCanvas->show();
    mOpenGLCanvas->updateGL();

    QWidget* parent = mOpenGLCanvas;
    while(parent != 0)
    {
        parent->setMouseTracking(true);
        parent = parent->parentWidget();
    }
    CreateModuleMenus();

    qApp->processEvents();

    //Inicialització dels uniforms dels shaders
    mUi->powerSpinBox->setMinimum( -1.0f / ( mUi->powerSlider->minimum() / 100 - 1.0f ) );
    mUi->powerSpinBox->setMaximum( mUi->powerSlider->maximum() / 100 + 1.0f );
    on_powerSlider_valueChanged(mUi->powerSlider->value());
    on_powerSpinBox_valueChanged(mUi->powerSpinBox->value());
    mOpenGLCanvas->GetShaderProgram()->SetUniform("faceCulling", mUi->faceCullingCheckBox->isChecked());
    mOpenGLCanvas->GetShaderProgram()->SetUniform("onlyPolygonalInformation", mUi->polygonalInformationCheckBox->isChecked() && !mUi->texturedCheckBox->isChecked());
    mOpenGLCanvas->GetShaderProgram()->SetUniform("applyPolygonalInformation", mUi->polygonalInformationCheckBox->isChecked());
    mOpenGLCanvas->GetShaderProgram()->SetUniform("drawRGB", mUi->rgbCheckBox->isChecked());
    on_luminanceSlider_valueChanged(mUi->luminanceSlider->value());
    on_luminanceSpinBox_valueChanged(mUi->luminanceSpinBox->value());
    mOpenGLCanvas->GetShaderProgram()->SetUniform("weight", 0.5f);
    mOpenGLCanvas->GetShaderProgram()->SetUniform("viewPolygonalVisibility", mUi->viewVisiblePolygonsCheckBox->isChecked());

    mUpdateView = false;
    mFullScreen = false;

    mNBestViews = NULL;
    mScene = NULL;
    mViewpointsMesh = NULL;
    mHistogram = NULL;

    mUi->nBestViewsSelectionMeasuresComboBox->addItem( QString("%1 (discarding triangles)").arg( "Projected I1" ) );
    mUi->nBestViewsSelectionMeasuresComboBox->addItem( QString("%1 (discarding triangles)").arg( "Projected I2" ) );
    mUi->nBestViewsSelectionMeasuresComboBox->addItem( QString("%1 (discarding triangles)").arg( "Projected I3" ) );

    mUi->nBestViewsCriteriaForDiscardingComboBox->addItem( "One pixel seen" );
    mUi->nBestViewsCriteriaForDiscardingComboBox->addItem( "% of the theoretical projected area" );

    mPolygonalI1 = new PolygonalI1("Polygonal I1");
    mPolygonalMeasures.push_back( mPolygonalI1 );
    mPolygonalI2 = new PolygonalI2("Polygonal I2");
    mPolygonalMeasures.push_back( mPolygonalI2 );
    mPolygonalI3 = new PolygonalI3("Polygonal I3");
    mPolygonalMeasures.push_back( mPolygonalI3 );

    mProjectedI1 = new ProjectedLocalMeasurePVO("Viewpoint polygonal I1 p(v|o)");
    mProjectedI1->AddDpendencyLocalMeasure(mPolygonalI1);
    mProjectedI1->SetScaleDependencyLocalMeasure(0.0f, 1.0f);
    mViewpointMeasures.push_back(mProjectedI1);
    mProjectedI2 = new ProjectedLocalMeasurePVO("Viewpoint polygonal I2 p(v|o)");
    mProjectedI2->AddDpendencyLocalMeasure(mPolygonalI2);
    mProjectedI2->SetScaleDependencyLocalMeasure(0.0f, 1.0f);
    mViewpointMeasures.push_back(mProjectedI2);
    mProjectedI3 = new ProjectedLocalMeasurePVO("Viewpoint polygonal I3 p(v|o)");
    mProjectedI3->AddDpendencyLocalMeasure(mPolygonalI3);
    mProjectedI3->SetScaleDependencyLocalMeasure(1.0f, 0.0f);
    mViewpointMeasures.push_back(mProjectedI3);

    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        mUi->measureInViewpointSphereList->addItem( mViewpointMeasures.at(i)->GetName() );
    }

    QHBoxLayout* horizontalLayout = new QHBoxLayout();
    QLabel* minimumTextLabel = new QLabel("Minimum");
    QFont font = minimumTextLabel->font();
    font.setBold(true);
    minimumTextLabel->setFont(font);
    minimumTextLabel->setAlignment(Qt::AlignLeft);
    QLabel* maximumTextLabel = new QLabel("Maximum");
    maximumTextLabel->setFont(font);
    maximumTextLabel->setAlignment(Qt::AlignRight);
    horizontalLayout->addWidget(minimumTextLabel);
    horizontalLayout->addWidget(maximumTextLabel);
    QWidget* minMaxWidget = new QWidget();
    minMaxWidget->setLayout(horizontalLayout);
    mViewpointMeasuresSliders.resize( mViewpointMeasures.size() );
    QVBoxLayout* verticalLayout = new QVBoxLayout();
    verticalLayout->addWidget(minMaxWidget);
    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        QLabel* textNameLabel = new QLabel(mViewpointMeasures.at(i)->GetName());
        textNameLabel->setAlignment(Qt::AlignHCenter);
        verticalLayout->addWidget(textNameLabel);
        ViewpointMeasureSlider* slider = new ViewpointMeasureSlider(i, Qt::Horizontal);
        verticalLayout->addWidget(slider);
        mViewpointMeasuresSliders[i] = slider;
        connect(slider, SIGNAL(valueChanged(int,int)), this, SLOT(SliderChanged(int, int)));
    }

    verticalLayout->addStretch();
    QWidget* widget = new QWidget();
    widget->setLayout(verticalLayout);
    mUi->leftTabWidget->addTab(widget, "Mesures");

    //Afegim els diferents nombres de punts de vista en el combo box
    for( int i = 0; i < 4; i++ )
    {
        mUi->numberOfViewpoints->addItem( QString("%1").arg( glm::pow( 2.0f, 2.0f * mUi->numberOfViewpoints->count() ) * 10 + 2 ) );
    }
    mUi->numberOfViewpoints->setCurrentIndex(3);

    //Amagem les parts d'interficie que no es necessiten
    mUi->leftTabWidget->hide();
    mUi->rightTabWidget->hide();

    mUi->bestNViewsByThresholdSlider->setRange( 0, 100 );
    mUi->bestNViewsByThresholdSpinBox->setRange( 0, 100 );

    mPolygonalTexture = 0;
    mPolygonalVisibilityTexture = 0;
    mObscurancesGenerator = NULL;
    mObscurancesTexture = 0;
    mObscurancesPerPolygonTexture = 0;
    mObscurancesComputed = false;

    mViewPolygonalVisibility = false;

    mCurrentViewpoint = 0;

    connect(mUi->bestNViewsSlider, SIGNAL(valueChanged(int)), mUi->bestNViewsSpinBox, SLOT(setValue(int)));
    connect(mUi->bestNViewsSpinBox, SIGNAL(valueChanged(int)), mUi->bestNViewsSlider, SLOT(setValue(int)));
    connect(mUi->bestNViewsByThresholdSlider, SIGNAL(valueChanged(int)), mUi->bestNViewsByThresholdSpinBox, SLOT(setValue(int)));
    connect(mUi->bestNViewsByThresholdSpinBox, SIGNAL(valueChanged(int)), mUi->bestNViewsByThresholdSlider, SLOT(setValue(int)));

    on_computeCameraAngleButton_clicked();

    UpdateRenderingGUI();
}

MainModuleController::~MainModuleController()
{
    for( int i = 0; i < mPolygonalMeasures.size(); i++ )
    {
        delete mPolygonalMeasures.at(i);
    }
    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        delete mViewpointMeasures.at(i);
    }
    //Destrucció de les textures de saliency i obscurances
    glDeleteTextures(1, &mPolygonalTexture);
    glDeleteTextures(1, &mPolygonalVisibilityTexture);
    glDeleteTextures(1, &mObscurancesTexture);
    glDeleteTextures(1, &mObscurancesPerPolygonTexture);

    delete mOpenGLCanvas;
    delete mUi;

    if( mHistogram != NULL )
    {//Scene and mesh of viewpoints loaded
        delete mNBestViews;
        delete mHistogram;
        delete mViewpointsMesh;
        delete mScene;
    }
    else if( mScene != NULL )
    {//Only scene loaded
        delete mScene;
    }
}

void MainModuleController::CreateModuleMenus()
{
    QMenu* menuFile = new QMenu("&File");

    QAction* actionOpen = new QAction("&Open...", this);
    actionOpen->setShortcut(Qt::CTRL + Qt::Key_O);
    menuFile->addAction(actionOpen);
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(OpenModel()));

    mActionExport = new QAction("&Export...", this);
    mActionExport->setShortcut(Qt::CTRL + Qt::Key_E);
    menuFile->addAction(mActionExport);
    connect(mActionExport, SIGNAL(triggered()), this, SLOT(ExportInformation()));

    QAction* actionQuit = new QAction("&Quit", this);
    actionQuit->setShortcut(Qt::CTRL + Qt::Key_Q);
    menuFile->addAction(actionQuit);
    connect(actionQuit, SIGNAL(triggered()), this, SLOT(close()));

    mMenuVisualization = new QMenu("&Visualization");

    QAction* actionBoundingBoxes = new QAction("Bounding &Boxes", this);
    actionBoundingBoxes->setCheckable(true);
    actionBoundingBoxes->setShortcut(Qt::CTRL + Qt::Key_B);
    mMenuVisualization->addAction(actionBoundingBoxes);
    connect(actionBoundingBoxes, SIGNAL(triggered(bool)), mOpenGLCanvas, SLOT(WillDrawBoundingBox(bool)));

    QAction* actionBoundingSpheres = new QAction("Bounding &Spheres", this);
    actionBoundingSpheres->setCheckable(true);
    actionBoundingSpheres->setShortcut(Qt::CTRL + Qt::Key_S);
    mMenuVisualization->addAction(actionBoundingSpheres);
    connect(actionBoundingSpheres, SIGNAL(triggered(bool)), mOpenGLCanvas, SLOT(WillDrawBoundingSphere(bool)));

    mActionViewpointsSphere = new QAction("&Viewpoints Sphere", this);
    mActionViewpointsSphere->setCheckable(true);
    mActionViewpointsSphere->setShortcut(Qt::CTRL + Qt::Key_V);
    mMenuVisualization->addAction(mActionViewpointsSphere);
    connect(mActionViewpointsSphere, SIGNAL(triggered(bool)), this, SLOT(WillDrawViewpointsSphere(bool)));

    mMenus.push_back(menuFile);
    mMenus.push_back(mMenuVisualization);

    mMenuVisualization->setEnabled(false);
    mActionExport->setEnabled(false);
}

void MainModuleController::ActiveModule()
{
    Debug::SetConsole(mUi->console);
}

void MainModuleController::keyPressEvent(QKeyEvent *pEvent)
{
    if( pEvent->key() == Qt::Key_N && mViewpointsMesh != NULL )
    {
        int viewpoint = NextViewpoint();
        Debug::Log(QString("Viewpoint %1 selected").arg(mViewpointsMesh->GetViewpoint(viewpoint)->mName));

        mUpdateView = false;
        for( int i = 0; i < mViewpointMeasuresSliders.size(); i++ )
        {
            mViewpointMeasuresSliders.at(i)->setValue(mViewpointMeasures.at(i)->GetPosition(viewpoint));
        }
        mUpdateView = true;

        QPixmap image( QString("%1/%2.jpg").arg(mScenePath).arg( mViewpointsMesh->GetViewpoint(viewpoint)->mName ) );
        if( !image.isNull() )
        {
            QPixmap smallImage = image.scaledToWidth( mUi->image->width() );
            mUi->image->setPixmap(smallImage);
        }
        else
        {
            mUi->image->setText("Image not found!");
        }

        pEvent->accept();
    }
    else if( pEvent->key() == Qt::Key_I && mViewpointsMesh != NULL )
    {
        ShowViewpointInformation(mCurrentViewpoint);
    }
    else if( pEvent->key() == Qt::Key_P )
    {
        QString fileName = mUi->polygonalInformationComboBox->currentText();
        fileName.append( QDateTime::currentDateTime().toString("yyyyMMdd-hhmmss") );

        mOpenGLCanvas->SaveScreenshot( QString("Screenshot-%1.png").arg(fileName) );
    }
    else if( pEvent->key() == Qt::Key_F11 )
    {
        SetFullScreen(!mFullScreen);
        pEvent->ignore();
    }
}

void MainModuleController::mouseMoveEvent(QMouseEvent *pEvent)
{
    const Camera* camera = mOpenGLCanvas->GetCamera();
    Scene* scene = mOpenGLCanvas->GetScene();
    if( camera != NULL && scene != NULL )
    {
        const BoundingSphere* boundingSphere = scene->GetBoundingSphere();
        glm::vec3 centerScene = boundingSphere->GetCenter();
        glm::vec3 prevCamPosition = camera->GetPosition();
        glm::vec3 prevCamLookAtPoint = camera->GetLookAt();
        glm::vec3 prevCamUpVector = glm::normalize( camera->GetUp() );
        glm::vec3 prevCamFrontVector = glm::normalize( prevCamLookAtPoint - prevCamPosition );

        int glWidth = mOpenGLCanvas->width();
        int glHeight = mOpenGLCanvas->height();
        if( window()->isFullScreen() )
        {
            QPoint globalPos = mapToGlobal(pEvent->pos());
            QPoint localPos = mOpenGLCanvas->mapFromGlobal(globalPos);
            int movimentX = localPos.x() - glWidth / 2;
            int movimentY = localPos.y() - glHeight / 2;
            if( movimentX != 0 || movimentY != 0 )
            {
                QCursor cursor(Qt::BlankCursor);
                Camera* newCamera = camera->Clone();
                float rotationAmount = glm::sqrt(movimentX*(float)movimentX + movimentY*movimentY) / 5.0f;
                glm::vec3 leftVector = glm::normalize( glm::cross(prevCamUpVector, prevCamFrontVector) );
                glm::mat4 transform = glm::translate(glm::mat4(1.0f), centerScene);
                glm::vec3 rotationVector = glm::normalize(prevCamUpVector*-(float)movimentX + leftVector*(float)movimentY);
                transform = glm::rotate(transform, glm::radians(rotationAmount), rotationVector);
                transform = glm::translate(transform, -centerScene);
                glm::vec4 aux = transform * glm::vec4(prevCamUpVector, 0.0f);
                newCamera->SetUp(glm::vec3(aux.x, aux.y, aux.z));
                aux = transform * glm::vec4(prevCamPosition, 1.0f);
                newCamera->SetPosition(glm::vec3(aux.x, aux.y, aux.z));

                cursor.setPos(mOpenGLCanvas->mapToGlobal(QPoint(glWidth / 2, glHeight / 2)));
                setCursor(cursor);
                mOpenGLCanvas->SetCamera(newCamera);
                delete newCamera;
            }
        }
        else
        {
            QCursor cursor;
            setCursor(cursor);
            if( pEvent->buttons() & Qt::RightButton )
            {
                Camera* newCamera = camera->Clone();
                glm::vec2 initVector(mLastMousePosition.x()-glWidth/2, mLastMousePosition.y()-glHeight/2);
                glm::vec2 finalVector(pEvent->pos().x()-glWidth/2, pEvent->pos().y()-glHeight/2);
                newCamera->SetUp( glm::rotate(prevCamUpVector, glm::atan(initVector.y, initVector.x) - glm::atan(finalVector.y, finalVector.x), prevCamFrontVector ) );
                mOpenGLCanvas->SetCamera(newCamera);
                delete newCamera;
            }

            if( pEvent->buttons() & Qt::LeftButton )
            {
                int movimentX = pEvent->pos().x() - mLastMousePosition.x();
                int movimentY = pEvent->pos().y() - mLastMousePosition.y();
                if( movimentX != 0 || movimentY != 0 )
                {
                    Camera* newCamera = camera->Clone();
                    float rotationAmount = glm::sqrt((float)movimentX*movimentX + movimentY*movimentY) / 5.0f;
                    glm::vec3 leftVector = glm::normalize( glm::cross(prevCamUpVector, prevCamFrontVector) );
                    glm::mat4 transform = glm::translate(glm::mat4(1.0f), centerScene);
                    glm::vec3 rotationVector = glm::normalize(prevCamUpVector*-(float)movimentX + leftVector*(float)movimentY);
                    transform = glm::rotate(transform, glm::radians(rotationAmount), rotationVector);
                    transform = glm::translate(transform, -centerScene);
                    glm::vec4 aux = transform * glm::vec4(prevCamUpVector, 0.0f);
                    newCamera->SetUp(glm::vec3(aux.x, aux.y, aux.z));
                    aux = transform * glm::vec4(prevCamPosition, 1.0f);
                    newCamera->SetPosition(glm::vec3(aux.x, aux.y, aux.z));
                    mOpenGLCanvas->SetCamera(newCamera);
                    delete newCamera;
                }
            }
        }
        mLastMousePosition = pEvent->pos();
    }
}

void MainModuleController::wheelEvent(QWheelEvent *pEvent)
{
    const Camera* camera = mOpenGLCanvas->GetCamera();
    Scene* scene = mOpenGLCanvas->GetScene();
    if( camera != NULL && scene != NULL )
    {
        Camera* newCamera = camera->Clone();

        BoundingSphere* boundingSphere = scene->GetBoundingSphere();
        glm::vec3 prevCamPosition = camera->GetPosition();
        glm::vec3 prevCamLookAtPoint = camera->GetLookAt();
        glm::vec3 prevCamUpVector = glm::normalize( camera->GetUp() );
        glm::vec3 prevCamFrontVector = glm::normalize( prevCamLookAtPoint - prevCamPosition );

        int numDegrees = pEvent->delta() / 8;
        float deltaFactor = (numDegrees / 360.0f) * 2.0f;

        if( window()->isFullScreen() )
        {
            if( numDegrees != 0 )
            {
                newCamera->SetUp( glm::rotate(prevCamUpVector, glm::radians(numDegrees/4.0f), prevCamFrontVector ) );
                mOpenGLCanvas->SetCamera(newCamera);
            }
        }
        else
        {
            glm::vec3 position = prevCamPosition + prevCamFrontVector * boundingSphere->GetRadius() * deltaFactor;
            newCamera->SetPosition( position );
            mOpenGLCanvas->SetCamera(newCamera);
        }
        delete newCamera;
    }
}

void MainModuleController::mousePressEvent(QMouseEvent *pEvent)
{
    mOpenGLCanvas->setFocus();
    mLastMousePosition = pEvent->pos();
}

void MainModuleController::LoadScene(const QString &pFileName)
{
    QTime t;

    QApplication::setOverrideCursor( Qt::WaitCursor );
    t.start();
    Debug::Log(QString("Carregant %1").arg(pFileName));

    mUpdateView = true;
    mMenuVisualization->setEnabled(true);
    mActionExport->setEnabled(false);
    mActionViewpointsSphere->setEnabled(false);
    mUi->shadingFrame->setEnabled(false);
    mUi->rightTabWidget->show();
    mUi->leftTabWidget->hide();
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->othersTab), false);
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->nBestViewsTab), false);
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->screenshotsTab), false);

    if( mHistogram != NULL )
    {
        delete mHistogram;
        mHistogram = NULL;
        delete mViewpointsMesh;
        mViewpointsMesh = NULL;
    }
    if( mScene != NULL )
    {
        delete mScene;
    }
    mScene = SceneLoader::LoadScene(pFileName);
    mScene->ShowInformation();
    mOpenGLCanvas->LoadScene(mScene);
    Debug::Log( QString("MainWindow::LoadScene - Total time elapsed: %1 ms").arg( t.elapsed() ) );

    mPolygonalTexturesSize = glm::ceil( glm::sqrt( (float)mScene->GetNumberOfPolygons() ) );
    mOpenGLCanvas->SetPolygonalTextureSize(mPolygonalTexturesSize);
    mObscurancesComputed = false;
    mUi->polygonalInformationCheckBox->setChecked(false);
    mUi->polygonalInformationComboBox->clear();

    on_polygonalInformationCheckBox_clicked( mUi->polygonalInformationCheckBox->isChecked() );
    mUi->texturedCheckBox->setChecked(true);
    on_texturedCheckBox_clicked( mUi->texturedCheckBox->isChecked() );
    QApplication::restoreOverrideCursor();
}

void MainModuleController::ComputeObscurances()
{
    int textureSize = mUi->textureSizeSpinBox->value();
    if(mObscurancesGenerator == NULL)
    {
        mObscurancesGenerator = new ObscuranceMap(mScene, textureSize);
    }
    else
    {
        mObscurancesGenerator->SetScene(mScene);
        mObscurancesGenerator->SetTextureSize(textureSize);
    }

    // Create obscurances per polygon
    mObscurancesGenerator->ComputeObscurances(mUi->numberOfIterationsSpinBox->value());
    const float *obscurances = mObscurancesGenerator->GetImgRgbBuffer();

    glDeleteTextures(1, &mObscurancesPerPolygonTexture);
    glGenTextures(1, &mObscurancesPerPolygonTexture);
    glBindTexture( GL_TEXTURE_2D, mObscurancesPerPolygonTexture );
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, mPolygonalTexturesSize, mPolygonalTexturesSize, 0, GL_RGBA, GL_FLOAT, obscurances);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glBindTexture( GL_TEXTURE_2D, 0 );
    CHECK_GL_ERROR();

    mObscurancesPerPolygon.resize(mScene->GetNumberOfPolygons());
    for( int i = 0; i < mObscurancesPerPolygon.size(); i++ )
    {
        mObscurancesPerPolygon[i] = obscurances[i*4];
    }

    if(!mObscurancesComputed)
    {
        mObscurancesComputed = true;
        mUi->polygonalInformationComboBox->insertItem(0, "Obscurances");
        on_polygonalInformationComboBox_currentIndexChanged(mUi->polygonalInformationComboBox->currentIndex());
    }
    UpdateRenderingGUI();
}

void MainModuleController::LoadViewpoints()
{
    QTime t;

    bool recomputePolygonalInformation = true;
    //Creació del canal d'informació
    if(mHistogram != NULL)
    {
        delete mHistogram;
        int answer = QMessageBox::question ( this, tr("Question"), tr("Do you want to preserve the polygonal information computed with the last set of viewpoints?"), QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
        if( answer == QMessageBox::Yes )
        {
            recomputePolygonalInformation = false;
        }
    }
    mHistogram = HistogramBuilder::CreateHistogram(mScene, mViewpointsMesh, mUi->widthResolutionSpinBox->value(), mUi->faceCullingCheckBox->isChecked());

    mMaxAreaPolygon.fill( 0, mScene->GetNumberOfPolygons() );
    for ( int currentViewpoint = 0; currentViewpoint < mViewpointsMesh->GetNumberOfViewpoints(); currentViewpoint++ )
    {
        for( int currentPolygon = 0; currentPolygon < mScene->GetNumberOfPolygons(); currentPolygon++ )
        {
            if( mMaxAreaPolygon.at(currentPolygon) < mHistogram->GetValue(currentViewpoint, currentPolygon) )
            {
                mMaxAreaPolygon[currentPolygon] = mHistogram->GetValue(currentViewpoint, currentPolygon);
            }
        }
    }

    mOpenGLCanvas->SetPerVertexMesh(mViewpointsMesh->GetMesh());
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("faceCulling", mUi->faceCullingCheckBox->isChecked());
    mActionViewpointsSphere->setEnabled(true);

    QApplication::setOverrideCursor( Qt::WaitCursor );
    t.start();
    QProgressDialog progress(this);
    progress.setLabelText("Computing measures...");
    progress.setCancelButton(0);
    progress.setRange(0, mPolygonalMeasures.size() + mViewpointMeasures.size());
    progress.show();
    progress.setValue(0);
    qApp->processEvents();
    //Reinicialitzem les mesures
    if(recomputePolygonalInformation)
    {
        for( int i = 0; i < mPolygonalMeasures.size(); i++ )
        {
            mPolygonalMeasures.at(i)->SetComputed(false);
        }
    }
    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        mViewpointMeasures.at(i)->SetComputed(false);
    }
    //Les calculem i només es recalculen si no s'ha fet abans a traves de dependències
    for( int i = 0; i < mPolygonalMeasures.size(); i++ )
    {
        if(!mPolygonalMeasures.at(i)->Computed())
        {
            mPolygonalMeasures.at(i)->Compute(mHistogram);
        }
        progress.setValue(i + 1);
        qApp->processEvents();
    }

    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        if(!mViewpointMeasures.at(i)->Computed())
        {
            mViewpointMeasures.at(i)->Compute(mHistogram);
        }
        progress.setValue(mPolygonalMeasures.size() + i + 1);
        qApp->processEvents();
    }
    progress.hide();
    Debug::Log( QString("MainWindow::Measures computed - Time elapsed: %1 ms").arg(t.elapsed()) );
    QApplication::restoreOverrideCursor();

    //N Best views
    if(mNBestViews != NULL)
    {
        delete mNBestViews;
    }
    mNBestViews = new NBestViews(mHistogram);
    mNBestViews->SetDependencyPolygonalI1(mPolygonalI1);
    mNBestViews->SetDependencyPolygonalI2(mPolygonalI2);
    mNBestViews->SetDependencyPolygonalI3(mPolygonalI3);
    mNBestViews->SetDependencyProjectedI1(mProjectedI1);
    mNBestViews->SetDependencyProjectedI2(mProjectedI2);
    mNBestViews->SetDependencyProjectedI3(mProjectedI3);

    //Polygonal information combo box
    if( mUi->polygonalInformationComboBox->count() < mPolygonalMeasures.size() )
    {
        for( int i = 0; i < mPolygonalMeasures.size(); i++ )
        {
            mUi->polygonalInformationComboBox->addItem( mPolygonalMeasures.at(i)->GetName() );
        }
    }

    on_polygonalInformationComboBox_currentIndexChanged( mUi->polygonalInformationComboBox->currentIndex() );
    on_measureInViewpointSphereList_currentIndexChanged( mUi->measureInViewpointSphereList->currentIndex() );
    on_polygonalInformationCheckBox_clicked( mUi->polygonalInformationCheckBox->isChecked() );

    mUpdateView = false;
    ChangeNumberOfViewpoints( mViewpointsMesh->GetNumberOfViewpoints() );
    mUpdateView = true;
    SetViewpoint(0);
    mUi->leftTabWidget->show();
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->othersTab), true);
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->nBestViewsTab), true);
    mUi->rightTabWidget->setTabEnabled(mUi->rightTabWidget->indexOf(mUi->screenshotsTab), true);
    mUi->rightTabWidget->setCurrentIndex( mUi->rightTabWidget->indexOf(mUi->othersTab) );
    mMenuVisualization->setEnabled(true);
    mActionExport->setEnabled(true);
    mActionViewpointsSphere->setChecked(true);
}

void MainModuleController::LoadViewpointsFromFile(const QString &pFileName)
{
    Debug::Log( QString("Viewpoints file: %1").arg(pFileName) );

    if( mViewpointsMesh != NULL )
    {
        delete mViewpointsMesh;
    }
    float radius = mScene->GetBoundingSphere()->GetRadius();
    glm::vec3 center = mScene->GetBoundingSphere()->GetCenter();
    mViewpointsMesh = new ViewpointsMesh(center, radius, pFileName);

    LoadViewpoints();
}

void MainModuleController::LoadViewpointsFromSphere(float pRadius, float pAngle, int pSubdivision)
{
    float radius = mScene->GetBoundingSphere()->GetRadius();
    glm::vec3 center = mScene->GetBoundingSphere()->GetCenter();

    if( mViewpointsMesh != NULL )
    {
        delete mViewpointsMesh;
    }
    mViewpointsMesh = new ViewpointsMesh(center, radius * pRadius, pAngle, mUi->cameraAspectRatioSpinBox->value(), pSubdivision );

    LoadViewpoints();
}

void MainModuleController::ChangeNumberOfViewpoints(int pNumberOfViewpoints)
{
    //Modifiquem el rang dels sliders en funció del nombre de viewpoints
    for( int i = 0; i < mViewpointMeasuresSliders.size(); i++ )
    {
        mViewpointMeasuresSliders.at(i)->setRange(0, pNumberOfViewpoints - 1);
    }

    mUi->bestNViewsSlider->setRange( 1, pNumberOfViewpoints );
    mUi->bestNViewsSpinBox->setRange( 1, pNumberOfViewpoints );
}

void MainModuleController::SaveViewpointMeasuresInformation(const QString &pFileName)
{
    QFile file(pFileName);

    QApplication::setOverrideCursor( Qt::WaitCursor );
    Debug::Log(QString("Exportant %1").arg(pFileName));

    if(file.open(QFile::WriteOnly))
    {
        QXmlStreamWriter stream(&file);
        stream.setAutoFormatting(true);
        stream.writeStartDocument();

        stream.writeStartElement("viewpoints");
        for( int i = 0; i < mViewpointsMesh->GetNumberOfViewpoints(); i++ )
        {
            stream.writeStartElement("viewpoint");
            stream.writeAttribute("name", mViewpointsMesh->GetViewpoint(i)->mName);
            for( int j = 0; j < mViewpointMeasures.size(); j++ )
            {
                QString name = mViewpointMeasures.at(j)->GetName();
                name = name.replace(" ","_");
                name = name.replace("(","_");
                name = name.replace(")","");
                name = name.replace("-","_");
                name = name.replace("|","_");
                stream.writeTextElement( QString("%1").arg(name), QString("%1").arg(mViewpointMeasures.at(j)->GetValue(i)) );
            }
            stream.writeEndElement();
        }
        stream.writeEndElement();
        stream.writeEndDocument();
        file.close();
        Debug::Log(QString("Informacio escrita al fitxer: %1").arg(pFileName));
    }
    else
    {
        Debug::Error(QString("Impossible escriure a fitxer: %1").arg(pFileName));
    }
    QApplication::restoreOverrideCursor();
}

int MainModuleController::NextViewpoint()
{
    mCurrentViewpoint++;
    if( mCurrentViewpoint >= mViewpointsMesh->GetNumberOfViewpoints() )
        mCurrentViewpoint = 0;
    SetViewpoint(mCurrentViewpoint);
    return mCurrentViewpoint;
}

void MainModuleController::SetViewpoint(int pViewpoint)
{
    mCurrentViewpoint = pViewpoint;
    Camera* currentCamera = mViewpointsMesh->GetViewpoint(mCurrentViewpoint);
    mOpenGLCanvas->SetCamera( currentCamera );

    if(mViewPolygonalVisibility)
    {
        ReloadVisibilityTexture(pViewpoint, 1);
    }

    //ShowViewpointInformation(pViewpoint);
    mOpenGLCanvas->updateGL();

    mUpdateView = false;
    for( int i = 0; i < mViewpointMeasuresSliders.size(); i++ )
    {
        mViewpointMeasuresSliders.at(i)->setValue(mViewpointMeasures.at(i)->GetPosition(pViewpoint));
    }
    mUpdateView = true;
}

void MainModuleController::ShowViewpointInformation(int pViewpoint)
{
    Debug::Log(QString("Selected viewpoint: %1").arg(mViewpointsMesh->GetViewpoint(pViewpoint)->mName));
    //Debug::Log(QString("  Latitude: %1").arg(mGeographicViewpoints[pViewpoint].y));
    //Debug::Log(QString("  Longitude: %1").arg(mGeographicViewpoints[pViewpoint].z));
    Debug::Log(QString("Viewpoint quality measures:"));
    for( int i = 0; i < mViewpointMeasures.size(); i++ )
    {
        Debug::Log( QString("  %1: %2").arg(mViewpointMeasures.at(i)->GetName()).arg(mViewpointMeasures.at(i)->GetValue(pViewpoint)) );
    }
}

void MainModuleController::SetFullScreen(bool pFullScreen)
{
    mFullScreen = pFullScreen;
    if(mFullScreen)
    {
        showFullScreen();
        mUi->console->hide();
        if(mScene != NULL)
        {
            mUi->rightTabWidget->hide();
            if(mViewpointsMesh != NULL)
            {
                mUi->leftTabWidget->hide();
            }
        }
    }
    else
    {
        showNormal();
        mUi->console->show();
        if(mScene != NULL)
        {
            mUi->rightTabWidget->show();
            if(mViewpointsMesh != NULL)
            {
                mUi->leftTabWidget->show();
            }
        }
    }
}

QVector<bool> MainModuleController::GetPolygonalVisibility( int pViewpoint, int pVisibilityCriteria )
{
    QVector<bool> visibility(mScene->GetNumberOfPolygons(), false);

    for( int i = 0; i < mScene->GetNumberOfPolygons(); i++ )
    {
        bool visible;
        if( pVisibilityCriteria == 0 )
        {
            visible = ( mHistogram->GetValue(pViewpoint, i) != 0 );
        }
        else if( pVisibilityCriteria == 1 )
        {
            visible = ( mHistogram->GetValue(pViewpoint, i) > mMaxAreaPolygon.at(i) * 0.50f );
        }
        else
        {
            Debug::Warning( QString("Discarding criteria %1 not implemented!").arg(pVisibilityCriteria) );
        }
        visibility[i] = visible;
    }
    return visibility;
}

void MainModuleController::ReloadVisibilityTexture(int pViewpoint, int pVisibilityCriteria)
{
    QVector<bool> visibility = GetPolygonalVisibility( pViewpoint, pVisibilityCriteria );
    QVector<float> visibilityFloat( mPolygonalTexturesSize * mPolygonalTexturesSize * 4, 0.0f );
    for( int i = 0; i < visibility.size(); i++ )
    {
        if( visibility.at(i) )
        {
            visibilityFloat[ i * 4 + 0] = 1.0f;
            visibilityFloat[ i * 4 + 1] = 1.0f;
            visibilityFloat[ i * 4 + 2] = 1.0f;
            visibilityFloat[ i * 4 + 3] = 1.0f;
        }
    }

    glDeleteTextures(1, &mPolygonalVisibilityTexture);
    glGenTextures(1, &mPolygonalVisibilityTexture);
    glBindTexture(GL_TEXTURE_2D, mPolygonalVisibilityTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mPolygonalTexturesSize, mPolygonalTexturesSize, 0, GL_RGBA, GL_FLOAT, visibilityFloat.data());
    mOpenGLCanvas->SetPolygonalVisibilityTexture(mPolygonalVisibilityTexture);
}

void MainModuleController::UpdateRenderingGUI()
{
    bool polygonalInformationChecked = mUi->polygonalInformationCheckBox->isChecked();
    bool texturedChecked = mUi->texturedCheckBox->isChecked();

    mUi->shadingFrame->setEnabled(mObscurancesComputed || mHistogram != NULL);
    mUi->polygonalInformationComboBox->setEnabled(polygonalInformationChecked);
    mUi->rgbCheckBox->setEnabled(polygonalInformationChecked);
    mUi->powerFrame->setEnabled(polygonalInformationChecked);
    mUi->luminanceFrame->setEnabled(false);
    mUi->weightFrame->setEnabled(false);

    if( texturedChecked && polygonalInformationChecked )
    {
        mUi->luminanceFrame->setEnabled(true);
        mUi->weightFrame->setEnabled(true);
    }
}

void MainModuleController::UpdateOthersGUI()
{
    mUi->viewVisiblePolygonsCheckBox->setEnabled(mUi->texturedCheckBox->isChecked());
}

QString MainModuleController::GetScreenshotName(int pViewpoint)
{
    QString name = mScene->GetName();
    name += "_";
    if( mUi->texturedCheckBox->isChecked() )
    {
        name += "Textured_";
    }
    if( mUi->polygonalInformationCheckBox->isChecked() )
    {
        QString measureName = mUi->polygonalInformationComboBox->currentText();

        name += measureName;
        name += "_";
        name += QString("%1").arg(mUi->powerSpinBox->value());
        name += "_";
    }
    if( mUi->texturedCheckBox->isChecked() && mUi->polygonalInformationCheckBox->isChecked() )
    {
        name += QString("%1").arg(mUi->weightSpinBox->value());
        name += "_";
    }
    if( pViewpoint != -1 )
    {
        name += mViewpointsMesh->GetViewpoint(mCurrentViewpoint)->mName;
    }
    name.replace( " ", "_" );
    name.replace( ".", "_" );

    return name;
}

void MainModuleController::OpenModel()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a model"), "./models", tr("Supported models (*.obj *.3ds *.dae *.ply);;Wavefront Object (*.obj);;3ds Max 3DS (*.3ds);;Collada (*.dae);;Stanford Polygon Library (*.ply);;All files (*.*)"));
    if(!fileName.isNull())
    {
        QFileInfo fileInfo(fileName);
        mScenePath = fileInfo.absolutePath();
        LoadScene(fileName);
    }
}

void MainModuleController::ExportInformation()
{
    QString fileName = QFileDialog::getSaveFileName(this, tr("Choose a file to export"), "./", tr("XML file (*.xml);;All files (*.*)"));
    if(!fileName.isEmpty())
    {
        SaveViewpointMeasuresInformation(fileName);
    }
}

void MainModuleController::WillDrawViewpointsSphere(bool pDraw)
{
    if(mViewpointsMesh != NULL)
    {
        mViewpointsMesh->GetMesh()->SetVisible(pDraw);
        mOpenGLCanvas->updateGL();
    }
}

void MainModuleController::on_measureInViewpointSphereList_currentIndexChanged(int pValue)
{
    if( mHistogram != NULL )
    {
        QVector<glm::vec4> colors;

        if( pValue < mViewpointMeasures.size() )
        {
            colors = Tools::ConvertFloatsToColors(mViewpointMeasures.at(pValue)->GetValues(), false);
        }
        for( int i = 0; i < colors.size(); i++ )
        {
            colors[i].a = mUi->alphaSpinBox->value();
        }
        mViewpointsMesh->GetMesh()->SetColorData( colors.size(), colors.data() );
        mOpenGLCanvas->updateGL();
    }
}

void MainModuleController::on_polygonalInformationComboBox_currentIndexChanged(int pIndex)
{
    if( mScene != NULL && pIndex >= 0 )
    {
        if( mObscurancesComputed && pIndex < 1 )
        {//Obscurances
            if(pIndex == 0)
            {
                mOpenGLCanvas->SetPolygonalTexture(mObscurancesPerPolygonTexture);
            }
        }
        else if( mHistogram != NULL )
        {
            int index = pIndex;
            if( mObscurancesComputed )
            {
                index--;
            }
            if( index < mPolygonalMeasures.size() )
            {
                //Actualitzem la GUI
                UpdateOthersGUI();

                //Assignem la informació per vèrtex
                QVector<float> scaledPolygonalSceneValues = mPolygonalMeasures.at(index)->GetScaledValues();
                if( scaledPolygonalSceneValues.size() > 0 )
                {
                    float luminance = Tools::Mean( scaledPolygonalSceneValues, mHistogram->GetMeanProjectedArea(), mUi->powerSpinBox->value() );
                    mOpenGLCanvas->GetShaderProgram()->UseProgram();
                    mOpenGLCanvas->GetShaderProgram()->SetUniform("luminance", (float)luminance);
                    mOpenGLCanvas->updateGL();
                }

                //Assignem la informació per polígon
                QVector<float> floatColors( mPolygonalTexturesSize * mPolygonalTexturesSize * 4, 0.0f );
                for( int i = 0; i < scaledPolygonalSceneValues.size(); i++ )
                {
                    floatColors[ i * 4 + 0] = scaledPolygonalSceneValues.at(i);
                    floatColors[ i * 4 + 1] = scaledPolygonalSceneValues.at(i);
                    floatColors[ i * 4 + 2] = scaledPolygonalSceneValues.at(i);
                    floatColors[ i * 4 + 3] = scaledPolygonalSceneValues.at(i);
                }

                glDeleteTextures(1, &mPolygonalTexture);
                glGenTextures(1, &mPolygonalTexture);
                glBindTexture(GL_TEXTURE_2D, mPolygonalTexture);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
                glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mPolygonalTexturesSize, mPolygonalTexturesSize, 0, GL_RGBA, GL_FLOAT, floatColors.data());

                CHECK_GL_ERROR();

                mOpenGLCanvas->SetPolygonalTexture(mPolygonalTexture);
            }
        }
    }
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_alphaSlider_valueChanged(int pValue)
{
    mUi->alphaSpinBox->setValue( pValue / 100.0f );
    on_measureInViewpointSphereList_currentIndexChanged(mUi->measureInViewpointSphereList->currentIndex());
}

void MainModuleController::on_alphaSpinBox_valueChanged(double pValue)
{
    mUi->alphaSlider->setValue( (int)(pValue * 100) );
    on_measureInViewpointSphereList_currentIndexChanged(mUi->measureInViewpointSphereList->currentIndex());
}

void MainModuleController::on_powerSlider_valueChanged(int pValue)
{
    float exponent = pValue / 100.0f;
    if(pValue >= 0)
    {
        exponent += 1.0f;
    }
    else
    {
        exponent = -1.0f / (exponent - 1.0f );
    }
    mUi->powerSpinBox->setValue( exponent );
}

void MainModuleController::on_powerSpinBox_valueChanged(double pValue)
{
    if( pValue >= 1.0f )
    {
        mUi->powerSlider->setValue( glm::round(pValue * 100) - 100 );
    }
    else
    {
        mUi->powerSlider->setValue( 100 - glm::round(100.0f / pValue) );
    }
    if( mObscurancesComputed && mUi->polygonalInformationComboBox->currentIndex() < 1 )
    {//Obscurances
        if(mUi->polygonalInformationComboBox->currentIndex() == 0)
        {
            float luminance = Tools::Mean( mObscurancesPerPolygon, mHistogram->GetMeanProjectedArea(), pValue );
            mOpenGLCanvas->GetShaderProgram()->UseProgram();
            mOpenGLCanvas->GetShaderProgram()->SetUniform("luminance", (float)luminance);
            mOpenGLCanvas->updateGL();
        }
    }
    else if( mHistogram != NULL )
    {
        int index = mUi->polygonalInformationComboBox->currentIndex();
        if( mObscurancesComputed )
        {
            index--;
        }
        if( index < mPolygonalMeasures.size() )
        {
            QVector<float> polygonalSceneValues = mPolygonalMeasures.at(index)->GetValues();
            if( polygonalSceneValues.size() > 0 )
            {
                float luminance = Tools::Mean( mPolygonalMeasures.at(index)->GetScaledValues(), mHistogram->GetMeanProjectedArea(), pValue );
                mOpenGLCanvas->GetShaderProgram()->UseProgram();
                mOpenGLCanvas->GetShaderProgram()->SetUniform("luminance", (float)luminance);
                mOpenGLCanvas->updateGL();
            }
        }
    }
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("power", (float)pValue);
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_luminanceSlider_valueChanged(int pValue)
{
    mUi->luminanceSpinBox->setValue( pValue / 100.0f );
}

void MainModuleController::on_luminanceSpinBox_valueChanged(double pValue)
{
    mUi->luminanceSlider->setValue( glm::round(pValue * 100) );
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("luminance", (float)pValue);
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_weightSlider_valueChanged(int pValue)
{
    mUi->weightSpinBox->setValue( pValue / 100.0f );
}

void MainModuleController::on_weightSpinBox_valueChanged(double pValue)
{
    mUi->weightSlider->setValue( glm::round(pValue * 100) );
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("weight", (float)pValue);
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_takeScreenshotsButton_clicked()
{
    ComputeObscurances();

    mUi->polygonalInformationComboBox->count();
    for( int i = 0; i < mUi->polygonalInformationComboBox->count(); i++ )
    {
        mUi->polygonalInformationComboBox->setCurrentIndex( i );
        mOpenGLCanvas->updateGL();

        QString fileName = GetScreenshotName(-1);

        mOpenGLCanvas->SaveScreenshot( QString("%1.png").arg(fileName) );
    }
}

void MainModuleController::on_takeScreenshotsFromAllViewpointsButton_clicked()
{
    QTime t;

    QApplication::setOverrideCursor( Qt::WaitCursor );
    t.start();
    QProgressDialog progress(this);
    progress.setLabelText("Taking screenshots...");
    progress.setCancelButton(0);
    progress.setRange(0, mViewpointsMesh->GetNumberOfViewpoints());
    progress.show();

    for( int i = 0; i < mViewpointsMesh->GetNumberOfViewpoints(); i++ )
    {
        progress.setValue(i);
        qApp->processEvents();
        SetViewpoint(i);

        QString fileName = GetScreenshotName(i);

        mOpenGLCanvas->SaveScreenshot( QString("%1.png").arg(fileName) );
    }
    progress.setValue(mViewpointsMesh->GetNumberOfViewpoints());
    qApp->processEvents();
    progress.hide();
    Debug::Log( QString("MainWindow::on_takeScreenshotsFromAllViewpointsButton_clicked - Time elapsed: %1 ms").arg(t.elapsed()) );
    QApplication::restoreOverrideCursor();
}

void MainModuleController::on_nBestViewsComputeButton_clicked()
{
    QVector< int > bestViews;
    QString measure = mUi->nBestViewsSelectionMeasuresComboBox->currentText();

    Debug::Log( QString("N Best Views with %1").arg( measure ) );

    if( measure.compare( QString("%1 (discarding triangles)").arg( "Projected I1" ) ) == 0 )
    {
        bestViews = mNBestViews->GetBestNViewsProjectedI1DiscardingPolygons( mUi->bestNViewsSlider->value(), mUi->bestNViewsByThresholdSlider->value() / 100.0f, mUi->nBestViewsCriteriaForDiscardingComboBox->currentIndex() );
    }
    else if( measure.compare( QString("%1 (discarding triangles)").arg( "Projected I2" ) ) == 0 )
    {
        bestViews = mNBestViews->GetBestNViewsProjectedI2DiscardingPolygons( mUi->bestNViewsSlider->value(), mUi->bestNViewsByThresholdSlider->value() / 100.0f, mUi->nBestViewsCriteriaForDiscardingComboBox->currentIndex() );
    }
    else if( measure.compare( QString("%1 (discarding triangles)").arg( "Projected I3" ) ) == 0 )
    {
        bestViews = mNBestViews->GetBestNViewsProjectedI3DiscardingPolygons( mUi->bestNViewsSlider->value(), mUi->bestNViewsByThresholdSlider->value() / 100.0f, mUi->nBestViewsCriteriaForDiscardingComboBox->currentIndex() );
    }
    else
    {
        Debug::Error( QString("Mesura %1 not implemented!").arg( measure ) );
    }

    for( int i = 0; i < bestViews.size(); i++ )
    {
        SetViewpoint( bestViews[i] );
        QString name = QString("%1_%2_%3.png").arg( mScene->GetName() ).arg( measure ).arg( i + 1 );
        name.replace(" ", "_");
        mOpenGLCanvas->SaveScreenshot( name );
        Debug::Log( mViewpointsMesh->GetViewpoint( bestViews[i] )->mName );
    }
}

void MainModuleController::on_nBestViewsComputeAllButton_clicked()
{
    QVector< int > bestViews;

    QApplication::setOverrideCursor( Qt::WaitCursor );

    QFile file("nBestViews.csv");
    if(file.open(QFile::WriteOnly))
    {
        QTextStream out(&file);

        int numberOfViewpoints = mViewpointsMesh->GetNumberOfViewpoints();
        out << ";";
        for( int i = 0; i < numberOfViewpoints; i++ )
        {
            out << QString("%1").arg(i+1) << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI1DiscardingPolygons( numberOfViewpoints, 0.95f, 0 );
        out << "Projected I1" << " (discarding triangles (one pixel))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI1DiscardingPolygons( numberOfViewpoints, 0.95f, 1 );
        out << "Projected I1" << " (discarding triangles (90% max area))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI2DiscardingPolygons( numberOfViewpoints, 0.95f, 0 );
        out << "Projected I2" << " (discarding triangles (one pixel))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI2DiscardingPolygons( numberOfViewpoints, 0.95f, 1 );
        out << "Projected I2" << " (discarding triangles (90% max area))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI3DiscardingPolygons( numberOfViewpoints, 0.95f, 0 );
        out << "Projected I3" << " (discarding triangles (one pixel))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        bestViews = mNBestViews->GetBestNViewsProjectedI3DiscardingPolygons( numberOfViewpoints, 0.95f, 1 );
        out << "Projected I3" << " (discarding triangles (90% max area))" << ";";
        for( int i = 0; i < bestViews.size(); i++ )
        {
            out << mViewpointsMesh->GetViewpoint( bestViews.at(i) )->mName << ";";
        }
        out << "\n";

        file.close();
        Debug::Log(QString("Informacio escrita al fitxer nBestViews.csv"));
    }
    else
    {
        Debug::Error(QString("Impossible escriure a fitxer nBestViews.csv"));
    }

    QApplication::restoreOverrideCursor();
}

void MainModuleController::SliderChanged(int pMeasure, int pValue)
{
    if(mUpdateView)
    {
        unsigned int viewpoint = mViewpointMeasures.at(pMeasure)->GetNth(pValue);
        SetViewpoint(viewpoint);
    }
}

void MainModuleController::on_loadViewpointsSphereButton_clicked()
{
    LoadViewpointsFromSphere( mUi->cameraDistanceSpinBox->value(), mUi->cameraAngleSpinBox->value(), mUi->numberOfViewpoints->currentIndex() );
}

void MainModuleController::on_loadFreeViewpointsMeshButton_clicked()
{
    //Load viewpoints from file
    QString fileName = QFileDialog::getOpenFileName(this, tr("Choose a file with viewpoints"), "./models", tr("Supported files (*.xml *.txt);;XML file (*.xml);;Text file (*.txt);;All files (*.*)"));
    if(!fileName.isEmpty())
    {
        LoadViewpointsFromFile(fileName);
    }
}

void MainModuleController::on_viewVisiblePolygonsCheckBox_clicked(bool pChecked)
{
    if( pChecked && !mViewPolygonalVisibility )
    {
        ReloadVisibilityTexture(mCurrentViewpoint, 1);
    }
    mViewPolygonalVisibility = pChecked;
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("viewPolygonalVisibility", pChecked);
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_computeObscurancesAndAmbientOcclusionButton_clicked()
{
    QTime t;
    QApplication::setOverrideCursor( Qt::WaitCursor );
    t.start();
    ComputeObscurances();
    Debug::Log( QString("MainWindow::ComputeObscurances - Total time elapsed: %1 ms").arg( t.elapsed() ) );
    QApplication::restoreOverrideCursor();
}

void MainModuleController::on_computeCameraDistanceButton_clicked()
{
    mUi->cameraAngleSpinBox->setValue( glm::round( 2.0f * glm::degrees( glm::asin( 1.0f / (float)mUi->cameraDistanceSpinBox->value() ) ) ) );
    mUi->cameraAspectRatioSpinBox->setValue( 1.0f );
}

void MainModuleController::on_computeCameraAngleButton_clicked()
{
    mUi->cameraDistanceSpinBox->setValue( 1.0f / glm::sin( glm::radians( mUi->cameraAngleSpinBox->value() / 2.0f ) ) );
    mUi->cameraAspectRatioSpinBox->setValue( 1.0f );
}

void MainModuleController::on_rgbCheckBox_clicked(bool pChecked)
{
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("drawRGB", pChecked);
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_polygonalInformationCheckBox_clicked(bool pChecked)
{
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("applyPolygonalInformation", pChecked);
    if( !pChecked && !mUi->texturedCheckBox->isChecked() )
    {
        mUi->texturedCheckBox->setChecked(true);
        mOpenGLCanvas->GetShaderProgram()->SetUniform("onlyPolygonalInformation", false);
    }
    UpdateRenderingGUI();
    UpdateOthersGUI();
    mOpenGLCanvas->updateGL();
}

void MainModuleController::on_texturedCheckBox_clicked(bool pChecked)
{
    mOpenGLCanvas->GetShaderProgram()->UseProgram();
    mOpenGLCanvas->GetShaderProgram()->SetUniform("onlyPolygonalInformation", !pChecked);
    if(pChecked)
    {
        mOpenGLCanvas->GetShaderProgram()->SetUniform("applyPolygonalInformation", mUi->polygonalInformationCheckBox->isChecked());
    }
    else
    {
        mUi->polygonalInformationCheckBox->setChecked(true);
        mOpenGLCanvas->GetShaderProgram()->SetUniform("onlyPolygonalInformation", true);
    }
    UpdateRenderingGUI();
    UpdateOthersGUI();
    mOpenGLCanvas->updateGL();
}

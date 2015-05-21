#-------------------------------------------------
#
# Project created by QtCreator 2011-02-09T17:08:35
#
#-------------------------------------------------

QT += opengl #core and gui are already included

TARGET = QuoniamTerrain
TEMPLATE = app

# Application version
VERSION = 1.0.1

# Define the preprocessor macro to get the application version in our application
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

CONFIG(debug, debug|release){
    COMPILATION = debug
}
else {
    COMPILATION = release
}

win32 {
    RC_FILE = QuoniamTerrain.rc

    DESTDIR = ./bin/win/$$COMPILATION

    LIBS += -L$$PWD/dependencies/assimp/lib/$$COMPILATION/ -lassimp
    LIBS += -L$$PWD/dependencies/glew/lib/ -lglew32

    PWD_WIN = $$replace(PWD,"/","\\")
    # Copy supporting library DLLs
    QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\glew\\bin\\glew32.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
    equals(COMPILATION,debug){
        LIBS += -L$$PWD/dependencies/vld/lib/ -lvld
        QT_INSTALL_BINS_WIN = $$[QT_INSTALL_BINS]
        QT_INSTALL_BINS_WIN = $$replace(QT_INSTALL_BINS_WIN,"/","\\")
        QT_INSTALL_PLUGINS_WIN = $$[QT_INSTALL_PLUGINS]
        QT_INSTALL_PLUGINS_WIN = $$replace(QT_INSTALL_PLUGINS_WIN,"/","\\")
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Cored.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Guid.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5OpenGLd.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Widgetsd.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(xcopy "$$QT_INSTALL_PLUGINS_WIN\\platforms\\qwindowsd.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\platforms\\" $$escape_expand(\\n))

        QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\assimp\\bin\\release\\Assimp64.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\Assimp64d.dll" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\vld\\bin\\vld_x64.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\vld\\bin\\dbghelp.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\vld\\bin\\Microsoft.DTfW.DHL.manifest" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
    }
    else {
        QT_INSTALL_BINS_WIN = $$[QT_INSTALL_BINS]
        QT_INSTALL_BINS_WIN = $$replace(QT_INSTALL_BINS_WIN,"/","\\")
        QT_INSTALL_PLUGINS_WIN = $$[QT_INSTALL_PLUGINS]
        QT_INSTALL_PLUGINS_WIN = $$replace(QT_INSTALL_PLUGINS_WIN,"/","\\")
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Core.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Gui.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5OpenGL.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(copy "$$QT_INSTALL_BINS_WIN\\Qt5Widgets.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
        QMAKE_POST_LINK += $$quote(xcopy "$$QT_INSTALL_PLUGINS_WIN\\platforms\\qwindows.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\platforms\\" $$escape_expand(\\n))

        QMAKE_POST_LINK += $$quote(copy "$$PWD_WIN\\dependencies\\assimp\\bin\\$$COMPILATION\\Assimp64.dll" "$$PWD_WIN\\bin\\win\\$$COMPILATION\\" $$escape_expand(\\n))
    }
}

INCLUDEPATH +=\
    $$PWD/dependencies/assimp/include \
    $$PWD/dependencies/glew/include \
    $$PWD/dependencies/glm \
    $$PWD/dependencies/miniball \
    $$PWD/dependencies/vld/include \
    $$PWD/inc \
    $$PWD/inc/core \
    $$PWD/inc/information-measures

DEPENDPATH +=\
    $$PWD/dependencies/assimp/include \
    $$PWD/dependencies/glew/include \
    $$PWD/dependencies/glm \
    $$PWD/dependencies/miniball \
    $$PWD/dependencies/vld/include \
    $$PWD/inc \
    $$PWD/inc/core \
    $$PWD/inc/information-measures

OBJECTS_DIR = ./tmp/$$COMPILATION
RCC_DIR = ./tmp/$$COMPILATION

MOC_DIR = ./tmp
UI_DIR = ./tmp

SOURCES +=\
    src/core/AxisAlignedBoundingBox.cpp \
    src/core/BoundingSphere.cpp \
    src/core/Camera.cpp \
    src/core/Debug.cpp \
    src/core/Geometry.cpp \
    src/core/Gizmo.cpp \
    src/core/GLCanvas.cpp \
    src/core/GLSLProgram.cpp \
    src/core/GLSLShader.cpp \
    src/core/GPUGeometry.cpp \
    src/core/Material.cpp \
    src/core/OrthographicCamera.cpp \
    src/core/PerspectiveCamera.cpp \
    src/core/Scene.cpp \
    src/core/SceneLoader.cpp \
    src/core/Texture.cpp \
    src/information-measures/PolygonalI1.cpp \
    src/information-measures/PolygonalI2.cpp \
    src/information-measures/PolygonalI3.cpp \
    src/information-measures/Measure.cpp \
    src/information-measures/ProjectedLocalMeasurePVO.cpp \
    src/information-measures/VisibilityChannelHistogram.cpp \
    src/HistogramBuilder.cpp \
    src/Main.cpp \
    src/MainModuleController.cpp \
    src/MainWindow.cpp \
    src/ModuleController.cpp \
    src/ModuleTabWidget.cpp \
    src/NBestViews.cpp \
    src/ObscuranceMap.cpp \
    src/SphereOfViewpoints.cpp \
    src/SpherePointCloud.cpp \
    src/Tools.cpp \
    src/ViewpointMeasureSlider.cpp \
    src/ViewpointsMesh.cpp

HEADERS  += \
    inc/core/AxisAlignedBoundingBox.h \
    inc/core/BoundingSphere.h \
    inc/core/Camera.h \
    inc/core/Debug.h \
    inc/core/Geometry.h \
    inc/core/Gizmo.h \
    inc/core/GLCanvas.h \
    inc/core/GLSLProgram.h \
    inc/core/GLSLShader.h \
    inc/core/GPUGeometry.h \
    inc/core/Material.h \
    inc/core/OrthographicCamera.h \
    inc/core/PerspectiveCamera.h \
    inc/core/Scene.h \
    inc/core/SceneLoader.h \
    inc/core/Texture.h \
    inc/information-measures/PolygonalI1.h \
    inc/information-measures/PolygonalI2.h \
    inc/information-measures/PolygonalI3.h \
    inc/information-measures/Measure.h \
    inc/information-measures/ProjectedLocalMeasurePVO.h \
    inc/information-measures/VisibilityChannelHistogram.h \
    inc/HistogramBuilder.h \
    inc/MainModuleController.h \
    inc/MainWindow.h \
    inc/ModuleController.h \
    inc/ModuleTabWidget.h \
    inc/NBestViews.h \
    inc/ObscuranceMap.h \
    inc/SphereOfViewpoints.h \
    inc/SpherePointCloud.h \
    inc/Tools.h \
    inc/ViewpointMeasureSlider.h \
    inc/ViewpointsMesh.h

FORMS    += \
    forms/MainModule.ui \
    forms/MainWindow.ui

OTHER_FILES += \
    shaders/Basic.vert \
    shaders/ColorPerFace.frag \
    shaders/DualPeelingBlend.frag \
    shaders/DualPeelingFinal.frag \
    shaders/DualPeelingInit.frag \
    shaders/DualPeelingPeel.frag \
    shaders/DualPeelingPeel.vert \
    shaders/Normalize.frag \
    shaders/Normalize.vert \
    shaders/Projection.frag \
    shaders/Projection.vert \
    shaders/Reflect.frag \
    shaders/Reflect.vert \
    shaders/ShadeFragment.frag \
    shaders/ShadePerVertexColor.frag \
    shaders/ThermalScale.frag \
    shaders/Transfer.frag \
    shaders/Transfer.vert \
    documentation/Quoniam.vpp \
    ../TODO.txt

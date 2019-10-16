win32 {
        CONFIG += windows
        DEFINES += STANDALONE
        LIBS += -static-libgcc
}

unix {
	MOC_DIR = .moc
	OBJECTS_DIR = .obj
}

macx{
	MOC_DIR = .moc
	OBJECTS_DIR = .obj
}

TEMPLATE = app	
LIBS +=  -L../../../lib -lpandore
INCLUDEPATH=../../../include
CONFIG += qt thread release warn_off

HEADERS += imagevisu.h
SOURCES += pvisu.cpp
HEADERS += imagemodel.h
SOURCES += imagemodel.cpp

HEADERS += viewer.h
HEADERS += canvasview.h
SOURCES += canvasview.cpp
HEADERS += valuesview.h
SOURCES += valuesview.cpp
HEADERS += histogramview.h
SOURCES += histogramview.cpp
HEADERS += profileview.h
SOURCES += profileview.cpp
HEADERS += propertiesview.h
SOURCES += propertiesview.cpp

HEADERS += controller.h
HEADERS += planeselector.h
SOURCES += planeselector.cpp
HEADERS += thresholdselector.h
SOURCES += thresholdselector.cpp
HEADERS += zoomselector.h
SOURCES += zoomselector.cpp
HEADERS += dynamicselector.h
SOURCES += dynamicselector.cpp
HEADERS += colorselector.h
SOURCES += colorselector.cpp
HEADERS += rotateselector.h
SOURCES += rotateselector.cpp

HEADERS += infoview.h
SOURCES += infoview.cpp

TARGET = ../../../bin/pvisu

DEFINES += MAIN

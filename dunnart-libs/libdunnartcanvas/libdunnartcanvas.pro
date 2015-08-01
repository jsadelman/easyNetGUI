
TEMPLATE = lib
TARGET = dunnartcanvas

CONFIG += shared

DEPENDPATH +=  .. .
INCLUDEPATH += .. .

CONFIG +=  qt 
QT += xml svg widgets

include(../common_options.pri)

DEFINES += NOGRAPHVIZ

win32 {
LIBS += -Wl,--export-all-symbols -Wl,--no-whole-archive
}
LIBS += -L$$DLLDESTDIR -lavoid -lvpsc -ltopology -lcola 

# Input
#RESOURCES += \
#	libdunnartcanvas.qrc



SOURCES += \ 
#	FMMLayout.cpp \
        oldcanvas.cpp \
	canvasitem.cpp \
	cluster.cpp \
	distribution.cpp \
	freehand.cpp \
	graphdata.cpp \
	graphlayout.cpp \
	graphvizlayout.cpp \
	guideline.cpp \
	indicator.cpp \
	interferencegraph.cpp \
	nearestpoint.cpp \
	placement.cpp \
	polygon.cpp \
	separation.cpp \
	shape.cpp \
	textshape.cpp \
	undo.cpp \
	visibility.cpp \
	canvas.cpp \
	canvasview.cpp \
	utility.cpp \
#	gmlgraph.cpp \
	template-constraints.cpp \
	templates.cpp \
	relationship.cpp \
	handle.cpp \
	svgshape.cpp \
	canvastabwidget.cpp \
#	ui/layoutproperties.cpp \
#	ui/zoomlevel.cpp \
#	ui/createalignment.cpp \
#	ui/createtemplate.cpp \
#	ui/createdistribution.cpp \
#	ui/createseparation.cpp \
#	ui/connectorproperties.cpp \
#	ui/propertieseditor.cpp \
    pluginshapefactory.cpp \
    pluginfileiofactory.cpp \
#    ui/shapepickerdialog.cpp \
    connectorhandles.cpp \
#    ui/undohistorydialog.cpp \
    stronglyconnectedcomponents.cpp \
    connectionpininfo.cpp \
    canvasapplication.cpp \
    pluginapplicationmanager.cpp \
#    ui/canvasoverview.cpp
    dunnart_connector.cpp

HEADERS += \
#	FMMLayout.h \
        oldcanvas.h \
	canvasitem.h \
	cluster.h \
	distribution.h \
	freehand.h \
#	gmlgraph.h \
	graphdata.h \
	graphlayout.h \
	graphvizlayout.h \
	guideline.h \
	indicator.h \
	interferencegraph.h \
	nearestpoint.h \
	placement.h \
	polygon.h \
	separation.h \
	shape.h \
	shared.h \
	textshape.h \
	undo.h \
	visibility.h \
	canvas.h \
	canvasview.h \
	template-constraints.h \
	templates.h \
	utility.h \
#	githash.h \
	relationship.h \
	handle.h \
	svgshape.h \
	canvastabwidget.h \
#	ui/layoutproperties.h \
#	ui/zoomlevel.h \
#	ui/createalignment.h \
#	ui/createtemplate.h \
#	ui/createdistribution.h \
#	ui/createseparation.h \
#	ui/connectorproperties.h \
#	ui/propertieseditor.h \
    pluginshapefactory.h \
    pluginfileiofactory.h \
#    ui/shapepickerdialog.h \
    connectorhandles.h \
#    ui/undohistorydialog.h \
    stronglyconnectedcomponents.h \
    connectionpininfo.h \
    shapeplugininterface.h \
    fileioplugininterface.h \
    applicationplugininterface.h \
    canvasapplication.h \
    pluginapplicationmanager.h \
#    ui/canvasoverview.h
    dunnart_connector.h













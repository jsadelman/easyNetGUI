#-------------------------------------------------
#
# Project created by QtCreator 2014-08-07T12:24:13
#
#-------------------------------------------------


QT      += core gui xml
QT      += webkit webkitwidgets
QT      += svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = easyNet
TEMPLATE = app


SOURCES +=  \
    main.cpp \
    treeitem.cpp \
    treemodel.cpp \
    querycontext.cpp \
    objexplorer.cpp \
    arrow.cpp \
   diagramitem.cpp \
   diagramscene.cpp \
    diagramtextitem.cpp \
    designwindow.cpp \
    highlighter.cpp \
    codeeditor.cpp \
    editwindow.cpp \
    plotwindow.cpp \
    lazynut.cpp \
    sessionmanager.cpp \
    commandsequencer.cpp \
        domitem.cpp \
    easyNetMainWindow.cpp \
    macrostate.cpp \
    lazynutjob.cpp \
    lazynutmacro.cpp \
    answerformatterfactory.cpp \
    answerformatter.cpp \
    lazynutjobparam.cpp \
    macroqueue.cpp \
    plotsettingsbasewidget.cpp \
    plotsettingsform.cpp \
    dataframemodel.cpp \
    lazynutlistmenu.cpp \
    xmlelement.cpp \
    lazynutlistcombobox.cpp \
    lazynutlistwidget.cpp \
    lazynutpairedlistwidget.cpp \
    bracketedparser.cpp \
 #   lazynutobjtablemodel.cpp \
    lazynutobjectmodel.cpp \
    lazynutobject.cpp \
    expandtofillbutton.cpp \
    objectcatalogue.cpp \
    lazynutobjectcacheelem.cpp \
    proxymodelextrarows.cpp \
    descriptionupdater.cpp \
    objectcataloguefilter.cpp \
    simplelistmodel.cpp \
    selectfromlistmodel.cpp \
    pairedlistwidget.cpp

HEADERS  += \
    treeitem.h \
    treemodel.h \
    querycontext.h \
    objexplorer.h \
   arrow.h \
    diagramitem.h \
    diagramscene.h \
    diagramtextitem.h \
    designwindow.h \
    highlighter.h \
    codeeditor.h \
    editwindow.h \
    plotwindow.h \
    jobqueue.h \
    lazynut.h \
    sessionmanager.h \
    commandsequencer.h \
    enumclasses.h \
        domitem.h \
    easyNetMainWindow.h \
    macrostate.h \
    lazynutjob.h\
    lazynutmacro.h \
    answerformatterfactory.h \
    answerformatter.h \
    lazynutjobparam.h \
    macroqueue.h \
    plotsettingsbasewidget.h \
    plotsettingsform.h \
    dataframemodel.h \
    lazynutlistmenu.h \
    xmlelement.h \
    lazynutlistcombobox.h \
    lazynutlistwidget.h \
    lazynutpairedlistwidget.h \
    bracketedparser.h \
 #   lazynutobjtablemodel.h \
    lazynutobjectmodel.h \
    lazynutobject.h \
    expandtofillbutton.h \
    objectcatalogue.h \
    lazynutobjectcacheelem.h \
    proxymodelextrarows.h \
    descriptionupdater.h \
    objectcataloguefilter.h \
    simplelistmodel.h \
    selectfromlistmodel.h \
    pairedlistwidget.h

RESOURCES   =	diagramscene.qrc \
    icons.qrc \
    #html.qrc

QMAKE_CXXFLAGS += -std=c++11

DISTFILES += \
    images/test.svg


CONFIG += c++11

ICON = images/easyNet.hqx

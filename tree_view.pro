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
    lazynutobject.cpp \
    easyNetMainWindow.cpp \
    macrostate.cpp \
    lazynutjob.cpp \
    lazynutmacro.cpp \
    answerformatterfactory.cpp \
    answerformatter.cpp \
    lazynutjobparam.cpp

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
    lazynutobject.h \
    easyNetMainWindow.h \
    macrostate.h \
    lazynutjob.h\
    lazynutmacro.h \
    answerformatterfactory.h \
    answerformatter.h \
    lazynutjobparam.h


RESOURCES   =	diagramscene.qrc \
    icons.qrc \
    #html.qrc

QMAKE_CXXFLAGS += -std=c++11

DISTFILES += \
    images/test.svg


CONFIG += c++11

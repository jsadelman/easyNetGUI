#-------------------------------------------------
#
# Project created by QtCreator 2014-08-07T12:24:13
#
#-------------------------------------------------


QT      += core gui xml
QT      += webkit webkitwidgets
QT      += svg
QT      += sql

DUNNARTHEADERS = $${PWD}/../dunnart-libs
INCLUDEPATH = .. \
$$DUNNARTHEADERS \
$$DUNNARTHEADERS/libvpsc \
$$DUNNARTHEADERS/libcola \
$$DUNNARTHEADERS/libtopology \
$$DUNNARTHEADERS/libdunnartcanvas \

DEPENDPATH =  .. .

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = easyNet
TEMPLATE = app


LIBS += -L$${PWD}/../dll -ldunnartcanvas -lavoid

SOURCES +=  \
    main.cpp \
    treeitem.cpp \
    treemodel.cpp \
    querycontext.cpp \
    objexplorer.cpp \
 #   arrow.cpp \
 #  diagramitem.cpp \
   diagramscene.cpp \
    diagramtextitem.cpp \
#    designwindow.cpp \
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
    dataframewindow.cpp \
    tableeditor.cpp \
    inputcmdline.cpp \
    cmdoutput.cpp \
    lazynutconsole.cpp \
    simplelistmodel.cpp \
    selectfromlistmodel.cpp \
    pairedlistwidget.cpp \
    maxminpanel.cpp \
    assistant.cpp \
    findfiledialog.cpp \
    textedit.cpp \
    helpwindow.cpp \
    parameditor.cpp \
    trialwidget.cpp \
    mycombobox.cpp \
    commandlog.cpp \
    scripteditor.cpp \
    console.cpp \
    finddialog.cpp \
    debuglog.cpp \
    plotviewer.cpp \
    diagramscenetabwidget.cpp \
    diagramwindow.cpp \
    diagramview.cpp

HEADERS  += \
    treeitem.h \
    treemodel.h \
    querycontext.h \
    objexplorer.h \
 #  arrow.h \
 #   diagramitem.h \
    diagramscene.h \
    diagramtextitem.h \
 #   designwindow.h \
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
    dataframewindow.h \
    tableeditor.h \
    inputcmdline.h \
    cmdoutput.h \
    lazynutconsole.h \
    simplelistmodel.h \
    selectfromlistmodel.h \
    pairedlistwidget.h \
    maxminpanel.h \
    assistant.h \
    findfiledialog.h \
    textedit.h \
    helpwindow.h \
    parameditor.h \
    trialwidget.h \
    mycombobox.h \
    commandlog.h \
    scripteditor.h \
    console.h \
    finddialog.h \
    debuglog.h \
    plotviewer.h \
    diagramscenetabwidget.h \
    diagramwindow.h \
    diagramview.h

RESOURCES   =	diagramscene.qrc \
    icons.qrc \
    documentation.qrc
    #html.qrc

QMAKE_CXXFLAGS += -std=c++11

DISTFILES += \
    images/test.svg \
    images/image001.jpg \
    documentation/easyNetHelp.qch \
    documentation/easyNetHelp.qhc \


CONFIG += c++11

ICON = images/easyNet.hqx

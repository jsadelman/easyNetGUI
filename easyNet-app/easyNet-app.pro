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

TARGET = easyNet1
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
    plotsettingswindow.cpp \
    lazynut.cpp \
    sessionmanager.cpp \
    commandsequencer.cpp \
        domitem.cpp \
    easyNetMainWindow.cpp \
    macrostate.cpp \
    answerformatterfactory.cpp \
    answerformatter.cpp \
    lazynutjobparam.cpp \
    plotsettingsbasewidget.cpp \
    plotsettingsform.cpp \
    dataframemodel.cpp \
    xmlelement.cpp \
    lazynutlistwidget.cpp \
    lazynutpairedlistwidget.cpp \
    bracketedparser.cpp \
 #   lazynutobjtablemodel.cpp \
    lazynutobjectmodel.cpp \
    lazynutobject.cpp \
    expandtofillbutton.cpp \
 #   objectcatalogue.cpp \
    lazynutobjectcacheelem.cpp \
    proxymodelextrarows.cpp \
    objectupdater.cpp \
    objectcachefilter.cpp \
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
    diagramview.cpp \
    box.cpp \
    arrow.cpp \
    xmlform.cpp \
    objectxml.cpp \
    trialxml.cpp \
    trialeditor.cpp \
    tableviewer2.cpp \
    settingsform.cpp \
    settingsformdialog.cpp \
    xmlaccessor.cpp \
    objectnamevalidator.cpp \
    lazynutjob.cpp \
    trialdataframemodel.cpp \
    tablewindow.cpp \
    tablewidgetinterface.cpp \
    tabstablewidget.cpp \
    combotablewidget.cpp \
    objectcache.cpp \
    resultswindow_if.cpp

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
    plotsettingswindow.h \
    lazynut.h \
    sessionmanager.h \
    commandsequencer.h \
    enumclasses.h \
        domitem.h \
    easyNetMainWindow.h \
    macrostate.h \
    answerformatterfactory.h \
    answerformatter.h \
    lazynutjobparam.h \
    plotsettingsbasewidget.h \
    plotsettingsform.h \
    dataframemodel.h \
    xmlelement.h \
    lazynutlistwidget.h \
    lazynutpairedlistwidget.h \
    bracketedparser.h \
 #   lazynutobjtablemodel.h \
    lazynutobjectmodel.h \
    lazynutobject.h \
    expandtofillbutton.h \
    lazynutobjectcacheelem.h \
    proxymodelextrarows.h \
    objectupdater.h \
    objectcachefilter.h \
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
    diagramview.h \
    box.h \
    arrow.h \
    xmlform.h \
    objectxml.h \
    trialxml.h \
    trialeditor.h \
    tableviewer2.h \
   settingsform.h \
    settingsformdialog.h \
    xmlaccessor.h \
    objectnamevalidator.h \
    lazynutjob.h \
    jobqueue.h \
    trialdataframemodel.h \
    tablewindow.h \
    tablewidgetinterface.h \
    tabstablewidget.h \
    combotablewidget.h \
    objectcache.h \
    resultswindow_if.h

RESOURCES   =	diagramscene.qrc \
    icons.qrc \
    documentation.qrc
    #html.qrc

QMAKE_CXXFLAGS += -std=c++11

DISTFILES += \
    images/test.svg \
    images/image001.jpg \
    images/clipboard.png \
    images/plot_settings.png \
    images/refresh.png


CONFIG += c++11

ICON = images/easyNet.hqx


macx {
  libdunnartcanvas.path = Contents/MacOS
  libdunnartcanvas.files  = $${PWD}/../dll/libdunnartcanvas.1.dylib
  libdunnartcanvas.files += $${PWD}/../dll/libavoid.1.dylib
  libdunnartcanvas.files += $${PWD}/../dll/libvpsc.1.dylib
  libdunnartcanvas.files += $${PWD}/../dll/libcola.1.dylib
  libdunnartcanvas.files += $${PWD}/../dll/libtopology.1.dylib
  QMAKE_BUNDLE_DATA += libdunnartcanvas
}


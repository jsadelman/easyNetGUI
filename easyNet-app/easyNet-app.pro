#-------------------------------------------------
#
# Project created by QtCreator 2014-08-07T12:24:13
#
#-------------------------------------------------


QT      += core gui xml
QT      += svg
QT      += sql


DEPENDPATH =  .. .

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = easyNet3
TEMPLATE = app


LIBS +=

SOURCES +=  \
    main.cpp \
    treeitem.cpp \
    treemodel.cpp \
    querycontext.cpp \
    objexplorer.cpp \
   diagramitem.cpp \
   diagramscene.cpp \
    diagramtextitem.cpp \
#    designwindow.cpp \
    highlighter.cpp \
    codeeditor.cpp \
    editwindow.cpp \
#    plotsettingswindow.cpp \
    lazynut.cpp \
    sessionmanager.cpp \
    commandsequencer.cpp \
        domitem.cpp \
    easyNetMainWindow.cpp \
    answerformatterfactory.cpp \
    answerformatter.cpp \
    lazynutjobparam.cpp \
    plotsettingsbasewidget.cpp \
    plotsettingsform.cpp \
    dataframemodel.cpp \
    xmlelement.cpp \
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
    inputcmdline.cpp \
    cmdoutput.cpp \
#    lazynutconsole.cpp \
    simplelistmodel.cpp \
    selectfromlistmodel.cpp \
    pairedlistwidget.cpp \
    maxminpanel.cpp \
    assistant.cpp \
    findfiledialog.cpp \
    textedit.cpp \
    helpwindow.cpp \
    trialwidget.cpp \
    mycombobox.cpp \
    commandlog.cpp \
    scripteditor.cpp \
    console.cpp \
    finddialog.cpp \
    debuglog.cpp \
#    plotviewer_old.cpp \
    diagramscenetabwidget.cpp \
    diagramscenestackedwidget.cpp \
    diagramwindow.cpp \
    diagramview.cpp \
    box.cpp \
    arrow.cpp \
    xmlform.cpp \
    objectxml.cpp \
    trialxml.cpp \
    trialeditor.cpp \
    settingsform.cpp \
    dataframemergesettingsformdialog.cpp \
    settingsformdialog.cpp \
    xmlaccessor.cpp \
    objectnamevalidator.cpp \
    lazynutjob.cpp \
#    prettyheadersmodel.cpp \
 #   tablewindow.cpp \
 #   tablewidgetinterface.cpp \
 #   tabstablewidget.cpp \
 #   combotablewidget.cpp \
    objectcache.cpp \
    resultswindow_if.cpp \
    dataviewer.cpp \
    ui_dataviewer.cpp \
    ui_datatabsviewer.cpp \
    ui_datacomboviewer.cpp \
    dataframeviewer.cpp \
    dataviewerdispatcher.cpp \
    dataframeviewerdispatcher.cpp \
    plotviewer.cpp \
    plotviewerdispatcher.cpp \
    booleandelegate.cpp \
    closebutton.cpp \
    historywidget.cpp \
    historytreemodel.cpp \
    trialruninfo.cpp \
    xmlmodel.cpp \
    settingsxml.cpp \
    xmldisplay.cpp \
    modelsettingsdisplay.cpp \
    settingswidget.cpp \
    floatingdialogwindow.cpp \
    diagramscenestackedwidget.cpp

HEADERS  += \
    treeitem.h \
    treemodel.h \
    querycontext.h \
    objexplorer.h \
   arrow.h \
     diagramitem.h \
    diagramscene.h \
    diagramtextitem.h \
 #   designwindow.h \
    highlighter.h \
    codeeditor.h \
    editwindow.h \
#    plotsettingswindow.h \
    lazynut.h \
    sessionmanager.h \
    commandsequencer.h \
    enumclasses.h \
        domitem.h \
    easyNetMainWindow.h \
    answerformatterfactory.h \
    answerformatter.h \
    lazynutjobparam.h \
    plotsettingsbasewidget.h \
    plotsettingsform.h \
    dataframemodel.h \
    xmlelement.h \
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
    inputcmdline.h \
    cmdoutput.h \
#    lazynutconsole.h \
    simplelistmodel.h \
    selectfromlistmodel.h \
    pairedlistwidget.h \
    maxminpanel.h \
    assistant.h \
    findfiledialog.h \
    textedit.h \
    helpwindow.h \
    trialwidget.h \
    mycombobox.h \
    commandlog.h \
    scripteditor.h \
    console.h \
    finddialog.h \
    debuglog.h \
#    plotviewer_old.h \
    diagramscenetabwidget.h \
    diagramscenestackedwidget.h \
    diagramwindow.h \
    diagramview.h \
    box.h \
    arrow.h \
    xmlform.h \
    objectxml.h \
    trialxml.h \
    trialeditor.h \
    settingsform.h \
    settingsformdialog.h \
    dataframemergesettingsformdialog.h \
    xmlaccessor.h \
    objectnamevalidator.h \
    lazynutjob.h \
    jobqueue.h \
#    prettyheadersmodel.h \
#    tablewindow.h \
#    tablewidgetinterface.h \
#    tabstablewidget.h \
#    combotablewidget.h \
    objectcache.h \
    resultswindow_if.h \
    dataviewer.h \
    ui_dataviewer.h \
    ui_datatabsviewer.h \
    ui_datacomboviewer.h \
    dataframeviewer.h \
    dataviewerdispatcher.h \
    dataframeviewerdispatcher.h \
    plotviewer.h \
    plotviewerdispatcher.h \
    booleandelegate.h \
    closebutton.h \
    historywidget.h \
    historytreemodel.h \
    trialruninfo.h \
    xmlmodel.h \
    settingsxml.h \
    xmldisplay.h \
    modelsettingsdisplay.h \
    settingswidget.h \
    floatingdialogwindow.h \
    diagramscenestackedwidget.h


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
    images/refresh.png \
    images/faint.zeb.png \
    images/faint.zeb2.png


CONFIG += c++11

ICON = images/easyNet.hqx



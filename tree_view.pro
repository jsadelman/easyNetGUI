#-------------------------------------------------
#
# Project created by QtCreator 2014-08-07T12:24:13
#
#-------------------------------------------------

# flex and bison settings
# http://hipersayanx.blogspot.co.uk/2013/03/using-flex-and-bison-with-qt.html
win32 {
LOCAL_FLEX_BISON_DIR = $$(FLEX_BISON_DIR)
isEmpty(LOCAL_FLEX_BISON_DIR) {
error(Please set FLEX_BISON_DIR environment variable to the directory where the win_flex and win_bison exe files are located. Please use fwd slashes "/".)
}

#FLEX_BISON_DIR = D:/Programs/win_flex_bison
#FLEX_BISON_DIR = C:/Users/Uasolo/faldone/programmi/win_flex_bison
FLEX = $${LOCAL_FLEX_BISON_DIR}/win_flex
BISON = $${LOCAL_FLEX_BISON_DIR}/win_bison
}
unix {
FLEX = $$system(which flex)
BISON = $$system(which bison)
}



FLEXSOURCES = scanner.l
BISONSOURCES = parser.y
OTHER_FILES += \
$$FLEXSOURCES \
$$BISONSOURCES

flexsource.input = FLEXSOURCES
#flexsource.output = ${QMAKE_FILE_BASE}.cpp
flexsource.output = $${_PRO_FILE_PWD_}/scanner.cpp
#flexsource.commands = $$FLEX -o ${QMAKE_FILE_BASE}.cpp ${QMAKE_FILE_IN}
flexsource.commands = $$FLEX -o $${_PRO_FILE_PWD_}/scanner.cpp ${QMAKE_FILE_IN}
flexsource.variable_out = SOURCES
flexsource.name = Flex Sources ${QMAKE_FILE_IN}
flexsource.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += flexsource
#flexheader.input = FLEXSOURCES
#flexheader.output = ${QMAKE_FILE_BASE}.h
#flexheader.output = $${_PRO_FILE_PWD_}/scanner.h
#flexheader.commands = @true
#flexheader.variable_out = HEADERS
#flexheader.name = Flex Headers ${QMAKE_FILE_IN}
#flexheader.CONFIG += target_predeps no_link
#QMAKE_EXTRA_COMPILERS += flexheader
bisonsource.input = BISONSOURCES
#bisonsource.output = ${QMAKE_FILE_BASE}.cpp
bisonsource.output = $${_PRO_FILE_PWD_}/parser.cpp
bisonsource.commands = $${BISON} --defines=$${_PRO_FILE_PWD_}/parser.h -o $${_PRO_FILE_PWD_}/parser.cpp ${QMAKE_FILE_IN}
bisonsource.variable_out = SOURCES
bisonsource.name = Bison Sources ${QMAKE_FILE_IN}
bisonsource.CONFIG += target_predeps
QMAKE_EXTRA_COMPILERS += bisonsource
bisonheader.input = BISONSOURCES
bisonheader.output = $${_PRO_FILE_PWD_}/parser.h
bisonheader.commands = @true
bisonheader.variable_out = HEADERS
bisonheader.name = Bison Headers ${QMAKE_FILE_IN}
bisonheader.CONFIG += target_predeps no_link
QMAKE_EXTRA_COMPILERS += bisonheader


QT      += core gui xml
QT      += webkit webkitwidgets
QT      += svg

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = easyNet
TEMPLATE = app


SOURCES += driver.cpp nmConsole.cpp \
    main.cpp \
    treeitem.cpp \
    treemodel.cpp \
    querycontext.cpp \
    objexplorer.cpp \
    lazynutobj.cpp \
    comboboxdelegate.cpp \
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
    commandsequencer.cpp

HEADERS  += \
    scanner.h driver.h  FlexLexer.h nmConsole.h \
    treeitem.h \
    treemodel.h \
    querycontext.h \
    objexplorer.h \
    comboboxdelegate.h \
    lazynutobj.h \
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
    enumclasses.h
  #  y.tab.h  location.hh position.hh stack.hh parserwindow.h tree.h

RESOURCES   =	diagramscene.qrc \
    icons.qrc \
    #html.qrc

QMAKE_CXXFLAGS += -std=c++11

DISTFILES += \
    images/test.svg




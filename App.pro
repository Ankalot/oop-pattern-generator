QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    baseelement.cpp \
    classmethods.cpp \
    classtext.cpp \
    codegenerator.cpp \
    element.cpp \
    exportwindow.cpp \
    importwindow.cpp \
    indicator.cpp \
    main.cpp \
    mainwindow.cpp \
    parsedelements.cpp \
    vectorelement.cpp

HEADERS += \
    argument.h \
    baseelement.h \
    classmethod.h \
    classmethods.h \
    classtext.h \
    codegenerator.h \
    element.h \
    exportwindow.h \
    importwindow.h \
    indicator.h \
    mainwindow.h \
    parsedelements.h \
    vectorelement.h

FORMS += \
    exportwindow.ui \
    importwindow.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    resource.qrc

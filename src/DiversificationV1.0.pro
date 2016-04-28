#-------------------------------------------------
#
# Project created by QtCreator 2015-04-01T10:22:14
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
QT		 += svg

TARGET = Diversification
TEMPLATE = app
ICON = diversification.icns
INCLUDEPATH += /usr/local/include
LIBS += -L /usr/local/lib


SOURCES += main.cpp\
	diversificationwindow.cpp \
	qscaletree.cpp \
	qtreedrawer.cpp \
	qtreedrawwidget.cpp \
	qtreefossildrawer.cpp \
	Fossil.c \
	FossilInt.c \
	MinimizeNLOpt.c \
	Model.c \
	Random.c \
	SimulFossil.c \
	SimulTree.c \
	Tree.c \
	Uncertainty.c \
	Utils.c \
	gnode.cpp \
	gtreegeometry.cpp \
	gtreewidget.cpp \
	gtreefossilgeometry.cpp \
	gtreefossilview.cpp \
	gfossil.cpp \
	gaddfossildialog.cpp \
	geditfossildialog.cpp \
	gfossiltablemodel.cpp \
	qspinboxdelegate.cpp \
	qtableresultmodel.cpp \
	gsaveoptiondialog.cpp \
	gsaveoption.cpp \
	gutil.cpp \
	DrawFossilTreePSTricks.c \
	TreeExtras.c \
	goptiondialog.cpp \
	gestimater.cpp \
	gminimizerthread.cpp \
	gnodedialog.cpp \
	gleafdialog.cpp \
	ghelpdialog.cpp

HEADERS  += diversificationwindow.h \
	Fossil.h \
	FossilInt.h \
	MinimizeNLOpt.h \
	Model.h \
	qscaletree.h \
	qtreedrawer.h \
	qtreedrawwidget.h \
	qtreefossildrawer.h \
	Random.h \
	SimulFossil.h \
	SimulTree.h \
	Tree.h \
	Uncertainty.h \
	Utils.h \
	gnode.h \
	gtreegeometry.h \
	gtreewidget.h \
	gtreefossilgeometry.h \
	gtreefossilview.h \
	gfossil.h \
	gaddfossildialog.h \
	geditfossildialog.h \
	gfossiltablemodel.h \
	qspinboxdelegate.h \
	qtableresultmodel.h \
	gsaveoptiondialog.h \
	gsaveoption.h \
	gutil.h \
	DrawFossilTreePSTricks.h \
	TreeExtras.h \
	goptiondialog.h \
	gestimater.h \
	gminimizerthread.h \
	gnodedialog.h \
	gleafdialog.h \
	ghelpdialog.h

FORMS    += diversificationwindow.ui \
	qhelpdialog.ui \
	gaddfossildialog.ui \
	geditfossildialog.ui \
	gsaveoptiondialog.ui \
	goptiondialog.ui \
	gnodedialog.ui \
	gleafdialog.ui \
	ghelpdialog.ui

RESOURCES += \
 data/diversification.qrc


unix|win32: LIBS += -lgsl

unix|win32: LIBS += -lgslcblas

unix|win32: LIBS += -lnlopt

DISTFILES += \
	data/Help.pdf

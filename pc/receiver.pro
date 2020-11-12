
TARGET = receiver
TEMPLATE = app
QT += core gui bluetooth widgets serialport


SOURCES += \
	BluetoothDeviceInfo.cpp \
	main.cpp\
	BLEInterface.cpp \
	BitWidget.cpp \
	MainWindow.cpp

HEADERS  += \
	BitWidget.h \
	BLEInterface.h \
	BitWidget.h \
	BluetoothDeviceInfo.h \
	MainWindow.h \
	jstream.h

FORMS    += \
	MainWindow.ui

RESOURCES += \
	resources/resources.qrc

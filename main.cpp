#include <QtGui/QApplication>
#include <QLibraryInfo>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	a.setQuitOnLastWindowClosed(false);
	MainWindow w;
	return a.exec();
}

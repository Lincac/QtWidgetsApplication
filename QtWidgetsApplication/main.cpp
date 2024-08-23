#include "QtWidgetsApplication.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
    QtWidgetsApplication w;
    w.show();
    return a.exec();
}

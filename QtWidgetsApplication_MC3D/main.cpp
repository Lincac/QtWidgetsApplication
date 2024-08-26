#include "QtWidgetsApplication_MC3D.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
	CreateCircleImageData();

	QCoreApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QApplication a(argc, argv);
    QtWidgetsApplication_MC3D w;
    w.show();
    return a.exec();
}

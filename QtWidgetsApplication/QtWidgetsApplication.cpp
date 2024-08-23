#include "QtWidgetsApplication.h"

unsigned int texture = -1;
unsigned int vbo = -1;
unsigned int vbo = -1;

QtWidgetsApplication::QtWidgetsApplication(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);
	openglWidget = new OpenGLWidget();
	setCentralWidget(openglWidget);
	osOpenGLWidget = new OSOpenGLWidget();
}

QtWidgetsApplication::~QtWidgetsApplication()
{}



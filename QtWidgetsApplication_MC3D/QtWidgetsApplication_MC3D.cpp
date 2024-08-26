#include "QtWidgetsApplication_MC3D.h"

unsigned int vbo = -1;
int vertices = 0;
glm::vec3 centre = glm::vec3(dimension[0] / 2, dimension[1] / 2, dimension[2] / 2);

void saveToPPM(const unsigned char* imageData, const std::string& fileName)
{
	std::ofstream file(fileName, std::ios::out | std::ios::binary);
	if (!file) {
		std::cerr << "Could not open the file for writing." << std::endl;
		return;
	}

	file << "P5\n" << dimension[0] << " " << dimension[1] << "\n255\n";
	file.write(reinterpret_cast<const char*>(imageData), dimension[0] * dimension[1]);
	file.close();
}

unsigned char* CreateCircleImageData()
{
	unsigned char* imageData = new unsigned char[size];
	memset(imageData, 0, size * sizeof(unsigned char));

	glm::vec3 centre = glm::vec3(dimension[0] / 2, dimension[1] / 2, dimension[2] / 2);
	float r = 50.0f;

	for (int z = 1; z < dimension[2] - 2; z++)
	{
		for (int y = 0; y < dimension[1]; y++)
		{
			for (int x = 0; x < dimension[0]; x++)
			{
				glm::vec3 pos = glm::vec3(x, y, z);
				float d = glm::distance(centre, pos);

				if (d <= r)
				{
					imageData[z * dimension[0] * dimension[1] + y * dimension[0] + x] = 255;
				}
			}
		}

		//std::string name = "circle" + std::to_string(z) + ".ppm";
		//saveToPPM(imageData + z * dimension[0] * dimension[1], name);
	}

	return imageData;
}

QtWidgetsApplication_MC3D::QtWidgetsApplication_MC3D(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);
	osOpenGLWidget = new OSOpenGLWidget();
	openglWidget = new OpenGLWidget();
	setCentralWidget(openglWidget);
}

QtWidgetsApplication_MC3D::~QtWidgetsApplication_MC3D()
{}


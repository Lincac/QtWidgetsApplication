#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>
#include <glm/glm.hpp>

const size_t dimension[3] = { 128,128,120 };
const size_t size = 128 * 128 * 120;

const glm::ivec3 normal = glm::ivec3(0, 0, 1);
extern glm::vec3 centre;

extern void saveToPPM(const unsigned char* imageData, const std::string& fileName);
extern unsigned char* CreateCircleImageData();

#include <CL/cl.h>
#include <CL/cl_gl.h>

extern unsigned int vbo;
extern int vertices;

static int EdgeTable[16] =
{
	0b0000,0b1001,0b0011,0b1010,
	0b0110,0b1111,0b0101,0b1100,
	0b1100,0b0101,0b1111,0b0110,
	0b1010,0b0011,0b1001,0b0000
};

static int VertTable[16][4] =
{
	{-1,-1,-1,-1},
	{3,0,-1,-1},
	{0,1,-1,-1},
	{1,3,-1,-1},
	{2,1,-1,-1},
	{3,2,0,1},
	{2,0,-1,-1},
	{3,2,-1,-1},
	{3,2,-1,-1},
	{2,0,-1,-1},
	{3,2,0,1},
	{2,1,-1,-1},
	{1,3,-1,-1},
	{0,1,-1,-1},
	{3,0,-1,-1},
	{-1,-1,-1,-1}
};

class OpenCL
{
public:
	OpenCL()
	{
		int error = 0;

		cl_platform_id platform;
		error = clGetPlatformIDs(1, &platform, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		cl_device_id device;
		error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR,(cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR,(cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM,(cl_context_properties)platform,
			0
		};
		context = clCreateContext(properties, 1, &device, NULL, NULL, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		queue = clCreateCommandQueue(context, device, 0, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		std::ifstream input_file;
		input_file.open("D:/user/source/repos/QtWidgetsApplication/QtWidgetsApplication_MC3D/kernel.cl");

		std::stringstream codeStream;
		codeStream << input_file.rdbuf();
		input_file.close();
		std::string str = codeStream.str();
		const char* CLcode = str.c_str();

		cl_program program = clCreateProgramWithSource(context, 1, &CLcode, NULL, NULL);
		error = clBuildProgram(program, 1, &device, NULL, NULL, NULL);
		if (error != CL_SUCCESS)
		{
			size_t log_size;
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &log_size);
			char *log = (char *)malloc(log_size);
			clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, log_size, log, NULL);
			printf("Build log:\n%s\n", log);
			free(log);
		}
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		kernel = clCreateKernel(program, "Func", &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		std::cout << "OpenCL Context : " << context << std::endl;
		std::cout << "OpenCL Command Queue : " << queue << std::endl;
		std::cout << "OpenCL Kernel : " << kernel << std::endl;

		edgeTableMem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(EdgeTable), EdgeTable, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		vertTableMem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(VertTable), VertTable, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		unsigned char* imageData = CreateCircleImageData();

		imageMem = clCreateBuffer(context, CL_MEM_READ_ONLY, size * sizeof(unsigned char), NULL,&error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clEnqueueWriteBuffer(queue, imageMem, CL_TRUE, 0, size * sizeof(unsigned char), imageData, 0, NULL, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
	}

	void run()
	{
		int error = 0;
		vertices = 0;

		int D = -(normal[0] * (int)centre[0] + normal[1] * (int)centre[1] + normal[2] * (int)centre[2]);
		int planeParam[4] = { normal[0],normal[1],normal[2],D };
		cl_mem planeParamMem = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, sizeof(planeParam), planeParam, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		cl_mem verticesMem = clCreateBuffer(context, CL_MEM_READ_WRITE | CL_MEM_COPY_HOST_PTR, sizeof(int), &vertices, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		cl_mem clvboMem = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, vbo, &error);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &imageMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clSetKernelArg(kernel, 1, sizeof(cl_mem), &planeParamMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clSetKernelArg(kernel, 2, sizeof(cl_mem), &clvboMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clSetKernelArg(kernel, 3, sizeof(cl_mem), &verticesMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clSetKernelArg(kernel, 4, sizeof(cl_mem), &edgeTableMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		error = clSetKernelArg(kernel, 5, sizeof(cl_mem), &vertTableMem);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clEnqueueAcquireGLObjects(queue, 1, &clvboMem, 0, NULL, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clEnqueueNDRangeKernel(queue, kernel, 3, NULL, dimension, NULL, 0, NULL, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clEnqueueReleaseGLObjects(queue, 1, &clvboMem, 0, NULL, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clFinish(queue);
		if (error != CL_SUCCESS) throw std::runtime_error("error");

		error = clEnqueueReadBuffer(queue, verticesMem, CL_TRUE, 0, sizeof(int), &vertices, 0, NULL, NULL);
		if (error != CL_SUCCESS) throw std::runtime_error("error");
		std::cout << "OpenCL Generate Vertices : " << vertices << std::endl;

		error = clReleaseMemObject(planeParamMem);
		error = clReleaseMemObject(verticesMem);
		error = clReleaseMemObject(clvboMem);
	}

private:
	cl_kernel kernel;
	cl_context context;
	cl_command_queue queue;

	cl_mem edgeTableMem;
	cl_mem vertTableMem;
	cl_mem imageMem;
};

#include <QtGui/QOpenGLContext>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFunctions_3_3_Core>
#include <QtGui/QImageReader>

class OSOpenGLWidget
{
public:
	OSOpenGLWidget()
	{
		QSurfaceFormat format;
		format.setDepthBufferSize(24);
		format.setVersion(3, 3);
		format.setProfile(QSurfaceFormat::CoreProfile);

		osSurface = new QOffscreenSurface();
		osSurface->setFormat(format);
		osSurface->create();

		context = new QOpenGLContext();
		context->setFormat(format);
		context->create();

		context->setShareContext(QOpenGLContext::globalShareContext());

		std::cout << "OSOpenGLWidget Share Context : " << context->shareContext() << std::endl;

		context->shareContext()->makeCurrent(osSurface);

		initVBO();

		openCL = new OpenCL();
		openCL->run();

		context->shareContext()->doneCurrent();
	}

	void initVBO()
	{
		auto glFunc = context->shareContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

		size_t num = dimension[0] * 4 * 2 * 3;
		
		glFunc->glGenBuffers(1, &vbo);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glFunc->glBufferData(GL_ARRAY_BUFFER, sizeof(float) * num, nullptr, GL_DYNAMIC_DRAW);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

		std::cout << "OSOpenGLWidget VBO ID : " << vbo << std::endl;
	}

	void update()
	{
		context->shareContext()->makeCurrent(osSurface);
		openCL->run();
		context->shareContext()->doneCurrent();
	}

private:
	OpenCL* openCL;
	QOpenGLContext* context;
	QOffscreenSurface* osSurface;
};

#include <QtWidgets/QOpenGLWidget>

#include <glm/gtc/matrix_transform.hpp>

typedef struct _Camera
{
	glm::vec3 position;

	glm::vec3 front;
	glm::vec3 up;

	float fov;

	_Camera(glm::vec3 pos = glm::vec3(0))
	{
		position = pos;

		front = glm::vec3(0.0f, 0.0f, -1.0f);
		up = glm::vec3(0.0f, 1.0f, 0.0f);

		fov = 45.0f;
	}

	glm::mat4 GetViewMatrix()
	{
		return glm::lookAt(position, position + front, up);
	}

}Camera;

class OpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{

public:

	OpenGLWidget(QWidget *parent = nullptr)
		: QOpenGLWidget(parent)
	{
		camera = new Camera(glm::vec3(dimension[0] / 2, dimension[1] / 2, dimension[2] * 1.3));
	}

protected:

	virtual void initializeGL() override
	{
		context()->setShareContext(QOpenGLContext::globalShareContext());
		std::cout << "OpenGLWidget Share Context : " << context()->shareContext() << std::endl;

		initializeOpenGLFunctions();

		createShaderProgram();
	}

	virtual void paintGL() override
	{
		glClear(GL_COLOR_BUFFER_BIT);
		glClearColor(1, 1, 1, 1);

		glm::mat4 model = glm::mat4(1);
		glm::mat4 view = camera->GetViewMatrix();
		glm::mat4 projection = glm::perspective(camera->fov, (float)width() / height(), 0.01f, 1000.0f);

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "Model"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "View"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "Projection"), 1, GL_FALSE, &projection[0][0]);
		renderContour();
	}

	virtual void resizeGL(int w, int h) override
	{
		glViewport(0, 0, w, h);
	}

	void createShaderProgram()
	{
		std::string vertexCode;
		std::string fragmentCode;
		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try
		{
			vShaderFile.open("D:/user/source/repos/QtWidgetsApplication/QtWidgetsApplication_MC3D/shader.vs");
			fShaderFile.open("D:/user/source/repos/QtWidgetsApplication/QtWidgetsApplication_MC3D/shader.fs");
			std::stringstream vShaderStream, fShaderStream;
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			vShaderFile.close();
			fShaderFile.close();
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
		}
		catch (std::ifstream::failure& e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
		}

		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		unsigned int vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);

		unsigned int fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);

		program = glCreateProgram();
		glAttachShader(program, vertex);
		glAttachShader(program, fragment);
		glLinkProgram(program);

		glDeleteShader(vertex);
		glDeleteShader(fragment);

		std::cout << "OpenGLWidget Shader Program : " << program << std::endl;
	}

	void renderContour()
	{
		if (VAO == -1)
		{
			glGenVertexArrays(1, &VAO);
			glBindVertexArray(VAO);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);
			
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		//glBindBuffer(GL_ARRAY_BUFFER, vbo);
		//void* ptr = glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		//if (ptr)
		//{
		//	float* data = static_cast<float*>(ptr);
		//	std::cout << "pre vbo data" << std::endl;
		//	for (int i = 0; i < 2424; i += 3)
		//	{
		//		std::cout << data[i + 0] << ' ' << data[i + 1] << ' ' << data[i + 2] << std::endl;
		//	}
		//}
		//glUnmapBuffer(GL_ARRAY_BUFFER);
		//glBindBuffer(GL_ARRAY_BUFFER, 0);

		glBindVertexArray(VAO);
		glDrawArrays(GL_LINES, 0, vertices / 3);
		glBindVertexArray(0);

		GLenum error = glGetError();
		if (error != GL_NO_ERROR)
		{
			std::cerr << "OpenGL Error: " << error << std::endl;
		}
	}

private:
	unsigned int VAO = -1;
	unsigned int program;

	Camera* camera;

};

#include <QtGui/QMouseEvent>
#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication_MC3D.h"

class QtWidgetsApplication_MC3D : public QMainWindow
{
	Q_OBJECT

public:
	QtWidgetsApplication_MC3D(QWidget *parent = nullptr);
	~QtWidgetsApplication_MC3D();

protected:

	void wheelEvent(QWheelEvent *event) override
	{
		if (event->angleDelta().y() != 0)
		{
			int delta = event->angleDelta().y() > 0 ? 1 : -1;
			centre[2] += delta;

			osOpenGLWidget->update();
			openglWidget->update();
		}
	}

private:
	Ui::QtWidgetsApplication_MC3DClass ui;
	OpenGLWidget* openglWidget;
	OSOpenGLWidget* osOpenGLWidget;
};

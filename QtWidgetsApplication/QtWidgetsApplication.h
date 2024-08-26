#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <Windows.h>

#include <CL/cl.h>
#include <CL/cl_gl.h>

extern unsigned int vbo;

class OpenCL
{
public:
	OpenCL()
	{
		int error;

		cl_platform_id platform;
		error = clGetPlatformIDs(1, &platform, NULL);

		cl_device_id device;
		error = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);

		cl_context_properties properties[] =
		{
			CL_GL_CONTEXT_KHR,(cl_context_properties)wglGetCurrentContext(),
			CL_WGL_HDC_KHR,(cl_context_properties)wglGetCurrentDC(),
			CL_CONTEXT_PLATFORM,(cl_context_properties)platform,
			0
		};
		context = clCreateContext(properties, 1, &device, NULL, NULL, &error);

		queue = clCreateCommandQueue(context, device, 0, &error);

		std::ifstream input_file;
		//input_file.open(R"(D:\user\source\repos\QtWidgetsApplication\QtWidgetsApplication\kernel.cl)");
		input_file.open("kernel.cl");

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

		kernel = clCreateKernel(program, "Func", &error);

		std::cout << "OpenCL Context : " << context << std::endl;
		std::cout << "OpenCL Command Queue : " << queue << std::endl;
		std::cout << "OpenCL Kernel : " << kernel << std::endl;
	}

	void run()
	{
		int error;

		cl_mem clvbo = clCreateFromGLBuffer(context, CL_MEM_WRITE_ONLY, vbo, &error);
		error = clEnqueueAcquireGLObjects(queue, 1, &clvbo, 0, NULL, NULL);

		error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &clvbo);

		size_t workSize[2] = { 6,6 };
		error = clEnqueueNDRangeKernel(queue, kernel, 2, NULL, workSize, NULL, 0, NULL, NULL);

		error = clEnqueueReleaseGLObjects(queue, 1, &clvbo, 0, NULL, NULL);
		error = clFinish(queue);

		error = clReleaseMemObject(clvbo);
	}

private:
	cl_kernel kernel;
	cl_context context;
	cl_command_queue queue;
};

#include <QtGui/QOpenGLContext>
#include <QtGui/QOffscreenSurface>
#include <QtGui/QOpenGLFunctions_3_3_Core>
#include <QtGui/QImageReader>

extern unsigned int texture;

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
		initTexture();

		openCL = new OpenCL();
		openCL->run();

		context->shareContext()->doneCurrent();
	}

	void initTexture()
	{
		//QImageReader reader(R"(D:\user\source\repos\QtWidgetsApplication\QtWidgetsApplication\icon.png)");
		QImageReader reader("icon.png");
		QImage image = reader.read();
		image = image.mirrored(false, true);

		auto glFunc = context->shareContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

		glFunc->glGenTextures(1, &texture);
		glFunc->glBindTexture(GL_TEXTURE_2D, texture);

		glFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

		glFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glFunc->glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glFunc->glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0, GL_RGBA, GL_UNSIGNED_BYTE, image.constBits());

		std::cout << "OSOpenGLWidget Texture ID : " << texture << std::endl;
	}

	void initVBO()
	{
		auto glFunc = context->shareContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();

		glFunc->glGenBuffers(1, &vbo);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		glFunc->glBufferData(GL_ARRAY_BUFFER, 30 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

		std::cout << "OSOpenGLWidget VBO ID : " << vbo << std::endl;
	}

	void update()
	{
		auto glFunc = context->shareContext()->versionFunctions<QOpenGLFunctions_3_3_Core>();
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		void* ptr = glFunc->glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		if (ptr)
		{
			float* data = static_cast<float*>(ptr);
			std::cout << "pre vbo data" << std::endl;
			for (int i = 0; i < 6; ++i)
			{
				std::cout
					<< data[i * 5 + 0] << ' '
					<< data[i * 5 + 1] << ' '
					<< data[i * 5 + 2] << ' '
					<< data[i * 5 + 3] << ' '
					<< data[i * 5 + 4] << std::endl;
			}
		}
		glFunc->glUnmapBuffer(GL_ARRAY_BUFFER);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);

		openCL->run();

		glFunc->glBindBuffer(GL_ARRAY_BUFFER, vbo);
		ptr = glFunc->glMapBuffer(GL_ARRAY_BUFFER, GL_READ_ONLY);
		if (ptr)
		{
			float* data = static_cast<float*>(ptr);
			std::cout << "last vbo data" << std::endl;
			for (int i = 0; i < 6; ++i)
			{
				std::cout
					<< data[i * 5 + 0] << ' '
					<< data[i * 5 + 1] << ' '
					<< data[i * 5 + 2] << ' '
					<< data[i * 5 + 3] << ' '
					<< data[i * 5 + 4] << std::endl;
			}
		}
		glFunc->glUnmapBuffer(GL_ARRAY_BUFFER);
		glFunc->glBindBuffer(GL_ARRAY_BUFFER, 0);
	}

private:
	OpenCL* openCL;
	QOpenGLContext* context;
	QOffscreenSurface* osSurface;
};

#include <QtWidgets/QOpenGLWidget>

#include <glm/glm.hpp>
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
		camera = new Camera(glm::vec3(0, 0, 3));
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
		glm::mat4 projection = glm::perspective(camera->fov, (float)width() / height(), 0.01f, 100.0f);

		glUseProgram(program);
		glUniformMatrix4fv(glGetUniformLocation(program, "Model"), 1, GL_FALSE, &model[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "View"), 1, GL_FALSE, &view[0][0]);
		glUniformMatrix4fv(glGetUniformLocation(program, "Projection"), 1, GL_FALSE, &projection[0][0]);
		glUniform1i(glGetUniformLocation(program, "_texture"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		renderQuad();
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
			//vShaderFile.open(R"(D:\user\source\repos\QtWidgetsApplication\QtWidgetsApplication\shader.vs)");
			vShaderFile.open("shader.vs");
			//fShaderFile.open(R"(D:\user\source\repos\QtWidgetsApplication\QtWidgetsApplication\shader.fs)");
			fShaderFile.open("shader.fs");
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

	void renderQuad()
	{
		if (quadVAO == -1)
		{
			glGenVertexArrays(1, &quadVAO);
			glBindVertexArray(quadVAO);

			glBindBuffer(GL_ARRAY_BUFFER, vbo);

			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);

			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));

			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glBindVertexArray(0);
		}

		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLES, 0, 6);
		glBindVertexArray(0);
	}

private:
	unsigned int quadVAO = -1;
	unsigned int program;

	Camera* camera;

};

#include <QtGui/QMouseEvent>
#include <QtWidgets/QMainWindow>
#include "ui_QtWidgetsApplication.h"

class QtWidgetsApplication : public QMainWindow
{
    Q_OBJECT

public:
	QtWidgetsApplication(QWidget *parent = nullptr);
	~QtWidgetsApplication();

protected:

	void mousePressEvent(QMouseEvent *event) override
	{
		if (event->button() == Qt::LeftButton)
		{
			osOpenGLWidget->update();
			openglWidget->update();
		}
	}

private:
    Ui::QtWidgetsApplicationClass ui;
	OpenGLWidget* openglWidget;
	OSOpenGLWidget* osOpenGLWidget;
};

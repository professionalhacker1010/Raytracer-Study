#include "stdafx.h"

#include "Tri.h"
#include "Math.h"
#include "Vertex.h"
#include "RayCast.h"
#include "tests.h"
#include "Constants.h"
#include "Camera.h"
#include "Light.h"
#include "BVH.h"
#include "RenderQuad.h"
#include "Mesh.h"
#include "MeshInstance.h"
#include "Application.h"
#include "Error.h"

#pragma comment( linker, "/subsystem:windows /ENTRY:mainCRTStartup" )

//using namespace Tmpl8;

// Enable usage of dedicated GPUs in notebooks
// Note: this does cause the linker to produce a .lib and .exp file;
// see http://developer.download.nvidia.com/devzone/devcenter/gamegraphics/files/OptimusRenderingPolicies.pdf
#ifdef WIN32
extern "C"
{
	__declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
}

extern "C"
{
	__declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

//GL window
static GLFWwindow* window = 0;
static bool hasFocus = true, running = true;
//static GLTexture* renderTarget = 0;

// static member data for instruction set support class
static const CPUCaps cpucaps;

//application
static Application* app;

// provide access to the render target, for OpenCL / OpenGL interop
//GLTexture* GetRenderTarget() { return renderTarget; }

// GLFW callbacks
//void InitRenderTarget(int w, int h)
//{
//	// allocate render target and surface
//	renderTarget = new GLTexture(w, h, GLTexture::INTTARGET);
//}
void ReshapeWindowCallback(GLFWwindow* window, int w, int h)
{
	glViewport(0, 0, w, h);
}
void KeyEventCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (key == GLFW_KEY_ESCAPE) running = false;
	if (action == GLFW_PRESS) { if (app) if (key >= 0) app->KeyDown( key ); }
	else if (action == GLFW_RELEASE) { if (app) if (key >= 0) app->KeyUp( key ); }
}
void CharEventCallback(GLFWwindow* window, uint code) { }
void WindowFocusCallback(GLFWwindow* window, int focused) { hasFocus = (focused == GL_TRUE); }
void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
	if (action == GLFW_PRESS) { if (app) app->MouseDown( button ); }
	else if (action == GLFW_RELEASE) { if (app) app->MouseUp( button ); }
}
void MouseScrollCallback(GLFWwindow* window, double x, double y)
{
	app->MouseWheel( (float)y );
}
void MousePosCallback(GLFWwindow* window, double x, double y)
{
	if (app) app->MouseMove( (int)x, (int)y );
}
void ErrorCallback(int error, const char* description)
{
	fprintf(stderr, "GLFW Error: %s\n", description);
}

void WindowCloseCallback(GLFWwindow* window) {
	app->Shutdown();
	Kernel::KillCL();
	glfwDestroyWindow(window);
	glfwTerminate();
}

bool Init()
{
	//create console window
	FILE* fp;
	AllocConsole();
	freopen_s(&fp, "CONIN$", "r", stdin);
	freopen_s(&fp, "CONOUT$", "w", stdout);
	freopen_s(&fp, "CONOUT$", "w", stderr);

	// open a window
	if (!glfwInit()) FatalError("glfwInit failed.");
	glfwSetErrorCallback(ErrorCallback);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // 3.3 is enough for our needs
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	glfwWindowHint(GLFW_STENCIL_BITS, GL_FALSE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

#ifdef FULLSCREEN
	window = glfwCreateWindow(WIDTH, HEIGHT, "Ray Tracer", glfwGetPrimaryMonitor(), 0);
#else
	window = glfwCreateWindow(WIDTH, HEIGHT, "Ray Tracer", 0, 0);
#endif
	if (!window) FatalError("glfwCreateWindow failed.");
	glfwMakeContextCurrent(window);

	// register callbacks
	glfwSetWindowSizeCallback(window, ReshapeWindowCallback);
	glfwSetKeyCallback(window, KeyEventCallback);
	glfwSetWindowFocusCallback(window, WindowFocusCallback);
	glfwSetMouseButtonCallback(window, MouseButtonCallback);
	glfwSetScrollCallback(window, MouseScrollCallback);
	glfwSetCursorPosCallback(window, MousePosCallback);
	glfwSetCharCallback(window, CharEventCallback);
	glfwSetWindowCloseCallback(window, WindowCloseCallback);

	// initialize GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) FatalError("gladLoadGLLoader failed.");
	glfwSwapInterval(0);

	// prepare OpenGL state
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	glDisable(GL_BLEND);
	CheckGL();

	app = &Application::Get();
	app->Init(window);
	
	return true;
}

int main (int argc, char ** argv)
{
  if (argc<2 || argc > 3)
  {  
    printf ("usage: %s <scenefile> [jpegname]\n", argv[0]);
    exit(0);
  }
  
  if (!Init()) exit(0);;

  float deltaTime = 0;
  bool timeInit = false;
  chrono::high_resolution_clock::time_point start = chrono::high_resolution_clock::now();

  while (!glfwWindowShouldClose(window))
  {
	  chrono::high_resolution_clock::time_point t2 = chrono::high_resolution_clock::now();
	  chrono::duration<double> time_span = chrono::duration_cast<chrono::duration<double>>(t2 - start);
	  deltaTime = /*1000.0f **/ time_span.count();
	  start = t2;

	  if (timeInit) {
		  app->Tick(deltaTime);
		  glfwSwapBuffers(window);
		  glfwPollEvents();
	  }
	  else timeInit = true;

	  if (!running) break;
  }

  WindowCloseCallback(window);
}

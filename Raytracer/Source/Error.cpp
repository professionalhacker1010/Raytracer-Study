#include "stdafx.h"
#include "Error.h"

void FatalError(const char* fmt, ...)
{
	char t[16384];
	va_list args;
	va_start(args, fmt);
	vsnprintf(t, sizeof(t), fmt, args);
	va_end(args);
#ifdef _MSC_VER
	MessageBox(NULL, t, "Fatal error", MB_OK);
#else
	fprintf(stderr, t);
#endif
	while (1) exit(0);
}

// OpenGL helper functions
void _CheckGL(const char* f, int l)
{
	GLenum error = glGetError();
	if (error != GL_NO_ERROR)
	{
		const char* errStr = "UNKNOWN ERROR";
		if (error == 0x500) errStr = "INVALID ENUM";
		else if (error == 0x502) errStr = "INVALID OPERATION";
		else if (error == 0x501) errStr = "INVALID VALUE";
		else if (error == 0x506) errStr = "INVALID FRAMEBUFFER OPERATION";
		FatalError("GL error %d: %s at %s:%d\n", error, errStr, f, l);
	}
}

void CheckProgram(GLuint id, const char* vshader, const char* fshader)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	GLsizei length = 0;
	glGetProgramInfoLog(id, sizeof(buffer), &length, buffer);
	CheckGL();
	FATALERROR_IF(length > 0, "Shader link error:\n%s", buffer);
}


void CheckShader(GLuint shader, const char* vshader, const char* fshader)
{
	char buffer[1024];
	memset(buffer, 0, sizeof(buffer));
	GLsizei length = 0;
	glGetShaderInfoLog(shader, sizeof(buffer), &length, buffer);
	CheckGL();
	FATALERROR_IF(length > 0 && strstr(buffer, "ERROR"), "Shader compile error:\n%s", buffer);
}

// CHECKCL method
// OpenCL error handling.
// ----------------------------------------------------------------------------
bool CheckCL(cl_int result, const char* file, int line)
{
	if (result == CL_SUCCESS) return true;
	if (result == CL_DEVICE_NOT_FOUND) FatalError("Error: CL_DEVICE_NOT_FOUND\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_DEVICE_NOT_AVAILABLE) FatalError("Error: CL_DEVICE_NOT_AVAILABLE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_COMPILER_NOT_AVAILABLE) FatalError("Error: CL_COMPILER_NOT_AVAILABLE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_MEM_OBJECT_ALLOCATION_FAILURE) FatalError("Error: CL_MEM_OBJECT_ALLOCATION_FAILURE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_OUT_OF_RESOURCES) FatalError("Error: CL_OUT_OF_RESOURCES\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_OUT_OF_HOST_MEMORY) FatalError("Error: CL_OUT_OF_HOST_MEMORY\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_PROFILING_INFO_NOT_AVAILABLE) FatalError("Error: CL_PROFILING_INFO_NOT_AVAILABLE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_MEM_COPY_OVERLAP) FatalError("Error: CL_MEM_COPY_OVERLAP\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_IMAGE_FORMAT_MISMATCH) FatalError("Error: CL_IMAGE_FORMAT_MISMATCH\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_IMAGE_FORMAT_NOT_SUPPORTED) FatalError("Error: CL_IMAGE_FORMAT_NOT_SUPPORTED\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_BUILD_PROGRAM_FAILURE) FatalError("Error: CL_BUILD_PROGRAM_FAILURE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_MAP_FAILURE) FatalError("Error: CL_MAP_FAILURE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_MISALIGNED_SUB_BUFFER_OFFSET) FatalError("Error: CL_MISALIGNED_SUB_BUFFER_OFFSET\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST) FatalError("Error: CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_VALUE) FatalError("Error: CL_INVALID_VALUE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_DEVICE_TYPE) FatalError("Error: CL_INVALID_DEVICE_TYPE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_PLATFORM) FatalError("Error: CL_INVALID_PLATFORM\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_DEVICE) FatalError("Error: CL_INVALID_DEVICE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_CONTEXT) FatalError("Error: CL_INVALID_CONTEXT\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_QUEUE_PROPERTIES) FatalError("Error: CL_INVALID_QUEUE_PROPERTIES\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_COMMAND_QUEUE) FatalError("Error: CL_INVALID_COMMAND_QUEUE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_HOST_PTR) FatalError("Error: CL_INVALID_HOST_PTR\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_MEM_OBJECT) FatalError("Error: CL_INVALID_MEM_OBJECT\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_IMAGE_FORMAT_DESCRIPTOR) FatalError("Error: CL_INVALID_IMAGE_FORMAT_DESCRIPTOR\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_IMAGE_SIZE) FatalError("Error: CL_INVALID_IMAGE_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_SAMPLER) FatalError("Error: CL_INVALID_SAMPLER\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_BINARY) FatalError("Error: CL_INVALID_BINARY\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_BUILD_OPTIONS) FatalError("Error: CL_INVALID_BUILD_OPTIONS\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_PROGRAM) FatalError("Error: CL_INVALID_PROGRAM\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_PROGRAM_EXECUTABLE) FatalError("Error: CL_INVALID_PROGRAM_EXECUTABLE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_KERNEL_NAME) FatalError("Error: CL_INVALID_KERNEL_NAME\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_KERNEL_DEFINITION) FatalError("Error: CL_INVALID_KERNEL_DEFINITION\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_KERNEL) FatalError("Error: CL_INVALID_KERNEL\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_ARG_INDEX) FatalError("Error: CL_INVALID_ARG_INDEX\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_ARG_VALUE) FatalError("Error: CL_INVALID_ARG_VALUE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_ARG_SIZE) FatalError("Error: CL_INVALID_ARG_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_KERNEL_ARGS) FatalError("Error: CL_INVALID_KERNEL_ARGS\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_WORK_DIMENSION) FatalError("Error: CL_INVALID_WORK_DIMENSION\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_WORK_GROUP_SIZE) FatalError("Error: CL_INVALID_WORK_GROUP_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_WORK_ITEM_SIZE) FatalError("Error: CL_INVALID_WORK_ITEM_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_GLOBAL_OFFSET) FatalError("Error: CL_INVALID_GLOBAL_OFFSET\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_EVENT_WAIT_LIST) FatalError("Error: CL_INVALID_EVENT_WAIT_LIST\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_EVENT) FatalError("Error: CL_INVALID_EVENT\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_OPERATION) FatalError("Error: CL_INVALID_OPERATION\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_GL_OBJECT) FatalError("Error: CL_INVALID_GL_OBJECT\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_BUFFER_SIZE) FatalError("Error: CL_INVALID_BUFFER_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_MIP_LEVEL) FatalError("Error: CL_INVALID_MIP_LEVEL\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_GLOBAL_WORK_SIZE) FatalError("Error: CL_INVALID_GLOBAL_WORK_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_PROPERTY) FatalError("Error: CL_INVALID_PROPERTY\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_IMAGE_DESCRIPTOR) FatalError("Error: CL_INVALID_IMAGE_DESCRIPTOR\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_COMPILER_OPTIONS) FatalError("Error: CL_INVALID_COMPILER_OPTIONS\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_LINKER_OPTIONS) FatalError("Error: CL_INVALID_LINKER_OPTIONS\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_DEVICE_PARTITION_COUNT) FatalError("Error: CL_INVALID_DEVICE_PARTITION_COUNT\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_PIPE_SIZE) FatalError("Error: CL_INVALID_PIPE_SIZE\n%s, line %i", file, line, "OpenCL error");
	if (result == CL_INVALID_DEVICE_QUEUE) FatalError("Error: CL_INVALID_DEVICE_QUEUE\n%s, line %i", file, line, "OpenCL error");
	return false;
}
#pragma once

// fatal error reporting (with a pretty window)
#define FATALERROR( fmt, ... ) FatalError( "Error on line %d of %s: " fmt "\n", __LINE__, __FILE__, ##__VA_ARGS__ )
#define FATALERROR_IF( condition, fmt, ... ) do { if ( ( condition ) ) FATALERROR( fmt, ##__VA_ARGS__ ); } while ( 0 )
#define FATALERROR_IN( prefix, errstr, fmt, ... ) FatalError( prefix " returned error '%s' at %s:%d" fmt "\n", errstr, __FILE__, __LINE__, ##__VA_ARGS__ );
#define FATALERROR_IN_CALL( stmt, error_parser, fmt, ... ) do { auto ret = ( stmt ); if ( ret ) FATALERROR_IN( #stmt, error_parser( ret ), fmt, ##__VA_ARGS__ ) } while ( 0 )
void FatalError(const char* fmt, ...);

// generic error checking for OpenGL code
#define CheckGL() { _CheckGL( __FILE__, __LINE__ ); }

void _CheckGL(const char* f, int l);

void CheckProgram(uint id, const char* vshader, const char* fshader);
void CheckShader(uint shader, const char* vshader, const char* fshader);

#define CHECKCL(r) CheckCL( r, __FILE__, __LINE__ )

bool CheckCL(cl_int result, const char* file, int line);
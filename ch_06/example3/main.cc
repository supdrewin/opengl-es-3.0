#include "esUtil.h"

struct UserData {
		GLuint program;
};

GLuint load_shader ( GLenum type, char const *shader_src )
{
	auto shader = glCreateShader ( type );

	if ( !shader ) return 0;

	glShaderSource ( shader, 1, &shader_src, nullptr );
	glCompileShader ( shader );

	GLint compiled;

	glGetShaderiv ( shader, GL_COMPILE_STATUS, &compiled );

	if ( compiled ) return shader;

	GLint info_len = 0;

	glGetShaderiv ( shader, GL_INFO_LOG_LENGTH, &info_len );

	if ( 1 < info_len ) {
		auto info_log = new char[info_len];

		glGetShaderInfoLog ( shader, info_len, nullptr, info_log );
		esLogMessage ( "Error compiling shader:\n%s\n", info_log );

		delete[] info_log;
	}

	glDeleteShader ( shader );

	return 0;
}

bool init ( ESContext *context )
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	auto vshader_src = R"(
                #version 300 es

                layout ( location = 0 ) in vec4 color;
                layout ( location = 1 ) in vec4 pos;

                out vec4 v_color;

                void main ( )
                {
                        v_color     = color;
                        gl_Position = pos;
                }
	)";

	auto fshader_src = R"(
                #version 300 es

                precision mediump float;

                in  vec4 v_color;
                out vec4 color;

                void main ( )
                {
                        color = v_color;
                }
	)";

	auto vshader = load_shader ( GL_VERTEX_SHADER, vshader_src );
	auto fshader = load_shader ( GL_FRAGMENT_SHADER, fshader_src );
	auto program = glCreateProgram ( );

	if ( !program ) return false;

	glAttachShader ( program, vshader );
	glAttachShader ( program, fshader );

	glLinkProgram ( program );

	GLint linked;

	glGetProgramiv ( program, GL_LINK_STATUS, &linked );

	if ( linked ) {
		user_data->program = program;

		glClearColor ( 0, 0, 0, 0 );

		return true;
	}

	GLint info_len = 0;

	glGetProgramiv ( program, GL_INFO_LOG_LENGTH, &info_len );

	if ( 1 < info_len ) {
		auto info_log = new char[info_len];

		glGetProgramInfoLog ( program, info_len, nullptr, info_log );
		esLogMessage ( "Error linking program:\n%s\n", info_log );

		delete[] info_log;
	}

	glDeleteProgram ( program );

	return false;
}

void draw ( ESContext *context )
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	glViewport ( 0, 0, context->width, context->height );
	glClear ( GL_COLOR_BUFFER_BIT );
	glUseProgram ( user_data->program );

	GLfloat color[] = { 1, 0, 0, 1 };

	glVertexAttrib4fv ( 0, color );

	GLfloat vertices[] = {
		0.0,  0.5,  0.0,  // v0
		-0.5, -0.5, 0.0,  // v1
		0.5,  -0.5, 0.0,  // v2
	};

	glVertexAttribPointer ( 1, 3, GL_FLOAT, GL_FALSE, 0, vertices );
	glEnableVertexAttribArray ( 1 );

	glDrawArrays ( GL_TRIANGLES, 0, 3 );
	glDisableVertexAttribArray ( 1 );
}

void shutdown ( ESContext *context )
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	glDeleteProgram ( user_data->program );
}

extern "C" int esMain ( ESContext *context )
{
	context->userData = new UserData;

	esCreateWindow ( context, "Red Triangle", 800, 600, ES_WINDOW_RGB );

	if ( !init ( context ) ) return GL_FALSE;

	esRegisterDrawFunc ( context, draw );
	esRegisterShutdownFunc ( context, shutdown );

	return GL_TRUE;
}

#include "esUtil.h"

// clang-format off

struct UserData {
	GLuint program;
};

// clang-format on

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

bool init ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	auto v_shader_str = R"(
		#version 300 es

		layout ( location = 0 ) in vec4 v_pos;

		void main ( )
		{
			gl_Position = v_pos;
		}
	)";

	auto f_shader_str = R"(
		#version 300 es

		precision mediump float;
		out vec4 frag_color;

		void main ( )
		{
			frag_color = vec4 ( 1.0, 0.0, 0.0, 1.0 );
		}
	)";

	auto vert_shader = load_shader ( GL_VERTEX_SHADER, v_shader_str );
	auto frag_shader = load_shader ( GL_FRAGMENT_SHADER, f_shader_str );
	auto program     = glCreateProgram ( );

	if ( !program ) return false;

	glAttachShader ( program, vert_shader );
	glAttachShader ( program, frag_shader );

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

void draw ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	glViewport ( 0, 0, es_context->width, es_context->height );
	glClear ( GL_COLOR_BUFFER_BIT );
	glUseProgram ( user_data->program );

	// clang-format off

	GLfloat vertices[] = {
		 0.0,  0.5, 0.0,
		-0.5, -0.5, 0.0,
		 0.5, -0.5, 0.0,
	};

	// clang-format on

	glVertexAttribPointer ( 0, 3, GL_FLOAT, GL_FALSE, 0, vertices );
	glEnableVertexAttribArray ( 0 );
	glDrawArrays ( GL_TRIANGLES, 0, 3 );
}

void shutdown ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	glDeleteProgram ( user_data->program );
}

extern "C" int esMain ( ESContext *es_context )
{
	es_context->userData = new UserData;

	esCreateWindow (
	    es_context,
	    "Hello Triangle",
	    800,
	    600,
	    ES_WINDOW_RGB
	);

	if ( !init ( es_context ) ) return GL_FALSE;

	esRegisterDrawFunc ( es_context, draw );
	esRegisterShutdownFunc ( es_context, shutdown );

	return GL_TRUE;
}

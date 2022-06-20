#include <algorithm>
#include <cstddef>
#include <vector>

#include "esUtil.h"

struct UserData {
		GLuint              program;
		std::vector<GLuint> vbo_ids;

		UserData ( size_t n )
		    : vbo_ids ( n )
		{
		}
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

                layout ( location = 0 ) in vec4 pos;
                layout ( location = 1 ) in vec4 color;

                out vec4 v_color;

                void main ( )
                {
                        gl_Position = pos;
                        v_color     = color;
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

void draw_primitive_with_VBOs (
    ESContext			       *context,
    std::vector<std::vector<GLfloat>> const &vtx_buf,
    std::vector<GLushort> const             &indices
)
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	if ( user_data->vbo_ids.cend ( )
	     != std::find (
		 user_data->vbo_ids.cbegin ( ),
		 user_data->vbo_ids.cend ( ),
		 0
	     ) )
	{
		glGenBuffers (
		    user_data->vbo_ids.size ( ),
		    user_data->vbo_ids.data ( )
		);

		size_t i = 0;

		for ( auto const &buf : vtx_buf ) {
			glBindBuffer (
			    GL_ARRAY_BUFFER,
			    user_data->vbo_ids[i++]
			);
			glBufferData (
			    GL_ARRAY_BUFFER,
			    buf.size ( ) * sizeof ( GLfloat ),
			    buf.data ( ),
			    GL_STATIC_DRAW
			);
		}

		glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, user_data->vbo_ids[i] );

		glBufferData (
		    GL_ELEMENT_ARRAY_BUFFER,
		    indices.size ( ) * sizeof ( GLushort ),
		    indices.data ( ),
		    GL_STATIC_DRAW
		);
	}

	glBindBuffer ( GL_ARRAY_BUFFER, user_data->vbo_ids[0] );
	glEnableVertexAttribArray ( 0 );

	glVertexAttribPointer (
	    0,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof ( GLfloat ) * 3,
	    0
	);

	glBindBuffer ( GL_ARRAY_BUFFER, user_data->vbo_ids[1] );
	glEnableVertexAttribArray ( 1 );

	glVertexAttribPointer (
	    1,
	    4,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof ( GLfloat ) * 4,
	    0
	);

	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, user_data->vbo_ids[2] );
	glDrawElements ( GL_TRIANGLES, indices.size ( ), GL_UNSIGNED_SHORT, 0 );

	glDisableVertexAttribArray ( 0 );
	glDisableVertexAttribArray ( 1 );

	glBindBuffer ( GL_ARRAY_BUFFER, 0 );
	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void draw ( ESContext *context )
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	glViewport ( 0, 0, context->width, context->height );
	glClear ( GL_COLOR_BUFFER_BIT );
	glUseProgram ( user_data->program );

	std::vector<GLfloat> vertices {
		0.0,  0.5,  0.0,  // v0
		-0.5, -0.5, 0.0,  // v1
		0.5,  -0.5, 0.0,  // v2
	};

	std::vector<GLfloat> colors {
		1, 0, 0, 1,  // c0
		0, 1, 0, 1,  // c1
		0, 0, 1, 1,  // c2
	};

	draw_primitive_with_VBOs ( context, { vertices, colors }, { 0, 1, 2 } );
}

void shutdown ( ESContext *context )
{
	auto user_data = reinterpret_cast<UserData *> ( context->userData );

	glDeleteProgram ( user_data->program );
}

extern "C" int esMain ( ESContext *context )
{
	context->userData = new UserData ( 3 );

	esCreateWindow (
	    context,
	    "Colorful Triangle",
	    800,
	    600,
	    ES_WINDOW_RGB
	);

	if ( !init ( context ) ) return GL_FALSE;

	esRegisterDrawFunc ( context, draw );
	esRegisterShutdownFunc ( context, shutdown );

	return GL_TRUE;
}

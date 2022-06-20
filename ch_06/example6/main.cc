#include <algorithm>
#include <cstddef>
#include <vector>

#include "esUtil.h"

struct UserData {
		GLuint              program;
		std::vector<GLuint> vboids;

		UserData ( size_t n )
		    : vboids ( n )
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

bool init ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	auto v_shader_str = R"(
                #version 300 es

                layout ( location = 0 ) in vec4 a_position;
                layout ( location = 1 ) in vec4 a_color;

                out vec4 v_color;

                void main ( )
                {
                        gl_Position = a_position;
                        v_color     = a_color;
                }
	)";

	auto f_shader_str = R"(
                #version 300 es

                precision mediump float;

                in  vec4 v_color;
                out vec4 o_color;

                void main ( )
                {
                        o_color = v_color;
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

void draw_primitive_with_VBOs (
    ESContext			       *es_context,
    std::vector<std::vector<GLfloat>> const &vtxbuf,
    std::vector<GLushort> const             &indices
)
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	if ( user_data->vboids.cend ( )
	     != std::find (
		 user_data->vboids.cbegin ( ),
		 user_data->vboids.cend ( ),
		 0
	     ) )
	{
		glGenBuffers (
		    user_data->vboids.size ( ),
		    user_data->vboids.data ( )
		);

		size_t i = 0;

		for ( auto const &buf : vtxbuf ) {
			glBindBuffer (
			    GL_ARRAY_BUFFER,
			    user_data->vboids[i++]
			);
			glBufferData (
			    GL_ARRAY_BUFFER,
			    buf.size ( ) * sizeof ( GLfloat ),
			    buf.data ( ),
			    GL_STATIC_DRAW
			);
		}

		glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, user_data->vboids[i] );

		glBufferData (
		    GL_ELEMENT_ARRAY_BUFFER,
		    indices.size ( ) * sizeof ( GLushort ),
		    indices.data ( ),
		    GL_STATIC_DRAW
		);
	}

	glBindBuffer ( GL_ARRAY_BUFFER, user_data->vboids[0] );
	glEnableVertexAttribArray ( 0 );

	glVertexAttribPointer (
	    0,
	    3,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof ( GLfloat ) * 3,
	    0
	);

	glBindBuffer ( GL_ARRAY_BUFFER, user_data->vboids[1] );
	glEnableVertexAttribArray ( 1 );

	glVertexAttribPointer (
	    1,
	    4,
	    GL_FLOAT,
	    GL_FALSE,
	    sizeof ( GLfloat ) * 4,
	    0
	);

	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, user_data->vboids[2] );
	glDrawElements ( GL_TRIANGLES, indices.size ( ), GL_UNSIGNED_SHORT, 0 );

	glDisableVertexAttribArray ( 0 );
	glDisableVertexAttribArray ( 1 );

	glBindBuffer ( GL_ARRAY_BUFFER, 0 );
	glBindBuffer ( GL_ELEMENT_ARRAY_BUFFER, 0 );
}

void draw ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	glViewport ( 0, 0, es_context->width, es_context->height );
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

	draw_primitive_with_VBOs (
	    es_context,
	    { vertices, colors },
	    { 0, 1, 2 }
	);
}

void shutdown ( ESContext *es_context )
{
	auto user_data = reinterpret_cast<UserData *> ( es_context->userData );

	glDeleteProgram ( user_data->program );
}

extern "C" int esMain ( ESContext *es_context )
{
	es_context->userData = new UserData ( 3 );

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

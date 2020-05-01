// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <iostream>
// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>

#define getErrors() handle_gl_errors( __LINE__ )

void handle_gl_errors(int LINE) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL Error: " << err << "at line " << LINE << std::endl;
    }
}


struct Framebuffer {
    GLuint render_ref;
    GLuint texture_ref;
};


Framebuffer gen_framebuffer(glm::ivec2 size, GLenum filter = GL_NEAREST, GLenum wrap = GL_REPEAT) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
    
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texture, 0);   
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf( stderr, "Failed to create framebuffer. Ooops!.\n" );
		glfwTerminate();
		exit(-1);
    }

    Framebuffer ret{fbo, texture};
    return ret;
}

void bind_framebuffer_target(GLuint dest, glm::ivec2 viewport_size) {
    glBindFramebuffer(GL_FRAMEBUFFER, dest);
    glViewport(0, 0, viewport_size.x, viewport_size.y);
}

GLuint screen_vertexbuffer;

void render_screen() {
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, screen_vertexbuffer);
    glVertexAttribPointer(
        0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
        2,                  // size
        GL_FLOAT,           // type
        GL_FALSE,           // normalized?
        0,                  // stride
        (void*)0            // array buffer offset
    );

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // 12*3 indices starting at 0 -> 12 triangles
    getErrors();

    glDisableVertexAttribArray(0);
}

// Render src with shader
void render_texture(GLuint src, GLuint shader) {
    glUseProgram(shader);
    
    // Bind our texture in Texture Unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, src);
    // Make the shader use Texture Unit 0
    glUniform1i(glGetUniformLocation(shader, "tex"), 0);
    getErrors();

	render_screen();
}

void bindTexture(GLenum unit, GLenum target, GLuint tex) {
	glActiveTexture(unit);
	glBindTexture(target, tex);
}

GLuint water_source_verts, water_source_amounts;
void render_water_sources() {

}

GLuint 	rain_shader, waterSource_shader, 
		waterSurface_shader, sedimentDeposition_shader, 
		erosionDeposition_shader, sedimentTransportation_shader, 
		outflowFlux_shader, evaporation_shader, velocityField_shader;

// Performs a single erosion pass on the given textures, updates the references accordingly
void erosion_pass_flat(Framebuffer *T1_bds, Framebuffer *T2_f, Framebuffer *T3_v, Framebuffer *temp) {
	
	// For sake of simplicity, I'll bind all the textures at the start and just set the uniforms in the shaders accordingly
	// I don't technically have to set the uniforms in the shaders every time, but so be it.
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds->texture_ref);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f->texture_ref);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v->texture_ref);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, temp->texture_ref);

	glm::ivec2 field_size(100,100);

	// temp := rain + bds
	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(rain_shader);
	// uniforms
	// 
	render_screen();
	// bds := sources + temp
	bind_framebuffer_target(T1_bds->render_ref, field_size);

	render_water_sources();

	// temp := outflow flux
	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(outflowFlux_shader);
	// uniforms
	// 
	render_screen();
	std::swap(*T2_f, *temp);


	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(waterSurface_shader);
	// uniforms
	// 
	render_screen();
	std::swap(*T2_f, *temp);

	
	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(velocityField_shader);
	// uniforms
	// 
	render_screen();
	std::swap(*T3_v, *temp);
	

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(erosionDeposition_shader);
	// uniforms
	// 
	render_screen();


	bind_framebuffer_target(T1_bds->render_ref, field_size);
	glUseProgram(sedimentTransportation_shader);
	// uniforms
	// 
	render_screen();

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(evaporation_shader);
	// uniforms
	// 
	render_screen();
	std::swap(*T1_bds, *temp);
}














void conway() {
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    glm::ivec2 screen_size{1024,768};

    // NxN pixel cells for game of life
    int cell_size = 16;
    glm::ivec2 fbuf_size = screen_size/cell_size;

	window = glfwCreateWindow( 1024, 768, "Conway's game of life", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. You should be using OpenGL 3.3 or greater (we're just modern like that).\n" );
		glfwTerminate();
		exit(-1);
	}

	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		exit(-1);
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 



	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

    glm::vec2 screen_verts[6] = {
        glm::vec2(-1,-1), glm::vec2(1,-1), glm::vec2(1,1), glm::vec2(1,1), glm::vec2(-1,1), glm::vec2(-1,-1)
    };

	glGenBuffers(1, &screen_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, screen_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(screen_verts), screen_verts, GL_STATIC_DRAW);




	// Create and compile our GLSL program from the shaders
	GLuint init_shader = LoadShaders( "assets/blit.vert", "assets/noise.frag" );
	glUseProgram(init_shader);
	glUniform2f(glGetUniformLocation(init_shader, "screen_size"), fbuf_size.x, fbuf_size.y);
    getErrors();

	GLuint conway_shader = LoadShaders( "assets/blit.vert", "assets/conway.frag" );
	glUseProgram(conway_shader);
	glUniform2f(glGetUniformLocation(conway_shader, "screen_size"), fbuf_size.x, fbuf_size.y);
    getErrors();

    GLuint passthrough_shader = LoadShaders( "assets/blit.vert", "assets/blit.frag");
    getErrors();





    Framebuffer a = gen_framebuffer(fbuf_size);
    Framebuffer b = gen_framebuffer(fbuf_size);

    // First, render noise to a:
    bind_framebuffer_target(a.render_ref, fbuf_size);
	glClear(GL_COLOR_BUFFER_BIT);
    render_texture(b.texture_ref, init_shader);
    
    // Then, execute render loop:
    do {

        bind_framebuffer_target(0, screen_size);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // blit a to screen
        render_texture(a.texture_ref, passthrough_shader);

        // next conway step, perform onto b
        bind_framebuffer_target(b.render_ref, fbuf_size);
        render_texture(a.texture_ref, conway_shader);

        // swap a and b
        std::swap(a,b);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

}


int main( void )
{

    conway();
    return 0;








	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 05 - Textured Cube", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	GLuint VertexArrayID;
	glGenVertexArrays(1, &VertexArrayID);
	glBindVertexArray(VertexArrayID);

	// Create and compile our GLSL program from the shaders
	GLuint programID = LoadShaders( "assets/TransformVertexShader.vert", "assets/TextureFragmentShader.frag" );

	// Get a handle for our "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");

	// Projection matrix : 45° Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 4.0f / 3.0f, 0.1f, 100.0f);
	// Camera matrix
	glm::mat4 View       = glm::lookAt(
								glm::vec3(4,3,3), // Camera is at (4,3,3), in World Space
								glm::vec3(0,0,0), // and looks at the origin
								glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
						   );
	// Model matrix : an identity matrix (model will be at the origin)
	glm::mat4 Model      = glm::mat4(1.0f);
	// Our ModelViewProjection : multiplication of our 3 matrices
	glm::mat4 MVP        = Projection * View * Model; // Remember, matrix multiplication is the other way around

	// Load the texture using any two methods
	//GLuint Texture = loadBMP_custom("uvtemplate.bmp");
	GLuint Texture = loadDDS("assets/uvtemplate.DDS");
	
	// Get a handle for our "myTextureSampler" uniform
	GLuint TextureID  = glGetUniformLocation(programID, "myTextureSampler");

	// Our vertices. Tree consecutive floats give a 3D vertex; Three consecutive vertices give a triangle.
	// A cube has 6 faces with 2 triangles each, so this makes 6*2=12 triangles, and 12*3 vertices
	static const GLfloat g_vertex_buffer_data[] = { 
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f
	};

	// Two UV coordinatesfor each vertex. They were created with Blender.
	static const GLfloat g_uv_buffer_data[] = { 
		0.000059f, 1.0f-0.000004f, 
		0.000103f, 1.0f-0.336048f, 
		0.335973f, 1.0f-0.335903f, 
		1.000023f, 1.0f-0.000013f, 
		0.667979f, 1.0f-0.335851f, 
		0.999958f, 1.0f-0.336064f, 
		0.667979f, 1.0f-0.335851f, 
		0.336024f, 1.0f-0.671877f, 
		0.667969f, 1.0f-0.671889f, 
		1.000023f, 1.0f-0.000013f, 
		0.668104f, 1.0f-0.000013f, 
		0.667979f, 1.0f-0.335851f, 
		0.000059f, 1.0f-0.000004f, 
		0.335973f, 1.0f-0.335903f, 
		0.336098f, 1.0f-0.000071f, 
		0.667979f, 1.0f-0.335851f, 
		0.335973f, 1.0f-0.335903f, 
		0.336024f, 1.0f-0.671877f, 
		1.000004f, 1.0f-0.671847f, 
		0.999958f, 1.0f-0.336064f, 
		0.667979f, 1.0f-0.335851f, 
		0.668104f, 1.0f-0.000013f, 
		0.335973f, 1.0f-0.335903f, 
		0.667979f, 1.0f-0.335851f, 
		0.335973f, 1.0f-0.335903f, 
		0.668104f, 1.0f-0.000013f, 
		0.336098f, 1.0f-0.000071f, 
		0.000103f, 1.0f-0.336048f, 
		0.000004f, 1.0f-0.671870f, 
		0.336024f, 1.0f-0.671877f, 
		0.000103f, 1.0f-0.336048f, 
		0.336024f, 1.0f-0.671877f, 
		0.335973f, 1.0f-0.335903f, 
		0.667969f, 1.0f-0.671889f, 
		1.000004f, 1.0f-0.671847f, 
		0.667979f, 1.0f-0.335851f
	};

	GLuint vertexbuffer;
	glGenBuffers(1, &vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

	GLuint uvbuffer;
	glGenBuffers(1, &uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_uv_buffer_data), g_uv_buffer_data, GL_STATIC_DRAW);

	do{

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Send our transformation to the currently bound shader, 
		// in the "MVP" uniform
		glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);

		// Bind our texture in Texture Unit 0
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Texture);
		// Set our "myTextureSampler" sampler to use Texture Unit 0
		glUniform1i(TextureID, 0);

		// 1rst attribute buffer : vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
		glVertexAttribPointer(
			0,                  // attribute. No particular reason for 0, but must match the layout in the shader.
			3,                  // size
			GL_FLOAT,           // type
			GL_FALSE,           // normalized?
			0,                  // stride
			(void*)0            // array buffer offset
		);

		// 2nd attribute buffer : UVs
		glEnableVertexAttribArray(1);
		glBindBuffer(GL_ARRAY_BUFFER, uvbuffer);
		glVertexAttribPointer(
			1,                                // attribute. No particular reason for 1, but must match the layout in the shader.
			2,                                // size : U+V => 2
			GL_FLOAT,                         // type
			GL_FALSE,                         // normalized?
			0,                                // stride
			(void*)0                          // array buffer offset
		);

		// Draw the triangle !
		glDrawArrays(GL_TRIANGLES, 0, 12*3); // 12*3 indices starting at 0 -> 12 triangles

		glDisableVertexAttribArray(0);
		glDisableVertexAttribArray(1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertexbuffer);
	glDeleteBuffers(1, &uvbuffer);
	glDeleteProgram(programID);
	glDeleteTextures(1, &Texture);
	glDeleteVertexArrays(1, &VertexArrayID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	return 0;
}
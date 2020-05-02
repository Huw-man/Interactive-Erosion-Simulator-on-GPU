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


Framebuffer gen_framebuffer(glm::ivec2 size, GLenum filter = GL_NEAREST, GLenum wrap = GL_REPEAT, GLenum texture_dat = GL_UNSIGNED_BYTE) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size.x, size.y, 0, GL_RGBA, texture_dat, NULL);

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

GLuint init_shader_erosion_flat;

GLuint passthrough_shader;

GLuint 	rain_shader, 
		waterSource_shader, 
		outflowFlux_shader, 
		waterSurface_shader, 
		velocityField_shader, 
		erosionDeposition_shader, 
		sedimentTransportation_shader,
		evaporation_shader;



glm::ivec2 screen_size(1024,768);

void init_erosion_shaders_flat() {
	init_shader_erosion_flat = 		LoadShaders( "assets/blit.vert", "assets/noise.frag" );
	glUseProgram(init_shader_erosion_flat);
	rain_shader = 					LoadShaders( "assets/blit.vert", "assets/rain.frag" );
	glUseProgram(rain_shader);
	waterSource_shader = 			LoadShaders( "assets/water_sources.vert", "assets/water_sources.frag" );
	glUseProgram(waterSource_shader);
	outflowFlux_shader = 			LoadShaders( "assets/blit.vert", "assets/outflow_flux.frag" );
	glUseProgram(outflowFlux_shader);
	waterSurface_shader =		    LoadShaders( "assets/blit.vert", "assets/water_surface.frag" );
	glUseProgram(waterSurface_shader);
	velocityField_shader = 			LoadShaders( "assets/blit.vert", "assets/velocity_field.frag" );
	glUseProgram(velocityField_shader);
	erosionDeposition_shader = 		LoadShaders( "assets/blit.vert", "assets/erosion_deposition.frag" );
	glUseProgram(erosionDeposition_shader);
	sedimentTransportation_shader = LoadShaders( "assets/blit.vert", "assets/sediment_transportation.frag" );
	glUseProgram(sedimentTransportation_shader);
	evaporation_shader = 			LoadShaders( "assets/blit.vert", "assets/evaporation.frag" );
	glUseProgram(evaporation_shader);
	getErrors();
}

void pass_texture_uniforms(GLuint shader, int T1_binding, int T2_binding, int T3_binding) {
	glUniform1i(glGetUniformLocation(shader, "T1_bds"), T1_binding);
	glUniform1i(glGetUniformLocation(shader, "T2_f"),   T2_binding);
	glUniform1i(glGetUniformLocation(shader, "T3_v"),   T3_binding);
}

// Performs a single erosion pass on the given textures, updates the references accordingly
void erosion_pass_flat(glm::ivec2 field_size, Framebuffer *T1_bds, Framebuffer *T2_f, Framebuffer *T3_v, Framebuffer *temp) {

	glBlendFunc(GL_ONE, GL_ZERO);
	
	// For sake of simplicity, I'll bind all the textures at the start and just set the uniforms in the shaders accordingly
	// I don't technically have to set the uniforms in the shaders every time, but so be it.
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds->texture_ref);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f->texture_ref);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v->texture_ref);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, temp->texture_ref);
	// corresponds to above, to help me keep track as textures get passed around
	int T1_binding = 0, T2_binding = 1, T3_binding = 2, temp_binding = 3;

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(rain_shader);
	pass_texture_uniforms(rain_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	// TODO: figure out where to put these
	float rain_intensity = 0.01;
	int timestep = 0;
	float delta_t = 0.016;

	glUniform1f(glGetUniformLocation(rain_shader, "rain_intensity"), rain_intensity);
	glUniform1i(glGetUniformLocation(rain_shader, "timestep"), timestep);
	glUniform1f(glGetUniformLocation(rain_shader, "delta_t"), timestep);

	render_screen();
	bind_framebuffer_target(T1_bds->render_ref, field_size);
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);



	// render_water_sources();

	


	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(outflowFlux_shader);
	pass_texture_uniforms(outflowFlux_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	float A = 1.0, l = 1.0, g = 9.81;
	glm::vec2 l_xy(1.0,1.0);

	glUniform3f(glGetUniformLocation(outflowFlux_shader, "alg"), A, l, g);
	glUniform2f(glGetUniformLocation(outflowFlux_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(outflowFlux_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(outflowFlux_shader, "delta_t"), delta_t);


	render_screen();
	std::swap(*T2_f, *temp);
	std::swap(T2_binding, temp_binding);


	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(waterSurface_shader);
	pass_texture_uniforms(waterSurface_shader, T1_binding, T2_binding, T3_binding);

	// uniforms

	glUniform2f(glGetUniformLocation(waterSurface_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(waterSurface_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(waterSurface_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T2_f, *temp);
	std::swap(T2_binding, temp_binding);

	
	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(velocityField_shader);
	pass_texture_uniforms(velocityField_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	glUniform2f(glGetUniformLocation(velocityField_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(velocityField_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(velocityField_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T3_v, *temp);
	std::swap(T3_binding, temp_binding);
	

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(erosionDeposition_shader);
	pass_texture_uniforms(erosionDeposition_shader, T1_binding, T2_binding, T3_binding);
	// uniforms
	float K_c = 1.0, K_s = 1.0, K_d = 1.0; 
	glUniform2f(glGetUniformLocation(erosionDeposition_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform3f(glGetUniformLocation(erosionDeposition_shader, "K"), K_c, K_s, K_d);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);


	bind_framebuffer_target(T1_bds->render_ref, field_size);
	glUseProgram(sedimentTransportation_shader);
	pass_texture_uniforms(sedimentTransportation_shader, T1_binding, T2_binding, T3_binding);
	// uniforms
	glUniform2f(glGetUniformLocation(sedimentTransportation_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(sedimentTransportation_shader, "delta_t"), delta_t);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(evaporation_shader);
	pass_texture_uniforms(evaporation_shader, T1_binding, T2_binding, T3_binding);
	float K_e = 1.0;
	// uniforms
	glUniform1f(glGetUniformLocation(evaporation_shader, "K_e"), K_e);
	glUniform1f(glGetUniformLocation(evaporation_shader, "delta_t"), delta_t);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

}

void erosion_loop_flat() {
	init_erosion_shaders_flat();

	glm::ivec2 field_size(100,100);
	
	Framebuffer T1_bds = gen_framebuffer(field_size, GL_NEAREST, GL_REPEAT, GL_FLOAT); // GL_FLOAT = HDR Framebuffers
	Framebuffer T2_f = gen_framebuffer(field_size, GL_NEAREST, GL_REPEAT, GL_FLOAT);
	Framebuffer T3_v = gen_framebuffer(field_size, GL_NEAREST, GL_REPEAT, GL_FLOAT);
	Framebuffer temp = gen_framebuffer(field_size, GL_NEAREST, GL_REPEAT, GL_FLOAT);
	

	glEnable(GL_BLEND);

	glUseProgram(init_shader_erosion_flat);
	glUniform2f(glGetUniformLocation(init_shader_erosion_flat, "screen_size"), field_size.x, field_size.y);
    getErrors();

    bind_framebuffer_target(T1_bds.render_ref, field_size);
	glClear(GL_COLOR_BUFFER_BIT);
    render_screen();
    
    // Then, execute render loop:
    do {
		glBlendFunc(GL_ONE, GL_ZERO);
        bind_framebuffer_target(0, screen_size);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // blit T1 to screen
        render_texture(T1_bds.texture_ref, passthrough_shader);

		erosion_pass_flat(field_size, &T1_bds, &T2_f, &T3_v, &temp);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );


}












void conway() {

    // NxN pixel cells for game of life
    int cell_size = 16;
    glm::ivec2 fbuf_size = screen_size/cell_size;

	// Create and compile our GLSL program from the shaders
	GLuint init_shader = LoadShaders( "assets/blit.vert", "assets/noise.frag" );
	glUseProgram(init_shader);
	glUniform2f(glGetUniformLocation(init_shader, "screen_size"), fbuf_size.x, fbuf_size.y);
    getErrors();

	GLuint conway_shader = LoadShaders( "assets/blit.vert", "assets/conway.frag" );
	glUseProgram(conway_shader);
	glUniform2f(glGetUniformLocation(conway_shader, "screen_size"), fbuf_size.x, fbuf_size.y);
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

void init_glfw_opengl() {
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		exit(-1);
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context

	window = glfwCreateWindow( screen_size.x, screen_size.y, "Erosion sim", NULL, NULL);
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

    passthrough_shader = LoadShaders( "assets/blit.vert", "assets/blit.frag");
    getErrors();
}


int main( void )
{
	init_glfw_opengl();
	erosion_loop_flat();
    // conway();
    return 0;
}
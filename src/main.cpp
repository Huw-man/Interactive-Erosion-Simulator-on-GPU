// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>
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
#include <common/controls.hpp>
#include <common/objloader.hpp>

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


Framebuffer gen_framebuffer(glm::ivec2 size, GLenum filter = GL_NEAREST, GLenum wrap = GL_REPEAT, GLenum texture_dat = GL_RGBA, glm::vec4 borderColor = glm::vec4(0.0)) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    
    glTexImage2D(GL_TEXTURE_2D, 0, texture_dat, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);  
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
	glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);
    
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
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*) 0);

    // Draw the triangle !
    glDrawArrays(GL_TRIANGLES, 0, 2*3); // 2 triangles
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
	init_shader_erosion_flat = 		LoadShaders( "assets/shaders/misc/height_to_r.vert", "assets/shaders/misc/height_to_r.frag" );
	glUseProgram(init_shader_erosion_flat);
	rain_shader = 					LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/rain.frag" );
	glUseProgram(rain_shader);
	waterSource_shader = 			LoadShaders( "assets/shaders/pipeline/water_sources.vert", "assets/shaders/pipeline/water_sources.frag" );
	glUseProgram(waterSource_shader);
	outflowFlux_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/outflow_flux.frag" );
	glUseProgram(outflowFlux_shader);
	waterSurface_shader =		    LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/water_surface.frag" );
	glUseProgram(waterSurface_shader);
	velocityField_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/velocity_field.frag" );
	glUseProgram(velocityField_shader);
	erosionDeposition_shader = 		LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/erosion_deposition.frag" );
	glUseProgram(erosionDeposition_shader);
	sedimentTransportation_shader = LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/sediment_transportation.frag" );
	glUseProgram(sedimentTransportation_shader);
	evaporation_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/evaporation.frag" );
	glUseProgram(evaporation_shader);
	getErrors();
}

void pass_texture_uniforms(GLuint shader, int T1_binding, int T2_binding, int T3_binding) {
	glUniform1i(glGetUniformLocation(shader, "T1_bds"), T1_binding);
	glUniform1i(glGetUniformLocation(shader, "T2_f"),   T2_binding);
	glUniform1i(glGetUniformLocation(shader, "T3_v"),   T3_binding);
}


GLuint terrain_vertexbuffer, terrain_normalbuffer, terrain_uvbuffer;
std::vector<glm::vec3> terrain_vertices;
std::vector<glm::vec2> terrain_uvs;
std::vector<glm::vec3> terrain_normals;

GLuint terrain_shader;

void load_terrain() {
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, 1024/2, 768/2);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	terrain_shader = LoadShaders( "assets/shaders/pipeline/visualization.vert", "assets/shaders/pipeline/visualization.frag" );

	// Get a handle for our "MVP" uniform


	// Read our .obj file
	bool res = loadOBJ("assets/terrain.obj", terrain_vertices, terrain_uvs, terrain_normals);

	// Load it into a VBO
	glGenBuffers(1, &terrain_vertexbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertexbuffer);
	glBufferData(GL_ARRAY_BUFFER, terrain_vertices.size() * sizeof(glm::vec3), &terrain_vertices[0], GL_STATIC_DRAW);

	glGenBuffers(1, &terrain_normalbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_normalbuffer);
	glBufferData(GL_ARRAY_BUFFER, terrain_normals.size() * sizeof(glm::vec3), &terrain_normals[0], GL_STATIC_DRAW);

	glGenBuffers(1, &terrain_uvbuffer);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_uvbuffer);
	glBufferData(GL_ARRAY_BUFFER, terrain_uvs.size() * sizeof(glm::vec2), &terrain_uvs[0], GL_STATIC_DRAW);
}

void render_terrain(GLuint programID) {
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// Use our shader
	glUseProgram(programID);

	// Compute the MVP matrix from keyboard and mouse input
	computeMatricesFromInputs();
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	glm::mat4 ModelMatrix = glm::mat4(1.0);
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	// This one is also important
	glUniformMatrix4fv(glGetUniformLocation(programID, "MV"), 1, GL_FALSE, &(ViewMatrix*ModelMatrix)[0][0]);

	// uniforms
	glm::vec3 u_cam_pos(20, 20, 20);
	glm::vec3 u_light_pos(0, 20, 0);
	glm::vec3 u_light_intensity(50, 50, 50);
	glm::vec3 water_color(0, 0, 1.0);
	glm::vec3 terrain_color(0, 1.0, 0);

	glUniform3f(glGetUniformLocation(programID, "u_cam_pos"), u_cam_pos.x, u_cam_pos.y, u_cam_pos.z);
	glUniform3f(glGetUniformLocation(programID, "u_light_pos"), u_light_pos.x, u_light_pos.y, u_light_pos.z);
	glUniform3f(glGetUniformLocation(programID, "u_light_intensity"), u_light_intensity.x, u_light_intensity.y, u_light_intensity.z);
	glUniform3f(glGetUniformLocation(programID, "water_color"), water_color.x, water_color.y, water_color.z);
	glUniform3f(glGetUniformLocation(programID, "terrain_color"), terrain_color.x, terrain_color.y, terrain_color.z);


	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertexbuffer);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	// 2nd attribute buffer : UVs
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_uvbuffer);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// 3rd attribute buffer : normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_normalbuffer);
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// Draw the triangle !
	glDrawArrays(GL_TRIANGLES, 0, terrain_vertices.size() );

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

int timestep = 0;
// Performs a single erosion pass on the given textures, updates the references accordingly
void erosion_pass_flat(glm::ivec2 field_size, Framebuffer *T1_bds, Framebuffer *T2_f, Framebuffer *T3_v, Framebuffer *temp) {

	//glBlendFunc(GL_ONE, GL_ZERO);
	
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
	float rain_intensity = 1;
	timestep += 1;
	float delta_t = 0.000125;

	glUniform1f(glGetUniformLocation(rain_shader, "rain_intensity"), rain_intensity);
	glUniform2f(glGetUniformLocation(rain_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1i(glGetUniformLocation(rain_shader, "timestep"), timestep);
	glUniform1f(glGetUniformLocation(rain_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

	// render_water_sources();

	bind_framebuffer_target(temp->render_ref, field_size);
	glUseProgram(outflowFlux_shader);
	pass_texture_uniforms(outflowFlux_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	float A = 10, l = .1, g = 9.81;
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
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

/*
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
	float K_c = 0.05, K_s = 0.05, K_d = 0.05; 
	glUniform2f(glGetUniformLocation(erosionDeposition_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform3f(glGetUniformLocation(erosionDeposition_shader, "K"), K_c, K_s, K_d);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

	bind_framebuffer_target(temp->render_ref, field_size);
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
	float K_e = .1;
	// uniforms
	glUniform1f(glGetUniformLocation(evaporation_shader, "K_e"), K_e);
	glUniform1f(glGetUniformLocation(evaporation_shader, "delta_t"), delta_t);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);
	*/
}

void erosion_loop_flat() {
	init_erosion_shaders_flat();
	load_terrain();

	glm::ivec2 field_size(4096,4096);
	
	Framebuffer T1_bds = gen_framebuffer(field_size, GL_LINEAR, GL_REPEAT, GL_RGBA32F); // GL_RGBA32F = HDR Framebuffers
	Framebuffer T2_f =   gen_framebuffer(field_size, GL_LINEAR, GL_REPEAT, GL_RGBA32F);
	Framebuffer T3_v =   gen_framebuffer(field_size, GL_LINEAR, GL_REPEAT, GL_RGBA32F);
	Framebuffer temp =   gen_framebuffer(field_size, GL_LINEAR, GL_REPEAT, GL_RGBA32F);

	
    getErrors();

	glDisable(GL_BLEND);
	//glBlendFunc(GL_ONE, GL_ZERO);
    bind_framebuffer_target(T1_bds.render_ref, field_size);
	glClear(GL_COLOR_BUFFER_BIT);
    bind_framebuffer_target(T2_f.render_ref, field_size);
	glClear(GL_COLOR_BUFFER_BIT);
    bind_framebuffer_target(T3_v.render_ref, field_size);
	glClear(GL_COLOR_BUFFER_BIT);

	glUseProgram(init_shader_erosion_flat);
    bind_framebuffer_target(temp.render_ref, field_size);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds.texture_ref);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f.texture_ref);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v.texture_ref);
	pass_texture_uniforms(init_shader_erosion_flat, 0, 1, 2);
	render_terrain(init_shader_erosion_flat);
	std::swap(T1_bds, temp);
    
    // Then, execute render loop:
    do {
    	getErrors();
		//glBlendFunc(GL_ONE, GL_ZERO);
        bind_framebuffer_target(0, screen_size);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUseProgram(terrain_shader);
		bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds.texture_ref);
		bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f.texture_ref);
		bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v.texture_ref);
		pass_texture_uniforms(terrain_shader, 0, 1, 2);
        render_terrain(terrain_shader);

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
	GLuint init_shader = LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/misc/noise.frag" );
	glUseProgram(init_shader);
	glUniform2f(glGetUniformLocation(init_shader, "screen_size"), fbuf_size.x, fbuf_size.y);
    getErrors();

	GLuint conway_shader = LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/misc/conway.frag" );
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

	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

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

    passthrough_shader = LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/misc/blit.frag");
    getErrors();
}

int main( void )
{
	init_glfw_opengl();
	// load_terrain();
	erosion_loop_flat();
    // conway();
    return 0;
}
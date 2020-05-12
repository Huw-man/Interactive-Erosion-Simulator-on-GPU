// Include standard headers
#include <cstdio>
#include <cstdlib>
#include <utility>
#include <vector>
#include <iostream>
#include <functional>
#include <string>
#include <main.hpp>
#include <random>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <GLFW/glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/ext.hpp"
using namespace glm;

#include <common/shader.hpp>
#include <common/texture.hpp>
#include <common/controls.hpp>
#include <common/objloader.hpp>

#include "plane_mesh.hpp"
#include "image_texture.hpp"
#include "buffers.h"

// gui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

#define getErrors() handle_gl_errors( __LINE__ )

glm::ivec2 screen_size(1920, 1080);

void handle_gl_errors(int LINE) {
    GLenum err;
    while((err = glGetError()) != GL_NO_ERROR)
    {
        std::cout << "OpenGL Error: " << err << "at line " << LINE << std::endl;
    }
}


/*************************************************
 *                                               *
 *                OPENGL HELPERS                 *
 *                                               *
 *************************************************/

struct Framebuffer {
    GLuint render_ref;
    GLuint texture_refs[8];
	int num_textures;
	GLuint depth_texture_ref; // only valid if specified
	GLuint depth_renderbuffer; // only valid if specified
	glm::ivec2 size;

	Framebuffer(GLuint render_ref, GLuint *texture_refs, int num_textures, GLuint depth_texture_ref, GLuint depth_renderbuffer, glm::ivec2 size) {
		this->render_ref = render_ref;
		for (int i = 0; i < 8; ++ i) this->texture_refs[i] = texture_refs[i];
		this->num_textures = num_textures;
		this->depth_texture_ref = depth_texture_ref;
		this->depth_renderbuffer = depth_renderbuffer;
		this->size = size;
	}
};

enum {
	NO_DEPTH_TEXTURE,
	DEPTH_RENDERBUFFER,
	DEPTH_TEXTURE	
};

Framebuffer gen_framebuffer(glm::ivec2 size, GLenum filter = GL_NEAREST, GLenum wrap = GL_REPEAT, GLenum texture_dat = GL_RGBA, 
							glm::vec4 borderColor = glm::vec4(0.0), int num_textures=1, int use_depth = NO_DEPTH_TEXTURE) {
    GLuint fbo;
    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo); 
    

	GLuint texture_refs[8];
	for (int i = 0; i < num_textures; ++i) {
		glGenTextures(1, texture_refs + i);
		glBindTexture(GL_TEXTURE_2D, texture_refs[i]);

    	glTexImage2D(GL_TEXTURE_2D, 0, texture_dat, size.x, size.y, 0, GL_RGBA, GL_FLOAT, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);  
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);
		glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, &borderColor[0]);

    	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, texture_refs[i], 0);
	}

	GLuint depth_texture_ref;
	GLuint depth_renderbuffer;

	if (use_depth == DEPTH_TEXTURE) {
		glGenTextures(1, &depth_texture_ref);
		glBindTexture(GL_TEXTURE_2D, depth_texture_ref);
		glTexImage2D(GL_TEXTURE_2D, 0,GL_DEPTH_COMPONENT24, size.x, size.y, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
		
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);  
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 0);

		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depth_texture_ref, 0);
	}
	else if (use_depth == DEPTH_RENDERBUFFER) {
		glGenRenderbuffers(1, &depth_renderbuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depth_renderbuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, size.x, size.y);
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depth_renderbuffer);
	}
    
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		fprintf( stderr, "Failed to create framebuffer. Ooops!.\n" );
		glfwTerminate();
		exit(-1);
    }

    Framebuffer ret(fbo, texture_refs, num_textures, depth_texture_ref, depth_renderbuffer, size);
    return ret;
}

void bindFramebuffer(Framebuffer* fbo) {
	if (fbo == 0) {
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
    	glViewport(0, 0, screen_size.x, screen_size.y);
	}
	else {
		glBindFramebuffer(GL_FRAMEBUFFER, fbo->render_ref);
		glViewport(0, 0, fbo->size.x, fbo->size.y);

		GLenum bufs[8] = {
			GL_COLOR_ATTACHMENT0, 
			GL_COLOR_ATTACHMENT1, 
			GL_COLOR_ATTACHMENT2, 
			GL_COLOR_ATTACHMENT3, 
			GL_COLOR_ATTACHMENT4, 
			GL_COLOR_ATTACHMENT5, 
			GL_COLOR_ATTACHMENT6, 
			GL_COLOR_ATTACHMENT7
		};
		glDrawBuffers(fbo->num_textures, bufs);
	}
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

/*************************************************
 *                                               *
 *            SIMULATION INITIALIZATION          *
 *                                               *
 *************************************************/

GLuint init_shader_erosion_flat;

GLuint passthrough_shader;

GLuint 	rain_shader, 
		waterSource_shader, 
		outflowFlux_shader, 
		waterSurface_shader, 
		velocityField_shader, 
		erosionDeposition_shader, 
		sedimentTransportation_shader,
		evaporation_shader,
		smoothing_shader;

// initialized in load_terrain()
StackBuffer<glm::vec2> river_sources = StackBuffer<glm::vec2>(BufferType::ARRAY, DrawType::DYNAMIC);
StackBuffer<glm::vec3> river_source_UVs = StackBuffer<glm::vec3>(BufferType::ARRAY, DrawType::DYNAMIC);
GLuint source_mask;


void init_erosion_shaders_flat() {
	init_shader_erosion_flat = 		LoadShaders( "assets/shaders/misc/height_to_r.vert", "assets/shaders/misc/height_to_r.frag" );
	rain_shader = 					LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/rain.frag" );
	waterSource_shader = 			LoadShaders( "assets/shaders/pipeline/sources.vert", "assets/shaders/pipeline/sources.frag" );
	outflowFlux_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/outflow_flux.frag" );
	waterSurface_shader =		    LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/water_surface.frag" );
	velocityField_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/velocity_field.frag" );
	erosionDeposition_shader = 		LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/erosion_deposition.frag" );
	sedimentTransportation_shader = LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/sediment_transportation.frag" );
	evaporation_shader = 			LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/evaporation.frag" );
	smoothing_shader = 				LoadShaders( "assets/shaders/misc/blit.vert", "assets/shaders/pipeline/smooth.frag" );
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

ImageTexture *dirt, *sand, *sand2;
GLuint skybox;
GLuint noiseTexture;
GLuint terrain_prepass_shader, water_prepass_shader, water_secondpass_shader, ssao_shader, env_shader;
void load_terrain() {
	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    // Hide the mouse and enable unlimited mouvement
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    
    // Set the mouse at the center of the screen
    glfwPollEvents();
    glfwSetCursorPos(window, screen_size.x / 2, screen_size.y / 2);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	// Create and compile our GLSL program from the shaders
	terrain_prepass_shader = LoadShaders( "assets/shaders/vis/terrain_prepass.vert", "assets/shaders/vis/terrain_prepass.frag" );
	water_prepass_shader = LoadShaders( "assets/shaders/vis/water_prepass.vert", "assets/shaders/vis/water_prepass.frag" );
	water_secondpass_shader = LoadShaders( "assets/shaders/vis/water_secondpass.vert", "assets/shaders/vis/water_secondpass.frag" );

	ssao_shader = LoadShaders("assets/shaders/misc/blit.vert","assets/shaders/vis/SSAO.frag");

	dirt  = new ImageTexture("assets/textures/dirt");
	sand  = new ImageTexture("assets/textures/sand");
	sand2 = new ImageTexture("assets/textures/dwayne");
	skybox = loadCubemap("assets/textures/sky1/sky");
	source_mask = loadDDS("assets/textures/circle_mask.dds");

	// Read our .obj file
	//bool res = loadOBJ("assets/terrain.obj", terrain_vertices, terrain_uvs, terrain_normals);
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

	river_sources.generateBuffer();
	river_source_UVs.generateBuffer();
}

void add_source(glm::vec2 pos, float diag, float intensity) {
	//std::cout << pos.x << " " << pos.y << std::endl;
	glm::vec2 topleft(pos-diag);
	glm::vec2 bottomright(pos+diag);
	glm::vec2 topright(pos + glm::vec2(diag,-diag));
	glm::vec2 bottomleft(pos + glm::vec2(-diag,diag));

	std::vector<glm::vec2> tris = {topleft,topright,bottomright,bottomright,bottomleft,topleft};
	river_sources.push(tris);
	
	glm::vec3 topleftUV(0,0,intensity);
	glm::vec3 bottomrightUV(1,1,intensity);
	glm::vec3 toprightUV(1,0,intensity);
	glm::vec3 bottomleftUV(0,1,intensity);

	std::vector<glm::vec3> uvs = {topleftUV,toprightUV,bottomrightUV,bottomrightUV,bottomleftUV,topleftUV};
	river_source_UVs.push(uvs);

	river_sources.generateBuffer();
	river_source_UVs.generateBuffer();
}

// used as the callback for mouse button
void handleSourcePlacements_callback(GLFWwindow* window, int button, int action, int mods) {
	if (top_view_toggle && button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		// place source on click
		vec2 cp = getCursorPos();
		vec4 mouse_clip(
			cp.x * 2 / screen_size.x  - 1, 
			1 - cp.y * 2 / screen_size.y, 
			1, 
			1
		);
		vec4 mouse_worldspace = glm::inverse(getProjectionMatrix() * getViewMatrix()) * mouse_clip;

		glm::vec3 o = getCameraPos();
		glm::vec3 d = normalize(mouse_worldspace - vec4(o, 1));
		float t = - o.y / d.y;

		vec3 i = o + t*d;

		// glm::vec2 mPos = getCursorPos() / glm::vec2(screen_size);
		glm::vec2 mPos = vec2(i.x, i.z);
		// std::cout<<cp.x << "," << cp.y <<std::endl;
		// std::cout<<o.x << "," << o.y << ", " << o.z <<std::endl;
		// std::cout<<mouse_worldspace.x << "," << mouse_worldspace.y << ", " << mouse_worldspace.z <<std::endl;
		// std::cout << t << std::endl;
		// std::cout<<i.x << "," << i.y << ", " << i.z <<std::endl;
		add_source(mPos,0.01,0.5);
	}
}


float global_source_intensity=100.0;
void render_sources(GLuint programID, float delta_t) {
	glBlendFunc(GL_ONE,GL_ONE);
	glUniform1f(glGetUniformLocation(programID,"global_source_intensity"), global_source_intensity);
	glUniform1f(glGetUniformLocation(programID,"delta_t"), delta_t);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	river_sources.bindBuffer();
	glVertexAttribPointer(0,2,GL_FLOAT,false,0,0);

	river_source_UVs.bindBuffer();
	glVertexAttribPointer(1,3,GL_FLOAT,false,0,0);

	glDrawArrays(GL_TRIANGLES, 0, river_sources.size);

	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(0);
}

void render_terrain(GLuint programID, std::function<void()> render_mesh) {

	// Use our shader
	glUseProgram(programID);

	// Compute the MVP matrix from keyboard and mouse input
	// computeMatricesFromInputs();
	glm::mat4 ProjectionMatrix = getProjectionMatrix();
	glm::mat4 ViewMatrix = getViewMatrix();
	glm::mat4 ModelMatrix(1.0);
	ModelMatrix = glm::scale(ModelMatrix, glm::vec3(10,1,10));
	glm::mat4 MVP = ProjectionMatrix * ViewMatrix * ModelMatrix;

	// Send our transformation to the currently bound shader, 
	// in the "MVP" uniform
	GLuint MatrixID = glGetUniformLocation(programID, "MVP");
	glUniformMatrix4fv(MatrixID, 1, GL_FALSE, &MVP[0][0]);
	// This one is also important for screen space stuff.
	glUniformMatrix4fv(glGetUniformLocation(programID, "MV"), 1, GL_FALSE, &(ViewMatrix*ModelMatrix)[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(programID, "P"), 1, GL_FALSE, &(ProjectionMatrix)[0][0]);

	// uniforms
	glm::vec4 u_cam_pos = glm::vec4(0);
	glm::vec4 u_light_pos(20, 10, 0, 1);
	u_light_pos = ViewMatrix * u_light_pos;
	glm::vec3 u_light_intensity(400, 400, 400);
	glm::vec3 terrain_color(154/255.0,  99/255.0,  72/255.0);
	glm::vec3 water_color  ( 52/255.0, 133/255.0, 157/255.0);

	glUniform3f(glGetUniformLocation(programID, "u_cam_pos"), u_cam_pos.x, u_cam_pos.y, u_cam_pos.z);
	glUniform3f(glGetUniformLocation(programID, "u_light_pos"), u_light_pos.x, u_light_pos.y, u_light_pos.z);
	glUniform3f(glGetUniformLocation(programID, "u_light_intensity"), u_light_intensity.x, u_light_intensity.y, u_light_intensity.z);
	glUniform3f(glGetUniformLocation(programID, "water_color"), water_color.x, water_color.y, water_color.z);
	glUniform3f(glGetUniformLocation(programID, "terrain_color"), terrain_color.x, terrain_color.y, terrain_color.z);

	render_mesh();
}

void render_obj_mesh() {
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

/*************************************************
 *                                               *
 *                VISUALIZATION                  *
 *                                               *
 *************************************************/

void PlaneMesh::makeBuffers() {
	glGenBuffers(1, &vbuf);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(float), verts, GL_STATIC_DRAW);

	glGenBuffers(1, &ibuf);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, nInds*sizeof(int), inds, GL_STATIC_DRAW);
}

void PlaneMesh::render() {
	// 1rst attribute buffer : vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbuf);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, (void*)0 );

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibuf);
	glDrawElements(GL_TRIANGLE_STRIP, nInds, GL_UNSIGNED_INT, 0);

	glDisableVertexAttribArray(0);
}

PlaneMesh *terrainPlaneMesh;
void renderTerrainPlaneMesh() {
	terrainPlaneMesh->render();
}

void render_visualization(
		glm::ivec2 screen_size, 
		glm::ivec2 field_size, 
		Framebuffer* T1_bds, 
		Framebuffer* T2_f, 
		Framebuffer* T3_v, 
		Framebuffer *water_prepass_fbo, 
		Framebuffer *terrain_prepass_fbo,
		Framebuffer *ssao_fbo) {
	getErrors();
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds->texture_refs[0]);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f->texture_refs[0]);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v->texture_refs[0]);

	glEnable(GL_DEPTH_TEST);
	bindFramebuffer(water_prepass_fbo);
	glBlendFunc(GL_ONE, GL_ZERO);
	// Clear the screen
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(water_prepass_shader);
	glUniform2f(glGetUniformLocation(water_prepass_shader, "texture_size"), field_size.x, field_size.y);
	pass_texture_uniforms(water_prepass_shader, 0, 1, 2);
	render_terrain(water_prepass_shader, &renderTerrainPlaneMesh);
	getErrors();

	bindFramebuffer(terrain_prepass_fbo);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glUseProgram(terrain_prepass_shader);
	glUniform2f(glGetUniformLocation(terrain_prepass_shader, "texture_size"), field_size.x, field_size.y);
	pass_texture_uniforms(terrain_prepass_shader, 0, 1, 2);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, dirt ->image);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, dirt ->normals);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, sand ->image);
	bindTexture(GL_TEXTURE4, GL_TEXTURE_2D, sand ->normals);
	bindTexture(GL_TEXTURE5, GL_TEXTURE_2D, sand2->image);
	bindTexture(GL_TEXTURE6, GL_TEXTURE_2D, sand2->normals);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex1"),       1);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex1_norms"), 2);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex2"),       3);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex2_norms"), 4);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex3"),       5);
	glUniform1i(glGetUniformLocation(terrain_prepass_shader, "tex3_norms"), 6);

	render_terrain(terrain_prepass_shader, &renderTerrainPlaneMesh);
	getErrors();

	// Here comes the ssao
	bindFramebuffer(ssao_fbo);
	glUseProgram(ssao_shader);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glBlendFunc(GL_ONE, GL_ZERO);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, terrain_prepass_fbo->texture_refs[2]);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, terrain_prepass_fbo->texture_refs[1]);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, noiseTexture);
	glUniform1i(glGetUniformLocation(ssao_shader, "gPosition"),1);
	glUniform1i(glGetUniformLocation(ssao_shader, "gNormal"),  2);
	glUniform1i(glGetUniformLocation(ssao_shader, "texNoise"), 3);
	glUniform2f(glGetUniformLocation(ssao_shader, "screen_size"), screen_size.x, screen_size.y);
	glm::mat4 proj = getProjectionMatrix();
	glUniformMatrix4fv(glGetUniformLocation(ssao_shader, "P"),  1, GL_FALSE, &proj[0][0]);
	glm::mat4 iproj = glm::inverse(proj);
	glUniformMatrix4fv(glGetUniformLocation(ssao_shader, "IP"), 1, GL_FALSE, &iproj[0][0]);

	render_screen();
	/*glClearColor(1.0,0.0,0.0,1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glClearColor(0.0,0.0,0.0,0.0);*/

	




	bindFramebuffer(0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glUseProgram(water_secondpass_shader);
	glBlendFunc(GL_ONE, GL_ZERO);
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, water_prepass_fbo->texture_refs[0]);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, water_prepass_fbo->texture_refs[1]);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, water_prepass_fbo->texture_refs[2]);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, terrain_prepass_fbo->texture_refs[0]);
	bindTexture(GL_TEXTURE4, GL_TEXTURE_2D, terrain_prepass_fbo->texture_refs[1]);
	bindTexture(GL_TEXTURE5, GL_TEXTURE_2D, terrain_prepass_fbo->texture_refs[2]);
	bindTexture(GL_TEXTURE6, GL_TEXTURE_2D, ssao_fbo->texture_refs[0]);


	glm::mat4 V = getViewMatrix();
	glm::mat4 ivp = glm::inverse(proj*V);
	glm::mat4 iv = glm::inverse(V);


	bindTexture(GL_TEXTURE7, GL_TEXTURE_CUBE_MAP, skybox);
	getErrors();

	glUniform1i(glGetUniformLocation(water_secondpass_shader, "water_colors"),    0);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "water_normals"),   1);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "water_positions"),    2);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "terrain_colors"),  3);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "terrain_normals"), 4);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "terrain_positions"),  5);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "terrain_ao"),      6);
	glUniform1i(glGetUniformLocation(water_secondpass_shader, "skybox"),      7);
	glUniform1f(glGetUniformLocation(water_secondpass_shader, "zNear"), getCameraNear());
	glUniform1f(glGetUniformLocation(water_secondpass_shader, "zFar"),  getCameraFar() );

	// So we can get ourselves back into camera space for refractions/reflections
	glUniformMatrix4fv(glGetUniformLocation(water_secondpass_shader, "IP"), 1, GL_FALSE, &iproj[0][0]);

	glUniformMatrix4fv(glGetUniformLocation(water_secondpass_shader, "IVP"), 1, GL_FALSE, &ivp[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(water_secondpass_shader, "IV"), 1, GL_FALSE, &iv[0][0]);


	render_screen();
}

void init_ssao() {
	std::uniform_real_distribution<float> randomFloats(-1.0, 1.0); // random floats between [0.0, 1.0]
	std::default_random_engine generator;
	std::vector<glm::vec3> ssaoKernel;
	for (unsigned int i = 0; i < 64; ++i)
	{
		glm::vec3 sample(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator)
		);
		sample  = glm::normalize(sample);
		sample *= randomFloats(generator);
		ssaoKernel.push_back(sample);  
	}
	glUseProgram(ssao_shader);
	glUniform3fv(glGetUniformLocation(ssao_shader, "samples"), 64, &ssaoKernel[0][0]);

	std::vector<glm::vec3> ssaoNoise;
	for (unsigned int i = 0; i < 16; i++)
	{
		glm::vec3 noise(
			randomFloats(generator) * 2.0 - 1.0, 
			randomFloats(generator) * 2.0 - 1.0, 
			0.0f); 
		ssaoNoise.push_back(noise);
	}  
	glGenTextures(1, &noiseTexture);
	glBindTexture(GL_TEXTURE_2D, noiseTexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, 4, 4, 0, GL_RGB, GL_FLOAT, &ssaoNoise[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT); 
}

/*************************************************
 *                                               *
 *                  SIMULATION                   *
 *                                               *
 *************************************************/

// some constants for our rain simulation
glm::vec2 bucket_position(0.5,0.5);

// simulation constants, editable from gui
int timestep = 0;
float rain_intensity = 20.0;
float delta_t = 0.0005;
float K_c = 0.01, K_s = 0.01, K_d = 0.01;
float K_e = 0.95;
float A = 0.6, l = 1.0, g = 9.81;
glm::vec2 l_xy(1.0,1.0);
float max_height_difference = 0.6f;
float K_s_depthfac = 3.0, K_s_noisefac = 2.0; // Multiply K_s by this per height/-lin noise (sorry for the awful joke)

int run_sim = 1; // 0 = restart sim, 1 = run sim, 2 = exit completely

// Performs a single erosion pass on the given textures, updates the references accordingly
void erosion_pass_flat(glm::ivec2 field_size, Framebuffer *T1_bds, Framebuffer *T2_f, Framebuffer *T3_v, Framebuffer *temp, Framebuffer *orig_T1) {

	glBlendFunc(GL_ONE, GL_ZERO);
	
	// For sake of simplicity, I'll bind all the textures at the start and just set the uniforms in the shaders accordingly
	// I don't technically have to set the uniforms in the shaders every time, but so be it.
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds->texture_refs[0]);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f->texture_refs[0]);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v->texture_refs[0]);
	bindTexture(GL_TEXTURE3, GL_TEXTURE_2D, temp->texture_refs[0]);
	// corresponds to above, to help me keep track as textures get passed around
	int T1_binding = 0, T2_binding = 1, T3_binding = 2, temp_binding = 3;

	bindFramebuffer(temp);
	glUseProgram(rain_shader);
	pass_texture_uniforms(rain_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	timestep += 1;

	glUniform1f(glGetUniformLocation(rain_shader, "rain_intensity"), rain_intensity);
	glUniform2f(glGetUniformLocation(rain_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1i(glGetUniformLocation(rain_shader, "timestep"), timestep);
	glUniform1f(glGetUniformLocation(rain_shader, "delta_t"), delta_t);

	if (timestep % 1 == 0) {
		bucket_position = glm::vec2(rand()/(float)RAND_MAX, rand()/(float)RAND_MAX);
	}

	glUniform2f(glGetUniformLocation(rain_shader, "bucket_position"), bucket_position.x, bucket_position.y);
	glUniform1f(glGetUniformLocation(rain_shader, "drop_bucket"), 1.0f);

	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);



	glBlendFunc(GL_ONE, GL_ONE);
	bindFramebuffer(T1_bds);
	glUseProgram(waterSource_shader);
	bindTexture(GL_TEXTURE4, GL_TEXTURE_2D, source_mask);
	glUniform1i(glGetUniformLocation(waterSource_shader, "mask"), 4);
	render_sources(waterSource_shader, delta_t);
	glBlendFunc(GL_ONE, GL_ZERO);


	bindFramebuffer(temp);
	glUseProgram(outflowFlux_shader);
	pass_texture_uniforms(outflowFlux_shader, T1_binding, T2_binding, T3_binding);

	// uniforms

	glUniform3f(glGetUniformLocation(outflowFlux_shader, "alg"), A, l, g);
	glUniform2f(glGetUniformLocation(outflowFlux_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(outflowFlux_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(outflowFlux_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T2_f, *temp);
	std::swap(T2_binding, temp_binding);

	bindFramebuffer(temp);
	glUseProgram(waterSurface_shader);
	pass_texture_uniforms(waterSurface_shader, T1_binding, T2_binding, T3_binding);

	// uniforms

	glUniform2f(glGetUniformLocation(waterSurface_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(waterSurface_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(waterSurface_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

	bindFramebuffer(temp);
	glUseProgram(velocityField_shader);
	pass_texture_uniforms(velocityField_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	glUniform2f(glGetUniformLocation(velocityField_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform2f(glGetUniformLocation(velocityField_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(velocityField_shader, "delta_t"), delta_t);

	render_screen();
	std::swap(*T3_v, *temp);
	std::swap(T3_binding, temp_binding);
	

	bindFramebuffer(temp);
	glUseProgram(erosionDeposition_shader);
	pass_texture_uniforms(erosionDeposition_shader, T1_binding, T2_binding, T3_binding);
	// uniforms
	bindTexture(GL_TEXTURE5, GL_TEXTURE_2D, orig_T1->texture_refs[0]);
	glUniform2f(glGetUniformLocation(erosionDeposition_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(erosionDeposition_shader, "K_s_depthfac"), K_s_depthfac);
	glUniform1f(glGetUniformLocation(erosionDeposition_shader, "K_s_noisefac"), K_s_noisefac);
	glUniform1i(glGetUniformLocation(erosionDeposition_shader, "orig_T1"), 5);
	glUniform2f(glGetUniformLocation(erosionDeposition_shader, "l_xy"), l_xy.x, l_xy.y);
	glUniform3f(glGetUniformLocation(erosionDeposition_shader, "K"), K_c, K_s, K_d);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);
	getErrors();
	

	bindFramebuffer(temp);
	glUseProgram(sedimentTransportation_shader);
	pass_texture_uniforms(sedimentTransportation_shader, T1_binding, T2_binding, T3_binding);
	glUniform2f(glGetUniformLocation(sedimentTransportation_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(sedimentTransportation_shader, "delta_t"), delta_t);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);


	bindFramebuffer(temp);
	glUseProgram(evaporation_shader);
	pass_texture_uniforms(evaporation_shader, T1_binding, T2_binding, T3_binding);
	// uniforms
	glUniform1f(glGetUniformLocation(evaporation_shader, "K_e"), K_e);
	glUniform1f(glGetUniformLocation(evaporation_shader, "delta_t"), delta_t);
	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);
	getErrors();

	


	bindFramebuffer(temp);
	glUseProgram(smoothing_shader);
	pass_texture_uniforms(smoothing_shader, T1_binding, T2_binding, T3_binding);

	// uniforms
	glUniform2f(glGetUniformLocation(smoothing_shader, "texture_size"), field_size.x, field_size.y);
	glUniform1f(glGetUniformLocation(smoothing_shader, "m_hdiff"), max_height_difference);

	render_screen();
	std::swap(*T1_bds, *temp);
	std::swap(T1_binding, temp_binding);

	
}

void erosion_loop_flat() {
	run_sim = 1;
	init_erosion_shaders_flat();
	load_terrain();
	init_ssao();

	glm::ivec2 field_size(1024,1024);
	terrainPlaneMesh = new PlaneMesh(field_size);

	Framebuffer T1_bds = gen_framebuffer(field_size, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_RGBA32F); // GL_RGBA32F = HDR Framebuffers
	Framebuffer T2_f =   gen_framebuffer(field_size, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_RGBA32F);
	Framebuffer T3_v =   gen_framebuffer(field_size, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_RGBA32F);
	Framebuffer temp =   gen_framebuffer(field_size, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_RGBA32F);
	Framebuffer orig_T1= gen_framebuffer(field_size, GL_NEAREST, GL_CLAMP_TO_BORDER, GL_RGBA32F);

	// Has render targets for colors, normal, and depth.
	Framebuffer water_prepass_fbo   = gen_framebuffer(screen_size, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_RGBA32F, glm::vec4(0), 3, DEPTH_TEXTURE);
	Framebuffer terrain_prepass_fbo = gen_framebuffer(screen_size, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_RGBA32F, glm::vec4(0), 3, DEPTH_TEXTURE);
	Framebuffer ssao_fbo            = gen_framebuffer(screen_size, GL_LINEAR, GL_CLAMP_TO_BORDER, GL_R32F,    glm::vec4(0), 1, NO_DEPTH_TEXTURE);
	
    getErrors();

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ZERO);

	bindFramebuffer(&T1_bds);
	glClear(GL_COLOR_BUFFER_BIT);
	bindFramebuffer(&T2_f);
	glClear(GL_COLOR_BUFFER_BIT);
	bindFramebuffer(&T3_v);
	glClear(GL_COLOR_BUFFER_BIT);
	getErrors();

	glUseProgram(init_shader_erosion_flat);
    bindFramebuffer(&temp);
	glClear(GL_COLOR_BUFFER_BIT);
	glDisable(GL_CULL_FACE);
	bindTexture(GL_TEXTURE0, GL_TEXTURE_2D, T1_bds.texture_refs[0]);
	bindTexture(GL_TEXTURE1, GL_TEXTURE_2D, T2_f.texture_refs[0]);
	bindTexture(GL_TEXTURE2, GL_TEXTURE_2D, T3_v.texture_refs[0]);
	pass_texture_uniforms(init_shader_erosion_flat, 0, 1, 2);
	render_terrain(init_shader_erosion_flat, &render_obj_mesh);
	std::swap(T1_bds, temp);

	bindFramebuffer(&orig_T1);
	glClear(GL_COLOR_BUFFER_BIT);
	render_texture(T1_bds.texture_refs[0], passthrough_shader);
    
	init_gui();
	init_controls();

	glfwSetMouseButtonCallback(window, handleSourcePlacements_callback);
	getErrors();

    // Then, execute render loop:
    do {
		computeMatricesFromInputs();
		render_visualization(screen_size, field_size, &T1_bds, &T2_f, &T3_v, &water_prepass_fbo, &terrain_prepass_fbo, &ssao_fbo);
		
		// Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
		gui_window();
		ImGui::Render();
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

		for (int i = 0; i < 16; i++)
			erosion_pass_flat(field_size, &T1_bds, &T2_f, &T3_v, &temp, &orig_T1);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
		getErrors();

		if (glfwGetKey(window, GLFW_KEY_ESCAPE ) == GLFW_PRESS) {
			run_sim = 2;
		}
    } while(run_sim == 1);

	
}


/*************************************************
 *                                               *
 *             CONWAY'S GAME OF LIFE             *
 *                                               *
 *************************************************/

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

    Framebuffer a = gen_framebuffer(fbuf_size, GL_NEAREST, GL_REPEAT, GL_RGBA, glm::vec4(0.0));
    Framebuffer b = gen_framebuffer(fbuf_size, GL_NEAREST, GL_REPEAT, GL_RGBA, glm::vec4(0.0));

    // First, render noise to a:
    bindFramebuffer(&a);
	glClear(GL_COLOR_BUFFER_BIT);
    render_texture(b.texture_refs[0], init_shader);
    
    // Then, execute render loop:
    do {
        bindFramebuffer(0);
		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // blit a to screen
        render_texture(a.texture_refs[0], passthrough_shader);

        // next conway step, perform onto b
        bindFramebuffer(&b);
        render_texture(a.texture_refs[0], conway_shader);

        // swap a and b
        std::swap(a,b);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();
    } while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

}

/*************************************************
 *                                               *
 *                WINDOW CREATION                *
 *                                               *
 *************************************************/

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

/*************************************************
 *                                               *
 *                GUI Creation                   *
 *                                               *
 *************************************************/

void init_gui() {
	//Setup IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	ImGui::StyleColorsDark();
	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init((char *)glGetString(GL_NUM_SHADING_LANGUAGE_VERSIONS));
}

void gui_window() {
	ImGui::Begin("Controls");

	ImGui::Text("w, a, s, d to move\nSpace: fly up, L_Shift: fly down\ne to toggle camera pan\nESC to quit");

	float step = 1.0e-5f;
	float step_fast = 0.1f;
	const char* format = "%.5f";
	float power = 1.0f;
	ImGui::InputFloat("rain_intensity", &rain_intensity, step, step_fast, format, power);
	ImGui::InputFloat("source_intensity", &global_source_intensity, step, step_fast, format, power);
	ImGui::InputFloat("delta_t", &delta_t, step, step_fast, format, power);
	ImGui::InputFloat("K_c:sediment capacity", &K_c, step, step_fast, format, power);
	ImGui::InputFloat("K_s:dissolving constant ", &K_s, step, step_fast, format, power);
	ImGui::InputFloat("K_s depthfac:variation with h", &K_s_depthfac, step, step_fast, format, power);
	ImGui::InputFloat("K_s noisefac:variation with noise", &K_s_noisefac, step, step_fast, format, power);
	ImGui::InputFloat("K_d:deposition constant ", &K_d, step, step_fast, format, power);
	ImGui::InputFloat("K_e:evaporation constant", &K_e, step, step_fast, format, power);
	ImGui::InputFloat("A: cross sectional area of pipe", &A, 0.01, 1, "%.2f", power);
	ImGui::InputFloat("l: length of pipe", &l, 0.01, 0.1, "%.2f", power);
	ImGui::InputFloat("g: gravity", &g, 0.01, 1, "%.2f", power);
	ImGui::InputFloat("delta_H:max height difference", &max_height_difference, step, step_fast, format, power);
	ImGui::InputFloat2("L_x, L_y", glm::value_ptr(l_xy), 3);
	
	ImGui::Checkbox("place_sources", &top_view_toggle);

	if (ImGui::Button("restart")) {
		run_sim = 0;
	}

	ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
	ImGui::End();
}

int main( void )
{
	while(run_sim != 2) {
		init_glfw_opengl();
		erosion_loop_flat();

		// TODO: clear all buffers
		river_sources = StackBuffer<glm::vec2>(BufferType::ARRAY, DrawType::DYNAMIC);
		river_source_UVs = StackBuffer<glm::vec3>(BufferType::ARRAY, DrawType::DYNAMIC);
		// Cleanup
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
    // conway();
    return 0;
}
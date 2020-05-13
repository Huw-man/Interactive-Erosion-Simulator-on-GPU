// Include GLFW
#include <GLFW/glfw3.h>
extern GLFWwindow* window; // The "extern" keyword here is to access the variable "window" declared in tutorialXXX.cpp. This is a hack to keep the tutorials simple. Please avoid this.

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include <iostream>
#include <common/controls.hpp>
#include <main.hpp>
// gui
#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>
#include <imgui/imgui_internal.h>

// Initial position
glm::vec3 position(0, 10, 0); 
glm::vec3 direction(0);
// Initial horizontal angle 
float horizontalAngle = 3.14f / 4.0f;
// Initial vertical angle 
float verticalAngle = -3.14f / 4.0f;
// Initial Field of View
float initialFoV = 70.0f;

float speed = 1.0f; // 3 units / second
float mouseSpeed = 0.002f;

glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;
double xpos, ypos;
bool pan_toggle = false;
bool top_view_toggle = false;
bool paused = false;
int pause_delay = 20;

void e_key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_E && action == GLFW_PRESS) {
		pan_toggle = !pan_toggle;
		if (pan_toggle) {
			glfwSetCursorPos(window, screen_size.x/2, screen_size.y/2);
			xpos = screen_size.x/2;
			ypos = screen_size.y/2;
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
		} else {
			glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
		}
	}
}

// bind key callback
void init_controls() {
	glfwSetKeyCallback(window, e_key_callback);
}

void computeMatricesFromInputs(){

	// glfwGetTime is called only once, the first time this function is called
	static double lastTime = glfwGetTime();

	// Compute time difference between current and last frame
	double currentTime = glfwGetTime();
	float deltaTime = float(currentTime - lastTime);

	// Get mouse position
	glfwGetCursorPos(window, &xpos, &ypos);
	
	if (top_view_toggle) {
		top_view();
	}

	if (pan_toggle) {
		// Reset mouse position for next frame
		glfwSetCursorPos(window, screen_size.x/2, screen_size.y/2);
		
		// Compute new orientation
		horizontalAngle += mouseSpeed * float(screen_size.x / 2 - xpos );
		verticalAngle   += mouseSpeed * float(screen_size.y / 2 - ypos );
	}

	// Direction : Spherical coordinates to Cartesian coordinates conversion
	direction = vec3(
		cos(verticalAngle) * sin(horizontalAngle), 
		sin(verticalAngle),
		cos(verticalAngle) * cos(horizontalAngle)
	);

	glm::vec3 xz_direction(direction.x, 0, direction.z);
	
	// Right vector
	glm::vec3 right = glm::vec3(
		sin(horizontalAngle - 3.14f/2.0f), 
		0,
		cos(horizontalAngle - 3.14f/2.0f)
	);
	

	// Up vector
	glm::vec3 up = glm::cross( right, direction );

	// Move forward
	if (glfwGetKey( window, GLFW_KEY_P ) == GLFW_PRESS && pause_delay <= 0){
		paused = !paused;
		pause_delay = 20;
	}
	pause_delay -= 1;
	// Move forward
	if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS){
		position += normalize(xz_direction) * deltaTime * speed;
	}
	// Move backward
	if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS){
		position -= normalize(xz_direction) * deltaTime * speed;
	}
	// Strafe right
	if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS){
		position += right * deltaTime * speed;
	}
	// Strafe left
	if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS){
		position -= right * deltaTime * speed;
	}
	// fly up
	if (glfwGetKey( window, GLFW_KEY_SPACE ) == GLFW_PRESS){
		position.y += deltaTime * speed;
	}
	// fly down
	if (glfwGetKey( window, GLFW_KEY_LEFT_SHIFT ) == GLFW_PRESS){
		position.y -= deltaTime * speed;
	}



	float FoV = initialFoV;// - 5 * glfwGetMouseWheel(); // Now GLFW 3 requires setting up a callback for this. It's a bit too complicated for this beginner's tutorial, so it's disabled instead.

	// Projection matrix : 45� Field of View, 4:3 ratio, display range : 0.1 unit <-> 100 units
	ProjectionMatrix = glm::perspective(glm::radians(FoV), 4.0f / 3.0f, getCameraNear(), getCameraFar());
	// Camera matrix
	ViewMatrix       = glm::lookAt(
								position,           // Camera is here
								position+direction, // and looks here : at the same position, plus "direction"
								vec3(0, 1, 0)                  // Head is up (set to 0,-1,0 to look upside-down)
						   );

	// For the next frame, the "last time" will be "now"
	lastTime = currentTime;
}

void top_view() {
	position = vec3(5, 9, 5);
	verticalAngle = -3.14f / 2.0f;
	horizontalAngle = 0.0f;
}

float getCameraNear() { return 0.1; }

float getCameraFar() { return 100.0; }

glm::vec3 getCameraPos() { return position; }

glm::vec3 getCameraDirection() { return direction; }

glm::mat4 getViewMatrix() {
	return ViewMatrix;
}

glm::mat4 getProjectionMatrix() {
	return ProjectionMatrix;
}

glm::vec2 getCursorPos() {
	double x,y;
	glfwGetCursorPos(window,&x,&y);
	return glm::vec2(x,y);
}


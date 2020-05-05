#ifndef CONTROLS_HPP
#define CONTROLS_HPP

/*
class Controls {
	// window
	GLFWwindow* window;
    vec2 screenSize;

	// Initial position : on +Z
	glm::vec3 position; 
	// Initial horizontal angle : toward -Z
	float horizontalAngle;
	// Initial vertical angle : none
	float verticalAngle;
	// Initial Field of View
	float initialFoV;

	float speed; // 3 units / second
	float mouseSpeed;

    glm::mat4 ViewMatrix;
    glm::mat4 ProjectionMatrix;

	public:
		Controls(
			GLFWwindow* window,
            glm::vec2 screenSize,
			vec3 position, 
			float horizontalAngle, 
			float verticaleAngle, 
			float initialFoV, 
			float speed, 
			float mouseSpeed) 
			{
			this->window = window;
            this->screenSize = screenSize;
			this->position = position;
			this->horizontalAngle = horizontalAngle;
			this->verticalAngle = verticalAngle;
			this->initialFoV = initialFoV;
			this->speed = speed;
			this->mouseSpeed = mouseSpeed;
		}

		Controls(GLFWwindow* window, glm::vec2 screen_size) {
			Controls(
				window,
                screen_size,
				glm::vec3( 0, 10, 0),
				0.0f,
				0.0f,
				70.0f,
				3.0f,
				0.01f
			);
		}
		glm::mat4 getViewMatrix();
		glm::mat4 getProjectionMatrix();
        void computeMatricesFromInputs();
}; */

void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();

#endif
#ifndef CONTROLS_HPP
#define CONTROLS_HPP

extern bool top_view_toggle;

void init_controls();
void computeMatricesFromInputs();
glm::mat4 getViewMatrix();
glm::mat4 getProjectionMatrix();
float getCameraNear();
float getCameraFar();
glm::vec3 getCameraPos();
void top_view();

#endif
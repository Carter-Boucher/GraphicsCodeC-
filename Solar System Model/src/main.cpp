// main.cpp

//#include <GL/glew.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>
#include <algorithm>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"
#include "Panel.h"
#include "planet.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/rotate_vector.hpp"

#include "UnitCube.h"
#include <random>
#include <imgui/imgui.h>
#include <thread>

// Callback class
class Assignment4 : public CallbackInterface {
public:
	Assignment4()
		: camera(glm::radians(45.f), glm::radians(45.f), 3.0)
		, aspect(1.0f)
		, rightMouseDown(false)
		, mouseOldX(0.0)
		, mouseOldY(0.0)
		, planetsPtr(nullptr)
		// Free camera initial values
		, cameraPos(0.0f, 0.0f, 5.0f)
		, cameraFront(0.0f, 0.0f, -1.0f)
		, cameraUp(0.0f, 1.0f, 0.0f)
		, yaw(-90.0f)
		, pitch(0.0f)
		, moveSpeed(0.1f)
		, mouseSensitivity(0.1f)
		, freeCamera(false)
	{
		for (int i = 0; i < 1024; ++i) {
			keys[i] = false;
		}
	}

	void setPlanets(std::vector<planet>* p) {
		planetsPtr = p;
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		//if w is pressed, move camera forward
		if (key == GLFW_KEY_W && action == GLFW_PRESS) {
			std::cout << "W key pressed" << std::endl;
		}

		/*if (key >= 0 && key < 1024) {
			if (action == GLFW_PRESS) {
				keys[key] = true;
				std::cout << "Key pressed: " << key << std::endl;
			}
			else if (action == GLFW_RELEASE) {
				keys[key] = false;
			}
		}*/
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override {
		if (button == GLFW_MOUSE_BUTTON_RIGHT) {
			if (action == GLFW_PRESS)
				rightMouseDown = true;
			else if (action == GLFW_RELEASE)
				rightMouseDown = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) override {
		float xoffset = static_cast<float>(xpos - mouseOldX);
		float yoffset = static_cast<float>(mouseOldY - ypos); // reversed since y-coords go from bottom to top

		mouseOldX = xpos;
		mouseOldY = ypos;

		if (rightMouseDown && !freeCamera) {
			// Orbit mode
			camera.incrementTheta(yoffset);
			camera.incrementPhi(xoffset);
		}

		if (rightMouseDown && freeCamera) {
			// Free camera mouse look
			xoffset *= mouseSensitivity;
			yoffset *= mouseSensitivity;

			yaw += xoffset;
			pitch += yoffset;

			if (pitch > 89.0f) pitch = 89.0f;
			if (pitch < -89.0f) pitch = -89.0f;

			glm::vec3 direction;
			direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
			direction.y = sin(glm::radians(pitch));
			direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
			cameraFront = glm::normalize(direction);
		}
	}

	virtual void scrollCallback(double xoffset, double yoffset) override {
		if (!freeCamera) {
			camera.incrementR(yoffset);
		}
		else {
			// Could implement zoom if desired
		}
	}

	virtual void windowSizeCallback(int width, int height) override {
		CallbackInterface::windowSizeCallback(width, height);
		aspect = float(width) / float(height);
	}

	void processInput() {
		// Only move if in free camera mode
		//if (!freeCamera) return;

		//glm::vec3 forward = cameraFront;
		//glm::vec3 right = glm::normalize(glm::cross(cameraFront, cameraUp));

		//if (keys[GLFW_KEY_W]) cameraPos += moveSpeed * forward;
		//if (keys[GLFW_KEY_S]) cameraPos -= moveSpeed * forward;
		//if (keys[GLFW_KEY_A]) cameraPos -= moveSpeed * right;
		//if (keys[GLFW_KEY_D]) cameraPos += moveSpeed * right;
		//if (keys[GLFW_KEY_SPACE]) cameraPos += moveSpeed * cameraUp;
		//if (keys[GLFW_KEY_LEFT_CONTROL]) cameraPos -= moveSpeed * cameraUp;
	}

	void viewPipeline(ShaderProgram& sp, glm::vec3 at) {
		glm::mat4 M = glm::mat4(1.0f);
		glm::mat4 V;
		if (!freeCamera) {
			V = camera.getView(at);
		}
		else {
			V = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		}
		glm::mat4 P = glm::perspective(glm::radians(45.0f), aspect, 0.01f, 1000.0f);

		glUniformMatrix4fv(glGetUniformLocation(sp, "M"), 1, GL_FALSE, glm::value_ptr(M));
		glUniformMatrix4fv(glGetUniformLocation(sp, "V"), 1, GL_FALSE, glm::value_ptr(V));
		glUniformMatrix4fv(glGetUniformLocation(sp, "P"), 1, GL_FALSE, glm::value_ptr(P));
	}

	Camera camera;
	bool freeCamera; // Moved freeCamera into Assignment4 for a single source of truth.

	glm::vec3 cameraPos;
	glm::vec3 cameraFront;
	glm::vec3 cameraUp;

private:
	bool rightMouseDown;
	float aspect;
	double mouseOldX;
	double mouseOldY;
	std::vector<planet>* planetsPtr;

	float yaw;
	float pitch;
	float moveSpeed;
	float mouseSensitivity;
	bool keys[1024];
};


class CurveEditorPanelRenderer : public PanelRendererInterface {
public:
	CurveEditorPanelRenderer(Assignment4* a4Ptr)
		: checkboxValue(false),
		comboSelection(0),
		speedR(1.0f),
		speedO(1.0f),
		reset(false),
		a4(a4Ptr)
	{
		// Initialize options for the combo box
		options[0] = "Sun";
		options[1] = "Mercury";
		options[2] = "Venus";
		options[3] = "Earth";
		options[4] = "Mars";
		options[5] = "Jupiter";
		options[6] = "Saturn";
		options[7] = "Uranus";
		options[8] = "Neptune";
		options[9] = "Pluto";
	}

	virtual void render() override {
		ImGui::Text("(%.1f FPS)", ImGui::GetIO().Framerate);

		ImGui::Combo("Center", &comboSelection, options, IM_ARRAYSIZE(options));

		ImGui::Checkbox("Pause", &checkboxValue);

		// Directly reference a4->freeCamera to ensure synchronization
		ImGui::Checkbox("Free Camera", &(a4->freeCamera));

		ImGui::SliderFloat("Rotation speed", &speedR, 0.01f, 100.0f);
		ImGui::SliderFloat("Orbit speed", &speedO, 0.01f, 20.0f);

		if (ImGui::Button("Reset"))
		{
			reset = true;
			speedO = 1.0f;
			speedR = 1.0f;
		}
	}

	int getMode() const { return comboSelection; }
	bool getPause() const { return checkboxValue; }
	float getSpeedR() const { return speedR; }
	float getSpeedO() const { return speedO; }
	bool getReset() const { return reset; }
	void resetReset() { reset = false; }

private:
	bool checkboxValue;
	int comboSelection;
	const char* options[10];
	float speedR;
	float speedO;
	bool reset;
	Assignment4* a4; // store a pointer to Assignment4 to access freeCamera
};

bool compareByDistance(planet& a, planet& b, Camera& camera) {
	float distanceA = glm::length(a.getCentre() - camera.getPos());
	float distanceB = glm::length(b.getCentre() - camera.getPos());
	return distanceA > distanceB;
}

void translateAllPlanets(planet& p, std::vector<planet>& planets) {
	glm::vec3 targetCenter = p.getCentre();
	glm::vec3 translationVector = -targetCenter;

	for (auto& planet : planets) {
		glm::vec3 newCenter = planet.getCentre() + translationVector;
		planet.setCentre(newCenter);
		for (auto& vertex : planet.m_cpu_geom.verts) {
			vertex += translationVector;
		}
		planet.m_gpu_geom.setVerts(planet.m_cpu_geom.verts);
	}
}

void resetPlanetPositions(std::vector<planet>& planets) {
	for (auto& planet : planets) {
		glm::vec3 translationVector = planet.getOriginalCentre() - planet.getCentre();
		planet.setCentre(planet.getOriginalCentre());
		for (auto& vertex : planet.m_cpu_geom.verts) {
			vertex += translationVector;
		}
		planet.m_gpu_geom.setVerts(planet.m_cpu_geom.verts);
	}
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453 - Assignment 4");
	Panel panel(window.getGLFWwindow());

	auto a4 = std::make_shared<Assignment4>();
	window.setCallbacks(a4);

	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>(a4.get());
	panel.setPanelRenderer(curve_editor_panel_renderer);

	std::vector<planet> planets;

	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	planets.push_back(planet("background", "textures/8k_stars.png", GL_LINEAR, glm::vec3(0.f, 0.f, 0.f), 350, true, nullptr, 0.f, glm::vec3(0.f, 0.f, 0.f), 0.f, glm::vec3(0.f, 0.f, 0.f)));
	planets.push_back(planet("sun", "textures/8k_sun.png", GL_LINEAR, glm::vec3(0.f, 0.f, 0.f), 6.f, false, nullptr, 0.0758f, glm::vec3(0.f, 1.f, 0.f), 0.f, glm::vec3(0.f, 0.f, 0.f)));
	planets.push_back(planet("mercury", "textures/8k_mercury.png", GL_LINEAR, glm::vec3(10.f, 0.f, 0.f), 0.35f, false, nullptr, 0.00011f, glm::vec3(0.2, 1, 0.1), 1.81f, glm::vec3(0.2, 1, 0.1)));
	planets.push_back(planet("venus", "textures/8k_venus_surface.png", GL_LINEAR, glm::vec3(18.f, 0.f, 0.f), 0.87f, false, nullptr, 0.00007f, glm::vec3(-0.1, 1, 0.2), 1.33f, glm::vec3(-0.1, 1, 0.2)));
	planets.push_back(planet("earth", "textures/8k_earth_daymap.png", GL_LINEAR, glm::vec3(25.f, 0.f, 0.f), 0.91f, false, nullptr, 0.0176f, glm::vec3(0.f, 1.f, 0.f), 1.13f, glm::vec3(0, 1, 0)));
	planets.push_back(planet("moon", "textures/8k_moon.png", GL_LINEAR, glm::vec3(23.3f, 0.f, 0.f), 0.1f, false, nullptr, 0.3f, glm::vec3(0.f, 1.f, 0.f), 15.f, glm::vec3(0, 1, 0)));
	planets.push_back(planet("mars", "textures/8k_mars.png", GL_LINEAR, glm::vec3(38.f, 0.f, 0.f), 0.49f, false, nullptr, 0.0091f, glm::vec3(0.3, 1, -0.2), 0.808f, glm::vec3(0.3, 1, -0.2)));
	planets.push_back(planet("jupiter", "textures/8k_jupiter.png", GL_LINEAR, glm::vec3(47.f, 0.f, 0.f), 3.004f, false, nullptr, 0.477f, glm::vec3(-0.2, 1, 0.2), .5f, glm::vec3(0.2, 1, 0.2)));
	planets.push_back(planet("saturn", "textures/8k_saturn.png", GL_LINEAR, glm::vec3(58.f, 0.f, 0.f), 2.36f, false, nullptr, 0.374f, glm::vec3(0.1, 1, 0.3), .37f, glm::vec3(0.1, 1, 0.3)));
	planets.push_back(planet("uranus", "textures/2k_uranus.png", GL_LINEAR, glm::vec3(69.f, 0.f, 0.f), 1.64f, false, nullptr, 0.098f, glm::vec3(-0.3, 1, -0.2), .26f, glm::vec3(-0.3, 1, -0.2)));
	planets.push_back(planet("neptune", "textures/2k_neptune.png", GL_LINEAR, glm::vec3(80.f, 0.f, 0.f), 1.53f, false, nullptr, 0.102f, glm::vec3(0.2, 1, -0.3), .21f, glm::vec3(0.2, 1, -0.3)));
	planets.push_back(planet("pluto", "textures/8k_moon.png", GL_LINEAR, glm::vec3(91.f, 0.f, 0.f), 0.17f, false, nullptr, 0.00178f, glm::vec3(0.2, 1, 0.1), .18f, glm::vec3(0.2, 1, 0.1)));

	a4->setPlanets(&planets);

	while (!window.shouldClose()) {
		if (curve_editor_panel_renderer->getReset()) {
			resetPlanetPositions(planets);
			curve_editor_panel_renderer->resetReset();
		}
		glfwPollEvents();

		a4->processInput(); // handle free camera movement

		shader.use();

		// Clear screen
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glFrontFace(GL_CCW);

		// Sort planets by distance from camera for correct drawing order
		std::sort(planets.begin() + 1, planets.end(), [&](planet& a, planet& b) {
			return compareByDistance(a, b, a4->camera);
			});

		// After sorting, reacquire pointers
		planet* sun = nullptr;
		planet* earth = nullptr;
		planet* moon = nullptr;

		for (auto& p : planets) {
			if (p.getName() == "sun") sun = &p;
			else if (p.getName() == "earth") earth = &p;
			else if (p.getName() == "moon") moon = &p;
		}

		if (moon && earth) {
			moon->setParent(earth);
		}

		if (sun) {
			glUniform3fv(glGetUniformLocation(shader, "lightPos"), 1, glm::value_ptr(sun->getCentre()));
		}
		glUniform3fv(glGetUniformLocation(shader, "viewPos"), 1, glm::value_ptr(a4->camera.getPos()));
		glUniform3fv(glGetUniformLocation(shader, "lightColor"), 1, glm::value_ptr(glm::vec3(1.0f, 1.0f, 1.0f)));

		a4->viewPipeline(shader, glm::vec3(0.0f, 0.0f, 0.0f));

		// Render planets
		for (auto& p : planets) {
			p.texture.bind();
			p.m_gpu_geom.setup(0, 1, 2);
			glDrawArrays(GL_TRIANGLES, 0, p.m_size);
			p.texture.unbind();
		}

		if (!curve_editor_panel_renderer->getPause()) {
			// Rotate planets
			float speedR = curve_editor_panel_renderer->getSpeedR();
			for (auto& p : planets) {
				if (p.getName() != "background") {
					p.rotatePlanet(speedR);
				}
			}
			// Orbit planets
			float speedO = curve_editor_panel_renderer->getSpeedO();
			if (sun) {
				for (int i = 0; i < (int)planets.size(); ++i) {
					if (planets[i].getName() != "sun" && planets[i].getName() != "background") {
						planets[i].orbitPlanet(speedO, sun->getCentre());
					}
				}
			}
		}

		glDisable(GL_FRAMEBUFFER_SRGB);

		// Only recenter planets if we're not in free camera mode
		if (!a4->freeCamera) {
			int mode = curve_editor_panel_renderer->getMode();
			planet* target = nullptr;
			const char* names[10] = { "sun","mercury","venus","earth","mars","jupiter","saturn","uranus","neptune","pluto" };
			for (auto& p : planets) {
				if (p.getName() == names[mode]) {
					target = &p;
					break;
				}
			}
			if (target) translateAllPlanets(*target, planets);
		}

		panel.render();
		window.swapBuffers();

		// Print camera position for debugging
		if (a4->freeCamera) {
			std::cout << "Free camera pos: "
				<< a4->cameraPos.x << " "
				<< a4->cameraPos.y << " "
				<< a4->cameraPos.z << std::endl;
		}
		else {
			auto camPos = a4->camera.getPos();
			std::cout << "Spherical camera pos: "
				<< camPos.x << " "
				<< camPos.y << " "
				<< camPos.z << std::endl;
		}
	}

	glfwTerminate();
	return 0;
}

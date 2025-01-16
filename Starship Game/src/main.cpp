/*
CPSC 453 F24 Assignment 2
Carter Boucher
30116690
October 25, 2024
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>
#include <ctime> 
#include <math.h>
#include <glm/gtc/type_ptr.hpp>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "GameObject.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

bool start = false;

class MyCallbacks : public CallbackInterface {
public:
	MyCallbacks(ShaderProgram& shader) :
		shader(shader)
	{}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
			resetGame();
		}

		if (key == GLFW_KEY_W) {
			input.movingForward = true;
			input.movingBackward = false;
			start = true;
		}
		else if (key == GLFW_KEY_W && action == GLFW_RELEASE) {
			input.movingForward = false;
		}

		if (key == GLFW_KEY_S) {
			input.movingBackward = true;
			input.movingForward = false;
			start = true;
		}
		else if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			input.movingBackward = false;
		}
	}

	//adapted from tutorial info
	virtual void cursorPosCallback(double xpos, double ypos) {
		input.cursor = glm::vec2((xpos + 0.5) / 800 * 2 - 1, (1 - (ypos + 0.5) / 800) * 2 - 1);
	}

	void resetGame() {resetInput(&input);}

	Input InputCallback() {
		Input news = input;
		input.movingForward = false;
		input.movingBackward = false;
		input.reset = false;
		return news;
	}

private:
	Input input;
	ShaderProgram& shader;
};

int main() {
	Log::debug("Starting main");

	std::srand(std::time(0));

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	GLDebug::enable();

	//SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	//CALLBACKS
	std::shared_ptr<MyCallbacks> callback = std::make_shared<MyCallbacks>(shader);
	window.setCallbacks(callback); 

	//game variables
	int score = 0;
	int diamondCount = 5;
	std::vector<int> capturedDiamonds;

	GameObject ship("textures/ship.png", GL_NEAREST, 0.15f, 0.1f);

	std::vector<GameObject> diamonds;
	for (int i = 0; i < diamondCount; i++) {
		diamonds.push_back(GameObject("textures/diamond.png", GL_NEAREST, 0.08f, 0.08f));
		diamonds[i].initializeDiamond();
	}

	while (!window.shouldClose()) {
		glfwPollEvents();
		Input input = callback->InputCallback();
		if (input.reset) {
			start = false;
			score = 0;
			capturedDiamonds.clear();
			for (int i = 0; i < diamonds.size(); i++) {
				diamonds[i].initializeDiamond();
			}
		}

		shader.use();

		// Clear screen
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//diamonds
		//loop through diamonds
		for (int i = 0; i < diamonds.size(); i++) {
			//skip captured diamonds
			if (std::find(capturedDiamonds.begin(), capturedDiamonds.end(), i) != capturedDiamonds.end()) continue;
			// Update diamond position
			if (diamonds[i].render) {
				if (collisionCheck(ship.position, diamonds[i].position) && start ) {
					score++;
					ship.resizeShip();
					diamonds[i].render = false;
					diamonds[i].initializeDiamond();
					capturedDiamonds.push_back(i);
				}
				else {
					diamonds[i].updateDiamond(input);
					glm::mat4 diamondTMatrix = diamonds[i].getTMatrix();
					glUniformMatrix4fv(0 ,1 , GL_FALSE, glm::value_ptr(diamondTMatrix));
					diamonds[i].ggeom.bind();
					diamonds[i].texture.bind();
					glDrawArrays(GL_TRIANGLES, 0, 6);
					diamonds[i].texture.unbind();
				}
			}
		}	

		//ship
		ship.updateShip(input);
		glm::mat4 TMatrix = ship.getTMatrix();
		glUniformMatrix4fv(0 ,1 ,GL_FALSE ,glm::value_ptr(TMatrix));

		ship.ggeom.bind();
		ship.texture.bind();
		glDrawArrays(GL_TRIANGLES, 0, 6);
		ship.texture.unbind();

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		// Starting the new ImGui frame
		ImGui_ImplOpenGL3_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();
		// Putting the text-containing window in the top-left of the screen.
		ImGui::SetNextWindowPos(ImVec2(5, 5));

		// Setting flags
		ImGuiWindowFlags textWindowFlags =
			ImGuiWindowFlags_NoMove |				// text "window" should not move
			ImGuiWindowFlags_NoResize |				// should not resize
			ImGuiWindowFlags_NoCollapse |			// should not collapse
			ImGuiWindowFlags_NoSavedSettings |		// don't want saved settings mucking things up
			ImGuiWindowFlags_AlwaysAutoResize |		// window should auto-resize to fit the text
			ImGuiWindowFlags_NoBackground |			// window should be transparent; only the text should be visible
			ImGuiWindowFlags_NoDecoration |			// no decoration; only the text should be visible
			ImGuiWindowFlags_NoTitleBar;			// no title; only the text should be visible

		// Begin a new window with these flags. (bool *)0 is the "default" value for its argument.
		ImGui::Begin("scoreText", (bool*)0, textWindowFlags);

		// Scale up text a little, and set its value
		ImGui::SetWindowFontScale(1.5f);

		if (score == 5) ImGui::Text("Winner  \nPress R to restart");
		else ImGui::Text("Score: %d", score);

		// End the window.
		ImGui::End();

		ImGui::Render();	// Render the ImGui window
		ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData()); // Some middleware thing

		window.swapBuffers();
	}
	// ImGui cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	glfwTerminate();
	return 0;
}

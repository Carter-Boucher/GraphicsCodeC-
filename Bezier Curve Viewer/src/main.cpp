/*
CPSC 453 Assignment 3
Carter Boucher
30116690
Nov 18, 2024
*/

#include <glad/glad.h>

#include <iostream>
#include <string>
#include <list>
#include <vector>
#include <limits>
#include <functional>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Panel.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <random>

void scaleControlPoints(std::vector<glm::vec3>& controlPoints);
bool pointCollisionCheck(glm::vec3 point, glm::vec3 cp_point);
float computeBinomial(int n, int k);
std::vector<glm::vec3> getBezierPoints(std::vector<glm::vec3> positions);
std::vector<glm::vec3> getBSplinePoints(const std::vector<glm::vec3>& controlPoints);
void scaleControlPoints(std::vector<glm::vec3>& controlPoints);
void resetScalePoints(std::vector<glm::vec3>& controlPoints);
std::vector<glm::vec3> surfaceOfRevolution(const std::vector<glm::vec3>& curvePoints);
void updateProjectionMatrix();
void updateViewMatrix();
std::vector<glm::vec3> tensorProductSurface(const std::vector<glm::vec3>& controlPoints);

//cardinal sin need to fix these global variables, (skull emoji :( )
std::vector<glm::vec3> cp_positions_vector = {
	{-.5f, -.5f, 0.f},
	{ .5f, -.5f, 0.f},
	{ .5f,  .5f, 0.f},
	{-.5f,  .5f, 0.f}
};
std::vector<glm::vec3> tp_positions_vector_original = {
	{-1.87, 0.12, -1.93}, {-0.82, 0.35, -1.89}, {0.15, -0.21, -1.78}, {1.23, 0.47, -1.95}, {2.11, -0.08, -1.82},
	{-1.95, 0.28, -0.87}, {-0.79, 1.12, -0.92}, {0.23, 0.85, -0.79}, {1.31, 1.21, -0.98}, {2.08, 0.19, -0.81},
	{-1.78, -0.15, 0.12}, {-0.92, 1.03, 0.08}, {0.35, 0.91, 0.25}, {1.43, 1.18, 0.02}, {2.21, -0.03, 0.18},
	{-1.82, 0.31, 1.15}, {-0.77, 1.24, 1.21}, {0.19, 0.98, 1.07}, {1.27, 1.32, 1.13}, {2.15, 0.25, 1.28},
	{-1.91, -0.02, 2.03}, {-0.86, 0.21, 2.11}, {0.11, -0.18, 1.97}, {1.19, 0.33, 2.05}, {2.07, -0.05, 2.12},
	{-1.89, -0.04, 2.11}, {-0.84, 0.18, 2.17}, {0.13, -0.22, 2.03}, {1.21, 0.31, 2.19}, {2.09, -0.07, 2.15}
};
std::vector<glm::vec3> tp_positions_vector = {
	{-1.87, 0.12, -1.93}, {-0.82, 0.35, -1.89}, {0.15, -0.21, -1.78}, {1.23, 0.47, -1.95}, {2.11, -0.08, -1.82},
	{-1.95, 0.28, -0.87}, {-0.79, 1.12, -0.92}, {0.23, 0.85, -0.79}, {1.31, 1.21, -0.98}, {2.08, 0.19, -0.81},
	{-1.78, -0.15, 0.12}, {-0.92, 1.03, 0.08}, {0.35, 0.91, 0.25}, {1.43, 1.18, 0.02}, {2.21, -0.03, 0.18},
	{-1.82, 0.31, 1.15}, {-0.77, 1.24, 1.21}, {0.19, 0.98, 1.07}, {1.27, 1.32, 1.13}, {2.15, 0.25, 1.28},
	{-1.91, -0.02, 2.03}, {-0.86, 0.21, 2.11}, {0.11, -0.18, 1.97}, {1.19, 0.33, 2.05}, {2.07, -0.05, 2.12},
	{-1.89, -0.04, 2.11}, {-0.84, 0.18, 2.17}, {0.13, -0.22, 2.03}, {1.21, 0.31, 2.19}, {2.09, -0.07, 2.15}
};
bool bezier = true,insertDelete = false,reset = false,overflow = false,lineDebug = false,movePoint = false,
pointSelected = false,scaled = false, zoom = false, resetCamera = false, randomize = false, threeD = false,
revolution = false, wireframe = false, moveShape = false, first = true, surfaceReset = false, points = true;
int selectedPoint;
int fov = 90, mode = 0;
float totalScale = 1,totalX = 0,totalY = 0,x = 0,y = 0,z = 0;
glm::mat4 viewMatrix = glm::mat4(1.0f);
glm::mat4 projectionMatrix = glm::mat4(1.0f);
glm::vec3 cursorPosUp;

bool pointCollisionCheck(glm::vec3 point, glm::vec3 cp_point) {
	float distance = glm::distance(point, cp_point);
	if (distance < 0.05f) {
		return true;
	}
	return false;
}

class CurveEditorCallBack : public CallbackInterface {
public:
	CurveEditorCallBack() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) override {
		Log::info("KeyCallback: key={}, action={}", key, action);
	}

	virtual void mouseButtonCallback(int button, int action, int mods) override {
		Log::info("MouseButtonCallback: button={}, action={}", button, action);
		//if button is clicked add new point to the curve
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && insertDelete && !threeD) {
			//add point at mouse position
			double xpos, ypos;
			glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
			xpos = (xpos / 400) - 1;
			ypos = 1 - (ypos / 400);
			if (cp_positions_vector.size() < 9) {
				cp_positions_vector.push_back(glm::vec3(xpos, ypos, 0.f));
				overflow = false;
			}
			if (cp_positions_vector.size() >= 9) overflow = true;
			
		}
		if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS && insertDelete && !threeD) {
			//remove point at mouse position if it exists
			double xpos, ypos;
			glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
			xpos = (xpos / 400) - 1;
			ypos = 1 - (ypos / 400);
			for (int i = 0; i < cp_positions_vector.size(); i++) {
				if (pointCollisionCheck(glm::vec3(xpos, ypos, 0.f), cp_positions_vector[i])) {
					cp_positions_vector.erase(cp_positions_vector.begin() + i);
					break;
				}
			}
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !insertDelete && !threeD) {
			movePoint = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !insertDelete && !threeD) {
			movePoint = false;
			pointSelected = false;
		}

		//if in 3d then move perspective
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !insertDelete && mode != 0) {
			moveShape = true;
			first = true;
		}
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && !insertDelete && mode != 0) {
			moveShape = false;
		}
	}

	virtual void cursorPosCallback(double xpos, double ypos) override {
			//Log::info("CursorPosCallback: xpos={}, ypos={}", xpos, ypos);
		}

	virtual void scrollCallback(double xoffset, double yoffset) override {
		if (mode == 0) return;
		//std::cout << "here" << std::endl;
		if (yoffset == 1 && !insertDelete) {
			fov += 5;
			zoom = true;
		}
		if (yoffset == -1 && !insertDelete) {
			fov -= 5;
			zoom = false;
		}
		scaled = true;
		}

	virtual void windowSizeCallback(int width, int height) override {
			Log::info("WindowSizeCallback: width={}, height={}", width, height);
			CallbackInterface::windowSizeCallback(width, height); // Important, calls glViewport(0, 0, width, height);
		}
};

class CurveEditorPanelRenderer : public PanelRendererInterface {
public:
	CurveEditorPanelRenderer()
		: inputText(""), buttonClickCount(0), sliderValue(0.0f),
		dragValue(0.0f), inputValue(0.0f), checkboxValue(false),
		comboSelection(0)
	{
		// Initialize options for the combo box
		options[0] = "Bezier";
		options[1] = "B-Spline";

		options1[0] = "Curve Editor - Editing";
		//options1[1] = "Curve Editor - Viewing";
		options1[1] = "Orbit View - Curve";
		options1[2] = "Orbit View - Surface of Revolution";
		options1[3] = "Tensor Product Surface";
		//options1[5] = "Tensor Product Surface 2";

		// Initialize color (white by default)
		colorValue[0] = 1.0f; // R
		colorValue[1] = 1.0f; // G
		colorValue[2] = 1.0f; // B
	}

	virtual void render() override {
		//// Color selector
		ImGui::ColorEdit3("Select Background Color", colorValue); // RGB color selector

		ImGui::Combo("Program Selection", &comboSelection1, options1, IM_ARRAYSIZE(options1));
		//ImGui::Text("Selected: %s", options[comboSelection]);
		mode = comboSelection1;

		//mode 0 is curve editor editing
		if (comboSelection1 == 0) {
			revolution = false;
			ImGui::Checkbox("Insert/Delete", &checkboxValue);
			//ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
			if (checkboxValue) insertDelete = true;
			else insertDelete = false;

			ImGui::Combo("Curve Option", &comboSelection, options, IM_ARRAYSIZE(options));
			//ImGui::Text("Selected: %s", options[comboSelection]);
			if (comboSelection == 0) bezier = true;
			else bezier = false;

			ImGui::Checkbox("Debug Lines", &checkboxValue1);
			//ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
			if (checkboxValue1) lineDebug = true;
			else lineDebug = false;

			//number of points
			ImGui::Text("%d/9 Points", cp_positions_vector.size());

			ImGui::Text("Aspect Ratio: %.3f", 1);
			ImGui::Text("Scale: %.3f", 1);
			ImGui::Text("Position: (%.3f, %.3f)", 1,1);

			//// Button
			if (ImGui::Button("Reset")) {
				reset = true;
			}
		}

		//mode 2 is orbit view curve
		if (comboSelection1 == 1) {
			revolution = false;
			insertDelete = false;
			ImGui::Combo("Curve Option", &comboSelection, options, IM_ARRAYSIZE(options));
			//ImGui::Text("Selected: %s", options[comboSelection]);
			if (comboSelection == 0) bezier = true;
			else bezier = false;

			ImGui::Checkbox("Debug Lines", &checkboxValue1);
			//ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
			if (checkboxValue1) lineDebug = true;
			else lineDebug = false;

			ImGui::Text("Position: (%.3f, %.3f, %.3f)", x, y, z);
			ImGui::Text("Distance: %.3f", 1 / totalScale * 2);
			//float theta = atan(1 / 1) * 180 / 3.14159;
			ImGui::Text("Theta: %.3f Deg", totalY);
			ImGui::Text("Distance: %.3f Deg ", totalX);
			ImGui::Text("Near Clip: %.3f", 0.1f);
			ImGui::Text("Far Clip: %.3f", 100.0f);

			if (ImGui::Button("Reset Camera")) {
				//reset camera to default position
				viewMatrix = glm::mat4(1.f);
				resetCamera = true;
			}
		}

		//mode 3 is orbit view surface of revolution
		if (comboSelection1 == 2) {
			revolution = true;
			insertDelete = false;
			ImGui::Checkbox("Wireframe", &checkboxValue3);
			//ImGui::Text("Feature Enabled: %s", checkboxValue ? "Yes" : "No");
			if (checkboxValue3) wireframe = true;
			else wireframe = false;

			ImGui::Text("Position: (%.3f, %.3f, %.3f)", x, y, z);
			ImGui::Text("Distance: %.3f", 1 / totalScale * 2);
			ImGui::Text("Theta: %.3f Deg", totalY);
			ImGui::Text("Distance: %.3f Deg ", totalX);
			ImGui::Text("Near Clip: %.3f", 0.1f);
			ImGui::Text("Far Clip: %.3f", 100.0f);

			if (ImGui::Button("Reset Camera")) {
				//reset camera to default position
				viewMatrix = glm::mat4(1.f);
				resetCamera = true;
			}
		}

		//mode 4 is tensor product surface 1
		if (comboSelection1 == 3) {
			revolution = false;
			insertDelete = false;
			ImGui::Checkbox("Wireframe", &checkboxValue3);
			if (checkboxValue3) wireframe = true;
			else wireframe = false;

			ImGui::Checkbox("View Points", &checkboxValue5);
			if (checkboxValue5) points = true;
			else points = false;

			if (ImGui::Button("Randomize Points")) {
				randomize = true;
			}

			if (ImGui::Button("Original Points")) {
				surfaceReset = true;
			}

			if (ImGui::Button("Reset Camera")) {
				//reset camera to default position
				viewMatrix = glm::mat4(1.f);
				resetCamera = true;
			}
		}
	}

	glm::vec3 getColor() const {
		return glm::vec3(colorValue[0], colorValue[1], colorValue[2]);
	}

	int getMode() const {
		return comboSelection1;
	}

private:
	float colorValue[3];  // Array for RGB color values
	char inputText[256];  // Buffer for input text
	int buttonClickCount; // Count button clicks
	int fovv;
	float sliderValue;    // Value for float slider
	float dragValue;      // Value for drag input
	float inputValue;     // Value for float input
	bool checkboxValue;   // Value for checkbox
	bool checkboxValue1;
	bool checkboxValue2 = false;
	bool checkboxValue3 = false;
	bool checkboxValue4 = false;
	bool checkboxValue5 = true;
	int comboSelection;   // Index of selected option in combo box
	int comboSelection1 = 0;
	const char* options[2]; // Options for the combo box
	const char* options1[4];
};

float computeBinomial(int n, int k) {
	if (k > n) {
		return 0.0f; 
	}
	if (k == 0 || k == n) {
		return 1.0f; 
	}
	// Recursive formula for calculating binomial coefficient
	return computeBinomial(n - 1, k) + computeBinomial(n - 1, k - 1);
}

std::vector<glm::vec3> getBezierPoints(std::vector<glm::vec3> positions) {
	std::vector<float> bCurveX;
	std::vector<float> bCurveY;
	std::vector<float> bCurveZ;
	std::vector<glm::vec3> answer;
	int d = positions.size() - 1;

	for (float u = 0.0; u <= 1.0; u += 0.01) {
		float bCurveXt = 0.0f;
		float bCurveYt = 0.0f;
		float bCurveZt = 0.0f;

		for (int i = 0; i <= d; ++i) {
			// Call the computeBinomial function here
			float binomial = computeBinomial(d, i);
			bCurveXt += binomial * std::pow((1 - u), (d - i)) * std::pow(u, i) * positions[i].x;
			bCurveYt += binomial * std::pow((1 - u), (d - i)) * std::pow(u, i) * positions[i].y;
			//bCurveZt += binomial * std::pow((1 - u), (d - i)) * std::pow(u, i) * positions[i].z;
			bCurveZt = 0;
		}
		answer.push_back(glm::vec3(bCurveXt, bCurveYt, bCurveZt));
	}

	return answer;
}

std::vector<glm::vec3> getBSplinePoints(const std::vector<glm::vec3>& controlPoints) {
	if (controlPoints.size() < 2) return controlPoints;
	std::vector<glm::vec3> points = controlPoints;

	for (int i = 0; i < controlPoints.size()-1; ++i) {
		std::vector<glm::vec3> newPoints;

		for (size_t j = 0; j < points.size() - 1; ++j) {
			glm::vec3 p0 = points[j];
			glm::vec3 p1 = points[j + 1];

			glm::vec3 p01 = (3.0f * p0 + p1) / 4.0f;
			glm::vec3 p10 = (p0 + 3.0f * p1) / 4.0f;

			newPoints.push_back(p01);
			newPoints.push_back(p10);
		}

		points = newPoints;
	}

	points.insert(points.begin(), controlPoints.front());
	points.push_back(controlPoints.back());
	return points;
}

void scaleControlPoints(std::vector<glm::vec3>& controlPoints) {
	float scale = 1.f;
	if (zoom == true) scale = 95.f / 90.f;
	if (zoom == false) scale = 85.f / 90.f;
	totalScale *= scale;
	//std::cout << totalScale << std::endl;
	//std::cout << fov << std::endl;
	for (int i = 0; i < controlPoints.size(); i++) {
		controlPoints[i].x *= scale;
		controlPoints[i].y *= scale;
		controlPoints[i].z *= scale;
	}
	scaled = false;
	return;
}

void resetScalePoints(std::vector<glm::vec3>& controlPoints) {
	//reset scale of control points using totalScale
	for (int i = 0; i < controlPoints.size(); i++) {
		controlPoints[i].x /= totalScale;
		controlPoints[i].y /= totalScale;
		controlPoints[i].z /= totalScale;
	}
}

std::vector<glm::vec3> surfaceOfRevolution(const std::vector<glm::vec3>& curvePoints) {
	std::vector<glm::vec3> surfacePoints;

	int numSegments = 360;
	int increment = 2;

	for (size_t i = 0; i < curvePoints.size() - 1; ++i) {
		const glm::vec3& p1 = curvePoints[i];
		const glm::vec3& p2 = curvePoints[i + 1];

		for (int j = 0; j < numSegments; j += increment) {
			float theta = j * 3.14159 / 180.0f;

			// Rotate p1 and p2 around the y-axis
			glm::vec3 rotatedP1 = glm::vec3(
				p1.x * cos(theta),
				p1.y,
				p1.x * sin(theta)
			);

			glm::vec3 rotatedP2 = glm::vec3(
				p2.x * cos(theta),
				p2.y,
				p2.x * sin(theta)
			);

			// Create quad of each segment
			surfacePoints.push_back(rotatedP1);
			surfacePoints.push_back(rotatedP2);
			surfacePoints.push_back(rotatedP1);

			surfacePoints.push_back(rotatedP2);
			surfacePoints.push_back(rotatedP1);
			surfacePoints.push_back(rotatedP2);
		}
	}
	//insert first point from curvePoints
	surfacePoints.insert(surfacePoints.begin(), curvePoints[0]);
	surfacePoints.push_back(curvePoints[curvePoints.size() - 1]);

	return surfacePoints;
}

std::vector<glm::vec3> subdivideQuad(const glm::vec3& p00, const glm::vec3& p10, const glm::vec3& p01, const glm::vec3& p11, int depth) {
	if (depth == 0) {
		return { p00, p10, p01, p10, p11, p01 };
	}

	//Calculate midpoint points
	glm::vec3 p0 = (p00 + p10) * 0.5f;
	glm::vec3 p1 = (p10 + p11) * 0.5f;
	glm::vec3 p2 = (p01 + p11) * 0.5f;
	glm::vec3 p3 = (p00 + p01) * 0.5f;
	glm::vec3 p4 = (p00 + p10 + p01 + p11) * 0.25f;

	//Recursively subdivide the four smaller quads
	std::vector<glm::vec3> points;
	std::vector<glm::vec3> one = subdivideQuad(p00, p0, p3, p4, depth - 1);
	points.insert(points.end(), one.begin(), one.end());
	std::vector<glm::vec3> two = subdivideQuad(p0, p10, p4, p1, depth - 1);
	points.insert(points.end(), two.begin(), two.end());
	std::vector<glm::vec3> three = subdivideQuad(p3, p4, p01, p2, depth - 1);
	points.insert(points.end(), three.begin(), three.end());
	std::vector<glm::vec3> four = subdivideQuad(p4, p1, p2, p11, depth - 1);
	points.insert(points.end(), four.begin(), four.end());

	return points;
}

std::vector<glm::vec3> tensorProductSurface(const std::vector<glm::vec3>& controlPoints) {
	int numControlPointsPerRow = sqrt(controlPoints.size());

	//Bernstein basis function
	auto bernstein = [](int i, int n, float u) {
		return std::pow(u, i) * std::pow(1 - u, n - i) * computeBinomial(n, i);
		};

	//Interpolate a point on the surface given u, v parameters
	auto interpolatePoint = [&controlPoints, numControlPointsPerRow, &bernstein](float u, float v) -> glm::vec3 {
		glm::vec3 point(0.0f);
		for (int i = 0; i < numControlPointsPerRow; ++i) {
			for (int j = 0; j < numControlPointsPerRow; ++j) {
				int index = i * numControlPointsPerRow + j;
				float basisU = bernstein(i, numControlPointsPerRow - 1, u);
				float basisV = bernstein(j, numControlPointsPerRow - 1, v);
				point += controlPoints[index] * basisU * basisV;
			}
		}
		return point;
		};

	//Triangulate the surface
	int subdivisionDepth = 4; // Adjust this value to control the level of subdivision
	std::vector<glm::vec3> trianglePoints;
	for (int i = 0; i < numControlPointsPerRow - 1; ++i) {
		for (int j = 0; j < numControlPointsPerRow - 1; ++j) {
			glm::vec3 p00 = interpolatePoint(i / (float)(numControlPointsPerRow - 1), j / (float)(numControlPointsPerRow - 1));
			glm::vec3 p10 = interpolatePoint((i + 1) / (float)(numControlPointsPerRow - 1), j / (float)(numControlPointsPerRow - 1));
			glm::vec3 p01 = interpolatePoint(i / (float)(numControlPointsPerRow - 1), (j + 1) / (float)(numControlPointsPerRow - 1));
			glm::vec3 p11 = interpolatePoint((i + 1) / (float)(numControlPointsPerRow - 1), (j + 1) / (float)(numControlPointsPerRow - 1));

			std::vector<glm::vec3> quadPoints = subdivideQuad(p00, p10, p01, p11, subdivisionDepth);
			trianglePoints.insert(trianglePoints.end(), quadPoints.begin(), quadPoints.end());
		}
	}

	return trianglePoints;
}

std::vector<glm::vec3> randomize_tp_positions(const std::vector<glm::vec3>& original_positions, float max_perturbation) {
	std::vector<glm::vec3> randomized_positions = original_positions;
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_real_distribution<float> dist(-max_perturbation, max_perturbation);

	for (glm::vec3& point : randomized_positions) {
		point.y += dist(gen);
	}

	return randomized_positions;
}

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453: Assignment 3");
	Panel panel(window.getGLFWwindow());

	//GLDebug::enable();

	// CALLBACKS
	auto curve_editor_callback = std::make_shared<CurveEditorCallBack>();
	//auto turn_table_3D_viewer_callback = std::make_shared<TurnTable3DViewerCallBack>();

	auto curve_editor_panel_renderer = std::make_shared<CurveEditorPanelRenderer>();

	//Set callback to window
	window.setCallbacks(curve_editor_callback);
	// Can swap the callback instead of maintaining a state machine
	//window.setCallbacks(turn_table_3D_viewer_callback);

	//Panel inputs
	panel.setPanelRenderer(curve_editor_panel_renderer);

	ShaderProgram shader_program_default(
		"shaders/test.vert",
		"shaders/test.frag"
	);

	glm::vec3 cp_point_colour = { 1.f,0.f,0.f };
	glm::vec3 cp_line_colour = { 0.f,1.f,0.f };
	glm::vec3 cp_line_colour_black = { 0.f,0.f,0.f };
	glm::vec3 cp_line_colour_red = { 1.f,0.f,0.f };

	CPU_Geometry cp_point_cpu;
	cp_point_cpu.verts = cp_positions_vector;
	cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
	GPU_Geometry cp_point_gpu;
	cp_point_gpu.setVerts(cp_point_cpu.verts);
	cp_point_gpu.setCols(cp_point_cpu.cols);

	CPU_Geometry cp_line_cpu;
	cp_line_cpu.verts	= cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
	cp_line_cpu.cols	= std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
	GPU_Geometry cp_line_gpu;
	cp_line_gpu.setVerts(cp_line_cpu.verts);
	cp_line_gpu.setCols(cp_line_cpu.cols);

	//curve for editing
	std::vector<glm::vec3> bez2d = getBezierPoints(cp_positions_vector);
	CPU_Geometry bez2d_cpu;
	bez2d_cpu.verts = bez2d;
	bez2d_cpu.cols = std::vector<glm::vec3>(bez2d_cpu.verts.size(), cp_line_colour_black);
	GPU_Geometry bez2d_gpu;
	bez2d_gpu.setVerts(bez2d_cpu.verts);
	bez2d_gpu.setCols(bez2d_cpu.cols);

	//curve for viewing
	std::vector<glm::vec3> bez2d_view = bez2d;
	CPU_Geometry bez2d_cpu_view;
	bez2d_cpu_view.verts = bez2d_view;
	bez2d_cpu_view.cols = std::vector<glm::vec3>(bez2d_cpu_view.verts.size(), cp_line_colour_black);
	GPU_Geometry bez2d_gpu_view;
	bez2d_gpu_view.setVerts(bez2d_cpu_view.verts);
	bez2d_gpu_view.setCols(bez2d_cpu_view.cols);

	std::vector<glm::vec3> surfacePoints = surfaceOfRevolution(bez2d);
	CPU_Geometry surface_cpu;
	//push back all to surface_cpu
	for (int i = 0; i < surfacePoints.size(); i++) {
		surface_cpu.verts.push_back(surfacePoints[i]);
	}
	surface_cpu.cols = std::vector<glm::vec3>(surface_cpu.verts.size(), cp_line_colour_black);
	GPU_Geometry surface_gpu;
	surface_gpu.setVerts(surface_cpu.verts);
	surface_gpu.setCols(surface_cpu.cols);

	//tensor product surface 1
	std::vector<glm::vec3> tp1_cpu;
	for (int i = 0; i < tp_positions_vector.size(); i++) {
		tp1_cpu.push_back(tp_positions_vector[i]);
	}
	CPU_Geometry tp1_cpu_geo;
	tp1_cpu_geo.verts = tp1_cpu;
	tp1_cpu_geo.cols = std::vector<glm::vec3>(tp1_cpu_geo.verts.size(), cp_line_colour_black);
	GPU_Geometry tp1_gpu;
	tp1_gpu.setVerts(tp1_cpu_geo.verts);
	tp1_gpu.setCols(tp1_cpu_geo.cols);

	int previousMode = curve_editor_panel_renderer->getMode();

	while (!window.shouldClose()) {
		glfwPollEvents();

		if (reset) {
			cp_positions_vector.clear();
			reset = false;
			overflow = false;
		}

		if (scaled) {
			if(mode == 3) scaleControlPoints(tp_positions_vector);
			else scaleControlPoints(cp_positions_vector);
			
		};

		if (resetCamera) {
			if (curve_editor_panel_renderer->getMode() != 3) resetScalePoints(cp_positions_vector);
			if (curve_editor_panel_renderer->getMode() == 3) resetScalePoints(tp_positions_vector);
			resetCamera = false;
			totalY = 0;
			totalX = 0;
			totalScale = 1;
		}

		if (randomize) {
			tp_positions_vector = randomize_tp_positions(tp_positions_vector, 0.1f);
			randomize = false;
		}

		if (surfaceReset) {
			tp_positions_vector = tp_positions_vector_original;
			surfaceReset = false;
			totalScale = 1;
		}

		if (previousMode != curve_editor_panel_renderer->getMode()) {
			wireframe = false;
			//reset scale of control points using totalScale
			if (previousMode != 3) resetScalePoints(cp_positions_vector);
			//reset scale of control points using totalScale
			if (previousMode == 3) resetScalePoints(tp_positions_vector);

			totalScale = 1;
			totalX = 0;
			totalY = 0;
			viewMatrix = glm::mat4(1.f);
			resetCamera = true;
			previousMode = curve_editor_panel_renderer->getMode();
		}

		//mode 0 is curve editor editing
		if (curve_editor_panel_renderer->getMode() == 0) {
			for (int i = 0; i < cp_positions_vector.size(); i++) {
				x += cp_positions_vector[i].x;
				y += cp_positions_vector[i].y;
				z += cp_positions_vector[i].z;
			}
			x /= cp_positions_vector.size();
			y /= cp_positions_vector.size();
			z /= cp_positions_vector.size();
			moveShape = false;
			double xpos, ypos;
			glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
			xpos = (xpos / 400) - 1;
			ypos = 1 - (ypos / 400);
			if (movePoint) {
				for (int i = 0; i < cp_positions_vector.size(); i++) {
					if (pointCollisionCheck(glm::vec3(xpos, ypos, 0.f), cp_positions_vector[i]) && !pointSelected) {
						//attach point to mouse until left click is released
						selectedPoint = i;
						pointSelected = true;
					}
				}
				if (pointSelected) {
					cursorPosUp = glm::vec3(xpos, ypos, 0.f);
					cp_positions_vector[selectedPoint] = cursorPosUp;
				}
			}

			glm::vec3 background_colour = curve_editor_panel_renderer->getColor();
			cp_point_cpu.verts = cp_positions_vector;
			cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
			cp_point_gpu.setVerts(cp_point_cpu.verts);
			cp_point_gpu.setCols(cp_point_cpu.cols);

			cp_line_cpu.verts = cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
			cp_line_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
			cp_line_gpu.setVerts(cp_line_cpu.verts);
			cp_line_gpu.setCols(cp_line_cpu.cols);

			if (!bezier) bez2d = getBSplinePoints(cp_positions_vector);
			else bez2d = getBezierPoints(cp_positions_vector);
			bez2d_cpu.verts = bez2d;
			bez2d_cpu.cols = std::vector<glm::vec3>(bez2d_cpu.verts.size(), cp_line_colour_black);
			bez2d_gpu.setVerts(bez2d_cpu.verts);
			bez2d_gpu.setCols(bez2d_cpu.cols);

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_FRAMEBUFFER_SRGB);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//------------------------------------------

			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(glm::mat4(1.0f)));

			// Use the default shader (can use different ones for different objects)
			shader_program_default.use();

			//Render control points
			cp_point_gpu.bind();
			glPointSize(15.f);
			glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

			if (lineDebug) {
				//Render curve connecting control points
				cp_line_gpu.bind();
				//glLineWidth(10.f); //May do nothing (like it does on my computer): https://community.khronos.org/t/3-0-wide-lines-deprecated/55426
				glDrawArrays(GL_LINE_STRIP, 0, cp_line_cpu.verts.size());
			}
			bez2d_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, bez2d_cpu.verts.size());

			//------------------------------------------
			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		//mode 1 is orbit view curve
		if (curve_editor_panel_renderer->getMode() == 1) {
			//calculate centre of curve
			for (int i = 0; i < cp_positions_vector.size(); i++) {
				x += cp_positions_vector[i].x;
				y += cp_positions_vector[i].y;
				z += cp_positions_vector[i].z;
			}
			x /= cp_positions_vector.size();
			y /= cp_positions_vector.size();
			z /= cp_positions_vector.size();
			if (moveShape) {
				//Calculate mouse movement
				double xpos, ypos;
				glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
				static double lastX = xpos, lastY = ypos;
				float xoffset = xpos - lastX;
				float yoffset = lastY - ypos; //Reversed sign to get the correct rotation direction
				lastX = xpos;
				lastY = ypos;

				if (first) {
					first = false;
					continue;
				}

				//Sensitivity factor
				float sensitivity = 0.2f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;

				//Calculate rotation angles
				float yaw = xoffset;
				float pitch = yoffset;

				totalX -= yaw;
				totalY -= pitch;

				if (totalY > 90) {
					totalY = 90;
					continue;
				}
				else if (totalY < -90) {
					totalY = -90;
					continue;
				}

				// Prevent excessive pitch to avoid flipping the camera
				pitch = glm::clamp(pitch, -89.0f, 89.0f);

				//std::cout << pitch << std::endl;

				//Create rotation matrices
				glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, -1.0f, 0.0f));

				// Apply rotations to the view matrix
				viewMatrix = rotationY * rotationX * viewMatrix;
			}

			glm::vec3 background_colour = curve_editor_panel_renderer->getColor();
			cp_point_cpu.verts = cp_positions_vector;
			cp_point_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_point_colour);
			cp_point_gpu.setVerts(cp_point_cpu.verts);
			cp_point_gpu.setCols(cp_point_cpu.cols);

			cp_line_cpu.verts = cp_positions_vector; // We are using GL_LINE_STRIP (change this if you want to use GL_LINES)
			cp_line_cpu.cols = std::vector<glm::vec3>(cp_point_cpu.verts.size(), cp_line_colour);
			cp_line_gpu.setVerts(cp_line_cpu.verts);
			cp_line_gpu.setCols(cp_line_cpu.cols);

			if (!bezier) bez2d = getBSplinePoints(cp_positions_vector);
			else bez2d = getBezierPoints(cp_positions_vector);
			bez2d_cpu.verts = bez2d;
			bez2d_cpu.cols = std::vector<glm::vec3>(bez2d_cpu.verts.size(), cp_line_colour_black);
			bez2d_gpu.setVerts(bez2d_cpu.verts);
			bez2d_gpu.setCols(bez2d_cpu.cols);

			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_FRAMEBUFFER_SRGB);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//------------------------------------------

			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			// Use the default shader (can use different ones for different objects)
			shader_program_default.use();

			//Render control points
			cp_point_gpu.bind();
			glPointSize(15.f);
			glDrawArrays(GL_POINTS, 0, cp_point_cpu.verts.size());

			if (lineDebug) {
				//Render curve connecting control points
				cp_line_gpu.bind();
				//glLineWidth(10.f); //May do nothing (like it does on my computer): https://community.khronos.org/t/3-0-wide-lines-deprecated/55426
				glDrawArrays(GL_LINE_STRIP, 0, cp_line_cpu.verts.size());
			}
			bez2d_gpu.bind();
			glDrawArrays(GL_LINE_STRIP, 0, bez2d_cpu.verts.size());

			//------------------------------------------
			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		//mode 3 is orbit view surface of revolution
		if (curve_editor_panel_renderer->getMode() == 2) {
			for (int i = 0; i < cp_positions_vector.size(); i++) {
				x += cp_positions_vector[i].x;
				y += cp_positions_vector[i].y;
				z += cp_positions_vector[i].z;
			}
			x /= cp_positions_vector.size();
			y /= cp_positions_vector.size();
			z /= cp_positions_vector.size();
			if (moveShape) {
				//Calculate mouse movement
				double xpos, ypos;
				glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
				static double lastX = xpos, lastY = ypos;
				float xoffset = xpos - lastX;
				float yoffset = lastY - ypos; //Reversed sign to get the correct rotation direction
				lastX = xpos;
				lastY = ypos;

				if (first) {
					first = false;
					continue;
				}

				//Sensitivity factor
				float sensitivity = 0.2f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;

				//Calculate rotation angles
				float yaw = xoffset;
				float pitch = yoffset;

				totalX -= yaw;
				totalY -= pitch;

				if (totalY > 90) {
					totalY = 90;
					continue;
				}
				else if (totalY < -90) {
					totalY = -90;
					continue;
				}

				//Prevent excessive pitch to avoid flipping the camera
				pitch = glm::clamp(pitch, -89.0f, 89.0f);
				//Create rotation matrices
				glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, -1.0f, 0.0f));

				//Apply rotations to the view matrix
				viewMatrix = rotationY * rotationX * viewMatrix;
			}

			glm::vec3 background_colour = curve_editor_panel_renderer->getColor();

			if (!bezier) bez2d = getBSplinePoints(cp_positions_vector);
			else bez2d = getBezierPoints(cp_positions_vector);

			surfacePoints = surfaceOfRevolution(bez2d);
			surface_cpu.verts.clear();
			surface_cpu.cols.clear();
			surface_cpu.verts = surfacePoints;
			//std::cout << surface_cpu.verts.size() << std::endl;
			surface_cpu.cols = std::vector<glm::vec3>(surface_cpu.verts.size(), cp_line_colour_black);
			//surface_cpu.cols.insert(surface_cpu.cols.begin(), 36364, cp_line_colour_red);
			surface_gpu.setVerts(surface_cpu.verts);
			surface_gpu.setCols(surface_cpu.cols);

			//------------------------------------------
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_FRAMEBUFFER_SRGB);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			//------------------------------------------

			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			//Set the desired polygon mode (wireframe or solid)
			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}
			//erase from 36360 to 36364
			//surface_cpu.verts.resize(surface_cpu.verts.size() - (surface_cpu.verts.size()-36360));
			surface_gpu.bind();
			glDrawArrays(GL_TRIANGLE_STRIP, 0, surface_cpu.verts.size());

			glDisable(GL_FRAMEBUFFER_SRGB);
            // Set the desired polygon mode (wireframe or solid)
            if (wireframe) {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
            }
            else {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }

            surface_gpu.bind();
            glDrawArrays(GL_TRIANGLE_STRIP, 0, surface_cpu.verts.size());

            glDisable(GL_FRAMEBUFFER_SRGB);
		}

		//mode 4 is tensor product surface 1
		if (curve_editor_panel_renderer->getMode() == 3) {
			if (moveShape) {
				//Calculate mouse movement
				double xpos, ypos;
				glfwGetCursorPos(glfwGetCurrentContext(), &xpos, &ypos);
				static double lastX = xpos, lastY = ypos;
				float xoffset = xpos - lastX;
				float yoffset = lastY - ypos; //Reversed sign to get the correct rotation direction
				lastX = xpos;
				lastY = ypos;

				if (first) {
					first = false;
					continue;
				}

				//Sensitivity factor
				float sensitivity = 0.2f;
				xoffset *= sensitivity;
				yoffset *= sensitivity;

				//Calculate rotation angles
				float yaw = xoffset;
				float pitch = yoffset;

				//max y value is 89 degrees and min is -89 degrees
				//std::cout << pitch << std::endl;

				//Create rotation matrices
				glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f));
				glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), glm::radians(yaw), glm::vec3(0.0f, -1.0f, 0.0f));

				//Apply rotations to the view matrix
				viewMatrix = rotationY * rotationX * viewMatrix;
			}

			glm::vec3 background_colour = curve_editor_panel_renderer->getColor();
			//render tensor product surface 1
			tp1_cpu_geo.verts.clear();

			for (int i = 0; i < tp_positions_vector.size(); i++) {
				tp1_cpu_geo.verts.push_back(tp_positions_vector[i]);
			}
			tp1_cpu_geo.cols = std::vector<glm::vec3>(tp1_cpu_geo.verts.size(), cp_line_colour_black);
			tp1_gpu.setVerts(tp1_cpu_geo.verts);
			tp1_gpu.setCols(tp1_cpu_geo.cols);

			//------------------------------------------
			glEnable(GL_LINE_SMOOTH);
			glEnable(GL_DEPTH_TEST);
			glEnable(GL_FRAMEBUFFER_SRGB);
			//glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			glClearColor(background_colour.r, background_colour.g, background_colour.b, 1.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glUniformMatrix4fv(0, 1, GL_FALSE, glm::value_ptr(viewMatrix));
			glUniformMatrix4fv(1, 1, GL_FALSE, glm::value_ptr(projectionMatrix));

			//Use the default shader (can use different ones for different objects)
			shader_program_default.use();

			//Render tensor product surface 1
			tp1_gpu.bind();
			glPointSize(10.f);
			if(points) glDrawArrays(GL_POINTS, 0, tp1_cpu_geo.verts.size());

			//glDrawArrays(GL_LINE_STRIP, 0, tp1_cpu_geo.verts.size());

			std::vector<glm::vec3> tp1_surface = tensorProductSurface(tp_positions_vector);
			CPU_Geometry tp1_surface_cpu;
			tp1_surface_cpu.verts = tp1_surface;
			if (!wireframe) {
				for (int i = 0; i < tp1_surface_cpu.verts.size(); i++) {
					// green
					if (tp1_surface_cpu.verts[i].y <= -0.4) {
						tp1_surface_cpu.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
					}
					//yellow
					if (tp1_surface_cpu.verts[i].y < 0 && tp1_surface_cpu.verts[i].y > -0.4) {
						tp1_surface_cpu.cols.push_back(glm::vec3(1.f, 1.f, 0.f));
					}
					//orange
					if(tp1_surface_cpu.verts[i].y > 0 && tp1_surface_cpu.verts[i].y < 0.4) {
						tp1_surface_cpu.cols.push_back(glm::vec3(1.f, 0.5f, 0.f));
					}
					//red
					if (tp1_surface_cpu.verts[i].y >= 0.4) {
						tp1_surface_cpu.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
					}
				}
			}
			if (wireframe) tp1_surface_cpu.cols = std::vector<glm::vec3>(tp1_surface_cpu.verts.size(), cp_line_colour_black);
			GPU_Geometry tp1_surface_gpu;
			tp1_surface_gpu.setVerts(tp1_surface_cpu.verts);
			tp1_surface_gpu.setCols(tp1_surface_cpu.cols);

			//Set the desired polygon mode (wireframe or solid)
			if (wireframe) {
				glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			}
			else {
				glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			}

			tp1_surface_gpu.bind();
			
			glDrawArrays(GL_TRIANGLES, 0, tp1_surface_cpu.verts.size());

			//------------------------------------------
			glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		}

		//------------------------------------------
		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui
		panel.render();
		//------------------------------------------
		window.swapBuffers();
		//------------------------------------------
	}

	glfwTerminate();
	return 0;
}

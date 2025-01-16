/*
CPSC 453 F24 Assignment 1
Carter Boucher
30116690
September 30, 2024
*/


#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Window.h"

#include <chrono>
#include <thread>

// EXAMPLE CALLBACKS
class MyCallbacks : public CallbackInterface {

public:
	MyCallbacks(ShaderProgram& shader) : shader(shader) {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			shader.recompile();
		}
	}

private:
	ShaderProgram& shader;
};

class MyCallbacks2 : public CallbackInterface {

public:
	MyCallbacks2() {}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS) {
			std::cout << "called back" << std::endl;
		}
	}
};
// END EXAMPLES

//calculation helper functions
//--------------------------------------------------------------------------------------------------------------
glm::vec3 rotatePoint(glm::vec3 point, glm::vec3 center, float angle) {
	//Calculate the angle in radians
	float angleRadians = angle * (float)(3.14159265358979323846f / 180.0f);

	//Rotate the point
	float rotatedX = center.x + (point.x - center.x) * cos(angleRadians) - (point.y - center.y) * sin(angleRadians);
	float rotatedY = center.y + (point.x - center.x) * sin(angleRadians) + (point.y - center.y) * cos(angleRadians);
	point.x = rotatedX;
	point.y = rotatedY;

	return point;
}

glm::vec3 translatePoint(glm::vec3 point, glm::vec3 translation) {
	point.x += translation.x;
	point.y += translation.y;
	point.z += translation.z;
	return point;
}
//--------------------------------------------------------------------------------------------------------------

//Fractals
//--------------------------------------------------------------------------------------------------------------
std::vector<glm::vec3> sierpinskyTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, int depth) {
	std::vector<glm::vec3> triangle;
	if (depth == 0) {
		triangle.push_back(a);
		triangle.push_back(b);
		triangle.push_back(c);
	}
	else {
		glm::vec3 ab = (a + b) / 2.0f;
		glm::vec3 ac = (a + c) / 2.0f;
		glm::vec3 bc = (b + c) / 2.0f;
		triangle = sierpinskyTriangle(a, ab, ac, depth - 1);
		std::vector<glm::vec3> triangle2 = sierpinskyTriangle(ab, b, bc, depth - 1);
		triangle.insert(triangle.end(), triangle2.begin(), triangle2.end());
		std::vector<glm::vec3> triangle3 = sierpinskyTriangle(ac, bc, c, depth - 1);
		triangle.insert(triangle.end(), triangle3.begin(), triangle3.end());
	}
	return triangle;
}

std::vector<glm::vec3> pythagorasTree(glm::vec3 a, glm::vec3 b, int depth) {
	std::vector<glm::vec3> tree;
	float x = b.x - a.x;
	float y = a.y - b.y;
	float cx = a.x - y;
	float cy = a.y - x;
	float dx = b.x - y;
	float dy = b.y - x;
	float ex = ((cx + dx) / 2.0f) - (y / 2.f);
	float ey = ((cy + dy) / 2.0f) - (x / 2.f);

	glm::vec3 c = glm::vec3(cx, cy, 0);
	glm::vec3 d = glm::vec3(dx, dy, 0);

	tree.push_back(rotatePoint(a, glm::vec3(0, -0.65, 0), 180.0f));
	tree.push_back(rotatePoint(c, glm::vec3(0, -0.65, 0), 180.0f));
	tree.push_back(rotatePoint(d, glm::vec3(0, -0.65, 0), 180.0f));
	tree.push_back(rotatePoint(b, glm::vec3(0, -0.65, 0), 180.0f));

	if (depth > 1) {
		std::vector<glm::vec3> leftTree = pythagorasTree(glm::vec3(cx, cy, 0), glm::vec3(ex, ey, 0), depth - 1);
		tree.insert(tree.end(), leftTree.begin(), leftTree.end());
		std::vector<glm::vec3> rightTree = pythagorasTree(glm::vec3(ex, ey, 0), glm::vec3(dx, dy, 0), depth - 1);
		tree.insert(tree.end(), rightTree.begin(), rightTree.end());
	}
	return tree;
}

void kochSnowflake(CPU_Geometry& flake, glm::vec3 a, glm::vec3 b, int depth) {
	if (depth > 0) {
		glm::vec3 firstPoint = a + 1.f / 3.f * (b - a);
		glm::vec3 lastPoint = a + 2.f / 3.f * (b - a);
		glm::vec3 middle = rotatePoint(firstPoint, lastPoint, 60.0);

		kochSnowflake(flake, a, firstPoint, depth - 1);
		kochSnowflake(flake, firstPoint, middle, depth - 1);
		kochSnowflake(flake, middle, lastPoint, depth - 1);
		kochSnowflake(flake, lastPoint, b, depth - 1);
	}
	else {
		flake.verts.push_back(a);
		flake.verts.push_back(b);
	}
}

std::vector<glm::vec3> dragonCurve(glm::vec3 a, glm::vec3 b, int depth) {
	std::vector<glm::vec3> dragon;
	if (depth == 0) {
		dragon.push_back(a);
		dragon.push_back(b);
	}
	else {
		glm::vec3 c = glm::vec3((a.x + b.x) / 2.0f + (b.y - a.y) / 2.0f, (a.y + b.y) / 2.0f - (b.x - a.x) / 2.0f, 0);
		std::vector<glm::vec3> dragon1 = dragonCurve(a, c, depth - 1);
		dragon.insert(dragon.end(), dragon1.begin(), dragon1.end());
		std::vector<glm::vec3> dragon2 = dragonCurve(b, c, depth - 1);
		dragon.insert(dragon.end(), dragon2.begin(), dragon2.end());
	}
	return dragon;
}

std::vector<glm::vec3> hilbertCurve(glm::vec3 a, float x1, float x2, float y1, float y2, int depth) {
	std::vector<glm::vec3> hilbert;
	if (depth <= 0) {
		float x = a.x + (x1 + y1) / 2.0f;
		float y = a.y + (x2 + y2) / 2.0f;
		hilbert.push_back(glm::vec3(x, y, 0));
	}
	if (depth > 0) {
		std::vector<glm::vec3> i = hilbertCurve(a, y1 / 2, y2 / 2, x1 / 2, x2 / 2, depth - 1);
		hilbert.insert(hilbert.end(), i.begin(), i.end());
		std::vector<glm::vec3> j = hilbertCurve(glm::vec3(a.x + x1 / 2, a.y + x2 / 2, 0), x1 / 2, x2 / 2, y1 / 2, y2 / 2, depth - 1);
		hilbert.insert(hilbert.end(), j.begin(), j.end());
		std::vector<glm::vec3> k = hilbertCurve(glm::vec3(a.x + x1 / 2 + y1 / 2, a.y + x2 / 2 + y2 / 2, 0), x1 / 2, x2 / 2, y1 / 2, y2 / 2, depth - 1);
		hilbert.insert(hilbert.end(), k.begin(), k.end());
		std::vector<glm::vec3> l = hilbertCurve(glm::vec3(a.x + x1 / 2 + y1, a.y + x2 / 2 + y2, 0), -y1 / 2, -y2 / 2, -x1 / 2, -x2 / 2, depth - 1);
		hilbert.insert(hilbert.end(), l.begin(), l.end());
	}
	return hilbert;
}

//--------------------------------------------------------------------------------------------------------------

//colour/render helper funtions
//--------------------------------------------------------------------------------------------------------------
void setLineColours(CPU_Geometry& colours) {
	for (int i = 0; i < colours.verts.size()/2; i++) {
		float randNum1 = (float)rand() / RAND_MAX;
		float randNum2 = (float)rand() / RAND_MAX;
		float randNum3 = (float)rand() / RAND_MAX;
		colours.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
		colours.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
	}
}

void setTriangleColours(CPU_Geometry& triangle) {
	for (int i = 0; i < triangle.verts.size() / 3; i++) {
		if (i % 3 == 0) {
			float randNum1 = (float)rand() / RAND_MAX;
			float randNum2 = (float)rand() / RAND_MAX;
			float randNum3 = (float)rand() / RAND_MAX;
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
		}
		else if (i % 3 == 1) {
			float randNum1 = (float)rand() / RAND_MAX;
			float randNum2 = (float)rand() / RAND_MAX;
			float randNum3 = (float)rand() / RAND_MAX;
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
		}
		else {
			float randNum1 = (float)rand() / RAND_MAX;
			float randNum2 = (float)rand() / RAND_MAX;
			float randNum3 = (float)rand() / RAND_MAX;
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
			triangle.cols.push_back(glm::vec3(randNum1, randNum2, randNum3));
		}
	}
}

void setTreeColours(CPU_Geometry& leaves) {
	for (int i = 0; i < leaves.verts.size() - 4; i++) {
		float randNum1 = (float)rand() / RAND_MAX;
		if (randNum1 < 0.5) randNum1 += 0.5;
		//random shades of green
		leaves.cols.push_back(glm::vec3(0.f, randNum1, 0.f));
	}
}

void setDragonColours(CPU_Geometry& dragon, int depth) {
	for (int i = 0; i < dragon.verts.size(); i++) {
		if (depth % 4 == 0) {
			dragon.cols.push_back(glm::vec3(1.f, 0.f, 0.f));
		}
		else if (depth % 4 == 1) {
			dragon.cols.push_back(glm::vec3(1.f, 1.f, 0.f));
		}
		else if (depth % 4 == 2) {
			dragon.cols.push_back(glm::vec3(0.f, 0.f, 1.f));
		}
		else {
			dragon.cols.push_back(glm::vec3(0.f, 1.f, 0.f));
		}
	}
}

void clearSceneNoCols(CPU_Geometry& scene) {
	scene.verts.clear();
}

//non system dependant sleep function
void sleep_ms(unsigned int milliseconds) {
	std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
}

//--------------------------------------------------------------------------------------------------------------

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();
	Window window(800, 800, "CPSC 453"); // can set callbacks at construction if desired

	//GLDebug::enable();

	// SHADERS
	ShaderProgram shader("shaders/test.vert", "shaders/test.frag");

	// CALLBACKS
	window.setCallbacks(std::make_shared<MyCallbacks>(shader)); // can also update callbacks to new ones

	// GEOMETRY
	CPU_Geometry cpuGeomTri;
	GPU_Geometry gpuGeomTri;

	CPU_Geometry cpuGeomTree;
	GPU_Geometry gpuGeomTree;

	CPU_Geometry cpuGeomFlake1;
	GPU_Geometry gpuGeomFlake1;
	CPU_Geometry cpuGeomFlake2;
	GPU_Geometry gpuGeomFlake2;
	CPU_Geometry cpuGeomFlake3;
	GPU_Geometry gpuGeomFlake3;

	CPU_Geometry cpuGeomDragon;
	GPU_Geometry gpuGeomDragon;

	CPU_Geometry cpuGeomHilbert;
	GPU_Geometry gpuGeomHilbert;

	//individual depth variables
	int triDepth = 0, treeDepth = 1, flakeDepth = 0, dragonDepth = 0, hilbertDepth = 1, type = 0;

	//sierpinsky
	cpuGeomTri.verts = sierpinskyTriangle(glm::vec3(-0.75f, -0.75f, 0.f), glm::vec3(0.75f, -0.75f, 0.f), glm::vec3(0.f, 0.55, 0.f), triDepth);
	setTriangleColours(cpuGeomTri);
	gpuGeomTri.setVerts(cpuGeomTri.verts);
	gpuGeomTri.setCols(cpuGeomTri.cols);

	//pythagoras tree
	cpuGeomTree.verts = pythagorasTree(glm::vec3(-0.15f, -0.75f, 0.f), glm::vec3(0.15f, -0.75f, 0.f), treeDepth);
	//make first square brown
	cpuGeomTree.cols.push_back(glm::vec3(0.5555f, 0.2773f, 0.05859f));
	cpuGeomTree.cols.push_back(glm::vec3(0.5555f, 0.2773f, 0.05859f));
	cpuGeomTree.cols.push_back(glm::vec3(0.5555f, 0.2773f, 0.05859f));
	cpuGeomTree.cols.push_back(glm::vec3(0.5555f, 0.2773f, 0.05859f));
	setTreeColours(cpuGeomTree);
	gpuGeomTree.setVerts(cpuGeomTree.verts);
	gpuGeomTree.setCols(cpuGeomTree.cols);

	//kock snowflake
	//glm::vec3(-0.25f * 3, -0.25f * 3, 0.f), glm::vec3(0.25f * 3, -0.25f * 3, 0.f), glm::vec3(0.f, -0.75 + sqrt(1.5), 0.f)
	kochSnowflake(cpuGeomFlake1, glm::vec3(-0.75f, -0.55f, 0.f), glm::vec3(0.75f, -0.55f, 0.f), flakeDepth);
	kochSnowflake(cpuGeomFlake2, glm::vec3(0.75f, -0.55f, 0.f), glm::vec3(0.f, 0.75, 0.f), flakeDepth);
	kochSnowflake(cpuGeomFlake3, glm::vec3(0.f, 0.75, 0.f), glm::vec3(-0.75f, -0.55f, 0.f), flakeDepth);
	setLineColours(cpuGeomFlake1);
	gpuGeomFlake1.setVerts(cpuGeomFlake1.verts);
	gpuGeomFlake1.setCols(cpuGeomFlake1.cols);
	setLineColours(cpuGeomFlake2);
	gpuGeomFlake2.setVerts(cpuGeomFlake2.verts);
	gpuGeomFlake2.setCols(cpuGeomFlake2.cols);
	setLineColours(cpuGeomFlake3);
	gpuGeomFlake3.setVerts(cpuGeomFlake3.verts);
	gpuGeomFlake3.setCols(cpuGeomFlake3.cols);

	//dragon curve
	cpuGeomDragon.verts = dragonCurve(glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f), dragonDepth);
	gpuGeomDragon.setVerts(cpuGeomDragon.verts);
	setDragonColours(cpuGeomDragon, dragonDepth);
	gpuGeomDragon.setCols(cpuGeomDragon.cols);

	//hilbert curve
	//variables to scale size and direction of curve
	float x1 = 0.f, x2 = 1.6f, y1 = 1.6f, y2 = 0.f;
	cpuGeomHilbert.verts = hilbertCurve(glm::vec3(-0.8f, -0.8f, 0.f), x1, x2, y1, y2, hilbertDepth);
	gpuGeomHilbert.setVerts(cpuGeomHilbert.verts);
	setDragonColours(cpuGeomHilbert, hilbertDepth);
	gpuGeomHilbert.setCols(cpuGeomHilbert.cols);

	// RENDER LOOP
	while (!window.shouldClose()) {
		glfwPollEvents();

		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_1) == GLFW_PRESS) { type = 0; }
		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_2) == GLFW_PRESS) { type = 1; }
		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_3) == GLFW_PRESS) { type = 2; }
		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_4) == GLFW_PRESS) { type = 3; }
		if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_5) == GLFW_PRESS) { type = 4; }

		shader.use();
		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//sierpinsky
		if (type == 0) {
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomTri);
				triDepth--;
				sleep_ms(100);
				if (triDepth < 0) triDepth = 0;
				cpuGeomTri.verts = sierpinskyTriangle(glm::vec3(-0.75f, -0.75f, 0.f), glm::vec3(0.75f, -0.75f, 0.f), glm::vec3(0.f, 0.55, 0.f), triDepth);
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomTri);
				triDepth++;
				sleep_ms(100);
				if (triDepth > 8) triDepth = 8;
				cpuGeomTri.verts = sierpinskyTriangle(glm::vec3(-0.75f, -0.75f, 0.f), glm::vec3(0.75f, -0.75f, 0.f), glm::vec3(0.f, 0.55, 0.f), triDepth);
			}
			setTriangleColours(cpuGeomTri);
			gpuGeomTri.setVerts(cpuGeomTri.verts);
			gpuGeomTri.setCols(cpuGeomTri.cols);
		}

		//pythagoras tree
		if (type == 1) {
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomTree);
				treeDepth--;
				sleep_ms(100);
				if (treeDepth < 1) treeDepth = 1;
				cpuGeomTree.verts = cpuGeomTree.verts = pythagorasTree(glm::vec3(-0.15f, -0.75f, 0.f), glm::vec3(0.15f, -0.75f, 0.f), treeDepth);
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomTree);
				treeDepth++;
				sleep_ms(100);
				if (treeDepth > 14) treeDepth = 14;
				cpuGeomTree.verts = cpuGeomTree.verts = pythagorasTree(glm::vec3(-0.15f, -0.75f, 0.f), glm::vec3(0.15f, -0.75f, 0.f), treeDepth);
			}
			setTreeColours(cpuGeomTree);
			gpuGeomTree.setVerts(cpuGeomTree.verts);
			gpuGeomTree.setCols(cpuGeomTree.cols);
		}

		//koch snowflake
		if (type == 2) {
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomFlake1); clearSceneNoCols(cpuGeomFlake2); clearSceneNoCols(cpuGeomFlake3);
				flakeDepth--;
				sleep_ms(100);
				if (flakeDepth < 0) flakeDepth = 0;
				kochSnowflake(cpuGeomFlake1, glm::vec3(-0.75f, -0.55f, 0.f), glm::vec3(0.75f, -0.55f, 0.f), flakeDepth);
				kochSnowflake(cpuGeomFlake2, glm::vec3(0.75f, -0.55f, 0.f), glm::vec3(0.f, 0.75, 0.f), flakeDepth);
				kochSnowflake(cpuGeomFlake3, glm::vec3(0.f, 0.75, 0.f), glm::vec3(-0.75f, -0.55f, 0.f), flakeDepth);
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomFlake1); clearSceneNoCols(cpuGeomFlake2); clearSceneNoCols(cpuGeomFlake3);
				flakeDepth++;
				sleep_ms(100);
				if (flakeDepth > 7) flakeDepth = 7;
				kochSnowflake(cpuGeomFlake1, glm::vec3(-0.75f, -0.55f, 0.f), glm::vec3(0.75f, -0.55f, 0.f), flakeDepth);
				kochSnowflake(cpuGeomFlake2, glm::vec3(0.75f, -0.55f, 0.f), glm::vec3(0.f, 0.75, 0.f), flakeDepth);
				kochSnowflake(cpuGeomFlake3, glm::vec3(0.f, 0.75, 0.f), glm::vec3(-0.75f, -0.55f, 0.f), flakeDepth);
			}
			setLineColours(cpuGeomFlake1);
			gpuGeomFlake1.setVerts(cpuGeomFlake1.verts);
			gpuGeomFlake1.setCols(cpuGeomFlake1.cols);
			gpuGeomFlake1.bind();
			glDrawArrays(GL_LINES, 0, cpuGeomFlake1.verts.size());

			setLineColours(cpuGeomFlake2);
			gpuGeomFlake2.setVerts(cpuGeomFlake2.verts);
			gpuGeomFlake2.setCols(cpuGeomFlake2.cols);
			gpuGeomFlake2.bind();
			glDrawArrays(GL_LINES, 0, cpuGeomFlake2.verts.size());

			setLineColours(cpuGeomFlake3);
			gpuGeomFlake3.setVerts(cpuGeomFlake3.verts);
			gpuGeomFlake3.setCols(cpuGeomFlake3.cols);
			gpuGeomFlake3.bind();
			glDrawArrays(GL_LINES, 0, cpuGeomFlake3.verts.size());
		}

		//dragon curve
		if (type == 3) {
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomDragon);
				dragonDepth--;
				sleep_ms(100);
				if (dragonDepth < 0) dragonDepth = 0;
				cpuGeomDragon.verts = dragonCurve(glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f), dragonDepth);
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomDragon);
				dragonDepth++;
				sleep_ms(100);
				if (dragonDepth > 16) dragonDepth = 16;
				cpuGeomDragon.verts = dragonCurve(glm::vec3(-0.5f, 0.f, 0.f), glm::vec3(0.5f, 0.f, 0.f), dragonDepth);
			}
			gpuGeomDragon.setVerts(cpuGeomDragon.verts);
			setDragonColours(cpuGeomDragon, dragonDepth);
			gpuGeomDragon.setCols(cpuGeomDragon.cols);
		}

		//hilbert curve
		if (type == 4) {
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_LEFT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomHilbert);
				hilbertDepth--;
				sleep_ms(100);
				if (hilbertDepth < 0) hilbertDepth = 0;
				cpuGeomHilbert.verts = hilbertCurve(glm::vec3(-0.8f, -0.8f, 0.f), x1, x2, y1, y2, hilbertDepth);
				gpuGeomHilbert.setVerts(cpuGeomHilbert.verts);
			}
			if (glfwGetKey(window.getGLFWwindow(), GLFW_KEY_RIGHT) == GLFW_PRESS) {
				clearSceneNoCols(cpuGeomHilbert);
				hilbertDepth++;
				sleep_ms(100);
				if (hilbertDepth > 10) hilbertDepth = 10;
				cpuGeomHilbert.verts = hilbertCurve(glm::vec3(-0.8f, -0.8f, 0.f), x1, x2, y1, y2, hilbertDepth);
				gpuGeomHilbert.setVerts(cpuGeomHilbert.verts);
			}
			setDragonColours(cpuGeomHilbert, hilbertDepth);
			gpuGeomHilbert.setCols(cpuGeomHilbert.cols);
		}

		if (type == 0) gpuGeomTri.bind();
		if (type == 1) gpuGeomTree.bind();
		if (type == 3) gpuGeomDragon.bind();
		if (type == 4) gpuGeomHilbert.bind();

		if (type == 0) glDrawArrays(GL_TRIANGLES, 0, cpuGeomTri.verts.size());
		if (type == 1) for (int i = 0; i < cpuGeomTree.verts.size(); i += 4) glDrawArrays(GL_TRIANGLE_FAN, i, 4);
		if (type == 3) glDrawArrays(GL_LINES, 0, cpuGeomDragon.verts.size());
		if (type == 4) for (int i = 0; i < cpuGeomHilbert.verts.size()-1; i ++) glDrawArrays(GL_LINES, i, 2);
		//std::cout << glGetString(GL_VERSION) << std::endl;

		glDisable(GL_FRAMEBUFFER_SRGB); // disable sRGB for things like imgui

		window.swapBuffers();

		//resize colour arrays to save space but still preserve original colours of fractals
		cpuGeomTri.cols.resize(cpuGeomTri.verts.size());
		cpuGeomTree.cols.resize(cpuGeomTree.verts.size());
		cpuGeomFlake1.cols.resize(cpuGeomFlake1.verts.size());
		cpuGeomFlake2.cols.resize(cpuGeomFlake2.verts.size());
		cpuGeomFlake3.cols.resize(cpuGeomFlake3.verts.size());
		cpuGeomDragon.cols.resize(cpuGeomDragon.verts.size());
		cpuGeomHilbert.cols.resize(cpuGeomHilbert.verts.size());
	}

	glfwTerminate();
	return 0;
}

/*
CPSC 453 Assignment 5
Carter Boucher
30116690
Dec 8 , 2024
*/

#define _USE_MATH_DEFINES
#include <cmath>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <string>

#include <glm/gtx/vector_query.hpp>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "imagebuffer.h"
#include "RayTrace.h"
#include "Scene.h"
#include "Lighting.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include <thread>
#include <mutex>

std::mutex rays_mutex;

int hasIntersection(Scene const &scene, Ray ray, int skipID){
	for (auto &shape : scene.shapesInScene) {
		Intersection tmp = shape->getIntersection(ray);
		if(
			shape->id != skipID
			&& tmp.numberOfIntersections!=0
			&& glm::distance(tmp.point, ray.origin) > 0.00001
			&& glm::distance(tmp.point, ray.origin) < glm::distance(ray.origin, scene.lightPosition) - 0.01
		){
			return tmp.id;
		}
	}
	return -1;
}

Intersection getClosestIntersection(Scene const &scene, Ray ray, int skipID){ //get the nearest
	Intersection closestIntersection;
	float min = std::numeric_limits<float>::max();
	for(auto &shape : scene.shapesInScene) {
		if(skipID == shape->id) {
			// Sometimes you need to skip certain shapes. Useful to
			// avoid self-intersection. ;)
			continue;
		}
		Intersection p = shape->getIntersection(ray);
		float distance = glm::distance(p.point, ray.origin);
		if(p.numberOfIntersections !=0 && distance < min){
			min = distance;
			closestIntersection = p;
		}
	}
	return closestIntersection;
}

glm::vec3 raytraceSingleRay(Scene const& scene, Ray const& ray, int level, int source_id) {
	if (level == 0) return glm::vec3(0, 0, 0);

	Intersection closestIntersection = getClosestIntersection(scene, ray, source_id);
	if (closestIntersection.numberOfIntersections == 0) {
		return glm::vec3(0, 0, 0);
	}

	glm::vec3 color = glm::vec3(0, 0, 0);
	glm::vec3 lightDirection = glm::normalize(scene.lightPosition - closestIntersection.point);
	glm::vec3 normal = closestIntersection.normal;
	glm::vec3 viewDirection = glm::normalize(ray.origin - closestIntersection.point);
	glm::vec3 reflectionDirection = glm::normalize(glm::reflect(-ray.direction, normal));

	float diffuse = glm::max(glm::dot(normal, lightDirection), 0.0f);
	float specular = glm::pow(glm::max(glm::dot(reflectionDirection, viewDirection), 0.0f), 32);

	glm::vec3 ambient = scene.ambientFactor * closestIntersection.material.ambient;
	glm::vec3 diffuseColor = closestIntersection.material.diffuse * diffuse;
	glm::vec3 specularColor = closestIntersection.material.specular * specular;

	// Soft shadows
	const int numShadowRays = 64; // Number of rays for area light sampling
	float shadowFactor = 0.0f;
	for (int i = 0; i < numShadowRays; ++i) {
		// Randomize shadow ray direction to simulate area light
		glm::vec3 jitteredLightPos = scene.lightPosition + glm::vec3(
			(float(rand()) / RAND_MAX - 0.5f) / 20,
			(float(rand()) / RAND_MAX - 0.5f) / 20,
			(float(rand()) / RAND_MAX - 0.5f) / 20
		);

		glm::vec3 jitteredLightDir = glm::normalize(jitteredLightPos - closestIntersection.point);
		Ray shadowRay = Ray(closestIntersection.point + normal * 1e-3f, jitteredLightDir);

		int shadowID = hasIntersection(scene, shadowRay, closestIntersection.id);
		if (shadowID != -1) {
			shadowFactor += 1.0f; // Increment shadow factor for blocked rays
		}
	}

	shadowFactor /= numShadowRays; // Normalize by the number of rays
	float attenuation = 1.0f - shadowFactor;

	// Apply lighting model
	color = ambient + attenuation * (diffuseColor + specularColor);

	// Reflection
	if (glm::length(closestIntersection.material.reflectionStrength) > 0) {
		Ray reflectionRay = Ray(closestIntersection.point + normal * 1e-4f, -reflectionDirection);
		glm::vec3 reflectionColor = raytraceSingleRay(scene, reflectionRay, level - 1, closestIntersection.id);
		color = color + closestIntersection.material.reflectionStrength * reflectionColor;
	}

	return color;
}



struct RayAndPixel {
	Ray ray;
	int x;
	int y;
};


//learned from CPSC 457 and adapted my assignment code to this
void task1(ImageBuffer& image, glm::vec3 viewPoint, std::vector<RayAndPixel>& rays, Scene const& scene, int startRow, int endRow) {
	std::vector<RayAndPixel> local_rays;

	for (int i = 0; i < image.Width(); i++) {
		for (int j = startRow; j < endRow; j++) {
			float x = i - image.Width() / 2;
			float y = j - image.Height() / 2;
			float z = -image.Width();

			glm::vec3 pinhole = glm::vec3(x, y, z);
			glm::vec3 direction = glm::normalize(pinhole - viewPoint);

			local_rays.push_back({ Ray(viewPoint, direction), i, j });
		}
	}

	//lock shared vector
	std::lock_guard<std::mutex> lock(rays_mutex);
	rays.insert(rays.end(), local_rays.begin(), local_rays.end());
}

std::vector<RayAndPixel> getRaysForViewpoint(Scene const& scene, ImageBuffer& image, glm::vec3 viewPoint) {
	//const int numThreads = std::thread::hardware_concurrency();
	const int numThreads = 16;
	std::vector<std::thread> threads;
	std::vector<RayAndPixel> rays;

	int rowsPerThread = image.Height() / numThreads;
	int remainder = image.Height() % numThreads;

	int startRow = 0;
	for (int t = 0; t < numThreads; t++) {
		int endRow = startRow + rowsPerThread + (t < remainder ? 1 : 0); // Handle uneven division
		threads.push_back(std::thread(task1, std::ref(image), viewPoint, std::ref(rays), std::ref(scene), startRow, endRow));
		startRow = endRow;
	}

	for (auto& t : threads) {
		t.join();
	}

	return rays;
}

void task2(std::vector<RayAndPixel>& rays, Scene const& scene, ImageBuffer& image, int start, int end) {
	for (int i = start; i < end; i++) {
		glm::vec3 color = raytraceSingleRay(scene, rays[i].ray, 6, -1);
		image.SetPixel(rays[i].x, rays[i].y, color);
	}
}

void raytraceImage(Scene const &scene, ImageBuffer &image, glm::vec3 viewPoint) {
	// Reset the image to the current size of the screen.
	image.Initialize();

	// Get the set of rays to cast for this given image / viewpoint
	std::vector<RayAndPixel> rays = getRaysForViewpoint(scene, image, viewPoint);

	//speed up loading of image omg so awesome
	//const int n_threads = std::thread::hardware_concurrency();
	const int n_threads = 16; //just going to set to 16 for simplicity
	std::vector<std::thread> threads;

	for (int i = 0; i < n_threads; i++) {
		int start = i * rays.size() / n_threads;
		int end = (i + 1) * rays.size() / n_threads;
		threads.push_back(std::thread(task2, std::ref(rays), std::ref(scene), std::ref(image), start, end));
	}

	for (int i = 0; i < n_threads; i++) {
		threads[i].join();
	}
}

// EXAMPLE CALLBACKS
class Assignment5 : public CallbackInterface {

public:
	Assignment5() {
		viewPoint = glm::vec3(0, 0, 0);
		scene = initScene1();
		raytraceImage(scene, outputImage, viewPoint);
	}

	virtual void keyCallback(int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_Q && action == GLFW_PRESS) {
			shouldQuit = true;
		}

		if (key == GLFW_KEY_1 && action == GLFW_PRESS) {
			scene = initScene1();
			raytraceImage(scene, outputImage, viewPoint);
		}

		if (key == GLFW_KEY_2 && action == GLFW_PRESS) {
			scene = initScene2();
			raytraceImage(scene, outputImage, viewPoint);
		}

		//p will export image to png
		if (key == GLFW_KEY_P && action == GLFW_PRESS) {
			outputScene = true;
		}
	}

	bool shouldQuit = false;

	ImageBuffer outputImage;
	Scene scene;
	glm::vec3 viewPoint;
	bool outputScene = false;

};
// END EXAMPLES

int main() {
	Log::debug("Starting main");

	// WINDOW
	glfwInit();

	// Change your image/screensize here.
	int width = 800;
	int height = 800;
	Window window(width, height, "CPSC 453");

	GLDebug::enable();

	// CALLBACKS
	std::shared_ptr<Assignment5> a5 = std::make_shared<Assignment5>(); // can also update callbacks to new ones
	window.setCallbacks(a5); // can also update callbacks to new ones

	// RENDER LOOP
	while (!window.shouldClose() && !a5->shouldQuit) {
		if (a5->outputScene) {
			a5->outputImage.SaveToFile("output.png");
			a5->outputScene = false;
		}
		glfwPollEvents();

		glEnable(GL_FRAMEBUFFER_SRGB);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		a5->outputImage.Render();

		window.swapBuffers();
	}

	// Save image to file:
	// outpuImage.SaveToFile("foo.png")

	glfwTerminate();
	return 0;
}

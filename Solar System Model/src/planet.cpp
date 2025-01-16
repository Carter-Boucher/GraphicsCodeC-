#include "planet.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>
#include <iostream>
#include <glm/ext/quaternion_trigonometric.hpp>
#include <string>

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#define M_PI 3.14159

std::vector<std::vector<glm::vec3>> generateSphereVertices(
	const float radius, const int segments, std::vector<glm::vec2>& texCoords,
	glm::vec3 center, bool inside, std::string name) {

	std::vector<glm::vec3> vertices;
	std::vector<glm::vec3> normals;

	glm::mat4 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));

	for (int i = 0; i <= segments; ++i) {
		float lat = -M_PI / 2.0f + i * M_PI / segments;
		float sinLat = sin(lat);
		float cosLat = cos(lat);

		for (int j = 0; j <= segments; ++j) {
			float lon = j * 2.0f * M_PI / segments;
			float sinLon = sin(lon);
			float cosLon = cos(lon);

			glm::vec3 vertex;
			if (inside) {
				vertex.x = radius * cosLat * cosLon;
				vertex.y = radius * cosLat * sinLon;
				vertex.z = radius * sinLat;
			}
			else {
				vertex.x = -radius * cosLat * cosLon;
				vertex.y = -radius * cosLat * sinLon;
				vertex.z = -radius * sinLat;
			}

			vertex += center;

			// Rotate the vertex
			vertex = glm::vec3(rotation * glm::vec4(vertex, 1.0f));

			// Calculate the normal vector
			glm::vec3 normal1 = vertex - center;
			glm::vec3 normal = glm::normalize(normal1);

			//if sun flip normals
			if (name == "sun") {
				normal = -normal;
			}

			vertices.push_back(vertex);
			normals.push_back(normal);
		}
	}

	std::vector<std::vector<glm::vec3>> final;
	final.push_back(vertices);
	final.push_back(normals);
	return final;
}

std::vector<std::vector<glm::vec3>> generateSphereIndices(const std::vector<glm::vec3> sphere, const int segments, std::vector<glm::vec2>& texCoords,
	const std::vector<glm::vec3> normals) {
	std::vector<glm::vec3> indices;
	std::vector<glm::vec3> newNormals;

	//push back triangles from the sphere points
	for (int i = 0; i < segments; ++i) {
		for (int j = 0; j < segments; ++j) {
			int first = (i * (segments + 1)) + j;
			int second = first + segments + 1;

			indices.push_back(sphere[first]);
			newNormals.push_back(normals[first]);
			indices.push_back(sphere[second]);
			newNormals.push_back(normals[second]);
			indices.push_back(sphere[first + 1]);
			newNormals.push_back(normals[first + 1]);

			indices.push_back(sphere[second]);
			newNormals.push_back(normals[second]);
			indices.push_back(sphere[second + 1]);
			newNormals.push_back(normals[second + 1]);
			indices.push_back(sphere[first + 1]);
			newNormals.push_back(normals[first + 1]);

			// Texture coordinates
			float u1 = (float)j / segments;
			float v1 = (float)i / segments;
			float u2 = (float)(j + 1) / segments;
			float v2 = (float)(i + 1) / segments;

			// First triangle tex coords
			texCoords.push_back(glm::vec2(u1, v1));
			texCoords.push_back(glm::vec2(u1, v2));
			texCoords.push_back(glm::vec2(u2, v1));

			// Second triangle tex coords
			texCoords.push_back(glm::vec2(u1, v2));
			texCoords.push_back(glm::vec2(u2, v2));
			texCoords.push_back(glm::vec2(u2, v1));
		}
	}
	std::vector<std::vector<glm::vec3>> end;
	end.push_back(indices);
	end.push_back(newNormals);
	return end;
}

void planet::generateGeometry(glm::vec3 center, float radius, bool back, std::string name) {
	this->m_centre = center;
	int segments = 35;

	std::vector<glm::vec2> texCoords;
	std::vector<std::vector<glm::vec3>> final = generateSphereVertices(radius, segments, texCoords, center, back, name);
	std::vector<glm::vec3> _coords = final[0];
	std::vector<glm::vec3> _normals = final[1];


	std::vector<std::vector<glm::vec3>> finalPoints = generateSphereIndices(_coords, segments, texCoords, _normals);
	m_cpu_geom.verts = finalPoints[0];
	m_cpu_geom.normals = finalPoints[1];

	m_gpu_geom.bind();
	m_gpu_geom.setVerts(m_cpu_geom.verts);
	m_gpu_geom.setNormals(m_cpu_geom.normals);
	m_gpu_geom.setTexCoords(texCoords);

	m_size = m_cpu_geom.verts.size();
}

void planet::rotatePlanet(float speed) {
	float angle = angleRotate * speed;
	if (parent != nullptr) {
		angle = angleRotate;
	}

	glm::vec3 centre = getCentre();
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::normalize(axis));

	// Rotate vertices
	for (auto& vertex : m_cpu_geom.verts) {
		glm::vec3 translatedVertex = vertex - centre;
		glm::vec4 rotatedVertex = rotationMatrix * glm::vec4(translatedVertex, 1.0f);
		vertex = glm::vec3(rotatedVertex) + centre;
	}

	// Rotate normals
	for (auto& normal : m_cpu_geom.normals) {
		glm::vec4 rotatedNormal = rotationMatrix * glm::vec4(normal, 0.0f); // No translation for normals
		normal = glm::normalize(glm::vec3(rotatedNormal));
	}

	m_gpu_geom.setVerts(m_cpu_geom.verts);
	m_gpu_geom.setNormals(m_cpu_geom.normals);
}


void planet::orbitPlanet(float speed, glm::vec3 icenter) {
	float angle = angleOrbit * speed;
	orbitPlaneNormal = glm::normalize(orbitPlaneNormal);

	if (parent != nullptr) {
		icenter = parent->getCentre();
	}

	glm::vec3 initialPosition = getCentre() - icenter;

	// Rotation quaternion
	glm::quat rotationQuat = glm::angleAxis(glm::radians(angle), orbitPlaneNormal);

	// Rotate the center
	glm::vec3 rotatedPosition = rotationQuat * initialPosition;
	setCentre(rotatedPosition + icenter);

	// Rotate vertices
	for (auto& vertex : m_cpu_geom.verts) {
		vertex = rotationQuat * (vertex - icenter) + icenter;
	}

	// Rotate normals (same rotation as vertices)
	for (auto& normal : m_cpu_geom.normals) {
		normal = glm::normalize(rotationQuat * normal);
	}

	m_gpu_geom.setVerts(m_cpu_geom.verts);
	m_gpu_geom.setNormals(m_cpu_geom.normals);
}

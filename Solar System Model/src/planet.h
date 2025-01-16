#pragma once

#include "Geometry.h"
#include "Texture.h"
#include "GLDebug.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Camera.h"
#include "UnitCube.h"
#include <glm/gtx/transform.hpp>
#include <glm/gtc/random.hpp>

#include <string>


class planet {
public:
	planet(std::string _name, std::string texturePath, GLenum textureInterpolation, glm::vec3 centre, float radius, bool back, planet* _parent,
		float _angleRotate, glm::vec3 _axis, float _angleOrbit, glm::vec3 _orbitPlane) :
		texture(texturePath, textureInterpolation),
		render(true),
		size(0),
		m_centre(centre),
		parent(_parent),
		angleRotate(_angleRotate),
		axis(_axis),
		angleOrbit(_angleOrbit),
		orbitPlaneNormal(_orbitPlane),
		name(_name),
		o_centre(centre)
	{
		generateGeometry(centre, radius, back, this->name);
	}
	CPU_Geometry m_cpu_geom;    // We dont really need atm
	GPU_Geometry m_gpu_geom;
	glm::mat4 m_model;
	GLsizei m_size;
	void generateGeometry(glm::vec3 centre, float radius, bool inside, std::string name);
	//void getNormals(CPU_Geometry& geom, std::vector<glm::vec3> const& verts, std::vector<glm::vec3>& faceNormals);
	glm::vec3 getCentre() { return this->m_centre; }
	glm::vec3 getOriginalCentre() { return this->o_centre; }
	std::string getName() { return this->name; }
	void setCentre(glm::vec3 centre) { this->m_centre = centre; }
	void rotatePlanet(float speed);
	void orbitPlanet(float speed, glm::vec3 center);
	void setParent(planet* _parent) { this->parent = _parent; }

	std::string name;

	Texture texture;
	bool render;
	int size;
	float tiltAngle;
	float rotation = 45.f;

	glm::vec3 orbitPlaneNormal;
	float angleRotate;
	float angleOrbit;
	glm::vec3 axis;
	
private:
	planet* parent;
	glm::vec3 m_centre;
	glm::vec3 o_centre;
	float rotationSpeed;
	float orbitSpeed;
};

/*
CPSC 453 F24 Assignment 2
Carter Boucher
30116690
October 25, 2024
*/

#include <iostream>
#include <string>
#include <ctime> 
#include <math.h>

#include "Geometry.h"
#include "GLDebug.h"
#include "Log.h"
#include "ShaderProgram.h"
#include "Shader.h"
#include "Texture.h"
#include "Window.h"
#include "Input.h"


struct GameObject {
	GameObject(std::string texturePath, GLenum textureInterpolation, float width, float height) :
		texture(texturePath, textureInterpolation),
		position(glm::vec2(0.0f, 0.0f)),
		direction(glm::vec2(0.0f, 0.0f)),
		render(true),
		size(0),
		firstMoving(true)
	{
		//vertex coordinates
		retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		retGeom.verts.push_back(glm::vec3(-1.f, -1.f, 0.f));
		retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		retGeom.verts.push_back(glm::vec3(-1.f, 1.f, 0.f));
		retGeom.verts.push_back(glm::vec3(1.f, -1.f, 0.f));
		retGeom.verts.push_back(glm::vec3(1.f, 1.f, 0.f));

		//texture coordinates
		retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		retGeom.texCoords.push_back(glm::vec2(0.f, 0.f));
		retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		retGeom.texCoords.push_back(glm::vec2(0.f, 1.f));
		retGeom.texCoords.push_back(glm::vec2(1.f, 0.f));
		retGeom.texCoords.push_back(glm::vec2(1.f, 1.f));

		//GPU Geometry
		ggeom.setVerts(retGeom.verts);
		ggeom.setTexCoords(retGeom.texCoords);

		//Setting Initial Matrices
		SMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(width, height, 1.0f));
		RMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		TMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
	}

	glm::mat4 getTMatrix() { return TMatrix * RMatrix * SMatrix; }

	void resizeShip() {
		size++;
		SMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f + (0.02 * size), 0.1f + (0.02 * size), 1.0f));
		if (size >= 5) size = 0;
	}

	void updateShip(Input input) {
		 
		if (input.reset) {
			position = glm::vec2(0.0f, 0.0f);
			direction = glm::vec2(0.0f, 1.0f);
			SMatrix = glm::scale(glm::mat4(1.0f), glm::vec3(0.15f, 0.1f, 1.0f));
			RMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
			TMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, 0.0f));
			return;
		}

		//direction vector
		glm::vec2 directionToCursor = input.cursor - position;
		if (glm::length(directionToCursor) < 0.05f) return;

		//angle to cursor
		float angleToCursor = atan2(directionToCursor.y, directionToCursor.x) - glm::radians(90.0f);

		//Set matrix to new angle
		RMatrix = glm::rotate(glm::mat4(1.0f), angleToCursor, glm::vec3(0.0f, 0.0f, 1.0f));

		glm::vec2 newDirection = glm::normalize(directionToCursor);

		glm::vec3 translation(0.0f);
		if (input.movingForward && !input.movingBackward && !firstMoving) return;
		else if (input.movingForward && !input.movingBackward && firstMoving) {
			translation = glm::vec3(newDirection * 0.02f, 0.0f);
			firstMoving = false;
		}
		else if (!input.movingForward && input.movingBackward && !firstMoving) return;
		else if (!input.movingForward && input.movingBackward && firstMoving) {
			translation = glm::vec3(-newDirection * 0.02f, 0.0f);
			firstMoving = false;
		}
		else if (!input.movingForward && !input.movingBackward) firstMoving = true;

		//new position
		glm::vec2 newPosition = position + glm::vec2(translation.x, translation.y);

		//if in bounds
		if (newPosition.x >= -1.0f + (0.15f / 2) && newPosition.x <= 1.0f - (0.15f / 2) &&
			newPosition.y >= -1.0f + (0.1f / 2) && newPosition.y <= 1.0f - (0.1f / 2)) {
			TMatrix = glm::translate(TMatrix, translation);
			position = newPosition;
		}
	}

	void initializeDiamond() {
		render = true;
		position = glm::vec2(-1.0f + rand() / (RAND_MAX / (2.0f)), -1.0f + rand() / (RAND_MAX / (2.0f)));
		direction = glm::vec2(-1.0f + rand() / (RAND_MAX / (2.0f)), -1.0f + rand() / (RAND_MAX / (2.0f)));
		TMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
		direction = glm::normalize(direction);
	}

	void updateDiamond(Input input) {
		position += direction * 0.003f;
		if (position.x <= -1.0f || position.x >= 1.0f) direction.x = -direction.x;
		if (position.y <= -1.0f || position.y >= 1.0f) direction.y = -direction.y;
		TMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(position, 0.0f));
	}

	CPU_Geometry retGeom;
	GPU_Geometry ggeom;
	Texture texture;

	bool render;
	int size;
	bool firstMoving;

	glm::vec2 position;
	glm::vec2 direction;

	glm::mat4 SMatrix;
	glm::mat4 RMatrix;
	glm::mat4 TMatrix;
};

//Helper functions
bool collisionCheck(const glm::vec2& shipPosition, const glm::vec2& diamondPosition) {
	if (glm::length(shipPosition - diamondPosition) <= 0.080f) return true;
	return false;
}

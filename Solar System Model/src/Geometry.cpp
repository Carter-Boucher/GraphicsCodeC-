#include "Geometry.h"

#include <utility>


GPU_Geometry::GPU_Geometry()
	: vao()
	, vertBuffer(0, 3, GL_FLOAT)
	, normalsBuffer(1, 3, GL_FLOAT)
	, texCoordBuffer(2, 2, GL_FLOAT)
{}


void GPU_Geometry::setVerts(const std::vector<glm::vec3>& verts) {
	vertBuffer.uploadData(sizeof(glm::vec3) * verts.size(), verts.data(), GL_STATIC_DRAW);
}


void GPU_Geometry::setNormals(const std::vector<glm::vec3>& norms) {
	normalsBuffer.uploadData(sizeof(glm::vec3) * norms.size(), norms.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setTexCoords(const std::vector<glm::vec2>& texCoords) {
	texCoordBuffer.uploadData(sizeof(glm::vec2) * texCoords.size(), texCoords.data(), GL_STATIC_DRAW);
}

void GPU_Geometry::setup(int vertLocation, int normalLocation, int texCoordLocation) {
	vao.bind();

	// Enable vertex attribute pointers
	glEnableVertexAttribArray(vertLocation);
	glEnableVertexAttribArray(normalLocation);
	glEnableVertexAttribArray(texCoordLocation);

	// Configure vertex buffer
	vertBuffer.bind();
	glVertexAttribPointer(vertLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Configure normals buffer
	normalsBuffer.bind();
	glVertexAttribPointer(normalLocation, 3, GL_FLOAT, GL_FALSE, 0, nullptr);

	// Configure texture coordinate buffer
	texCoordBuffer.bind();
	glVertexAttribPointer(texCoordLocation, 2, GL_FLOAT, GL_FALSE, 0, nullptr);
}

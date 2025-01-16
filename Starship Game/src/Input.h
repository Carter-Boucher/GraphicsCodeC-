/*
CPSC 453 F24 Assignment 2
Carter Boucher
30116690
October 25, 2024
*/

struct Input {
	glm::vec2 cursor = glm::vec2(0.0f, 1.0f);
	bool reset = false;
	bool movingForward = false;
	bool movingBackward = false;
};

void resetInput(Input* input) {
	input->reset = true;
	input->movingBackward = false;
	input->movingForward = false;
	input->cursor = glm::vec2(0.0f, 1.0f);
}

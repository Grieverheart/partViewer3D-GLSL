#include "../include/gui.h"

Cgui::Cgui(void){
	m_num_buttons = 0;
}

Cgui::~Cgui(void){
	delete sh_overlay;
}

void Cgui::Init(unsigned int windowWidth, unsigned windowHeight){
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	sh_overlay = new Shader("shaders/overlay.vert", "shaders/overlay.frag");
}

void Cgui::newButton(unsigned int width, unsigned int height, glm::vec2 position, glm::vec4 color){
	buttons.push_back(CButton(float(width) / m_windowWidth, float(height) / m_windowHeight, position, color));
	m_num_buttons++;
	std::cout << "Button " << m_num_buttons << " created." << std::endl;
}

void Cgui::Draw(void){
	sh_overlay->bind();
	{
		for(unsigned int i = 0; i < m_num_buttons; i++){
			buttons[i].Draw();
		}
	}
	sh_overlay->unbind();
}
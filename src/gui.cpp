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
	m_plusTex = new Texture(GL_TEXTURE_2D, "textures/plus.bmp");
	m_minusTex = new Texture(GL_TEXTURE_2D, "textures/minus.bmp");
	if(!m_plusTex->Load()) std::cout << "Couldn't load plus texture!" << std::endl;
	if(!m_minusTex->Load()) std::cout << "Couldn't load minus texture!" << std::endl;
	samplerLocation = glGetUniformLocation(sh_overlay->id(), "g_Sampler");
	if(samplerLocation == -1) std::cout << "Couldn't bind gui uniforms" << std::endl;
	else{
		sh_overlay->bind();
		{
			glUniform1i(samplerLocation, 0);
		}
		sh_overlay->unbind();
	}
}

void Cgui::newButton(unsigned int width, unsigned int height, glm::ivec2 position, std::string type){
	float ndc_width = float(width) / m_windowWidth;
	float ndc_height = float(height) / m_windowHeight;
	glm::vec2 ndc_position = glm::vec2(2.0 * float(position.x) / m_windowWidth - 1.0, 1.0 - 2.0 * float(position.y) / m_windowHeight);
	buttons.push_back(CButton(ndc_width, ndc_height, ndc_position, type));
	m_num_buttons++;
	std::cout << "Button " << m_num_buttons << " created." << std::endl;
}

unsigned char Cgui::buttonClicked(int x, int y){
	for(unsigned int i = 0; i < m_num_buttons; i++){
		float ndc_x = 2.0 * float(x) / m_windowWidth - 1.0;
		float ndc_y = 1.0 - 2.0 * float(y) / m_windowHeight;
		if(buttons[i].isClicked(ndc_x, ndc_y)) return (unsigned char)(i + 1);
	}
	return 0;
}

void Cgui::Resize(unsigned int windowWidth, unsigned windowHeight){
	float width_fact = float(windowWidth) / m_windowWidth;
	float height_fact = float(windowHeight) / m_windowHeight;
	m_windowWidth = windowWidth;
	m_windowHeight = windowHeight;
	for(unsigned int i = 0; i < m_num_buttons; i++){
		buttons[i].Resize(width_fact, height_fact);
	}
}

void Cgui::Draw(void){
	sh_overlay->bind();
	{
		for(unsigned int i = 0; i < m_num_buttons; i++){
			if(buttons[i].m_type == "plus") m_plusTex->Bind(GL_TEXTURE0);
			else if(buttons[i].m_type == "minus") m_minusTex->Bind(GL_TEXTURE0);
			buttons[i].Draw();
		}
	}
	sh_overlay->unbind();
}
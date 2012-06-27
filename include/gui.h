#ifndef __GUI_H
#define __GUI_H
#include <vector>
#include "main.h"
#include "shader.h"
#include "button.h"

class Cgui{
public:
	Cgui(void);
	~Cgui(void);
	void Init(unsigned int windowWidth, unsigned windowHeight);
	void Draw(void);
	void newButton(unsigned int width, unsigned int height, glm::vec2 position, glm::vec4 color);
	
private:
	Shader *sh_overlay;
	std::vector<CButton> buttons;
	unsigned int m_windowWidth;
	unsigned int m_windowHeight;
	unsigned int m_num_buttons;
};

#endif
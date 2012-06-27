#ifndef __GUI_H
#define __GUI_H
#include <vector>
#include <string>
#include "main.h"
#include "shader.h"
#include "button.h"
#include "texture.h"

class Cgui{
public:
	Cgui(void);
	~Cgui(void);
	void Init(unsigned int windowWidth, unsigned windowHeight);
	void Draw(void);
	void newButton(unsigned int width, unsigned int height, glm::ivec2 position, std::string type);
	unsigned char buttonClicked(int x, int y);
	void Resize(unsigned int windowWidth, unsigned windowHeight);
	
private:
	Shader *sh_overlay;
	std::vector<CButton> buttons;
	unsigned int m_windowWidth;
	unsigned int m_windowHeight;
	unsigned int m_num_buttons;
	Texture *m_plusTex;
	Texture *m_minusTex;
	
	int samplerLocation;
};

#endif
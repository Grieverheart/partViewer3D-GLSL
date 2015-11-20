#ifndef __GBUFF_H
#define __GBUFF_H

class CGBuffer{
public:
	enum GBUFF_TEXTURE_TYPE{
		GBUFF_TEXTURE_TYPE_DIFFUSE,
		GBUFF_TEXTURE_TYPE_NORMAL,
		GBUFF_TEXTURE_TYPE_DEPTH,
		GBUFF_NUM_TEXTURES
	};
	CGBuffer(void);
	~CGBuffer(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void Bind(void)const;
	void UnBind(void)const;
    void BindTexture(GBUFF_TEXTURE_TYPE tex_type, int attachment_point)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight)const;
	unsigned int m_fbo;
private:
	unsigned int m_textures[GBUFF_NUM_TEXTURES];
};

#endif

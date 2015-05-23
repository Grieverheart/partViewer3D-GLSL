#ifndef __SHADOWMAP_H
#define __SHADOWMAP_H

class CShadowmap{
public:
	CShadowmap(void);
	~CShadowmap(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void Bind(void)const;
    void BindTexture(int attachment_point)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight)const;
private:
	unsigned int m_fbo;
	unsigned int m_texture;
	unsigned int m_depth_rb;
};

#endif

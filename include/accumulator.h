#ifndef __ACCUMULATOR_H
#define __ACCUMULATOR_H

class Accumulator{
public:
	Accumulator(void);
	~Accumulator(void);
	bool Init(unsigned int WindowWidth, unsigned int WindowHeight);
	void Bind(void)const;
	void UnBind(void)const;
    void BindTexture(int attachment_point)const;
	void Resize(unsigned int WindowWidth, unsigned int WindowHeight)const;
private:
	unsigned int m_fbo;
	unsigned int m_texture;
};

#endif

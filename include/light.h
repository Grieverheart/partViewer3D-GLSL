#ifndef __LIGHT_H
#define __LIGHT_H

#include <AntTweakBar.h>
#include <glm/glm.hpp>

class CLight{
public:
	CLight(void);
	CLight(glm::vec3 direction);
	~CLight(void);
	
	bool Init(unsigned int shade_id);
	void uploadDirection(glm::mat4 ViewMatrix)const;
	
	const glm::vec3& getDirection(void)const;
	void setDirection(const glm::vec3&);
	
	float getSpecInt(void)const;
	void setSpecInt(float value);
	
	float getDiffInt(void)const;
	void setDiffInt(float value);
	
	float getAmbientInt(void)const;
	void setAmbientInt(float value);
	
	float getInt(void)const;
	void setInt(float value);
	
	static void TW_CALL SetDirCallback(const void *value, void *clientData){
		static_cast<CLight*>(clientData)->setDirection(*static_cast<const glm::vec3*>(value));
	}
	
	static void TW_CALL GetDirCallback(void *value, void *clientData){
		*static_cast<glm::vec3*>(value) = static_cast<const CLight*>(clientData)->getDirection();
	}
	
	static void TW_CALL SetSpecIntCallback(const void *value, void *clientData){
		static_cast<CLight *>(clientData)->setSpecInt(*static_cast<const float *>(value));
	}
	
	static void TW_CALL GetSpecIntCallback(void *value, void *clientData){
		*static_cast<float *>(value) = static_cast<const CLight *>(clientData)->getSpecInt();
	}
	
	static void TW_CALL SetDiffIntCallback(const void *value, void *clientData){
		static_cast<CLight *>(clientData)->setDiffInt(*static_cast<const float *>(value));
	}
	
	static void TW_CALL GetDiffIntCallback(void *value, void *clientData){
		*static_cast<float *>(value) = static_cast<const CLight *>(clientData)->getDiffInt();
	}
	
	static void TW_CALL SetAmbIntCallback(const void *value, void *clientData){
		static_cast<CLight *>(clientData)->setAmbientInt(*static_cast<const float *>(value));
	}
	
	static void TW_CALL GetAmbIntCallback(void *value, void *clientData){
		*static_cast<float *>(value) = static_cast<const CLight *>(clientData)->getAmbientInt();
	}
	
	static void TW_CALL SetIntCallback(const void *value, void *clientData){
		static_cast<CLight *>(clientData)->setInt(*static_cast<const float *>(value));
	}
	
	static void TW_CALL GetIntCallback(void *value, void *clientData){
		*static_cast<float *>(value) = static_cast<const CLight *>(clientData)->getInt();
	}
	
private:
	glm::vec3 m_direction;
	int m_DirectionLocation;
	int m_SpecularIntensityLocation;
	int m_DiffuseIntensityLocation;
	int m_IntensityLocation;
	int m_AmbientIntensityLocation;
	
	float m_SpecularIntensity;
	float m_DiffuseIntensity;
	float m_Intensity;
	float m_AmbientIntensity;
};

#endif

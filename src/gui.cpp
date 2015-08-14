#include "include/gui.h"
#include "include/scene.h"
#include <AntTweakBar.h>

static void TW_CALL set_light_direction_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_light_direction(*static_cast<const glm::vec3*>(value));
}

static void TW_CALL set_light_ambient_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_light_ambient_intensity(*static_cast<const float *>(value));
}

static void TW_CALL set_light_diffuse_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_light_diffuse_intensity(*static_cast<const float *>(value));
}

static void TW_CALL set_light_specular_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_light_specular_intensity(*static_cast<const float *>(value));
}

static void TW_CALL set_light_intensity_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_light_intensity(*static_cast<const float *>(value));
}

static void TW_CALL get_light_direction_callback(void *value, void *clientData){
    *static_cast<glm::vec3*>(value) = static_cast<const Scene*>(clientData)->get_light_direction();
}

static void TW_CALL get_light_ambient_callback(void *value, void *clientData){
    *static_cast<float*>(value) = static_cast<const Scene*>(clientData)->get_light_ambient_intensity();
}

static void TW_CALL get_light_diffuse_callback(void *value, void *clientData){
    *static_cast<float*>(value) = static_cast<const Scene*>(clientData)->get_light_diffuse_intensity();
}

static void TW_CALL get_light_specular_callback(void *value, void *clientData){
    *static_cast<float*>(value) = static_cast<const Scene*>(clientData)->get_light_specular_intensity();
}

static void TW_CALL get_light_intensity_callback(void *value, void *clientData){
    *static_cast<float*>(value) = static_cast<const Scene*>(clientData)->get_light_intensity();
}

static void TW_CALL set_ssao_blur_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_ssao_blur(*static_cast<const bool*>(value));
}

static void TW_CALL get_ssao_blur_callback(void *value, void *clientData){
    *static_cast<bool*>(value) = static_cast<const Scene*>(clientData)->is_ssao_blur_enabled();
}

static void TW_CALL set_ssao_radius_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_ssao_radius(*static_cast<const float*>(value));
}

static void TW_CALL get_ssao_radius_callback(void *value, void *clientData){
    *static_cast<float*>(value) = static_cast<const Scene*>(clientData)->get_ssao_radius();
}

static void TW_CALL set_ssao_num_samples_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_ssao_num_samples(*static_cast<const unsigned int*>(value));
}

static void TW_CALL get_ssao_num_samples_callback(void *value, void *clientData){
    *static_cast<unsigned int*>(value) = static_cast<const Scene*>(clientData)->get_ssao_num_samples();
}

static void TW_CALL set_sky_color_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_sky_color(*static_cast<const glm::vec3*>(value));
}

static void TW_CALL get_sky_color_callback(void *value, void *clientData){
    *static_cast<glm::vec3*>(value) = static_cast<const Scene*>(clientData)->get_sky_color();
}

static void TW_CALL set_background_color_callback(const void *value, void *clientData){
    static_cast<Scene*>(clientData)->set_background_color(*static_cast<const glm::vec3*>(value));
}

static void TW_CALL get_background_color_callback(void *value, void *clientData){
    *static_cast<glm::vec3*>(value) = static_cast<const Scene*>(clientData)->get_background_color();
}

Gui::Gui(Scene* scene, int width, int height):
    scene_(scene)
{
	TwInit(TW_OPENGL_CORE, NULL);
	TwWindowSize(width, height);

	bar_ = TwNewBar("Parameters");

	TwDefine("Parameters position='0 0' size='200 250' iconified=true");

	TwAddVarCB(bar_, "Direction", TW_TYPE_DIR3F, set_light_direction_callback, get_light_direction_callback, scene_, "\
		group=Light");

	TwAddVarCB(bar_, "Specular", TW_TYPE_FLOAT, set_light_specular_callback, get_light_specular_callback, scene_, "\
		min=0.0 max=2.0	step=0.01 group=Light");

	TwAddVarCB(bar_, "Diffuse", TW_TYPE_FLOAT, set_light_diffuse_callback, get_light_diffuse_callback, scene_, "\
		min=0.0 max=2.0	step=0.01 group=Light");

	TwAddVarCB(bar_, "Ambient", TW_TYPE_FLOAT, set_light_ambient_callback, get_light_ambient_callback, scene_, "\
		min=0.0 max=2.0	step=0.01 group=Light");

	TwAddVarCB(bar_, "Intensity", TW_TYPE_FLOAT, set_light_intensity_callback, get_light_intensity_callback, scene_, "\
		min=0.0 max=2.0	step=0.01 group=Light");

	TwAddVarCB(bar_, "Blur", TW_TYPE_BOOLCPP, set_ssao_blur_callback, get_ssao_blur_callback, scene_, "\
		group=AO");

	TwAddVarCB(bar_, "Radius", TW_TYPE_FLOAT, set_ssao_radius_callback, get_ssao_radius_callback, scene_,"\
		help='The radius of the ambient occlusion sampling kernel.'\
		min=0.1 max=5.0 step=0.02 group=AO");

	TwAddVarCB(bar_, "Samples", TW_TYPE_UINT32, set_ssao_num_samples_callback, get_ssao_num_samples_callback, scene_,"\
		help='Number of samples for ambient occlusion. Increase for higher quality.'\
		min=4 max=256 step=2 group=AO");

	TwAddVarCB(bar_, "Sky Color", TW_TYPE_COLOR3F, set_sky_color_callback, get_sky_color_callback, scene_, "\
        colormode=hls ");
	TwAddVarCB(bar_, "Background Color", TW_TYPE_COLOR3F, set_background_color_callback, get_background_color_callback, scene_, "\
        colormode=hls ");
}

Gui::~Gui(void){
    TwDeleteBar(bar_);
	TwTerminate();
}

void Gui::resize(int width, int height){
	TwWindowSize(width, height);
}

void Gui::draw(void)const{
    TwDraw();
}


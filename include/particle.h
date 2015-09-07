#ifndef PV_PARTICLE_H
#define PV_PARTICLE_H

//TODO: Might need recosidering
struct Particle{
    int shape_id;
    //Axis-angle
    glm::vec4 rot;
    glm::vec3 pos;
    float size;
};

#endif

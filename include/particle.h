#ifndef PV_PARTICLE_H
#define PV_PARTICLE_H

struct Particle{
    int shape_id;
    //Axis-angle
    glm::vec4 rot;
    glm::vec3 pos;
    float size;
};

#endif

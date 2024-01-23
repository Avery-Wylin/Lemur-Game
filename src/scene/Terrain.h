#ifndef TERRAIN_H
#define TERRAIN_H

#include "definitions.h"
#include <vector>
#include "../graphics/VAO.h"
#include <cglm/cglm.h>
#include "./physics/CollisionShape.h"

class Terrain {
    uint8_t data[TERRAIN_DIM][TERRAIN_DIM];

    inline void createPlane(float ax, float az, float bx, float bz, float ay, float by, std::vector<float> &pos,  std::vector<uint32_t> &index );
    void generateNormals(std::vector<float> &pos, std::vector<uint32_t> &index, std::vector<float> &norm);

public :
    void generate();
    void loadVAO(VAO& vao);

    void collide(CollisionShape a, vec3 resolve);
    void pointProjection(vec3 p, vec3 normal = nullptr);

};

#endif // TERRAIN_H

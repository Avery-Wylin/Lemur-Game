#include "Terrain.h"
#include <iostream>
#include <queue>
#include "../graphics/DebugDraw.h"
#include <glm/gtc/noise.hpp>


void Terrain::generate() {
    float continentalness, small_noise, large_noise;
    for( uint32_t z = 0; z < TERRAIN_DIM; ++z ) {
        for( uint32_t x = 0; x < TERRAIN_DIM; ++x ) {

            continentalness = fmax( pow(1- (pow(pow(x- TERRAIN_DIM /2.0,4) + pow(z- TERRAIN_DIM /2.0,4),.25)) / ( TERRAIN_DIM /2.0), .8),0 );
            small_noise = (glm::simplex( glm::vec2( x * 0.04, z * 0.04 ) ) + 1)*.5;
            large_noise = pow( (glm::simplex(glm::vec2( x * 0.01, z * 0.01 ))+ 1) *.5, 1.5);
            large_noise = large_noise*.9 + .02* floor(large_noise*5);
            small_noise = small_noise*.75 + .025* floor(small_noise*10);

            data[z][x] = 255 * continentalness * fmin(fmax( (.5*small_noise + .5) * large_noise,0), 1);
        }
    }

    // smoothNoise();
}


void Terrain::createPlane( float ax, float az, float bx, float bz, float ay, float by, std::vector<float> &pos, std::vector<uint32_t> &index ) {
    uint32_t pos_index = pos.size() / 3;

    // Create 2 triangles aligned on the y axis from A to B

    // bl 0
    pos.push_back( ax );
    pos.push_back( ay );
    pos.push_back( az );

    // br 1
    pos.push_back( bx );
    pos.push_back( ay );
    pos.push_back( bz );

    // tr 2
    pos.push_back( bx );
    pos.push_back( by );
    pos.push_back( bz );

    // tl 3
    pos.push_back( ax );
    pos.push_back( by );
    pos.push_back( az );

    // bl br tr 0 1 2
    index.push_back( pos_index );
    index.push_back( pos_index + 1 );
    index.push_back( pos_index + 2 );

    // bl tr tl 0 2 3
    index.push_back( pos_index );
    index.push_back( pos_index + 2 );
    index.push_back( pos_index + 3 );
}


void Terrain::generateNormals( std::vector<float> &pos, std::vector<uint32_t> &index, std::vector<float> &norm ) {
    // This only works if the positions are separated, reused positions will be overwritten (okay in flat quads/polygons)

    vec3 normal, ab, ac;
    norm.insert( norm.begin(), pos.size(), 0 );
    std::vector<float> face_count;
    face_count.insert( face_count.begin(), pos.size() / 3, 0 );
    for( uint32_t i = 0; i < index.size(); i += 3 ) {
        // cross ab and ac using the triangle positions
        glm_vec3_sub( &pos[index[i + 1] * 3], &pos[index[i] * 3], ab );
        glm_vec3_sub( &pos[index[i + 2] * 3], &pos[index[i] * 3], ac );
        glm_vec3_cross( ab, ac, normal );
        glm_vec3_normalize( normal );

        // add the normal
        glm_vec3_add( &norm[index[i] * 3], normal, &norm[index[i] * 3] );
        glm_vec3_add( &norm[index[i + 1] * 3], normal, &norm[index[i + 1] * 3] );
        glm_vec3_add( &norm[index[i + 2] * 3], normal, &norm[index[i + 2] * 3] );
        face_count[index[i]]++;
        face_count[index[i + 1]]++;
        face_count[index[i + 2]]++;
    }

    for( uint32_t i = 0; i < face_count.size(); ++i ) {
        norm[i * 3] /= face_count[i];
        norm[i * 3 + 1] /= face_count[i];
        norm[i * 3 + 2] /= face_count[i];
    }
}

void Terrain::loadVAO( VAO &vao ) {
    std::vector<float> pos;
    std::vector<float> norm;
    std::vector<uint32_t> index;

    uint32_t bl, br, tr, tl;

    pos.resize( TERRAIN_DIM * TERRAIN_DIM * 3 );
    for( uint32_t i = 0; i < TERRAIN_DIM * TERRAIN_DIM; ++i ) {
        pos[3 * i] = i % TERRAIN_DIM * TERRAIN_SCALE;
        pos[3 * i + 1] = data[i / TERRAIN_DIM][i % TERRAIN_DIM] * TERRAIN_HEIGHT_SCALE;
        pos[3 * i + 2] = i / TERRAIN_DIM * TERRAIN_SCALE;
    }

    for( uint32_t z = 0; z < TERRAIN_DIM - 1; ++z ) {
        for( uint32_t x = 0; x < TERRAIN_DIM - 1; ++x ) {
            bl = ( z * TERRAIN_DIM + x );
            br = bl + 1;
            tr = br + TERRAIN_DIM;
            tl = bl + TERRAIN_DIM;

            if( abs( pos[3 * bl + 1] - pos[3 * tr + 1] ) > abs( pos[3 * br + 1] - pos[3 * tl + 1] ) ) {
                // split from tl to br

                // tr br tl
                index.push_back( tr );
                index.push_back( br );
                index.push_back( tl );

                // tl br bl
                index.push_back( tl );
                index.push_back( br );
                index.push_back( bl );
            }
            else {
                // split from bl to tr

                // br bl tl
                index.push_back( tr );
                index.push_back( br );
                index.push_back( bl );

                // tr bl tl
                index.push_back( tr );
                index.push_back( bl );
                index.push_back( tl );
            }
        }
    }

    // Create floor plane
    // bl
    uint32_t i = pos.size()/3;
    pos.push_back(-(float)TERRAIN_DIM * TERRAIN_SCALE );
    pos.push_back(0);
    pos.push_back(-(float)TERRAIN_DIM * TERRAIN_SCALE );
    // br
    pos.push_back(2* TERRAIN_DIM * TERRAIN_SCALE );
    pos.push_back(0);
    pos.push_back(-(float)TERRAIN_DIM * TERRAIN_SCALE );
    // tr
    pos.push_back(2* TERRAIN_DIM * TERRAIN_SCALE );
    pos.push_back(0);
    pos.push_back(2* TERRAIN_DIM * TERRAIN_SCALE );
    // tl
    pos.push_back(-(float)TERRAIN_DIM * TERRAIN_SCALE );
    pos.push_back(0);
    pos.push_back(2* TERRAIN_DIM * TERRAIN_SCALE );

    index.push_back( i+2 );
    index.push_back( i+1 );
    index.push_back( i+3 );

    index.push_back( i+3 );
    index.push_back( i+1 );
    index.push_back( i+0 );


    generateNormals( pos, index, norm );

    vao.loadAttributeFloat( ATTRB_POS, 0, 0, 3, pos.size(), pos.data() );
    vao.loadAttributeFloat( ATTRB_NORM, 0, 0, 3, norm.size(), norm.data() );
    vao.loadIndex( index.size(), index.data() );
}

float barycentric(vec3 a, vec3 b, vec3 c, float x, float z){
        float denom = (b[2]-c[2])*(a[0]-c[0])+(c[0]-b[0])*(a[2]-c[2]);
        float d0 = ((b[2]-c[2])*(x-c[0])+(c[2]-b[0])*(z-c[2]))/denom;
        float d1 = ((c[2]-a[2])*(x-c[0])+(a[0]-c[0])*(z-c[2]))/denom;
        float d2 = 1-d0-d1;
        return d0 * a[1] + d1 * b[1] + d2 * c[1];
}

void Terrain::pointProjection(vec3 p, vec3 normal){
    // Out of bounds case
    unsigned int x = p[0]/ TERRAIN_SCALE, z = p[2]/ TERRAIN_SCALE;
    if(x < 0 || x >= TERRAIN_DIM || z < 0 || z >= TERRAIN_DIM ){
        p[1] = 0;
        return;
    }

    // One of two triangles case, choose the triangle
    vec3 a = GLM_VEC3_ZERO_INIT,b = GLM_VEC3_ZERO_INIT,c = GLM_VEC3_ZERO_INIT;
    if( abs( data[z][x] - data[z+1][x+1] ) < abs( data[z][x+1] - data[z+1][x] ) ){
        // Split bl to tr

        if( (p[0] - x* TERRAIN_SCALE ) < (p[2] - z* TERRAIN_SCALE )){

            // bl
            a[0] = 0;
            a[1] = TERRAIN_HEIGHT_SCALE *data[z][x];
            a[2] = 0;

            // tr
            b[0] = TERRAIN_SCALE;
            b[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x+1];
            b[2] = TERRAIN_SCALE;

            // tl
            c[0] = 0;
            c[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x];
            c[2] = TERRAIN_SCALE;

        }
        else{

            // tr
            a[0] = TERRAIN_SCALE;
            a[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x+1];
            a[2] = TERRAIN_SCALE;

            // bl
            b[0] = 0;
            b[1] = TERRAIN_HEIGHT_SCALE *data[z][x];
            b[2] = 0;

            // br
            c[0] = TERRAIN_SCALE;
            c[1] = TERRAIN_HEIGHT_SCALE *data[z][x+1];
            c[2] = 0;

        }
    }
    else{
        // Split tl to br

        if( TERRAIN_SCALE -(p[0] - x* TERRAIN_SCALE ) < (p[2] - z* TERRAIN_SCALE )){

            // br
            a[0] = TERRAIN_SCALE;
            a[1] = TERRAIN_HEIGHT_SCALE *data[z][x+1];
            a[2] = 0;

             // tr
            b[0] = TERRAIN_SCALE;
            b[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x+1];
            b[2] = TERRAIN_SCALE;

            // tl
            c[0] = 0;
            c[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x];
            c[2] = TERRAIN_SCALE;
        }
        else{
            // tl
            a[0] = 0;
            a[1] = TERRAIN_HEIGHT_SCALE *data[z+1][x];
            a[2] = TERRAIN_SCALE;

            // bl
            b[0] = 0;
            b[1] = TERRAIN_HEIGHT_SCALE *data[z][x];
            b[2] = 0;

            // br
            c[0] = TERRAIN_SCALE;
            c[1] = TERRAIN_HEIGHT_SCALE *data[z][x+1];
            c[2] = 0;

        }
    }

    // DebugDraw::line(a[0] + x*SCALE, a[1], a[2]+z*SCALE, b[0] + x*SCALE,b[1],b[2] + z*SCALE);
    // DebugDraw::line(b[0] + x*SCALE, b[1], b[2]+z*SCALE, c[0] + x*SCALE,c[1],c[2] + z*SCALE);
    // DebugDraw::line(a[0] + x*SCALE, a[1], a[2]+z*SCALE, c[0] + x*SCALE,c[1],c[2] + z*SCALE);

    // The winding order must be inverted to match the upwards facing normal
    p[1] = barycentric(a, c, b, p[0] - x* TERRAIN_SCALE, p[2] - z* TERRAIN_SCALE );
    if(normal){
        glm_vec3_sub(b,a,b);
        glm_vec3_sub(c,a,c);
        glm_vec3_cross(b,c,normal);
        glm_vec3_normalize(normal);
    }
}

void collide(CollisionShape a, vec3 resolve){

}

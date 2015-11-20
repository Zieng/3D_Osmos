//
// Created by zieng on 11/1/15.
//

#ifndef SOLARSYSTEM_PLANET_H
#define SOLARSYSTEM_PLANET_H

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <vector>
#include <map>
#include <string.h>  // for memcmp()
#include <string>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"

#include <SOIL.h>

#define MAX_RADIUS (10)
#define MIN_RADIUS (0.1)

using namespace std;

struct PackedVertex
{
    glm::vec3 pos;
    glm::vec2 uv;
    glm::vec3 normal;
    bool operator < (const PackedVertex that) const{
        return memcmp((void *)this, (void *) &that, sizeof(PackedVertex))>0;
    };
};

enum PLANET_TYPE{
    CenterStar,
    InvisibleStar,
    SwallowStar,
    RepulsiveStar,
    SwiftStar,
    NutriStar,
    DarkStar,
    ChaosStar,
    BreatheStar,
    PlayerStar,
};

class Planet
{
public:
    Planet(string obj_path, string texture_path, double r, PLANET_TYPE t);
    ~Planet();

//    void set_uniform(GLuint MVPID,
//                     GLuint RenderID,
//                     GLuint MyTextureSamplereID,
//                     GLuint modelMatrixID,
//                     GLuint viewMatrixID,
//                     GLuint lightPositionID);

    bool load_OBJ(const char * objPath,
                  std::vector<glm::vec3> & out_vertex,
                  std::vector<glm::vec2> & out_uv,
                  std::vector<glm::vec3> & out_normal);

    void VBO_indexer(vector<glm::vec3> & in_vertices,
                     vector<glm::vec2> & in_uvs,
                     vector<glm::vec3> & in_normals,
                     vector<glm::vec3> & out_vertices,
                     vector<glm::vec2> & out_uvs,
                     vector<glm::vec3> & out_normals,
                     vector<unsigned short> & indices);

    GLuint load_texture(const char * image_path);

    void set_velocity(glm::vec3 speed);
    void set_radius(double r);
    void set_orbital(int aa,int bb);
    void set_position(glm::vec3 p);
    glm::vec3 get_position();
    void set_position(glm::mat4 new_model_matrix);
    void update_position();
    void update_position(const glm::mat4 &parentModelMatrix);
    glm::mat4 get_model_matrix();
    void draw_orbital();
    void print_info();
    void destroy();
    bool check_collison(const Planet & planet);
    double get_radius();
    void set_active(bool b);

    // planet type
    PLANET_TYPE  type;
    // object data
    GLuint texture;

    vector<unsigned short> indices;
    vector<glm::vec3> indexed_vertices;
    vector<glm::vec2> indexed_uvs;
    vector<glm::vec3> indexed_normals;

    // variable for orbital
    float theta;
    int a,b;      // a,b for ellipse function
    vector<glm::vec3> orbital_vertices;

    // parameters of the planet
    glm::vec3 orientation;
    glm::vec3 up;
    glm::vec3 velocity;
    double radius;  // the radius of a planet
    bool destroyed;
    bool isActive;

    glm::vec3 worldLocation;   // center of the planet in world-coordinate

    GLuint vertexBuffer,uvBuffer,normalBuffer,elementBuffer,orbital_vertexBuffer;

    glm::mat4 self_ModelMatrix;
    glm::mat4 parent_ModelMatrix;

    // uniform variable for shader
    GLuint MVPID;
    GLuint RenderID;
    GLuint MyTextureSamplerID;
    GLuint modelMatrixID;
    GLuint viewMatrixID;
    GLuint lightPositionID;






    float lastTime;  // to help calculate update

};


#endif //SOLARSYSTEM_PLANET_H

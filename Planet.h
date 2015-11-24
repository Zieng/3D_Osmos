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
#define MIN_RADIUS (0)

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
    NormalStar,
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
    Planet(double r,PLANET_TYPE t);
    ~Planet();

//    void set_uniform(GLuint MVPID,
//                     GLuint RenderID,
//                     GLuint MyTextureSamplereID,
//                     GLuint modelMatrixID,
//                     GLuint viewMatrixID,
//                     GLuint lightPositionID);

//    bool load_OBJ(const char * objPath,
//                  std::vector<glm::vec3> & out_vertex,
//                  std::vector<glm::vec2> & out_uv,
//                  std::vector<glm::vec3> & out_normal);

//    void VBO_indexer(vector<glm::vec3> & in_vertices,
//                     vector<glm::vec2> & in_uvs,
//                     vector<glm::vec3> & in_normals,
//                     vector<glm::vec3> & out_vertices,
//                     vector<glm::vec2> & out_uvs,
//                     vector<glm::vec3> & out_normals,
//                     vector<unsigned short> & indices);

//    GLuint load_texture(const char * image_path);

    void set_velocity(glm::vec3 speed);
    glm::vec3 get_velocity();
    void set_radius(double r);
    void set_position(glm::vec3 p);
    glm::vec3 get_position();
    void set_position(glm::mat4 new_model_matrix);
    void update_position();
    void update_position(const glm::mat4 &parentModelMatrix);
    glm::mat4 get_model_matrix();
    void print_info();
    bool check_collison(const Planet & planet);
    double get_radius();
    void set_active(bool b);
    bool get_active_state();



    // planet type
    PLANET_TYPE  type;
    // object data
    GLuint texture;

    vector<unsigned short> indices;
    vector<glm::vec3> indexed_vertices;
    vector<glm::vec2> indexed_uvs;
    vector<glm::vec3> indexed_normals;

    // p    arameters of the planet
    glm::vec3 orientation;
    glm::vec3 up;
    glm::vec3 velocity;
    double radius;  // the radius of a planet
    bool isActive;

    glm::vec3 worldLocation;   // center of the planet in world-coordinate

    GLuint vertexBuffer,uvBuffer,normalBuffer,elementBuffer;

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




   // static variable for optimization
    static bool planetInitOK;
    static bool planetTerminate;

    static GLuint share_uvBuffer,share_normalBuffer,share_elementBuffer;
    static GLuint playerStarTexture  ,
            centerStarTexture   ,
            normalStarTexture   ,
            repulsiveStarTexture,
            invisibleStarTexture,
            swiftStarTexture    ,
            swallowStarTexture  ,
            nutriStarTexture    ,
            darkStarTexture     ,
            chaosStarTexture    ,
            breatheStarTexture  ,
            undefinedTexture    ;

    static vector<unsigned short > share_indices;
    static vector<glm::vec3> share_vertices;
    static vector<glm::vec2> share_uvs;
    static vector<glm::vec3> share_normals;

    static GLuint load_texture(const char * image_path)
    {
        GLuint Texture;
        int image_channels;
        int image_width_d, image_height_d;
        unsigned char * image_buffer;

        glGenTextures(1,&Texture);
        glBindTexture(GL_TEXTURE_2D, Texture);
        image_buffer = SOIL_load_image(image_path, &image_width_d, &image_height_d, &image_channels, 4);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width_d, image_height_d, 0, GL_RGBA, GL_UNSIGNED_BYTE, image_buffer);
        SOIL_free_image_data(image_buffer);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glGenerateMipmap(GL_TEXTURE_2D);

        /* check for an error during the load process */
        if( 0 == Texture )
        {
            printf( "SOIL loading error: '%s'\n", SOIL_last_result() );
        }

        cout<<"Texture "<<image_path<<" load successfully! texture="<<Texture<<endl;

        return Texture;
    }

    static bool load_OBJ(const char * objPath,
                  std::vector<glm::vec3> & out_vertex,
                  std::vector<glm::vec2> & out_uv,
                  std::vector<glm::vec3> & out_normal)
    {
        vector<unsigned int > vertexIndices,uvIndices,normalIndices;
        vector<glm::vec3> temp_vertices;
        vector<glm::vec2> temp_uvs;
        vector<glm::vec3> temp_normals;

        FILE * file=fopen(objPath, "rb");
        if (file==NULL)
        {
            std::cout<<"open obj file failed!"<<std::endl;
            return false;
        }
        while (1)
        {
            char lineHeader[128];
            if (fscanf(file, "%s",lineHeader)==EOF)
            {
                break;
            }

            if (strcmp(lineHeader, "v")==0)
            {
                glm::vec3 newVertex;
                fscanf(file, "%f %f %f\n",&newVertex.x,&newVertex.y,&newVertex.z);
                temp_vertices.push_back(newVertex);
            }
            else if (strcmp(lineHeader, "vt")==0)
            {
                glm::vec2 newUV;
                fscanf(file, "%f %f\n",&newUV.x,&newUV.y);
                newUV.y=-newUV.y;
                temp_uvs.push_back(newUV);
            }
            else if (strcmp(lineHeader, "vn")==0)
            {
                glm::vec3 newNormal;
                fscanf(file, "%f %f %f\n",&newNormal.x,&newNormal.y,&newNormal.z);
                temp_normals.push_back(newNormal);
            }
            else if (strcmp(lineHeader, "f")==0)
            {
                std::string vertex1,vertex2,vertex3;
                unsigned int vertexIndex[3],uvIndex[3],normalIndex[3];
                int matches=fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d",&vertexIndex[0],&uvIndex[0],&normalIndex[0],
                                   &vertexIndex[1],&uvIndex[1],&normalIndex[1],
                                   &vertexIndex[2],&uvIndex[2],&normalIndex[2]);
                if (matches!=9)
                {
                    std::cout<<"format mis-match"<<std::endl;
                    return false;
                }
                vertexIndices.push_back(vertexIndex[0]);
                vertexIndices.push_back(vertexIndex[1]);
                vertexIndices.push_back(vertexIndex[2]);
                uvIndices.push_back(uvIndex[0]);
                uvIndices.push_back(uvIndex[1]);
                uvIndices.push_back(uvIndex[2]);
                normalIndices.push_back(normalIndex[0]);
                normalIndices.push_back(normalIndex[1]);
                normalIndices.push_back(normalIndex[2]);
            }
            else
            {
                char stupidBuffer[1000];
                fgets(stupidBuffer, 1000, file);
            }

        }
        for( unsigned int i=0; i<vertexIndices.size(); i++ )
        {

            // Get the indices of its attributes
            unsigned int vertexIndex = vertexIndices[i];
            unsigned int uvIndex = uvIndices[i];
            unsigned int normalIndex = normalIndices[i];

            // Get the attributes thanks to the index
            glm::vec3 vertex = temp_vertices[ vertexIndex-1 ];
            glm::vec2 uv = temp_uvs[ uvIndex-1 ];
            glm::vec3 normal = temp_normals[ normalIndex-1 ];

            // Put the attributes in buffers
            out_vertex.push_back(vertex);
            out_uv     .push_back(uv);
            out_normal .push_back(normal);
        }

        return true;
    }

    static void VBO_indexer(vector<glm::vec3> & in_vertices,
                     vector<glm::vec2> & in_uvs,
                     vector<glm::vec3> & in_normals,
                     vector<glm::vec3> & out_vertices,
                     vector<glm::vec2> & out_uvs,
                     vector<glm::vec3> & out_normals,
                     vector<unsigned short> & indices)
    {
        map<PackedVertex,unsigned short> indexedVertices;

        for (int i=0; i<in_vertices.size(); i++) {
            PackedVertex p={in_vertices[i],in_uvs[i],in_normals[i]};
            bool Indexed=false;
            unsigned short index;

            map<PackedVertex, unsigned short>::iterator it=indexedVertices.find(p);
            if (it!=indexedVertices.end()) {
                Indexed=true;
                index=it->second;
            }

            if (Indexed) {
                indices.push_back(index);
            }
            else{
                out_vertices.push_back(in_vertices[i]);
                out_uvs.push_back(in_uvs[i]);
                out_normals.push_back(in_normals[i]);
                unsigned short newIndex=(unsigned short)out_vertices.size()-1;
                indices.push_back(newIndex);
                indexedVertices[p]=newIndex;
            }
        }
    }



    static bool planet_init()
    {
        playerStarTexture = load_texture("./texture/player.png");

        normalStarTexture = load_texture("./texture/normal.png");

        centerStarTexture = load_texture("./texture/sun.png");

        invisibleStarTexture = load_texture("./texture/invisible.png");

        swallowStarTexture = load_texture("./texture/swallow.png");

        repulsiveStarTexture = load_texture("./texture/repulsive.png");

        swiftStarTexture = load_texture("./texture/swift.png");

        nutriStarTexture = load_texture("./texture/nutri.png");

        darkStarTexture = load_texture("./texture/dark.png");

        chaosStarTexture = load_texture("./texture/chaos.png");

        breatheStarTexture = load_texture("./texture/breathe.png");

        undefinedTexture = load_texture("./texture/undefined.png");


        vector<glm::vec3> vertices,normals;
        vector<glm::vec2> uvs;

        if (load_OBJ("./object/ball_hd2.obj", vertices, uvs, normals) == false)
        {
            cout<<"res=false"<<endl;
            return false;
        }

        VBO_indexer(vertices, uvs, normals, share_vertices, share_uvs, share_normals, share_indices);

        for(int i=0;i<share_vertices.size();i++)
        {
            share_vertices[i] /= 2;  // the original object has radius = 2, so need divide by 2
        }

        glGenBuffers(1,&share_uvBuffer);
        glBindBuffer(GL_ARRAY_BUFFER,share_uvBuffer);
        glBufferData(GL_ARRAY_BUFFER,share_uvs.size()* sizeof(glm::vec2),&share_uvs[0],GL_STATIC_DRAW);

        glGenBuffers(1,&share_normalBuffer);
        glBindBuffer(GL_ARRAY_BUFFER,share_normalBuffer);
        glBufferData(GL_ARRAY_BUFFER,share_normals.size()* sizeof(glm::vec3),&share_normals[0],GL_STATIC_DRAW);


        glGenBuffers(1,&share_elementBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,share_elementBuffer);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,share_indices.size() * sizeof(unsigned short), &share_indices[0],GL_STATIC_DRAW);

        planetInitOK = true;

        cout<<endl;
        cout<<endl;
        cout<<"Init OK!"<<endl;
        cout<<endl<<endl;

        return planetInitOK;
    }

    static void planet_terminate()
    {
        glDeleteBuffers(1,&share_uvBuffer);
        glDeleteBuffers(1,&share_normalBuffer);
        glDeleteBuffers(1,&share_elementBuffer);

        glDeleteTextures(1,&playerStarTexture);
        glDeleteTextures(1,&centerStarTexture);
        glDeleteTextures(1,&normalStarTexture);
        glDeleteTextures(1,&repulsiveStarTexture);
        glDeleteTextures(1,&invisibleStarTexture);
        glDeleteTextures(1,&swiftStarTexture);
        glDeleteTextures(1,&swallowStarTexture);
        glDeleteTextures(1,&nutriStarTexture);
        glDeleteTextures(1,&darkStarTexture);
        glDeleteTextures(1,&chaosStarTexture);
        glDeleteTextures(1,&breatheStarTexture);
        glDeleteTextures(1,&undefinedTexture);

        planetTerminate = true;
    }
};


#endif //SOLARSYSTEM_PLANET_H

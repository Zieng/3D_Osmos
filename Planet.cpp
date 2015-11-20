//
// Created by zieng on 11/1/15.
//

#include "Planet.h"

Planet::Planet(string obj_path, string texture_path, double r, bool is_fixed)
{
//    MVPID = 0;
//    RenderID = 0;
//    MyTextureSamplerID = 0;
//    modelMatrixID = 0;
//    viewMatrixID = 0;
//    lightPositionID = 0;
    isFixedStar = is_fixed;
    self_ModelMatrix = glm::mat4(1.0f);
    velocity = glm::vec3(0,0,0);
    orientation = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    radius = (r < 0) ? -r : r;
    lastTime = glfwGetTime();
    destroyed = false;
    isActive = true;


    vector<glm::vec3> vertices,normals;
    vector<glm::vec2> uvs;

    if (load_OBJ(obj_path.c_str(),vertices,uvs,normals) == false)
    {
        cout<<"res=false"<<endl;
        return;
    }


    texture = load_texture(texture_path.c_str());
    if(texture == 0)
    {
        cout<<"load textre failed"<<endl;
        return ;
    }

    if(r == 0)
    {
        cout<<"the scale of an object can't be zero"<<endl;
        return ;
    }

    VBO_indexer(vertices, uvs, normals, indexed_vertices, indexed_uvs, indexed_normals, indices);

    for(int i=0;i<indexed_vertices.size();i++)
    {
        indexed_vertices[i] *= radius/2;  // the original object has radius = 2, so need divide by 2
    }

//    float minX=99999,maxX=-9999;
//    for(int i=0;i<indexed_vertices.size();i++)
//    {
//        if(minX > indexed_vertices[i].x)
//            minX=indexed_vertices[i].x;
//        if(maxX < indexed_vertices[i].x)
//            maxX=indexed_vertices[i].x;
//    }
//    cout<<"The origin radius is "<<(maxX-minX)/2<<endl;

    glGenBuffers(1,&vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

    glGenBuffers(1,&uvBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,uvBuffer);
    glBufferData(GL_ARRAY_BUFFER,indexed_uvs.size()* sizeof(glm::vec2),&indexed_uvs[0],GL_STATIC_DRAW);

    glGenBuffers(1,&normalBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,normalBuffer);
    glBufferData(GL_ARRAY_BUFFER,indexed_normals.size()* sizeof(glm::vec3),&indexed_normals[0],GL_STATIC_DRAW);


    glGenBuffers(1,&elementBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,elementBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indices.size() * sizeof(unsigned short), &indices[0],GL_STATIC_DRAW);
}

Planet::~Planet()
{
    if(!destroyed)
        destroy();
}

//void Planet::set_uniform(GLuint MVPID_, GLuint RenderID_, GLuint MyTextureSamplerID_, GLuint modelMatrixID_,
//                         GLuint viewMatrixID_, GLuint lightPositionID_)
//{
//    MVPID = MVPID_;
//    RenderID = RenderID_;
//    MyTextureSamplereID = MyTextureSamplereID_;
//    modelMatrixID = modelMatrixID_;
//    viewMatrixID = viewMatrixID_;
//    lightPositionID = lightPositionID_;
//}

bool Planet::load_OBJ(const char *objPath, std::vector<glm::vec3> &out_vertex, std::vector<glm::vec2> &out_uv, std::vector<glm::vec3> &out_normal)
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

void Planet::VBO_indexer(vector<glm::vec3> &in_vertices, vector<glm::vec2> &in_uvs, vector<glm::vec3> &in_normals,
                         vector<glm::vec3> &out_vertices, vector<glm::vec2> &out_uvs, vector<glm::vec3> &out_normals,
                         vector<unsigned short> &indices)
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

GLuint Planet::load_texture(const char *image_path)
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

void Planet::set_position(glm::vec3 p)
{
    worldLocation = p;

    //calculate the model matrix from position
    glm::vec4 c1,c2,c3,c4;

    c1 = glm::vec4(1,0,0,0);
    c2 = glm::vec4(0,1,0,0);
    c3 = glm::vec4(0,0,1,0);
    c4 = glm::vec4(worldLocation.x, worldLocation.y, worldLocation.z, 1);

    self_ModelMatrix = glm::mat4(c1,c2,c3,c4);

}

void Planet::set_position(glm::mat4 new_model_matrix)
{
    self_ModelMatrix = new_model_matrix;

    //calclute the center from modle_matrix

    glm::vec4 temp = self_ModelMatrix * glm::vec4(1.f,1.f,1.f,1.f);
    worldLocation = glm::vec3(temp.x , temp.y , temp.z);

}
void Planet::update()
{
    glm::mat4 m = glm::mat4(1.0f);

    update(m);
}

void Planet::update(const glm::mat4 & parentModelMatrix)
{
    parent_ModelMatrix = parentModelMatrix;
    float parentX=0,parentY=0,parentZ=0;
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

//    cout<<"delta time="<<deltaTime<<endl;
    if(isFixedStar)  // fixed star no need to update
        return;

    worldLocation += 1.f * velocity;

    //update the model matrix
    glm::vec4 c1,c2,c3,c4;

    c1 = glm::vec4(1,0,0,0);
    c2 = glm::vec4(0,1,0,0);
    c3 = glm::vec4(0,0,1,0);
    c4 = glm::vec4(worldLocation.x, worldLocation.y, worldLocation.z, 1);

    self_ModelMatrix = parentModelMatrix * glm::mat4(c1,c2,c3,c4);

}

void Planet::set_fixed(bool b)
{
    isFixedStar = b;
}

void Planet::set_orbital(int aa, int bb)
{
    if(isFixedStar)
    {
        cout<<"Fixed star no need set orbital!"<<endl;
        return ;
    }
    a = aa;
    b = bb;
//    theta = (double) random();
    theta = 0;

    glm::vec3 vertex;
    for(float i=0;i<2*3.11415926539;i+=0.01)
    {
        vertex.x = a * glm::cos(theta+i);
        vertex.y = b * glm::sin(theta+i);
        vertex.z = 0;

        orbital_vertices.push_back(vertex);
    }

    glGenBuffers(1,&orbital_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,orbital_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, orbital_vertices.size() * sizeof(glm::vec3), &orbital_vertices[0], GL_STATIC_DRAW);

    // calcualte the self model matrix
    update();
}

void Planet::set_radius(double r)
{
    if(r == 0)
    {
        cout<<"the scale of an object can't be zero"<<endl;
        return;
    }
    r = (r < 0) ? -r : r;

    radius = r;
    for(int i=0;i<indexed_vertices.size();i++)
    {
        indexed_vertices[i] *= radius;
    }

    glBindBuffer(GL_ARRAY_BUFFER,vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, indexed_vertices.size() * sizeof(glm::vec3), &indexed_vertices[0], GL_STATIC_DRAW);

}

void Planet::destroy()
{
    glDeleteBuffers(1,&vertexBuffer);
    glDeleteBuffers(1,&uvBuffer);
    glDeleteBuffers(1,&normalBuffer);
    glDeleteTextures(1,&texture);
    destroyed = true;
}

glm::mat4 Planet::get_model_matrix()
{
    return self_ModelMatrix;
}

void Planet::set_velocity(glm::vec3 speed)
{
    velocity = speed;
}

bool Planet::check_collison(const Planet & planet)
{
    const glm::vec3 temp = worldLocation - planet.worldLocation;

//    double distance = glm::distance(temp ,temp);
//    float distance = temp.length();

//    double distance = pow( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z, 1.0/2);
    double distance = sqrt( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z );

    cout<<"current planet has radius="<<radius<<", other planet has radius="<<planet.radius<<endl;
    cout<<"current planet at "<<glm::to_string(worldLocation)<<", other planet at "<<glm::to_string(planet.worldLocation)<<endl;
    cout<<"The distance between them are:"<<distance<<endl;
    if(planet.radius+radius >= distance)
        return true;

    return false;
}

void Planet::print_info()
{
    // debug print info
    cout<<endl;
    cout<<endl;
    cout<<"---------------------------------------------------------------------------"<<endl;
    cout<<"\tvelocity ("<<velocity.x<<" , "<<velocity.y<<" , "<<velocity.z<<")"<<endl;
    cout << "\tposition (" << worldLocation.x << " , " << worldLocation.y << " , " << worldLocation.z << ")." << endl;
    cout<<"\torientation ("<<orientation.x<<" , "<<orientation.y<<" , "<<orientation.z<<")"<<endl;
    cout<<"\tup ("<<up.x<<" , "<<up.y<<" , "<<up.z<<" )"<<endl;
    cout<<endl;
}

void Planet::set_active(bool b)
{
    isActive = b;
}

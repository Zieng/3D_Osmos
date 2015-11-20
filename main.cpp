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
#include "glm/gtx/rotate_vector.hpp"
#include <glm/gtc/type_ptr.hpp>
#include "glm/gtx/string_cast.hpp"

#include <SOIL.h>
#include <iostream>

#include "Planet.h"


#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <  (b) ? (a) : (b))
#define PI 3.14159265358979323846
#define WORLD_MAX_X (100)
#define WORLD_MIN_X (-100)
#define WORLD_MAX_Y (100)
#define WORLD_MIN_Y (-100)
#define WORLD_MAX_Z (100)
#define WORLD_MIN_Z (-100)
#define radius_to_volume(r) (4.0/3 * (PI) * pow((r),3) )
#define volume_to_radius(v) ( pow( 3.0/(4* (PI)) * (v), 1.0/3) )

using namespace std;
using namespace glm;

GLFWwindow * window;
GLint windowWidth;
GLint windowHeight;

// universe for all objects
glm::mat4 ViewMatrix;
glm::mat4 ProjectionMatrix;


glm::vec3 lightPosition=glm::vec3(0,0,0);

glm::vec3 eye = glm::vec3( 0,0, 30 );
glm::vec3 up = glm::vec3(0,1,0);

glm::vec3 position = glm::vec3( 0,0, 30 );
float horizontalAngle = 3.14;
float verticalAngle = 0;
float rotateAngle = 0;
float initialFoV = 45.0f;
float speed = 0.3f;   // 3 units / second
float mouseSpeed = 0.005f;

GLint MVPID;
GLint RenderID;
GLint MyTextureSamplerID;
GLint modelMatrixID;
GLint viewMatrixID;
GLint lightPositionID;

bool paused = false;

GLuint loadShaders(const char * vertex_file_path,const char * fragment_file_path)
{
    //A shader object is used to maintain the source code strings that define a shader.
    GLuint VertexShaderID=glCreateShader(GL_VERTEX_SHADER);  //creates an empty shader object and returns a non-zero value by which it can be referenced
    GLuint FragmentShaderID=glCreateShader(GL_FRAGMENT_SHADER);

    // open shader file to get its source code
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path,std::ios::in);
    if(VertexShaderStream.is_open()){
        std::string line="";
        while(getline(VertexShaderStream,line)){
            VertexShaderCode+=line+"\n";
        }
        VertexShaderStream.close();
    }
    else{
        std::cout<<"Fatal Error! can't open "<<vertex_file_path<<std::endl;
        return -1;
    }

    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path,std::ios::in);
    if(FragmentShaderStream.is_open()){
        std::string line="";
        while(getline(FragmentShaderStream,line)){
            FragmentShaderCode+=line+"\n";
        }
        FragmentShaderStream.close();
    }
    else{
        std::cout<<"Fatal Error! can't open "<<fragment_file_path<<std::endl;
        return -1;
    }

    GLint Result=GL_FALSE;
    int infoLogLength;

    char const * VertexSourcePointer=VertexShaderCode.c_str();
    glShaderSource(VertexShaderID,1,&VertexSourcePointer,NULL); //sets the source code in shader to the source code in the array of strings specified by string
    glCompileShader(VertexShaderID); //compiles the source code strings that have been stored in the shader object specified by shader.

    glGetShaderiv(VertexShaderID,GL_COMPILE_STATUS,&Result);  //returns in Result the value of a parameter for a specific shader object.
    glGetShaderiv(VertexShaderID,GL_INFO_LOG_LENGTH,&infoLogLength);  // similiar, but returns  in infoLogLength.
    char VertexShaderErrorMessage[(const int )infoLogLength+1];
    glGetShaderInfoLog(VertexShaderID,infoLogLength,NULL,VertexShaderErrorMessage);    //returns the information log for the specified shader object.
    //    std::cout<<"The state of compiling vertex shader is: "<<VertexShaderErrorMessage<<std::endl;

    //    std::cout<<"Compiling shader "<<fragment_file_path<<std::endl;
    char const * FragmentSourcePointer=FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID,1,&FragmentSourcePointer,NULL);
    glCompileShader(FragmentShaderID);

    glGetShaderiv(FragmentShaderID,GL_COMPILE_STATUS,&Result);
    glGetShaderiv(FragmentShaderID,GL_INFO_LOG_LENGTH,&infoLogLength);
    char FragmentShaderErrorMessage[(const int )infoLogLength+1];
    glGetShaderInfoLog(FragmentShaderID,infoLogLength,NULL,FragmentShaderErrorMessage);
    //    std::cout<<"The state of compiling fragment shader is: "<<FragmentShaderErrorMessage<<std::endl;

    std::cout<<"Linking program..."<<std::endl;
    GLuint ProgramID=glCreateProgram();   //creates an empty program object and returns a non-zero value by which it can be referenced.
    glAttachShader(ProgramID,VertexShaderID);  //  attach order doesn't matter
    glAttachShader(ProgramID,FragmentShaderID);  //Multiple shader objects of the same type may not be attached to a single program object.
    glLinkProgram(ProgramID);   // once the link is successful, then each shader can do its work

    glGetProgramiv(ProgramID,GL_LINK_STATUS,&Result);     // similiar to glGetShaderiv()
    glGetProgramiv(ProgramID,GL_INFO_LOG_LENGTH,&infoLogLength);
    char LinkErrorMessage[(const int )infoLogLength+1];
    glGetProgramInfoLog(ProgramID,infoLogLength,NULL,LinkErrorMessage);
    //    std::cout<<"The state of linking program is: "<<LinkErrorMessage<<std::endl;

    glDeleteShader(VertexShaderID);  //frees the memory and invalidates the name associated with the shader object specified by shader
    glDeleteShader(FragmentShaderID);  // undoes all effect by calling glCreateShader()

    return ProgramID;
}

int initWindow()
{
    if (!glfwInit())
    {
        std::cout<<"glfw init failed!"<<std::endl;
        return -1;
    }
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window=glfwCreateWindow(1200,800, "My SolarSystem!", NULL, NULL);
    if (window==NULL)
    {
        std::cout<<"create window failed!"<<std::endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental=true;
    if (glewInit()!=GLEW_OK)
    {
        std::cout<<"Failed to init glew"<<std::endl;
        return -1;
    }

    glfwGetWindowSize(window,&windowWidth,&windowHeight);

    glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);
//    glEnable(GL_CULL_FACE);

    // to see the orbital clearly , need change the point size
    glEnable(GL_PROGRAM_POINT_SIZE);

    return 0;
}

void input_handle(glm::vec3 position, glm::vec3 &direction, glm::vec3 &velocity)
{
    bool keyPress = false;

    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

    static double old_xpos,old_ypos;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    glm::vec3 rightHand = glm::cross(direction,up);
    // rotate
    if(glfwGetKey(window,GLFW_KEY_U)==GLFW_PRESS)
    {
        up = glm::rotate(up,0.05f,direction);
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_O)==GLFW_PRESS)
    {
        up = glm::rotate(up,-0.05f,direction);
        keyPress = true;
    }

    // change direction
    if(glfwGetKey(window,GLFW_KEY_I)==GLFW_PRESS)
    {
        direction = glm::rotate(direction,0.03f,rightHand);
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_K)==GLFW_PRESS)
    {
        direction = glm::rotate(direction,-0.03f,rightHand);
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_J)==GLFW_PRESS)
    {
        direction = glm::rotate(direction,-0.03f,up);
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_L)==GLFW_PRESS)
    {
        direction = glm::rotate(direction,0.03f,up);
        keyPress = true;
    }

    // change the velocity or center
    if(glfwGetKey(window,GLFW_KEY_W)==GLFW_PRESS)
    {
        velocity += direction * deltaTime;
        position += direction * deltaTime;
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_A)==GLFW_PRESS)
    {
        velocity -= rightHand * deltaTime;
        position -= rightHand * deltaTime;
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_S)==GLFW_PRESS)
    {
        velocity -= direction * deltaTime;
        position -= direction * deltaTime;
        keyPress = true;
    }
    if(glfwGetKey(window,GLFW_KEY_D)==GLFW_PRESS)
    {
        velocity += rightHand * deltaTime;
        position += rightHand * deltaTime;
        keyPress = true;
    }

    if(glfwGetKey(window,GLFW_KEY_SPACE)==GLFW_PRESS)
    {
        paused = ! paused;
    }
    // compute the matrix
    ViewMatrix = glm::lookAt(position-direction, position + direction, up);
    ProjectionMatrix = glm::perspective(45.f,windowWidth/(float)windowHeight,0.1f,100.0f);


    // debug printf info
    if(keyPress)
    {
        cout<<"--------------------------------"<<endl;
        cout << "You are at (" << position.x << " , " << position.y << " , " << position.z << ")" << endl;
        cout<<"\theading to ("<<direction.x<<" , "<<direction.y<<" , "<<direction.z<<")"<<endl;
        cout<<"\t\twith velocity ("<<velocity.x<<" , "<<velocity.y<<" , "<<velocity.z<<")"<<endl;
        cout<<"\tup is ("<<up.x<<" , "<<up.y<<" , "<<up.z<<" )"<<endl;
    }

}

void computeMatricesFromInputs(Planet & player)
{
    glm::vec3 & player_position = player.worldLocation;
    glm::vec3 & velocity = player.velocity;
    glm::vec3 direction;

    static double lastTime = glfwGetTime();
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

    static double old_xpos,old_ypos;
    double xpos, ypos;
    glfwGetCursorPos(window, &xpos, &ypos);


    // 通过鼠标来确定前进角度
    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS)
    {
        horizontalAngle += mouseSpeed * float(old_xpos - xpos);
        verticalAngle   += mouseSpeed * float(old_ypos - ypos );
    }

    // 通过jkli按键来确定前进角度
    if(glfwGetKey (window, GLFW_KEY_J) ==GLFW_PRESS)
    {
        horizontalAngle -= 0.01;
    }

    if(glfwGetKey (window, GLFW_KEY_L) ==GLFW_PRESS)
    {
        horizontalAngle += 0.01;
    }

    if(glfwGetKey (window, GLFW_KEY_I) ==GLFW_PRESS)
    {
        verticalAngle += 0.01;
    }

    if(glfwGetKey (window, GLFW_KEY_K) ==GLFW_PRESS)
    {
        verticalAngle -= 0.01;
    }

    old_xpos=xpos;
    old_ypos=ypos;

    direction = glm::vec3(
            cos(verticalAngle) * sin(horizontalAngle),
            sin(verticalAngle),
            cos(verticalAngle) * cos(horizontalAngle)
    );

    glm::vec3 right = glm::vec3(
            sin(horizontalAngle - 3.14f/2.0f),
            0,
            cos(horizontalAngle - 3.14f/2.0f)
    );


    glm::vec3 up = glm::cross( right, direction );

    if(glfwGetKey(window,GLFW_KEY_U)==GLFW_PRESS)
    {
        up = glm::rotate(up,0.05f,direction);
    }
    if(glfwGetKey(window,GLFW_KEY_O)==GLFW_PRESS)
    {
        up = glm::rotate(up,-0.05f,direction);
    }

    // Move forward
    if (glfwGetKey( window, GLFW_KEY_W ) == GLFW_PRESS )
    {
        velocity += direction * deltaTime * speed;
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
    {
        velocity -= direction * deltaTime * speed;
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
    {
        velocity += right * deltaTime * speed;
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
    {
        velocity -= right * deltaTime * speed;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE ) == GLFW_PRESS)
    {
        glm::vec3 anti_velocity = - velocity;
        if( velocity.length() >= 0.1f )     /// why glm::length(velocity) return glm::vec3????/todo
        {
            velocity *= 0.9;
        }
        else
            velocity = glm::vec3(0,0,0);

    }

    if( glfwGetKey(window, GLFW_KEY_TAB ) == GLFW_PRESS )
    {
        position = glm::vec3( 10, 10, 10 );
        up = glm::vec3(0.0, 1.0, 0.0);
        horizontalAngle = 3.92;
        verticalAngle = -0.79;

    }

    position = player_position - 10.f * direction;
//    cout<<"Now you are at:("<<position.x<<","<<position.y<<","<<position.z<<"),with verticalAngle="<<verticalAngle<<",horizontalAngle"<<horizontalAngle<<endl;


    if(glfwGetKey(window, GLFW_KEY_P ) == GLFW_PRESS )
    {
        paused = (paused)?false:true;
    }

    float FoV = initialFoV ;
    ProjectionMatrix = glm::perspective(FoV, windowWidth/(float)windowHeight, 0.1f, 100.0f);
    ViewMatrix       = glm::lookAt(
            position,           // Camera is here
            position+direction, // and looks here : at the same position, plus "direction"
            up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

}

void draw(Planet & planet)
{
    if(planet.destroyed || planet.isActive== false)
        return;

    int choose=2;

    glm::mat4 MVP=ProjectionMatrix * ViewMatrix * planet.self_ModelMatrix;

    glActiveTexture(GL_TEXTURE0);   //selects which texture unit subsequent texture state calls will affect. The initial value is GL_TEXTURE0.
    glBindTexture(GL_TEXTURE_2D, planet.texture);   //bind a named texture to a texturing target
    glUniform1i(MyTextureSamplerID,0);    //  0-->GL_TEXTURE0
    glUniformMatrix4fv(MVPID,1,GL_FALSE,&MVP[0][0]);
    glUniformMatrix4fv(modelMatrixID,1,GL_FALSE,&planet.self_ModelMatrix[0][0]);
    glUniformMatrix4fv(viewMatrixID,1,GL_FALSE,&ViewMatrix[0][0]);
    glUniform3f(lightPositionID,lightPosition.x,lightPosition.y,lightPosition.z);
    glUniform1i(RenderID,choose);

    // vertex center
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,planet.vertexBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
    // texture uv
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER,planet.uvBuffer);
    glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE,0,(void *)0);
    // vertex normal
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER,planet.normalBuffer);
    glVertexAttribPointer(3,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,planet.elementBuffer);
    glDrawElements(GL_TRIANGLES, (GLuint)planet.indices.size(), GL_UNSIGNED_SHORT, (void *)0);

    if(!planet.isFixedStar)      // draw orbital
    {
        MVP=ProjectionMatrix * ViewMatrix * planet.parent_ModelMatrix;
        glUniformMatrix4fv(MVPID,1,GL_FALSE,&MVP[0][0]);
        glUniformMatrix4fv(modelMatrixID,1,GL_FALSE,&planet.parent_ModelMatrix[0][0]);

        glBindBuffer(GL_ARRAY_BUFFER,planet.orbital_vertexBuffer);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

        glDrawArrays(GL_POINTS,0,(GLuint) planet.orbital_vertices.size());
    }

    glDisableVertexAttribArray(0);   // AttribArray 必须在调用glDrawArrays之后才能关闭
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
}

void handle_collision(Planet & p1, Planet & p2)
{
    static double lastCollisionTime = 0;
    double currentCollisionTIme = glfwGetTime();

    Planet & bigPlanet = p1;
    Planet & smallPlanet = p1;

    if(p1.radius >= p2.radius)   // if equal, let P1 be the bigger one
        smallPlanet = p2;
    else
        bigPlanet = p2;

    //todo handle collision
    if(currentCollisionTIme - lastCollisionTime >= 0.5)
    {
        //todo how to use glm::distance
//        double  distance = glm::distance( (glm::dvec3) p1.worldLocation, (glm::dvec3) p2.worldLocation);
        glm::vec3 temp = p1.worldLocation - p2.worldLocation;
        double distance = temp.length();
        double wholeVolume = radius_to_volume(smallPlanet.radius) + radius_to_volume(bigPlanet.radius);
//        double volume = pow(bigPlanet.radius,3) + pow(smallPlanet.radius,3);

        if(distance <= max(bigPlanet.radius,smallPlanet.radius))
        {
            cout<<"+++++++++++++++++Merge++++++++++++++"<<endl;
            bigPlanet.radius = volume_to_radius(wholeVolume);
            smallPlanet.radius = 0;
            smallPlanet.set_active(false);
        }
        else
        {
            cout<<"-----------Absorption---------------"<<endl;
//            bigPlanet.radius = sqrt(volume / 3 / distance - pow(distance, 2) / 12) + distance / 2;
//            smallPlanet.radius = distance - bigPlanet.radius;
//            bigPlanet.set_radius(sqrt(volume / 3 / distance - pow(distance, 2) / 12) + distance / 2);
//            smallPlanet.set_radius(distance - bigPlanet.radius);

            bigPlanet.set_radius(bigPlanet.radius + 0.01);
            smallPlanet.set_radius(smallPlanet.radius - 0.01);
        }
        //todo set velocity change
        float M1 = pow(bigPlanet.radius, 3);
        float m1 = pow(smallPlanet.radius, 3);
        glm::vec3 v1 = bigPlanet.velocity;
        glm::vec3 v2 = smallPlanet.velocity;
        bigPlanet.velocity = ( (M1-m1) * v1 + 2* m1 * v2) / (M1 + m1);
        smallPlanet.velocity = ( (m1 - M1)*v2 + 2*M1 * v1 )/(M1+m1);
        lastCollisionTime = currentCollisionTIme;

    }

}

int main(int argc, const char * argv[])
{
    if (initWindow()!=0) {
        return -1;
    }

    GLint ProgramID=loadShaders("VertexShader.glsl", "FragmentShader.glsl");
    if (ProgramID==-1)
    {
        cout<<"ProgramID=-1"<<endl;
        return -1;
    }

    glUseProgram(ProgramID);

    MVPID=glGetUniformLocation(ProgramID,"MVP");
    RenderID=glGetUniformLocation(ProgramID,"choose");
    MyTextureSamplerID=glGetUniformLocation(ProgramID,"MyTextureSampler");
    modelMatrixID=glGetUniformLocation(ProgramID,"modelMatrix");
    viewMatrixID=glGetUniformLocation(ProgramID,"viewMatrix");
    lightPositionID=glGetUniformLocation(ProgramID,"lightPosition_worldspace");

    GLuint vertexArray;
    glGenVertexArrays(1,&vertexArray);
    glBindVertexArray(vertexArray);


    string objPath = "./object/ball_hd2.obj";
    string texturePath = "./texture/sun_3000x1500.png";
    Planet sun = Planet(objPath,texturePath,1.5,true);
    texturePath = "./texture/earth_low.png";
    Planet player = Planet(objPath,texturePath,1,false);
    player.set_position(glm::vec3(10,0,0));


    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    cout<<"Earth radius = "<<player.radius<<endl;
    cout<<"sun radius = "<<sun.radius<<endl;
//    int collisionTimes = 0;
    do
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(ProgramID);

        computeMatricesFromInputs(player);

//        cout<<"sun at "<<glm::to_string(sun.worldLocation)<<"\n player at "<<glm::to_string(player.worldLocation)<<endl;
        //check collision
        if(player.check_collison(   sun ))
        {
//            cout<<"Collision occured "<<collisionTimes<<" times!"<<endl;
//            collisionTimes++;
            handle_collision(player,sun);
        }

        sun.update();
        player.update();

        draw(sun);
        draw(player);

        glfwSwapBuffers(window);
        glfwPollEvents();

    }while (glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS && glfwWindowShouldClose(window)==0);

    player.print_info();
    sun.destroy();
    player.destroy();
    glDeleteProgram(ProgramID);
    glDeleteVertexArrays(1,&vertexArray);
    glfwTerminate();

}

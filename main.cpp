#include <fstream>
#include <cstdlib>
#include <vector>
#include <set>
#include <map>
#include <string.h>  // for memcmp()
#include <string>
#include <cstdlib>
#include <cstdio>
#include <memory>

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
#include <list>

#define max(a, b) ((a) >= (b) ? (a) : (b))
#define min(a, b) ((a) <  (b) ? (a) : (b))
#define PI 3.14159265358979323846
#define WORLD_MAX_X (20)
#define WORLD_MIN_X (-20)
#define WORLD_MAX_Y (20)
#define WORLD_MIN_Y (-20)
#define WORLD_MAX_Z (20)
#define WORLD_MIN_Z (-20)
#define radius_to_volume(r) ( pow((r),3) )
#define volume_to_radius(v) ( pow( (v), 1.0/3) )
#define G_CONST 0.0067f       // universal gravitation const G in our game

using namespace std;
using namespace glm;

GLFWwindow * window;
GLint windowWidth;
GLint windowHeight;
GLint ProgramID;
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
float viewDistance = 10.0f;

GLint MVPID;
GLint RenderID;
GLint MyTextureSamplerID;
GLint modelMatrixID;
GLint viewMatrixID;
GLint lightPositionID;

bool paused = true;
bool inChaos = false;

// debug options
bool drawBg = false;
bool noChild = true;
bool applyField = true;

// for background
struct node
{
    glm::vec3 pos;   // z will be 0
    glm::vec3 velocity;   // z will be 0
    glm::vec3 color;
};
typedef struct  node particle;
std::vector<particle> particleSet;
int particleNum=200;
std::vector<glm::vec3> posSet;
std::vector<glm::vec3> colorSet;
std::vector<glm::vec3> velocitySet;
GLuint bg_vertexBuffer;
GLuint bg_colorBuffer;

vector<Planet * > plist;
//set<Planet> plist;
//list<Planet> plist;

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

void mousescroll(GLFWwindow* window, double xoffset, double yoffset)
{
    if (yoffset > 0)
    {
        if(viewDistance <= 45)
            viewDistance += 0.5;

        cout<<"You change viewDistance to "<<viewDistance<<endl;
    }
    else
    {
        if(viewDistance >= 5)
            viewDistance -= 0.5;
        cout<<"You change viewDistance to "<<viewDistance<<endl;
    }
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

    window=glfwCreateWindow(1280,720, "My SolarSystem!", NULL, NULL);
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
    glfwSetScrollCallback(window,mousescroll);

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

void jet(Planet & player,glm::vec3 & direction, glm::vec3 & right ,int buttonCode )
{
    if(player.type != PlayerStar || paused || noChild )
        return ;

    float r = player.radius;
    if(r <= 0.3 )
    {
        cout<<"too small to jet child planet"<<endl;
        return ;
    }

    glm::vec3 v = player.get_velocity();
    glm::vec3 pos = player.get_position();
    float volume = radius_to_volume(r);

    r *= 0.3;
    v *= 2;

    player.set_radius( volume_to_radius( volume - radius_to_volume( r))   );
    float R = player.get_radius();
    Planet * childPlanet = new Planet(r,NormalStar);

    if(buttonCode == 'a')
    {
        v = right * v;
        pos += right * (r + R);
    }
    else if( buttonCode == 'd')
    {
        v = - right * v;
        pos -= right * (r + R);
    }
    else if( buttonCode == 'w')
    {
        pos += direction * (r +R);
    }
    else if( buttonCode == 's')
    {
        v = - v;
        pos -= direction * (r + R);
    }
    else
    {
        cout<<"invalid button directions"<<endl;
    }

    childPlanet->set_velocity(v);
    childPlanet->set_position(pos);

    plist.push_back(childPlanet);
}

void computeMatricesFromInputs(Planet & player)
{
    if(glfwGetKey(window, GLFW_KEY_P ) == GLFW_PRESS )
    {
        paused = ! paused;
    }
    if(glfwGetKey(window,GLFW_KEY_B) == GLFW_PRESS)
    {
        drawBg = ! drawBg;
    }
    if(glfwGetKey(window,GLFW_KEY_N) == GLFW_PRESS)
    {
        noChild = ! noChild;
    }

    glm::vec3  player_position = player.get_position();
    glm::vec3  velocity = player.get_velocity();
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
        if(inChaos)
        {
            velocity -= right * deltaTime * speed;
            jet(player,direction,right,'a');
        }
        else
        {
            velocity += direction * deltaTime * speed;
            jet(player,direction,right,'w');
        }
    }
    // Move backward
    if (glfwGetKey( window, GLFW_KEY_S ) == GLFW_PRESS)
    {
        if(inChaos)
        {
            velocity += right * deltaTime * speed;
            jet(player,direction,right,'d');
        }
        else
        {
            velocity -= direction * deltaTime * speed;
            jet(player,direction,right,'s');
        }
    }
    // Strafe right
    if (glfwGetKey( window, GLFW_KEY_D ) == GLFW_PRESS)
    {
        if(inChaos)
        {
            velocity += direction * deltaTime * speed;
            jet(player,direction,right,'w');
        }
        else
        {
            velocity += right * deltaTime * speed;
            jet(player,direction,right,'d');
        }
    }
    // Strafe left
    if (glfwGetKey( window, GLFW_KEY_A ) == GLFW_PRESS)
    {
        if(inChaos)
        {
            velocity -= direction * deltaTime * speed;
            jet(player,direction,right,'s');
        }
        else
        {
            velocity -= right * deltaTime * speed;
            jet(player,direction,right,'a');
        }
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE ) == GLFW_PRESS && inChaos == false)
    {
        if( velocity.length() >= 0.1f )     /// why glm::length(velocity) return glm::vec3????/todo
        {
            velocity *= 0.9;
        }
        else
        {
            velocity = glm::vec3(0,0,0);
        }
    }

    if( glfwGetKey(window, GLFW_KEY_TAB ) == GLFW_PRESS )
    {
        position = glm::vec3( 10, 10, 10 );
        up = glm::vec3(0.0, 1.0, 0.0);
        horizontalAngle = 3.92;
        verticalAngle = -0.79;

    }

    position = player_position - viewDistance * direction;
//    cout<<"Now you are at:("<<position.x<<","<<position.y<<","<<position.z<<"),with verticalAngle="<<verticalAngle<<",horizontalAngle"<<horizontalAngle<<endl;

    float FoV = initialFoV ;
    ProjectionMatrix = glm::perspective(FoV, windowWidth/(float)windowHeight, 0.1f, 100.0f);
    ViewMatrix       = glm::lookAt(
            position,           // Camera is here
            position+direction, // and looks here : at the same position, plus "direction"
            up                  // Head is up (set to 0,-1,0 to look upside-down)
    );

    if( !paused )
        player.set_velocity(velocity);
}

void draw_background()
{
    typedef vector<glm::vec3> group;
    vector< group> lineSet;
    double cursorPosX,cursorPosY;

    glfwGetCursorPos(window, &cursorPosX, &cursorPosY);
    cursorPosX= (2*cursorPosX - windowWidth) / windowWidth;
    cursorPosY= (windowHeight - 2 * cursorPosY ) / windowHeight;
    // cout<<cursorPosX<<","<<cursorPosY<<endl;
    group cursorGroup;
    glm::vec3 cursorPoint=glm::vec3(cursorPosX,cursorPosY,0);

    cursorGroup.push_back(cursorPoint);
    for(int i=0;i<particleNum;i++){
        if(abs(posSet[i].x-cursorPosX) + abs(posSet[i].y-cursorPosY) <= 0.2)
            cursorGroup.push_back(posSet[i]);
    }
    lineSet.push_back(cursorGroup);

    for(int i=0;i<particleNum;i++){
        // border collision check
        if(std::abs(posSet[i].x+velocitySet[i].x)>1)
            velocitySet[i].x=-velocitySet[i].x;
        if(std::abs(posSet[i].y+velocitySet[i].y)>1)
            velocitySet[i].y=-velocitySet[i].y;
        posSet[i].x=posSet[i].x+velocitySet[i].x;
        posSet[i].y=posSet[i].y+velocitySet[i].y;
        // line them up
        group g;
        g.push_back(posSet[i]);
        for(int j=i+1;j<particleNum;j++){
            if(abs(posSet[i].x-posSet[j].x) + abs(posSet[i].y-posSet[j].y) <= 0.1)
                g.push_back(posSet[j]);
        }
        lineSet.push_back(g);
    }

    glBindBuffer(GL_ARRAY_BUFFER,bg_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, posSet.size() * sizeof(glm::vec3), &posSet[0], GL_STATIC_DRAW);

//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glUseProgram(ProgramID);

    glm::mat4 MVP=glm::mat4(1.0f);

    glUniformMatrix4fv(MVPID,1,GL_FALSE,&MVP[0][0]);
    glUniform1i(RenderID,2);

    // vertex position
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,bg_vertexBuffer);
    glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    // vertex color
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER,bg_colorBuffer);
    glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,0,(void *)0);

    glDrawArrays(GL_POINTS, 0, (GLuint) posSet.size());

    for(int i=0;i<lineSet.size();i++){
        group g=lineSet[i];

        GLuint lineBuffer;
        glGenBuffers(1,&lineBuffer);
        glBindBuffer(GL_ARRAY_BUFFER,lineBuffer);
        glBufferData(GL_ARRAY_BUFFER,g.size() * sizeof(glm::vec3),&g[0],GL_STATIC_DRAW);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,0,(void *)0);
        glDrawArrays(GL_LINE_LOOP,0,(GLuint) g.size());

        glDeleteBuffers(1,&lineBuffer);
    }

    glDisableVertexAttribArray(0);   // AttribArray 必须在调用glDrawArrays之后才能关闭
    glDisableVertexAttribArray(1);
}

void draw(Planet & planet)
{
    if(planet.planetTerminate || planet.isActive == false)
        return ;

    int choose=1;

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

    glDisableVertexAttribArray(0);   // AttribArray 必须在调用glDrawArrays之后才能关闭
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

}

void handle_collision(Planet & p1, Planet & p2)
{
    if( paused )
        return ;


    double currentCollisionTIme = glfwGetTime();
    static double lastCollisionTime = currentCollisionTIme;

    if(p1.isActive== false || p2.isActive== false)
        return ;

    bool flag = p1.get_radius()>=p2.get_radius();

    Planet & bigPlanet = flag?p1:p2;
    Planet & smallPlanet = flag?p2:p1;

    //todo handle collision
    double deltaTime = currentCollisionTIme - lastCollisionTime;
    cout<<"delataTime = "<<deltaTime<<endl;

    //todo how to use glm::distance
//        float  distance = glm::distance( p1.worldLocation,  p2.worldLocation);
    glm::vec3 temp = p1.get_position() - p2.get_position();
    double distance = sqrt( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z );
    double bigVolume = radius_to_volume( bigPlanet.get_radius() );   // volume of bigPlanet
    double smallVolume = radius_to_volume( smallPlanet.get_radius() );  // volume of smallPlanet
    double wholeVolume = bigVolume + smallVolume;
    float M1 = (float) bigVolume;
    float m1 = (float) smallVolume;

    cout<<"P1 is at "<<glm::to_string(p1.get_position())<<"\t P2 is at "<<glm::to_string(p2.get_position())<<endl;
    cout<<"P1 has radius = "<<p1.get_radius()<<"\t P2 has radius = "<<p2.get_radius()<<endl;
    cout<<"Distance between them are:"<<distance<<endl;

    if( distance < max(bigPlanet.radius,smallPlanet.radius))
    {
        cout<<"+++++++++++++++++Merge++++++++++++++"<<endl;
        bigPlanet.set_radius(  volume_to_radius(wholeVolume) );
        smallPlanet.set_radius( 0 );
        smallPlanet.set_active(false);
    }
    else
    {
        cout<<"-----------Absorption---------------"<<endl;

        bigPlanet.set_radius( sqrt( wholeVolume/3/distance - pow(distance,2)/12.0  ) + distance/2  );
        smallPlanet.set_radius(  distance - bigPlanet.get_radius()  );

    }
//    //todo set velocity change
//    float M1 = radius_to_volume(  bigPlanet.get_radius()  );
//    float m1 = radius_to_volume(  smallPlanet.get_radius()  );

    glm::vec3 v1 = bigPlanet.get_velocity();
    glm::vec3 v2 = smallPlanet.get_velocity();

    // if set bigPlanet's velocity , may stuck when absorption
//    bigPlanet.set_velocity( ( M1 * v1 +  m1 * v2) / (M1 + m1) );

    lastCollisionTime = currentCollisionTIme;

}

void field_effect(Planet & p1, Planet & p2)
{
    if( paused == true)
        return ;


    bool flag = (p1.type == CenterStar) || (p1.type == RepulsiveStar) || (p1.type == ChaosStar) || (p1.type == DarkStar);
    Planet & source = flag?p1:p2;
    Planet & target = flag?p2:p1;


    if(source.get_active_state()==false || target.get_active_state() ==false)
    {
//        cout<<"no need check non-active planet"<<endl;
        return ;
    }
    if(source.type != CenterStar && source.type != RepulsiveStar && source.type != ChaosStar && source.type != DarkStar)
    {
//        cout<<"source planet has no filed!"<<endl;
        return ;
    }

    if( (source.type == CenterStar && target.type==RepulsiveStar) || (source.type==RepulsiveStar && target.type == CenterStar) || target.type == ChaosStar )
    {
//        cout<<"CenterStar field and RepulsiveStar field neutralize!"<<endl;
        return ;
    }

    glm::vec3 sourcePos = source.get_position();
    glm::vec3 targetPos = target.get_position();
    float sourceM = radius_to_volume( source.get_radius() );
    float targetM = radius_to_volume( target.get_radius() );
    glm::vec3 temp = sourcePos - targetPos;
    float distance = sqrt( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z );
    glm::vec3 direction = glm::normalize(sourcePos - targetPos);

    if(source.type == ChaosStar && target.type == PlayerStar)
    {
        const glm::vec3 temp = source.get_position() - target.get_position();
        float distance = sqrt( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z );

        inChaos = (distance <= 3* source.get_radius() )? true: false;
    }
    else if(source.type == CenterStar)   // attraction
    {
        target.set_velocity( target.get_velocity() + (G_CONST*sourceM/ (float)pow(distance,2)) * direction  );
    }
    else if(source.type == RepulsiveStar)
    {
        target.set_velocity( target.get_velocity() - (G_CONST*sourceM/ (float)pow(distance,2)) * direction  );   // repulsive
        source.set_velocity( source.get_velocity() +  (G_CONST*targetM/ (float)pow(distance,2)) * direction   );
    }

}

void inside_world(Planet & planet)
{
    if( paused )
        return ;


    glm::vec3 v = planet.get_velocity();
    glm::vec3 t = planet.get_position();
    double r = planet.get_radius();
    float offset = 0.0001f;   // to avoid planet stuck at the boarder
    bool changed = false;

    if( t.x < WORLD_MIN_X + r )
    {
        t.x = WORLD_MIN_X + r + offset;
        v.x = -v.x;
        changed = true;
    }
    if( t.y < WORLD_MIN_Y + r)
    {
        t.y = WORLD_MIN_Y + r + offset;
        v.y = -v.y;
        changed = true;
    }
    if( t.z < WORLD_MIN_Z + r)
    {
        t.z = WORLD_MIN_Z + r + offset;
        v.z = -v.z;
        changed = true;
    }
    if( t.x > WORLD_MAX_X - r)
    {
        t.x = WORLD_MAX_X - r -offset;
        v.x = -v.x;
        changed = true;
    }
    if( t.y > WORLD_MAX_Y - r)
    {
        t.y = WORLD_MAX_Y - r -offset;
        v.y = -v.y;
        changed = true;
    }
    if( t.z > WORLD_MAX_Z - r)
    {
        t.z = WORLD_MAX_Z - r -offset;
        v.z = -v.z;
        changed = true;
    }

    if(changed)
    {
        planet.set_velocity(v);
        planet.set_position(t);
    }
}

int main(int argc, const char * argv[])
{
//    vector<Planet> plist;

    if (initWindow()!=0) {
        return -1;
    }

    ProgramID=loadShaders("VertexShader.glsl", "FragmentShader.glsl");
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

    // create background
    for(int i=0;i<particleNum;i++)
    {
        particle p;
        p.pos=glm::vec3(  2*(rand()%windowWidth-windowWidth/2)/(float)windowWidth, 2*(rand()%windowHeight-windowHeight/2)/(float)windowHeight,  0  );

        if(std::find(posSet.begin(),posSet.end(),p.pos)  !=  posSet.end()){
            i--;
        }
        else{
            p.velocity=glm::vec3(  (rand()%10-5)/(float)5000,  (rand()%10-5)/(float)5000,  0  );
            p.color=glm::vec3( rand()%100/(float)100,rand()%100/(float)100,rand()%100/(float)100);
            posSet.push_back(p.pos);
            colorSet.push_back(p.color);
            velocitySet.push_back(p.velocity);

            particleSet.push_back(p);
        }
    }


    glGenBuffers(1,&bg_vertexBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,bg_vertexBuffer);
    glBufferData(GL_ARRAY_BUFFER, posSet.size() * sizeof(glm::vec3), &posSet[0], GL_STATIC_DRAW);


    glGenBuffers(1,&bg_colorBuffer);
    glBindBuffer(GL_ARRAY_BUFFER,bg_colorBuffer);
    glBufferData(GL_ARRAY_BUFFER,colorSet.size() * sizeof(glm::vec3), &colorSet[0], GL_STATIC_DRAW);

    // create Planet
    Planet::planet_init();
    Planet player = Planet(1,PlayerStar);
    player.set_position(glm::vec3(10,0,0));

//    Planet * p1 = new Planet(2,CenterStar);
//    plist.push_back(p1);
//    Planet * p2 = new Planet(0.9,ChaosStar);
//    p2->set_position(glm::vec3(5,3,0));
//    plist.push_back(p2);


    for(int i=0;i<10;i++)
    {
        Planet * temp = new Planet(0.9,NormalStar);

        glm::vec3 pos = glm::vec3(   random()%WORLD_MAX_X,  random()%WORLD_MAX_Y,   random()%WORLD_MAX_Z   );

        temp->set_position(pos);
        plist.push_back(temp);
    }

    // For speed computation
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    do
    {

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(ProgramID);

        computeMatricesFromInputs(player);

        // remove non-active planet
        vector<Planet *>::iterator iter;
        for( iter = plist.begin(); iter != plist.end(); )
        {
            if ( (*iter)->isActive == false)
            {
                delete * iter;
                iter = plist.erase(iter);
            }
            else
                iter++;
        }

        // apply changes when not pause
        if( paused == false)
        {
            //apply field effect
            if( applyField )
            {
                for(int i=0;i<plist.size();i++)
                {
                    field_effect(*plist[i],player);
                    for(int j=i+1;j<plist.size();j++)
                    {
                        field_effect(*plist[i],*plist[j]);
                    }
                }
            }


/*****        check collision     ***********/
            // check border
            inside_world(player);
            for(int i=0;i<plist.size();i++)
            {
                inside_world(*plist[i]);
            }

            // check between stars
            for(int i=0;i<plist.size();i++)
            {
                if(player.check_collison(*plist[i]))
                    handle_collision(player,*plist[i]);

                for(int j=i+1;j<plist.size();j++)
                {
                    if(plist[i]->check_collison(*plist[j]))
                        handle_collision(*plist[i],*plist[j]);
                }
            }

            // update position
            player.update_position();
            for(int i=0;i<plist.size();i++)
            {
                plist[i]->update_position();
            }
        }

        // draw planet
        draw(player);
        for ( int i=0;i<plist.size();i++)
        {
            draw( * plist[i] );
        }

        if(drawBg)
        {
            draw_background();
        }

        glfwSwapBuffers(window);
        glfwPollEvents();

    }while (glfwGetKey(window, GLFW_KEY_ESCAPE)!=GLFW_PRESS && glfwWindowShouldClose(window)==0);

    player.print_info();
    // free space
    vector<Planet *>::iterator iter;
    for( iter = plist.begin(); iter != plist.end();  )
    {
        delete * iter;
        iter = plist.erase(iter);
    }
    Planet::planet_terminate();



    glDeleteProgram(ProgramID);
    glDeleteBuffers(1,&bg_colorBuffer);
    glDeleteBuffers(1,&bg_vertexBuffer);
    glDeleteVertexArrays(1,&vertexArray);
    glfwTerminate();


}

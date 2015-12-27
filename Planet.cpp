//
// Created by zieng on 11/1/15.
//

#include "Planet.h"


Planet::Planet(double r, PLANET_TYPE t)
{
    if( planetInitOK == false)
    {
        cout<<"Please init the class planet first!"<<endl;
        planet_init();
    }

    // choose the right texture according to the planet type
    switch (t)
    {
        case PlayerStar:
            texture = playerStarTexture;
            break;
        case CenterStar:
            texture = centerStarTexture;
            break;
        case InvisibleStar:
            texture = invisibleStarTexture;
            break;
        case SwallowStar:
            texture = swallowStarTexture;
            break;
        case RepulsiveStar:
            texture = repulsiveStarTexture;
            break;
        case SwiftStar:
            texture = swiftStarTexture;
            break;
        case NutriStar:
            texture = nutriStarTexture;
            break;
        case DarkStar:
            texture = darkStarTexture;
            break;
        case BreatheStar:
            texture = breatheStarTexture;
            break;
        case NormalStar:
            texture = normalStarTexture;
            break;
        case ChaosStar:
            texture = chaosStarTexture;
            break;
        default:
            cout<<endl<<endl<<endl<<endl<<"Undefined!!!!"<<endl<<endl<<endl;
            texture = undefinedTexture;
            break;
    }

    worldLocation = glm::vec3(0,0,0);
    velocity = glm::vec3(0,0,0);
    orientation = glm::vec3(0,0,-1);
    up = glm::vec3(0,1,0);
    lastTime = glfwGetTime();
    planetTerminate = false;
    isActive = true;
    type = t;
    r = (r < 0) ? -r : r;
    radius = ( r>MAX_RADIUS )?MAX_RADIUS:r;
    self_ScaleMatrix = glm::scale(glm::mat4(1.0f),glm::vec3(radius,radius,radius));
    self_ModelMatrix = glm::translate(glm::mat4(1.0f),worldLocation) * self_ScaleMatrix;

    vertexBuffer = share_vertexBuffer;
    uvBuffer = share_uvBuffer;
    normalBuffer = share_normalBuffer;
    elementBuffer = share_elementBuffer;

}

Planet::~Planet()
{
    cout<<"Planet::~Planet--->Destroy a planet"<<endl;
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

void Planet::update_position()
{
    glm::mat4 m = glm::mat4(1.0f);

    update_position(m);
}

void Planet::update_position(const glm::mat4 &parentModelMatrix)
{
    parent_ModelMatrix = parentModelMatrix;
    double currentTime = glfwGetTime();
    float deltaTime = float(currentTime - lastTime);
    lastTime = currentTime;

    if(type == CenterStar)  // center star no need to update
        return;

    worldLocation += 1.f * velocity;

    self_ModelMatrix = parentModelMatrix *  glm::translate(glm::mat4(1.0f),worldLocation) * self_ScaleMatrix;
}

void Planet::set_radius(double r)
{
    if( isActive == false)
        return ;

    r = (r < 0) ? -r : r;

    radius = ( r>MAX_RADIUS )?MAX_RADIUS:r;

    if ( radius < MIN_RADIUS  )  // too small , set active = false
    {
        isActive = false;
        return ;
    }

    self_ScaleMatrix = glm::scale(glm::mat4(1.0f),glm::vec3(radius,radius,radius));
    update_position();
}


glm::mat4 Planet::get_model_matrix()
{
    return self_ModelMatrix;
}

void Planet::set_velocity(glm::vec3 speed)
{
    if(type == CenterStar || isActive == false )
        return ;

    velocity = speed;

}

bool Planet::check_collison(const Planet & planet)
{
    if(isActive == false || planet.isActive == false)
    {
        cout<<"No need check collision for non-Active star"<<endl;
        return false;
    }

    const glm::vec3 temp = worldLocation - planet.worldLocation;

//    double distance = glm::distance(temp ,temp);
//    float distance = temp.length();

//    double distance = pow( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z, 1.0/2);
    double distance = sqrt( temp.x*temp.x   +   temp.y*temp.y  +   temp.z*temp.z );

//    cout<<"current planet has radius="<<radius<<", other planet has radius="<<planet.radius<<endl;
//    cout<<"current planet at "<<glm::to_string(worldLocation)<<", other planet at "<<glm::to_string(planet.worldLocation)<<endl;
//    cout<<"The distance between them are:"<<distance<<endl;
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

double Planet::get_radius()
{
    return radius;
}

glm::vec3 Planet::get_position()
{
    return worldLocation;
}

glm::vec3 Planet::get_velocity()
{
    return velocity;
}

bool Planet::get_active_state()
{
    return isActive;
}


// init static members
bool Planet::planetInitOK = false;
bool Planet::planetTerminate = false;
GLuint Planet::playerStarTexture   = 0;
GLuint Planet::centerStarTexture   = 0;
GLuint Planet::normalStarTexture   = 0;
GLuint Planet::repulsiveStarTexture= 0;
GLuint Planet::invisibleStarTexture= 0;
GLuint Planet::swiftStarTexture    = 0;
GLuint Planet::swallowStarTexture  = 0;
GLuint Planet::nutriStarTexture    = 0;
GLuint Planet::darkStarTexture     = 0;
GLuint Planet::chaosStarTexture    = 0;
GLuint Planet::breatheStarTexture  = 0;
GLuint Planet::undefinedTexture    = 0;

vector<unsigned short > Planet::share_indices(1)   ;
vector<glm::vec3> Planet::share_vertices(1)        ;
vector<glm::vec2> Planet::share_uvs(1)             ;
vector<glm::vec3> Planet::share_normals(1)         ;

GLuint Planet::share_uvBuffer = 0;
GLuint Planet::share_normalBuffer = 0;
GLuint Planet::share_elementBuffer = 0;
GLuint Planet::share_vertexBuffer = 0;
#include <iostream>
#include <list>
#include <GL/glut.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <ctime>

using namespace std;

//initial window size/camera viewing angle
int width = 800;
int height = 600;
float vangle;// = atan((double)height/1600)*180/M_PI;

//lighting parameters
float lpos[] = {-50.0, 50.0, 0.0, 0.0};
float scolor[] = {1.0,1.0,1.0,1.0};
float mcolor[] = {0.5,0.5,0.5,1.0};
float ambient[] = {0.2,0.2,0.2,1.0};
float diffuse[] = {0.8,0.8,0.8,1.0};
float nospec[] = {0.0,0.0,0.0,1.0};
float mat_shininess[] = { 50.0 };

//OpenGL objects
GLuint back;
GLuint ramp;
GLuint slist;
GLUquadricObj *qobj;

//initial game state
float angle = M_PI/2;
float fangle = 0.0;
float jumph = 0.0;
float jumpa = 1.0;
float situp = 0.0;
float xvelocity=0.0,zvelocity=0.0;
int tim,delta,base=0;
int status = 0;
int cstat = 0;
int m_x = 0;
int m_y = 600;
float dist_tot = 0.0;
float fast_mode = 1.0;
float hard_mode = 1.0;
bool nosnowman = true;
bool kill = false;
bool alive = true;
float snowj = 0.0;
float bottomsup = 1.0;
bool restart = true;

//terrain elements
class terrain{
public:
    terrain(float xval,int d):x(xval),y(-50),z(-1400),dl(d) {}
    terrain(float xval,float zval,int d):x(xval),y(0),z(zval),dl(d) {}
    //void move(float,float,float);
    bool operator==(const terrain& r)const{
        return z >= r.z;
    }
    void print(float);
    int stat();
    float x;
    float y;
    float z;
    int dl; //terrain type (0-6)
};

//terrain position update
void terrain::print(float d) {

    //abominable snowman comes to eat skier
    if(dl==6 && kill) {
        x *= 0.9;
        z *= 0.9;
    }
    //other other terrain elements moving normally
    else{
        x += fast_mode*d*xvelocity/5;
        z += fast_mode*d*zvelocity/5;
    }
    
    //update y component for terrain that is "below the horizon"
    if(y < 0.0) {
        y = (z+1000.0)/8;
        if(y > 0.0)y = 0.0;
    }

    //translate function call, with extra jump y movement for snowman only
    glTranslatef(x, (dl==6?y+snowj:y), z);
    glCallList(slist+dl);

}

//terrain collision detection
int terrain::stat() {
    if(fabs(x)<6.0 && fabs(z)<6.0)return dl+1;
    else return 0;
}

//list of terrain elements, plus threshold instance for comparison
list<terrain> s_trees;
list<terrain>::iterator iter;
terrain threshold(0.0, 50.0, 0);

//declarations for skier states/animations
void uncrash(int);
void fall(int);
void land(int);
void jump();
void crash();
void hijump();
void sjump(int);
void landhi(int);
void feast();
void die(int);
void movement(int, int);

//skier arm
void drawArm(float a) {
    gluCylinder(qobj,0.6,0.6,2.0,8,4);
    
    glPushMatrix();  //elbow
      glTranslatef(0.0,0.0,2.0);
      glutSolidSphere(0.6,8,8);
      
      glPushMatrix();  //elbow joint
        glRotatef(60.0+60.0*cos(angle)*jumpa,-1.0,0.0,0.0);
        gluCylinder(qobj,0.6,0.6,2.0,8,4);
        
        glPushMatrix();
          glTranslatef(0.0,0.0,2.0);
          glColor3f(1.0,0.75,0.5);
          glutSolidSphere(0.6,8,8);
          
          glPushMatrix();
            glTranslatef(0.0,0.0,0.4);
            glRotatef(30.0*cos(angle)+(1.0-jumpa)*60.0,0.0,a,0.0);
            glRotatef(90.0+60.0*cos(angle),1.0,0.0,0.0);
            glColor3f(0.0,0.0,0.0);
            gluCylinder(qobj,0.05,0.05,8.0,4,1);
            glTranslatef(0.0,0.0,7.5);
            gluDisk(qobj,0.4,0.5,8,1);
          glPopMatrix();
        glPopMatrix();
      glPopMatrix();
    glPopMatrix();
}

//skier ski
void drawSki() {
    glColor3f(0.8,0.4,0.4);
    glBegin(GL_TRIANGLE_STRIP);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(-0.5,0.0,5.0);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(0.5,0.0,5.0);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(-0.5,0.0,-5.0);
      glNormal3f(0.0,1.0,0.0);
      glVertex3f(0.5,0.0,-5.0);
      glNormal3f(0.0,1.0,1.0);
      glVertex3f(0.0,1.0,-6.0);
    glEnd();
    glColor3f(0.0,0.8,0.4);
    glBegin(GL_QUADS);
      glNormal3f(-1.0,0.0,1.0);
      glVertex3f(-0.5,0.0,0.5);
      glNormal3f(-1.0,0.0,1.0);
      glVertex3f(-0.5,1.0,0.5);
      glNormal3f(-1.0,1.0,-1.0);
      glVertex3f(-0.5,1.0,-0.5);
      glNormal3f(-1.0,1.0,-1.0);
      glVertex3f(-0.5,0.0,-1.5);
      glNormal3f(-1.0,0.0,1.0);
      glVertex3f(-0.5,0.0,0.5);
      glNormal3f(-1.0,0.0,1.0);
      glVertex3f(-0.5,1.0,0.5);
      glNormal3f(1.0,0.0,1.0);
      glVertex3f(0.5,1.0,0.5);
      glNormal3f(1.0,0.0,1.0);
      glVertex3f(0.5,0.0,0.5);
      glNormal3f(1.0,0.0,1.0);
      glVertex3f(0.5,0.0,0.5);
      glNormal3f(1.0,0.0,1.0);
      glVertex3f(0.5,1.0,0.5);
      glNormal3f(1.0,1.0,-1.0);
      glVertex3f(0.5,1.0,-0.5);
      glNormal3f(1.0,1.0,-1.0);
      glVertex3f(0.5,0.0,-1.5);
      glNormal3f(1.0,1.0,-1.0);
      glVertex3f(0.5,0.0,-1.5);
      glNormal3f(1.0,1.0,-1.0);
      glVertex3f(0.5,1.0,-0.5);
      glNormal3f(-1.0,1.0,-1.0);
      glVertex3f(-0.5,1.0,-0.5);
      glNormal3f(-1.0,1.0,-1.0);
      glVertex3f(-0.5,0.0,-1.5);
    glEnd();
}

//whole skier
void drawSkier() {

    glPushMatrix(); //draw skis and boots
      
      glRotatef(angle*180/M_PI,0.0,1.0,0.0);
      
      glRotatef(fangle*90.0*bottomsup,-1.0,0.0,0.0);
      
      glPushMatrix();
      glTranslatef(-1.0,6.0,fangle*0.8);
      glRotatef(45.0*(1.0-jumpa),0.0,0.0,-1.0);
      glTranslatef(0.0,-6.0,0.0);
      
      glRotatef(-fangle*30.0,0.0,1.0,0.0);
      glRotatef(15.0*sin(angle),0.0,0.0,1.0);
      drawSki();
      glPopMatrix();
      
      glPushMatrix();
      glTranslatef(1.0,6.0,fangle*0.8);
      glRotatef(45.0*(1.0-jumpa),0.0,0.0,1.0);
      glTranslatef(0.0,-6.0,0.0);
      
      glRotatef(fangle*30.0,0.0,1.0,0.0);
      glRotatef(15.0*sin(angle),0.0,0.0,1.0);
      drawSki();
      glPopMatrix();
      
    glPopMatrix();

    glPushMatrix(); //draw skier
      
        glTranslatef(0.0,0.8,0.0);
        glRotatef(jumpa*(30.0*cos(angle)+60.0)+(1.0-jumpa)*90.0,-1.0,0.0,0.0);
        glRotatef(angle*180/M_PI,0.0,0.0,1.0);
        
        glRotatef(fangle*90.0*bottomsup-situp*20.0,-1.0,0.0,0.0);
      
        glColor3f(0.0,0.0,0.8);
      
        glPushMatrix(); //right shin
        glTranslatef(1.0-(1.0-jumpa)*0.5,0.0,6.0);
        glRotatef(45.0*(1.0-jumpa),0.0,-1.0,0.0);
        glTranslatef(0.0,0.0,-6.0);
        glRotatef(-7.0*jumpa,0.0,1.0,0.0);
        gluCylinder(qobj,1.0,0.6,4.0,6,3);
        glPopMatrix();
      
        glPushMatrix(); //left shin
        glTranslatef((1.0-jumpa)*0.5-1.0,0.0,6.0);
        glRotatef(45.0*(1.0-jumpa),0.0,1.0,0.0);
        glTranslatef(0.0,0.0,-6.0);
        glRotatef(7.0*jumpa,0.0,1.0,0.0);
        gluCylinder(qobj,1.0,0.6,4.0,6,3);
        glPopMatrix();
      
        glPushMatrix(); //knee joint
            
            glTranslatef(0.0,0.0,4.0);
            glRotatef(jumpa*90.0*cos(angle),1.0,0.0,0.0);
            
            glPushMatrix(); //right knee
            glTranslatef(0.5,0.0,2.0);
            glRotatef(45.0*(1.0-jumpa),0.0,-1.0,0.0);
            glTranslatef(0.0,0.0,-2.0);
            glutSolidSphere(0.6,8,8);
            glPopMatrix();
            
            glPushMatrix(); //left knee
            glTranslatef(-0.5,0.0,2.0);
            glRotatef(45.0*(1.0-jumpa),0.0,1.0,0.0);
            glTranslatef(0.0,0.0,-2.0);
            glutSolidSphere(0.6,8,8);
            glPopMatrix();
            
            glPushMatrix(); //left thigh
            glTranslatef(-0.5,0.0,2.0);
            glRotatef(45.0*(1.0-jumpa),0.0,1.0,0.0);
            glTranslatef(0.0,0.0,-2.0);//////////////////////change
            gluCylinder(qobj,0.6,0.6,2.0,6,3);
            glPopMatrix();
            
            glPushMatrix(); //right thigh
            glTranslatef(0.5,0.0,2.0);
            glRotatef(45.0*(1.0-jumpa),0.0,-1.0,0.0);
            glTranslatef(0.0,0.0,-2.0);//////////////////////change
            gluCylinder(qobj,0.6,0.6,2.0,6,3);
            glPopMatrix();
            
            glPushMatrix(); //hips
            glTranslatef(0.0,0.0,2.4);
            glScalef(1.0,0.7,0.7);
            glutSolidSphere(1.3,10,10);
            glPopMatrix();
            
            glPushMatrix(); //hip joint
                
                glTranslatef(0.0,0.0,2.4);
                glRotatef((jumpa+situp)*90.0*cos(angle),-1.0,0.0,0.0);//////////change
                
                glPushMatrix(); //mid torso
                glScalef(1.0,0.7,1.0);
                gluCylinder(qobj,1.3,1.3,3.5,8,4);
                glPopMatrix();
                
                glPushMatrix(); //upper torso
                glTranslatef(0.0,0.0,3.5);
                glScalef(1.0,0.7,0.7);
                glutSolidSphere(1.3,10,10);
                glPopMatrix();
                
                glColor3f(0.8,0.0,0.4);
                
                glPushMatrix(); //left shoulder
                glTranslatef(-1.0,0.0,3.5);
                glutSolidSphere(0.6,8,8);
                glPopMatrix();
                
                glPushMatrix(); //right shoulder
                glTranslatef(1.0,0.0,3.5);
                glutSolidSphere(0.6,8,8);
                glPopMatrix();
                
                glPushMatrix(); //shoulder joint
                    
                    glTranslatef(0.0,0.0,3.5);
                    glRotatef(60.0*cos(angle),-1.0,0.0,0.0);
                    
                    glPushMatrix(); //left arm
                    glTranslatef(-1.0,0.0,0.0);
                    glRotatef(150.0-(1.0-jumpa)*50.0,0.0,-1.0,0.0);
                    drawArm(1.0);
                    glPopMatrix();
                    
                    glColor3f(0.8,0.0,0.4);
                    
                    glPushMatrix(); //right arm
                    glTranslatef(1.0,0.0,0.0);
                    glRotatef(150.0-(1.0-jumpa)*50.0,0.0,1.0,0.0);
                    drawArm(-1.0);
                    glPopMatrix();
                    
                glPopMatrix();
                
                glColor3f(1.0,0.75,0.5);
                
                glPushMatrix(); //head
                glTranslatef(0.0,0.0,5.5);
                glPushMatrix();
                    glScalef(1.0,1.0,1.2);
                    glutSolidSphere(1.2,8,8);
                glPopMatrix();
                glPushMatrix(); //nose
                    glTranslatef(0.0,1.1,-0.3);
                    glBegin(GL_TRIANGLE_FAN);
                    glNormal3f(0.0,1.0,0.0);
                    glVertex3f(0.0,0.3,0.0);
                    glNormal3f(-1.0,1.0,0.0);
                    glVertex3f(-0.3,0.0,0.0);
                    glNormal3f(0.0,1.0,0.0);
                    glVertex3f(0.0,0.0,0.5);
                    glNormal3f(1.0,1.0,0.0);
                    glVertex3f(0.3,0.0,0.0);
                    glEnd();
                glPopMatrix();
                glColor3f(0.0,0.0,0.0);
                glPushMatrix(); //sunglasses left ear
                    glTranslatef(-0.92,1.15,0.3);
                    glRotatef(12.0,0.0,0.0,-1.0);
                    glBegin(GL_POLYGON);
                    glNormal3f(-1.0,0.0,0.0);
                    glVertex3f(0.0,0.0,-0.1);
                    glNormal3f(-1.0,0.0,0.0);
                    glVertex3f(0.0,0.0,0.1);
                    glNormal3f(-1.0,0.0,0.0);
                    glVertex3f(0.0,-1.2,0.1);
                    glNormal3f(-1.0,0.0,0.0);
                    glVertex3f(0.0,-1.2,-0.1);
                    glEnd();
                glPopMatrix();
                glPushMatrix(); //sunglasses right ear
                    glTranslatef(0.92,1.15,0.3);
                    glRotatef(12.0,0.0,0.0,1.0);
                    glBegin(GL_POLYGON);
                    glNormal3f(1.0,0.0,0.0);
                    glVertex3f(0.0,0.0,-0.1);
                    glNormal3f(1.0,0.0,0.0);
                    glVertex3f(0.0,0.0,0.1);
                    glNormal3f(1.0,0.0,0.0);
                    glVertex3f(0.0,-1.2,0.1);
                    glNormal3f(1.0,0.0,0.0);
                    glVertex3f(0.0,-1.2,-0.1);
                    glEnd();
                glPopMatrix();
                glScalef(1.0,1.0,0.8);
                glPushMatrix(); //left sunglass
                    glTranslatef(-0.5,1.15,0.3);
                    glRotatef(90.0,1.0,0.0,0.0);
                    gluDisk(qobj,0.0,0.5,8,1);
                glPopMatrix();
                glPushMatrix(); //right sunglass
                    glTranslatef(0.5,1.15,0.3);
                    glRotatef(90.0,1.0,0.0,0.0);
                    gluDisk(qobj,0.0,0.5,8,1);
                glPopMatrix();
                glPopMatrix();
                
                glColor3f(0.9,0.0,0.0);
                
                glPushMatrix(); //cap
                    glTranslatef(0.0,-0.16,6.0);
                    glRotatef(30.0,1.0,0.0,0.0);
                    gluCylinder(qobj,1.2,0.5,1.5,8,3);
                    
                    glPushMatrix();
                    glTranslatef(0.0,0.0,1.4);
                    glutSolidSphere(0.5,8,8);
                    glRotatef(120.0-60.0*cos(angle),1.0,0.0,0.0);
                    glutSolidCone(0.53,3.0,8,3);
                    glColor3f(0.0,0.0,0.8);
                    glTranslatef(0.0,0.0,3.0);
                    glutSolidSphere(0.3,8,8);
                    glPopMatrix();
                    
                glPopMatrix();
                
            glPopMatrix();
            
        glPopMatrix();
      
    glPopMatrix();
}

//terrain type 1
void largeTree() {
    glColor3f(0.8,0.4,0.4);
    glRotatef(60.0,-1.0,0.0,0.0);
    gluCylinder(qobj,3.0,2.0,12.0,6,2);
    glColor3f(0.0,0.7,0.1);
    glTranslatef(0.0,0.0,12.0);
    gluCylinder(qobj,9.0,6.0,18.0,8,3);
    glTranslatef(0.0,0.0,18.0);
    glutSolidCone(6.0,18.0,8,3);
}

//type 2
void smallTree() {
    glColor3f(0.8,0.4,0.4);
    glRotatef(60.0,-1.0,0.0,0.0);
    gluCylinder(qobj,3.0,2.0,10.0,6,2);
    glColor3f(0.0,0.9,0.0);
    glTranslatef(0.0,0.0,7.0);
    glutSolidCone(8.0,18.0,8,3);
}

//type 3
void mogul() {
    glColor3f(1.0,1.0,1.0);
    glLightfv(GL_LIGHT0, GL_AMBIENT, mcolor);
    glTranslatef(0.0,-0.7,0.0);
    glScalef(1.0,0.7,1.0);
    glutSolidSphere(7.0,10,10);
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
}

//type 4
void rock() {
    glColor3f(0.7,0.7,0.7);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, nospec);
    glutSolidSphere(7.0,10,10);
    //glMaterialfv(GL_FRONT, GL_SPECULAR, scolor);
}

//type 5
void stump() {
    glColor3f(0.8,0.4,0.4);
    glRotatef(60.0,-1.0,0.0,0.0);
    gluCylinder(qobj,5.0,4.0,7.0,6,2);
    glTranslatef(0.0,0.0,7.0);
    gluDisk(qobj,0.0,4.0,6,1);
}

//type 6 (rainbow ramps)
void dramp() {
    glColor3f(1.0,1.0,1.0);
    glRotatef(30.0,-1.0,0.0,0.0);
    glEnable(GL_TEXTURE_2D);
    glBindTexture( GL_TEXTURE_2D, ramp );
    glBegin( GL_QUADS );
    glTexCoord2d(0.0,0.0); glVertex3f(7.0,3.0,0.0);
    glTexCoord2d(1.0,0.0); glVertex3f(-7.0,3.0,0.0);
    glTexCoord2d(1.0,1.0); glVertex3f(-7.0,0.0,0.0);
    glTexCoord2d(0.0,1.0); glVertex3f(7.0,0.0,0.0);
    glEnd();
    glDisable(GL_TEXTURE_2D);
}

//terrain type 7 (abominable snowman)
void monster() {

    glLightfv(GL_LIGHT0, GL_AMBIENT, mcolor);
    glColor3f(0.5,0.5,0.5);
    glRotatef(30.0,1.0,0.0,0.0);
    glTranslatef(0.0,13.0,0.0);
    
    glPushMatrix();
    glScalef(1.0,1.6,0.7);
    glutSolidSphere(6.0,20,20);
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(0.0,11.0,1.0);
    glRotatef(15.0,-1.0,0.0,0.0);
    glScalef(1.0,1.4,0.8);
    glutSolidSphere(3.0,20,20);
      glPushMatrix();
        glTranslatef(0.5,0.7,2.7);
        glColor3f(0.0,0.0,0.0);
        glBegin(GL_TRIANGLES);
        glVertex3f(-0.5,0.3,0.0);
        glVertex3f(2.0,1.3,0.0);
        glVertex3f(2.0,1.8,0.0);
        glEnd();
        glColor3f(1.0,0.0,0.0);
        glutSolidSphere(0.3,4,4);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(-0.5,0.7,2.7);
        glColor3f(0.0,0.0,0.0);
        glBegin(GL_TRIANGLES);
        glVertex3f(0.5,0.3,0.0);
        glVertex3f(-2.0,1.3,0.0);
        glVertex3f(-2.0,1.8,0.0);
        glEnd();
        glColor3f(1.0,0.0,0.0);
        glutSolidSphere(0.3,4,4);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0,-1.0,3.0);
        glColor3f(1.0,1.0,0.0);
        glBegin(GL_TRIANGLES);
        glVertex3f(-2.0,1.0,0.0);
        glVertex3f(-1.5,-0.5,0.0);
        glVertex3f(-1.0,0.5,0.0);
        glVertex3f(-1.0,0.5,0.0);
        glVertex3f(-0.5,-1.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.0,0.0,0.0);
        glVertex3f(0.5,-1.0,0.0);
        glVertex3f(1.0,0.5,0.0);
        glVertex3f(1.0,0.5,0.0);
        glVertex3f(1.5,-0.5,0.0);
        glVertex3f(2.0,1.0,0.0);
        glEnd();
      glPopMatrix();
    glPopMatrix();
    
    glColor3f(0.0,0.0,0.0);
    
    glPushMatrix();
    glTranslatef(3.0,-8.0,0.0);
    glRotatef(7.0,0.0,0.0,1.0);
    glRotatef(90.0,1.0,0.0,0.0);
    gluCylinder(qobj,0.1,0.1,5.0,4,1);
      glPushMatrix();
        glTranslatef(0.0,0.0,3.5);
        glRotatef(30.0,0.0,1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0,0.0,3.5);
        glRotatef(30.0,0.0,-1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-3.0,-8.0,0.0);
    glRotatef(7.0,0.0,0.0,-1.0);
    glRotatef(90.0,1.0,0.0,0.0);
    gluCylinder(qobj,0.1,0.1,5.0,4,1);
      glPushMatrix();
        glTranslatef(0.0,0.0,3.5);
        glRotatef(30.0,0.0,1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0,0.0,3.5);
        glRotatef(30.0,0.0,-1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(-3.0,8.0,0.0);
    glRotatef(20.0,0.0,0.0,1.0);
    glRotatef(90.0,-1.0,0.0,0.0);
    gluCylinder(qobj,0.1,0.1,7.0,4,1);
      glPushMatrix();
        glTranslatef(0.0,0.0,5.5);
        glRotatef(30.0,0.0,1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0,0.0,5.5);
        glRotatef(30.0,0.0,-1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
    glPopMatrix();
    
    glPushMatrix();
    glTranslatef(3.0,8.0,0.0);
    glRotatef(45.0,0.0,0.0,-1.0);
    glRotatef(90.0,-1.0,0.0,0.0);
    gluCylinder(qobj,0.1,0.1,7.0,4,1);
      glPushMatrix();
        glTranslatef(0.0,0.0,5.5);
        glRotatef(30.0,0.0,1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
      glPushMatrix();
        glTranslatef(0.0,0.0,5.5);
        glRotatef(30.0,0.0,-1.0,0.0);
        gluCylinder(qobj,0.1,0.1,1.5,4,1);
      glPopMatrix();
    glPopMatrix();
    glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);

}

//game loop callback rendering each frame
void display(void) {

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); /*clear the window */

    //glLightfv(GL_LIGHT0, GL_POSITION, lpos); //put this in main()

    glMaterialfv(GL_FRONT, GL_SPECULAR, scolor);
    glMaterialfv(GL_FRONT, GL_SHININESS, mat_shininess);

    //snow layer polygon
    glPushMatrix();
      glColor3f(1.0,1.0,1.0);
      glBegin( GL_QUADS );
      glNormal3f(-1.0,1.0,0.0);
      glVertex3f(-600.0,0.0,-1000.0);
      glNormal3f(-1.0,1.0,0.0);
      glVertex3f(600.0,0.0,-1000.0);
      glNormal3f(-1.0,1.0,0.0);
      glVertex3f(600.0,0.0,50.0);
      glNormal3f(-1.0,1.0,0.0);
      glVertex3f(-600.0,0.0,50.0);
      glEnd();
    glPopMatrix();
    
    //background texture
    glPushMatrix();
      glEnable(GL_TEXTURE_2D);
      glBindTexture( GL_TEXTURE_2D, back );
      glBegin( GL_QUADS );
      glTexCoord2d(0.0,0.0); glVertex3f(750.0,400.0,-1400.0);
      glTexCoord2d(1.0,0.0); glVertex3f(-750.0,400.0,-1400.0);
      glTexCoord2d(1.0,1.0); glVertex3f(-750.0,-250.0,-1400.0);
      glTexCoord2d(0.0,1.0); glVertex3f(750.0,-250.0,-1400.0);
      glEnd();
      glDisable(GL_TEXTURE_2D);
    glPopMatrix();
    
    //draw skier normally at (x,y,z) = (0.0,0.1,0.0) if not getting eaten
    if(alive) {
        glPushMatrix();
          glTranslatef(0.0,0.1+jumph,0.0); //add jump position
          drawSkier();
        glPopMatrix();
    }

    //get delta time since last call to display()
    tim = glutGet(GLUT_ELAPSED_TIME);
    delta = tim - base;

    int hit;
    for(iter=s_trees.begin(); iter!=s_trees.end(); iter++) {

        //update each terrain element
        glPushMatrix();
        iter->print(delta);
        glPopMatrix();

        //terrain element returns its type if colliding (otherwise 0)
        hit = iter->stat();

        //collide only while moving (unless terrain type 7--snowman)
        if(hit>0 && fabs(xvelocity)>0.01 && zvelocity>0.01 || hit==7) {

            //change skier state
            switch(status) {
                case 0: //skiing normally
                    if(hit==3)jump();
                    else if(hit==6)hijump();
                    else if(hit==7)feast();
                    else crash();
                    break;
                case 1: //crashing
                    if(hit==7)feast();
                    break;
                case 2: //jumping
                    if(hit<3)crash();
                    else if(hit==4 || hit ==5)hijump();
                    else if(hit==7)feast();
                    break;
                case 3: //hi-jump (only collide with large tree)
                    if(hit<2)crash();
                default: break;
            }

        }
    }
    //remove terrain that moves beyond threshold (set to 50 units behind skier)
    s_trees.remove(threshold);
    
    dist_tot += fast_mode*delta*zvelocity/5;
    base = tim;

    //cout<<status<<"\r";
    //cout<<(int)dist_tot/10<<"\r";
    if(dist_tot>19750.0)kill=true;
    
    glutSwapBuffers();
}

/*-------------------------------------------------*/
/*    skier state changes & animation functions    */
/*-------------------------------------------------*/

void feast() {
    xvelocity = 0.0;
    zvelocity = 0.0;
    status = 4;
    fangle = 1.0;
    bottomsup = 2.0;
    jumpa = 0.0;
    jumph = 30.0;
    glutTimerFunc(10,die,0);
}

void die(int v) {
    if(v<150) {
        jumph -= 0.1;
        glutTimerFunc(10,die,v+1);
    }
    else alive = false;
}

void crash() {
    cstat = status;
    if(cstat == 0)cstat=1; //if crashing from normal skiing cstat=1, else cstat=2 (from jump)
    fangle = 1.0;
    status = 1;
    jumpa = 0.0;
    glutTimerFunc(10,fall,0);
}

void hijump() {
    cstat = 10;
    if(status == 0)cstat=0; //if hijumping from... see above
    zvelocity*=1.5;
    xvelocity*=1.5;
    status = 3;
    jumpa = 0.0;
    glutTimerFunc(10,landhi,cstat);
}

void landhi(int v) {
    if(status!=3)return;
    if(v<100) {
        jumph = 25.0*sin(M_PI*(float)v/100);
        glutTimerFunc(5,landhi,v+1);
    }
    else{
        status = 0;
        jumpa = 1.0;
        jumph = 0.0;
        movement(m_x,m_y);
    }
}

void jump() {
    movement(m_x,m_y);
    zvelocity*=1.3;
    xvelocity*=1.3;
    fangle = 0.0;
    status = 2;
    jumpa = 0.0;
    glutTimerFunc(10,land,0);
}

void fall(int v) {
    if(v<25*cstat) {
        jumph = (float)(cstat)*5.0*cos(M_PI*(float)v/(50*cstat));
        glutTimerFunc(10/cstat,fall,v+1);
    }
    else{
        fangle = -1.0;
        situp = 1.0;
        xvelocity = 0.0;
        zvelocity = 0.0;
        glutTimerFunc(1000,uncrash,0);
    }
}

void uncrash(int v) {
    status = 0;
}

void land(int v) {
    if(status != 2)return;
    if(v<40) {
        jumph = 8.0*sin(M_PI*(float)v/40);
        glutTimerFunc(10,land,v+1);
    }
    else{
        status = 0;
        jumpa = 1.0;
        jumph = 0.0;
        movement(m_x,m_y);
    }
}

void sjump(int v) {
    if(restart);
    else if(snowj==0.0) {
        snowj=7.0;
        glutTimerFunc(150,sjump,1);
    }
    else{
        snowj=0.0;
        glutTimerFunc(1000,sjump,1);
    }
}

/*    end skier state changes & animation functions    */

//adding initial terrain elements
void init_populate() {

    srand((unsigned)time(NULL));
    rand();

    //iterate through x values (left-right) and add random terrain element in random y position (forward-back)
    float num;
    for(int c=-1600;c<1600;c+=20) {
        num = (float)rand();
        s_trees.push_back(*(new terrain((float)c,-1000.0*num/RAND_MAX,(int)num%6)));
    }

}

//adding more random terrain at regular intervals
void rand_populate(int v) {

    //add new random terrain element (more likely for faster skier)
    float num;
    num = (float)rand();
    if(num/RAND_MAX < zvelocity) {
        s_trees.push_back(*(new terrain(2800.0*num/RAND_MAX-1400.0,(int)num%6)));
    }

    //add abominable snowman terrain element after certain distance
    if(dist_tot>18600.0 && nosnowman) {
        s_trees.push_back(*(new terrain(0.0,6)));
        restart = false;
        glutTimerFunc(1000,sjump,1);
        nosnowman = false;
    }

    //call this function again
    glutTimerFunc(50/((int)(fast_mode*hard_mode)),rand_populate,0);

}

/*----------------------------*/
/*    user input callbacks    */
/*----------------------------*/

void changeSize(int w, int h) {

    width = w;
    height = h;
    vangle = atan((double)h/1600)*180/M_PI;
    //don't divide by zero down there
    if(h==0)h=1;
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glViewport(0, 0, w, h);
    //viewing angle corresponds to height of window
    gluPerspective(2*vangle, (float)w/h, 1.0, 6400.0);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    //        position        face point     up vector
    gluLookAt(0.0,30.0,75.0,  0.0,20.0,0.0,  0.0,1.0,0.0); //camera facing negative z direction

}

void mouse(int button, int state, int x, int y) {
    if(button == GLUT_LEFT_BUTTON && status==0) {
        if(state == GLUT_DOWN) {
            jump();
        }
    }
    //else if(button==3)cam_z-=5.0;
    //else if(button==4)cam_z+=5.0;
}

void movement(int x, int y) {
    
    m_x = x;
    m_y = y;
    
    if(status>0)return;
    
    situp = 0.0;
    fangle = 0.0;
    jumpa = 1.0;
    
    if(y>=500) {
      if(x>400)angle = -M_PI/2;
      else angle = M_PI/2;
    }
    else angle = -atan((float)(x-400)/(-y+500));
    
    float i = (cos(angle)<0.5?M_PI/3:angle);
    xvelocity = cos(i*3.0/2.0)*sin(angle);
    zvelocity = cos(i*3.0/2.0)*cos(angle);

}

void amovement(int x, int y) {
    movement(x, y);
}

void special(int key, int x, int y) {
    if(key==2) {
        angle = M_PI/2;
        fangle = 0.0;
        jumph = 0.0;
        jumpa = 1.0;
        situp = 0.0;
        xvelocity=0.0;
        zvelocity=0.0;
        status = 0;
        cstat = 0;
        m_x = 0;
        m_y = 600;
        dist_tot = 0.0;
        fast_mode = 1.0;
        hard_mode = 1.0;
        nosnowman = true;
        kill = false;
        alive = true;
        snowj = 0.0;
        bottomsup = 1.0;
        restart = true;
        s_trees.clear();
        init_populate();
    }
}

void keys(unsigned char key, int x, int y) {
    switch(key) {
        case 27:exit(0);break;
        case 'f':
            if(fast_mode==1.0)fast_mode=2.0;
            else fast_mode=1.0;
            break;
        case 'h':
            if(hard_mode==1.0)hard_mode=5.0;
            else hard_mode=1.0;
            break;
        default: break;
    }
}

/*    end user input callbacks    */

//load texture files (.raw)--code copied from somewhere
GLuint LoadTextureRAW( const char * filename, int wrap, int width, int height) {
    
    GLuint texture;
    void * data;
    FILE * file;
    
    // open texture data
    //errno_t err = fopen_s( &file, filename, "rb" ); //use fopen_s when compiling with MSVC
    file = fopen( filename, "rb" ); //MinGW
    if ( file == NULL ) return 0;
    
    // allocate buffer
    //width = 256;
    //height = 256;
    data = malloc( width * height * 3 );
    
    // read texture data
    fread( data, width * height * 3, 1, file );
    fclose( file );
    
    // allocate a texture name
    glGenTextures( 1, &texture );
    
    // select our current texture
    glBindTexture( GL_TEXTURE_2D, texture );
    
    // select modulate to mix texture with color for shading
    glTexEnvf( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );
    
    // when texture area is small, bilinear filter the closest MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                     GL_LINEAR_MIPMAP_NEAREST );
    // when texture area is large, bilinear filter the first MIP map
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    
    // if wrap is true, the texture wraps over at the edges (repeat)
    //       ... false, the texture ends at the edges (clamp)
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                     wrap ? GL_REPEAT : GL_CLAMP );
    glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                     wrap ? GL_REPEAT : GL_CLAMP );
    
    // build our texture MIP maps
    gluBuild2DMipmaps( GL_TEXTURE_2D, 3, width,
      height, GL_RGB, GL_UNSIGNED_BYTE, data );
    
    // free buffer
    free( data );
    
    return texture;
    
}

//start the game!
int main(int argc, char** argv) {

    glutInit(&argc, argv);
    //use double buffering and depth testing
    glutInitDisplayMode (GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
    glutInitWindowSize(width, height); /* 800 x 600 initial pixel window */
    glutInitWindowPosition(-1, -1);    /* window manager dictates position */
    glutCreateWindow("Skifree 3D");   /* window title */

    //enable
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_NORMALIZE);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_COLOR_MATERIAL);
    
    glLightfv(GL_LIGHT0, GL_POSITION, lpos);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, diffuse);
    //glLightfv(GL_LIGHT0, GL_AMBIENT, ambient);
    glLightfv(GL_LIGHT0, GL_SPECULAR, scolor);
    glLightModelfv(GL_LIGHT_MODEL_AMBIENT, ambient);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);
    glShadeModel (GL_SMOOTH);
    
    //glLightModeli(GL_LIGHT_MODEL_LOCAL_VIEWER, GL_TRUE);

    //callback functions
    glutIdleFunc(display);
    glutDisplayFunc(display);
    //glutReshapeFunc(changeSize);
    glutMouseFunc(mouse);
    glutMotionFunc(amovement);
    glutPassiveMotionFunc(movement); 
    glutKeyboardFunc(keys);
    glutSpecialFunc(special);
    
    //init
    changeSize(width, height);
    
    //glEnable (GL_BLEND);
    //glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    //black background
    glClearColor(0.0, 0.0, 0.0, 0.0);

    ramp = LoadTextureRAW("ramp.raw", true, 64, 16);
    back = LoadTextureRAW("background.raw", true, 768, 256);

    qobj = gluNewQuadric();
    gluQuadricDrawStyle(qobj, GLU_FILL);
    gluQuadricNormals(qobj, GLU_SMOOTH);

    slist = glGenLists(7);
    glNewList(slist, GL_COMPILE); //type 1
      largeTree();
    glEndList();
    glNewList(slist+1, GL_COMPILE); //type 2
      smallTree();
    glEndList();
    glNewList(slist+2, GL_COMPILE); //type 3
      mogul();
    glEndList();
    glNewList(slist+3, GL_COMPILE); //type 4
      rock();
    glEndList();
    glNewList(slist+4, GL_COMPILE); //type 5
      stump();
    glEndList();
    glNewList(slist+5, GL_COMPILE); //type 6
      dramp();
    glEndList();
    glNewList(slist+6, GL_COMPILE); //type 7
      monster();
    glEndList();

    init_populate();
    glutTimerFunc(100, rand_populate, 0);

    glutMainLoop(); /* enter event loop */
    return 0;

}

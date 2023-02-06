#include <gl/gl.h>
#include <gl/glu.h>
#include <gl/glut.h>
#include <string>
#include <math.h>
#include <iostream>
using namespace std;

#define PI 3.1415926535
#define DR 0.0174533
#define WIDTH 1080
#define HEIGHT 600
#define HITBOX mapC/8.0
#define GAMEW WIDTH
#define GAMEH HEIGHT
double px, py, pdx, pdy, pa, sensitivity, step, brightness, *rayX, *rayY,
       frame1, frame2, fpsInv;
int dof, fov, res;
const int mapX = 16, mapY = 16, mapC = 16, screenO = 0;
const std::string playground[] = {
"################",
"#              #",
"## # # # # # # #",
"#              #",
"# % % %   % # ##",
"#              #",
"## $ $     % # #",
"#              #",
"# # $ #   $ # ##",
"#              #",
"##             #",
"#          #$###",
"#  % $$        #",
"#  %  %%$$  %  #",
"#  %           #",
"################"
};
// # is Red block
// % is Blue block
// $ is Green block
enum objectType : char{
    redWall = '#', blueWall = '%', greenWall = '$',
    emptySpace = ' '
};
typedef struct{
    bool w, a, s, d, up, left, down, right;
}controls;
controls keys;

void display();
void reshape(int sx, int sy);
void buttonDown(unsigned char key, int x, int y);
void buttonDown2(int key, int x, int y);
void buttonUp(unsigned char key, int x, int y);
void buttonUp2(int key, int x, int y);

bool collisionCheck(int x, int y, objectType& obj);
bool hitboxUpdate();
void movementUpdate();

void drawBlock(int x, int y, double r, double g, double b);
void drawPlayground();
void drawPlayer(double r, double g, double b);
void drawRay();

double distance(double x1, double y1, double x2, double y2);
void objectColor(objectType obj, double opacity);
double makeRay(double ra, bool& wall, objectType& obj);
void raycasting();

void init(){
    glClearColor(.2, .2, .2, 1);
    px = (mapY*mapC)/2;
    py = (mapX*mapC)/2;
    step = mapC*3.5;
    pa = 3*PI/2;
    pdx = cos(pa)*step;
    pdy = sin(pa)*step;
    sensitivity = 0.9;
    dof = (mapX>=mapY)?mapX:mapY;
    fov = 60;
    res = GAMEW/1;
    brightness = 20;
    rayX = new double[res];
    rayY = new double[res];
}

int main(int argc, char** argv){
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);

    glutInitWindowPosition(100, 100);
    glutInitWindowSize(WIDTH, HEIGHT);
    glutCreateWindow("Ray casting");

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutKeyboardFunc(buttonDown);
    glutSpecialFunc(buttonDown2);
    glutKeyboardUpFunc(buttonUp);
    glutSpecialUpFunc(buttonUp2);
    init();
    //glEnable(GL_LINE_SMOOTH);
    glutMainLoop();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void display(){
    glClear(GL_COLOR_BUFFER_BIT);
    //glLoadIdentity();

    glBegin(GL_QUADS);
    glColor3d(0, .3, .7);
    glVertex2d(WIDTH, 0);
    glVertex2d(0, 0);
    glColor3d(0, 0, .3);
    glVertex2d(0, HEIGHT/2);
    glVertex2d(WIDTH, HEIGHT/2);
    glColor3d(.3, .1, 0);
    glVertex2d(WIDTH, HEIGHT/2);
    glVertex2d(0, HEIGHT/2);
    glColor3d(.5, .3, 0);
    glVertex2d(0, HEIGHT);
    glVertex2d(WIDTH, HEIGHT);
    glEnd();

    movementUpdate();

    raycasting();
    drawPlayground();
    drawRay();
    drawPlayer(1, 1, 0);

    glutSwapBuffers();
}

void reshape(int sx, int sy){
    glutReshapeWindow(WIDTH, HEIGHT);
    glViewport(0, 0, sx, sy);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0, WIDTH, HEIGHT, 0);
    //glMatrixMode(GL_MODELVIEW);
}

void buttonDown(unsigned char key, int x, int y){
    if (key == 'w')
        keys.w = true;
    if (key == 'a')
        keys.a = true;
    if (key == 's')
        keys.s = true;
    if (key == 'd')
        keys.d = true;
    glutPostRedisplay();
}

void buttonDown2(int key, int x, int y){
    if (key == GLUT_KEY_UP)
        keys.up = true;
    if (key == GLUT_KEY_LEFT)
        keys.left = true;
    if (key == GLUT_KEY_DOWN)
        keys.down = true;
    if (key == GLUT_KEY_RIGHT)
        keys.right = true;
    glutPostRedisplay();
}

void buttonUp(unsigned char key, int x, int y){
    if (key == 'w')
        keys.w = false;
    if (key == 'a')
        keys.a = false;
    if (key == 's')
        keys.s = false;
    if (key == 'd')
        keys.d = false;
    glutPostRedisplay();
}

void buttonUp2(int key, int x, int y){
    if (key == GLUT_KEY_UP)
        keys.up = false;
    if (key == GLUT_KEY_LEFT)
        keys.left = false;
    if (key == GLUT_KEY_DOWN)
        keys.down = false;
    if (key == GLUT_KEY_RIGHT)
        keys.right = false;
    glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
bool collisionCheck(int x, int y, objectType& obj){
    int i = x/mapC, j = y/mapC;
    obj = emptySpace;
    if (j >= mapX || i >= mapY || i < 0 || j < 0)
        return false;
    if (playground[j][i] != ' '){
        obj = (objectType)playground[j][i];
        return true;
    }
    return false;
}

bool hitboxUpdate(){
    objectType temp;
    return collisionCheck(px-HITBOX, py-HITBOX, temp) || collisionCheck(px+HITBOX, py-HITBOX, temp) ||
           collisionCheck(px-HITBOX, py+HITBOX, temp) || collisionCheck(px+HITBOX, py+HITBOX, temp);
}

void movementUpdate(){
    frame2 = glutGet(GLUT_ELAPSED_TIME);
    fpsInv = (frame2-frame1)/1000;
    frame1 = glutGet(GLUT_ELAPSED_TIME);

    double temp;
    if (keys.w || keys.up){
        px += pdx*fpsInv;
        if (hitboxUpdate()) px -= pdx*fpsInv;
        py += pdy*fpsInv;
        if (hitboxUpdate()) py -= pdy*fpsInv;
    }
    if (keys.a){
        temp = sin(pa)*step*fpsInv;
        px += temp;
        if (hitboxUpdate()) px -= temp;
        temp = cos(pa)*step*fpsInv;
        py -= temp;
        if (hitboxUpdate()) py += temp;
    }
    if (keys.s || keys.down){
        px -= pdx*fpsInv;
        if (hitboxUpdate()) px += pdx*fpsInv;
        py -= pdy*fpsInv;
        if (hitboxUpdate()) py += pdy*fpsInv;
    }
    if (keys.d){
        temp = sin(pa)*step*fpsInv;
        px -= temp;
        if (hitboxUpdate()) px += temp;
        temp = cos(pa)*step*fpsInv;
        py += temp;
        if (hitboxUpdate()) py -= temp;
    }
    if (keys.left){
        pa -= sensitivity*fpsInv;
        pa += (pa<0)?2*PI:0;
        pdx = cos(pa)*step;
        pdy = sin(pa)*step;
    }
    if (keys.right){
        pa += sensitivity*fpsInv;
        pa -= (pa>2*PI)?2*PI:0;
        pdx = cos(pa)*step;
        pdy = sin(pa)*step;
    }
    glutPostRedisplay();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
void drawBlock(int x, int y, double r, double g, double b){
    glColor3d(r, g, b);
    glBegin(GL_QUADS);
    glVertex2d(x+1, y+1);
    glVertex2d(x+1, y+mapC-1);
    glVertex2d(x+mapC-1, y+mapC-1);
    glVertex2d(x+mapC-1, y+1);
    glEnd();
}

void drawPlayground(){
    glColor3d(0, 0, 0);
    glBegin(GL_QUADS);
    glVertex2d(0, 0);
    glVertex2d(0, mapX*mapC);
    glVertex2d(mapY*mapC, mapX*mapC);
    glVertex2d(mapY*mapC, 0);
    glEnd();

    for (int i = 0; i < mapX; ++i)
        for (int j = 0; j < mapY; ++j){
            if (playground[i][j] == '#')
                drawBlock(j * mapC, i * mapC, .8, .6, .6);
            else if (playground[i][j] == '%')
                drawBlock(j * mapC, i * mapC, .6, .6, .8);
            else if (playground[i][j] == '$')
                drawBlock(j * mapC, i * mapC, .6, .8, .6);
            else
                drawBlock(j * mapC, i * mapC, .3, .3, .3);
        }
}

void drawPlayer(double r, double g, double b){
    if (hitboxUpdate()) glColor3d(1, 0, 0);
    else glColor3d(r, g, b);
    glBegin(GL_TRIANGLES);
    glVertex2d(px+pdx*mapC/2/step, py+pdy*mapC/2/step);
    glVertex2d(px+(-pdx+pdy)*mapC/4/step, py+(-pdy-pdx)*mapC/4/step);
    glVertex2d(px+(-pdx-pdy)*mapC/4/step, py+(-pdy+pdx)*mapC/4/step);
    glEnd();

    /*glColor3d(0, 1, 0);
    glPointSize(HITBOX*2);
    glBegin(GL_POINTS);
    glVertex2d(px, py);
    glEnd();*/
}

void drawRay(){
    glLineWidth(1);
    glColor3d(0, .6, .6);
    glBegin(GL_LINES);
    for (int i = 0; i < res; ++i){
        glVertex2d(px, py);
        glVertex2d(rayX[i], rayY[i]);
    }
    glEnd();
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
double distance(double x1, double y1, double x2, double y2){
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
}

void objectColor(objectType obj, double opacity){
    if (obj == blueWall)
        glColor3d(0, 0, opacity);
    else if (obj == redWall)
        glColor3d(opacity, 0, 0);
    else if (obj == greenWall)
        glColor3d(0, opacity, 0);
    else
        glColor3d(1, 1, 1);
}

double makeRay(double ra, bool& wall, objectType& obj, int index){
    ra += (ra<0)?2*PI:0;
    ra -= (ra>2*PI)?2*PI:0;
    double rx1, ry1, rx2, ry2, ox1, oy1, ox2, oy2, rx, ry, dist1, dist2, dist;
    double temp1 = tan(ra), temp2 = -1/temp1;
    bool c1, c2;
    objectType o1, o2, t1, t2;
    rx1 = rx2 = px;
    ry1 = ry2 = py;
    dist1 = dist2 = 1e6;
    c1 = c2 = true;
    if (ra > PI){
        ry1 = (int)(py/mapC)*mapC;
        rx1 = px+((py-ry1)*temp2);
        oy1 = -mapC;
        ox1 = mapC*temp2;
    }
    else if (ra != 0 && ra != PI){
        ry1 = (int)(py/mapC)*mapC+mapC;
        rx1 = px+((py-ry1)*temp2);
        oy1 = mapC;
        ox1 = -mapC*temp2;
    }
    else
        c1 = false;
    if (ra > PI/2 && ra < 3*PI/2){
        rx2 = (int)(px/mapC)*mapC;
        ry2 = py+((rx2-px)*temp1);
        ox2 = -mapC;
        oy2 = -mapC*temp1;
    }
    else if (ra != 3*PI/2 && ra != PI/2){
        rx2 = (int)(px/mapC)*mapC+mapC;
        ry2 = py+((rx2-px)*temp1);
        ox2 = mapC;
        oy2 = mapC*temp1;
    }
    else
        c2 = false;

    for (int i = 0; c1 && i < dof; ++i){
        if (collisionCheck(rx1, ry1-0.1, t1)|| collisionCheck(rx1, ry1+0.1, t2)){
            dist1 = distance(px, py, rx1, ry1);
            o1 = (t1!=emptySpace)?t1:t2;
            break;
        }
        else {
            rx1 += ox1;
            ry1 += oy1;
        }
    }

    for (int i = 0; c2 && i < dof; ++i){
        if (collisionCheck(rx2-0.1, ry2, t1) || collisionCheck(rx2+0.1, ry2, t2)){
            dist2 = distance(px, py, rx2, ry2);
            o2 = (t1!=emptySpace)?t1:t2;
            break;
        }
        else {
            rx2 += ox2;
            ry2 += oy2;
        }
    }
    if (dist1 <= dist2){
        rx = rx1;
        ry = ry1;
        obj = o1;
        wall = true;
        dist = dist1;
    }
    else {
        rx = rx2;
        ry = ry2;
        obj = o2;
        wall = false;
        dist = dist2;
    }

    rayX[index] = rx;
    rayY[index] = ry;

    return dist;
}

void raycasting(){
    double dist, lineH, lineO, ra, ca, thickness = (double)GAMEW/res,
           opacity, widthO = thickness/2;
    bool wall;
    objectType obj;
    for (int i = 0; i < res; ++i){
        ra = pa - fov/2.0*DR + (double)fov/res*DR*i;
        dist = makeRay(ra, wall, obj, i);
        ca = pa-ra;
        ca += (ca<0)?2*PI:0;
        ca -= (ca>2*PI)?2*PI:0;
        dist = cos(ca)*dist;

        lineH = (mapC*GAMEH)/dist;
        if (lineH > GAMEH)
            lineH = GAMEH;
        lineO = (HEIGHT-lineH)/2;

        opacity = brightness * (mapC*mapC)/(dist*dist);
        opacity = (opacity>1)?1:opacity;
        opacity *= (wall)?0.8: 0.5;
        objectColor(obj, opacity);

        glLineWidth(thickness);
        glBegin(GL_LINES);
        glVertex2d(i*thickness+screenO+widthO, lineO);
        glVertex2d(i*thickness+screenO+widthO, lineO+lineH);
        glEnd();
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////

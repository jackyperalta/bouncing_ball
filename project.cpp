//NAME: JACQUELINE PERALTA
//author:  Gordon Griesel
//date:  10-6-2020
//OpenGL
//lab-7 starting framework
//lab-6 movement framework
//
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glu.h>
#include "fonts.h"

#include <iostream>
#include <fstream>
using namespace std;

const float GRAVITY = -0.01;

typedef float Flt;
typedef Flt Vec[3];
#define rnd() (Flt)rand() / (Flt)RAND_MAX
#define PI 3.14159265358979323846264338327950

class Global {
public:
	int xres, yres;
	
	// LIGHT0
	GLfloat lightAmbient[4];
	GLfloat lightDiffuse[4];
	GLfloat lightSpecular[4];
	GLfloat lightPosition[4];
	GLfloat shininess[1];
	
	GLfloat lmodel_ambient[4];
	
	// LIGHT1
	GLfloat lightAmbient1[4];
	GLfloat lightDiffuse1[4];
	GLfloat lightSpecular1[4];
	GLfloat lightPosition1[4];
	GLfloat shininess1[1];
	
	int lesson_num;
	Flt rtri;
	Flt rquad;
	Flt cubeRot[3];
	Flt cubeAng[3];

	int animation, capture, snapshot;
	float Ypos, Yvel, Ymin, Xpos, Xscale, Yscale;
	double radius;
	

	GLuint texID; //unsigned integer
	Global() {
		srand(time(NULL));
		xres = 300;
		yres = 600;
		
		// LIGHT0
		GLfloat la[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // ambient 
		GLfloat ld[] = { 0.7f, 0.7f, 0.7f, 1.0f }; // diffuse
		GLfloat ls[] = { 1.2f, 1.2f, 1.2f, 1.0f }; // specular
		GLfloat shine[] = { 60.0f };
		GLfloat lp[] = { -45.0f, 5.0f, 65.0f, 1.0f };
		
		GLfloat lm[] = { 0.0f, 0.0f, 0.0f, 1.0f}; // model ambient
		
		// LIGHT1
		GLfloat ka[] = { 0.0f, 0.0f, 0.0f, 1.0f }; // ambient 
		GLfloat kd[] = { 0.5f, 0.5f, 0.5f, 1.0f }; // diffuse
		GLfloat ks[] = { 0.4f, 0.4f, 0.4f, 1.0f }; // specular
		GLfloat shine1[] = { 60.0f };
		GLfloat lp1[] = { 120.0f, 120.0f, 70.0f, 1.0f };		

		//lp[0] = rnd() * 200.0 - 100.0;
		//lp[1] = rnd() * 100.0 + 20.0;
		//lp[2] = rnd() * 300.0 - 150.0;
		
		// LIGHT0
		memcpy(lightAmbient, la, sizeof(GLfloat)*4);
		memcpy(lightDiffuse, ld, sizeof(GLfloat)*4);
		memcpy(lightSpecular, ls, sizeof(GLfloat)*4);
		memcpy(lightPosition, lp, sizeof(GLfloat)*4);
		memcpy(shininess, shine, sizeof(GLfloat)*1);
		
		memcpy(lmodel_ambient, lm, sizeof(GLfloat)*4);
		
		// lIGHT1
		memcpy(lightAmbient1, ka, sizeof(GLfloat)*4);
		memcpy(lightDiffuse1, kd, sizeof(GLfloat)*4);
		memcpy(lightSpecular1, ks, sizeof(GLfloat)*4);
		memcpy(lightPosition1, lp1, sizeof(GLfloat)*4);
		memcpy(shininess1, shine1, sizeof(GLfloat)*1);
		
		lesson_num=2;
		rquad = 0.0f;
		 
		Flt gcubeRot[3]={2.0,0.0,0.0};
		Flt gcubeAng[3]={0.0,0.0,0.0};
		memcpy(cubeRot, gcubeRot, sizeof(Flt)*3);
		memcpy(cubeAng, gcubeAng, sizeof(Flt)*3);

		animation = 0;
		Ypos = 1.2f;
		Yvel = 0.0;
		Ymin = -1.3f;
		radius = 0.55;
		capture = 0;
		snapshot = 0;
		Xscale = 1.0f; Yscale = 1.0f;

		glGenTextures(1, &texID);
	}
} g;


//-----------------------------------------------------------------------------
//X11_wrapper class functions
//-----------------------------------------------------------------------------
class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	X11_wrapper(void) {
		GLint att[] = {GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None};
		//GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, None };
		XSetWindowAttributes swa;
		setup_screen_res(300, 600);
		dpy = XOpenDisplay(NULL);
		if (dpy == NULL) {
			printf("\n\tcannot connect to X server\n\n");
			exit(EXIT_FAILURE);
		}
		Window root = DefaultRootWindow(dpy);
		XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
		if (vi == NULL) {
			printf("\n\tno appropriate visual found\n\n");
			exit(EXIT_FAILURE);
		} 
		Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
		swa.colormap = cmap;
		swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
			StructureNotifyMask | SubstructureNotifyMask;
		win = XCreateWindow(dpy, root, 0, 0, g.xres, g.yres, 0,
			vi->depth, InputOutput, vi->visual,
			CWColormap | CWEventMask, &swa);
		set_title();
		glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
		glXMakeCurrent(dpy, win, glc);
	}
	~X11_wrapper(void) {
		XDestroyWindow(dpy, win);
		XCloseDisplay(dpy);
	}
	void set_title(void) {
		//Set the window title bar.
		XMapWindow(dpy, win);
		XStoreName(dpy, win, "3480 project");
	}
	void setup_screen_res(const int w, const int h) {
		g.xres = w;
		g.yres = h;
	}
	void reshape_window(int width, int height) {
		//window has been resized.
		setup_screen_res(width, height);
		//
		glViewport(0, 0, (GLint)width, (GLint)height);
		glMatrixMode(GL_PROJECTION); glLoadIdentity();
		glMatrixMode(GL_MODELVIEW); glLoadIdentity();
		glOrtho(0, g.xres, 0, g.yres, -1, 1);
		set_title();
	}
	void check_resize(XEvent *e) {
		//The ConfigureNotify is sent by the
		//server if the window is resized.
		if (e->type != ConfigureNotify)
			return;
		XConfigureEvent xce = e->xconfigure;
		if (xce.width != g.xres || xce.height != g.yres) {
			//Window size did change.
			reshape_window(xce.width, xce.height);
		}
	}
	bool getXPending() {
		return XPending(dpy);
	}
	XEvent getXNextEvent() {
		XEvent e;
		XNextEvent(dpy, &e);
		return e;
	}
	void swapBuffers() {
		glXSwapBuffers(dpy, win);
	}
} x11;

// Function prototypes
void init_opengl(void);
void init_textures(void);
void check_mouse(XEvent *e);
int check_keys(XEvent *e);
void movement();
void render(void);

//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main(void)
{
	init_opengl();
	g.snapshot++;
	int done = 0;
	while (!done) {
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			check_mouse(&e);
			done = check_keys(&e);
		}
		movement();
		render();
		x11.swapBuffers();
        // you see the objects right here
	}
	cleanup_fonts();
	return 0;
}
//-----------------------------------------------------------------------------
// GET IMAGE FUNCTION
//-----------------------------------------------------------------------------
unsigned char *get_image_stream(const char *fname, int *w, int *h) {
    ifstream fin(fname);
    char P, six;
    fin >> P >> six;
    cout << "P: " << P << "  six: " << six << endl;
    int width, height, max;
    fin >> width >> height >> max;
    cout << width << "x" << height << endl;
    unsigned char *data = new unsigned char [width * height * 3];
    fin.read((char *)data, width*height*3);
    fin.close();
    *w = width;
    *h = height;
    return data + 1;
}

#define VecCross(a,b,c) \
(c)[0]=(a)[1]*(b)[2]-(a)[2]*(b)[1]; \
(c)[1]=(a)[2]*(b)[0]-(a)[0]*(b)[2]; \
(c)[2]=(a)[0]*(b)[1]-(a)[1]*(b)[0]

void vecCrossProduct(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[1]*v1[2] - v1[1]*v0[2];
	dest[1] = v0[2]*v1[0] - v1[2]*v0[0];
	dest[2] = v0[0]*v1[1] - v1[0]*v0[1];
}

Flt vecDotProduct(Vec v0, Vec v1)
{
	return v0[0]*v1[0] + v0[1]*v1[1] + v0[2]*v1[2];
}

void vecZero(Vec v)
{
	v[0] = v[1] = v[2] = 0.0;
}

void vecMake(Flt a, Flt b, Flt c, Vec v)
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
}

void vecCopy(Vec source, Vec dest)
{
	dest[0] = source[0];
	dest[1] = source[1];
	dest[2] = source[2];
}

Flt vecLength(Vec v)
{
	return sqrt(vecDotProduct(v, v));
}

void vecNormalize(Vec v)
{
	Flt len = vecLength(v);
	if (len == 0.0) {
		vecMake(0,0,1,v);
		return;
	}
	len = 1.0 / len;
	v[0] *= len;
	v[1] *= len;
	v[2] *= len;
}

void vecSub(Vec v0, Vec v1, Vec dest)
{
	dest[0] = v0[0] - v1[0];
	dest[1] = v0[1] - v1[1];
	dest[2] = v0[2] - v1[2];
}



void init_opengl(void)
{
	//OpenGL initialization
	glClearColor(105.0f/255.0f, 105.0f/255.0f, 105.0f/255.0f, 0.0f); // background color
	glClearDepth(1.0);
	glDepthFunc(GL_LESS);
	glEnable(GL_DEPTH_TEST);
	glShadeModel(GL_SMOOTH);
	glEnable(GL_BLEND);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f); // angle, near and far
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,5,10,  0,0,0,  0,1,0);
	
	//Enable this so material colors are the same as vert colors.
	glEnable(GL_COLOR_MATERIAL);
	
	// LIGHT0
	glMaterialfv(GL_FRONT, GL_SPECULAR, g.lightSpecular);
	glMaterialfv(GL_FRONT, GL_SHININESS, g.shininess);
	
	// LIGHT1
	glMaterialfv(GL_FRONT, GL_SPECULAR, g.lightSpecular1);
	glMaterialfv(GL_FRONT, GL_SHININESS, g.shininess1);
	
	glEnable( GL_LIGHTING );
	
	// LIGHT0
	glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
	glLightfv(GL_LIGHT0, GL_DIFFUSE, g.lightDiffuse);
	glLightfv(GL_LIGHT0, GL_SPECULAR, g.lightSpecular);
	glLightfv(GL_LIGHT0, GL_AMBIENT, g.lightAmbient);
	
	// LIGHT1
	glLightfv(GL_LIGHT1, GL_POSITION, g.lightPosition1);
	glLightfv(GL_LIGHT1, GL_DIFFUSE, g.lightDiffuse1);
	glLightfv(GL_LIGHT1, GL_SPECULAR, g.lightSpecular1);
	glLightfv(GL_LIGHT1, GL_AMBIENT, g.lightAmbient1);
	
	glLightModelfv(GL_LIGHT_MODEL_AMBIENT, g.lmodel_ambient);

	glEnable(GL_LIGHT0);
	glEnable(GL_LIGHT1);
	
	
	//Do this to allow fonts
	initialize_fonts();
	glEnable(GL_TEXTURE_2D);

	static int firsttime = 1;
	if(firsttime){
		firsttime = 0;
		glGenTextures(1, &g.texID);
		//Built a texture map
		glBindTexture(GL_TEXTURE_2D, g.texID);
		int w, h;
		unsigned char *data = get_image_stream("TB_sphere_texture.ppm", &w, &h);
		printf("w: %i h:%i\n", w, h);
		
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
		glTexImage2D(GL_TEXTURE_2D, 0, 3, w, h, 0,
			GL_RGB, GL_UNSIGNED_BYTE, data);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
}

void check_mouse(XEvent *e)
{
	//Did the mouse move?
	//Was a mouse button clicked?
	static int savex = 0;
	static int savey = 0;
	
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button is down
		}
		if (e->xbutton.button==3) {
			//Right button is down
		}
	}
	if (savex != e->xbutton.x || savey != e->xbutton.y) {
		//Mouse moved
		savex = e->xbutton.x;
		savey = e->xbutton.y;
	}
}

void show_menu(){
	Rect r;
	r.bot = g.yres - 20;
	r.left = 10;
	r.center = 0;
	ggprint8b(&r, 16, 0x00887766, "3480");
	ggprint8b(&r, 16, 0x008877aa, "2 - Sphere");
	ggprint8b(&r, 16, 0x008877aa, "A - Animation");
	ggprint8b(&r, 16, 0x008877aa, "B - Capture");
	//ggprint8b(&r, 16, 0x008877aa, "L - change light position");
	
	return;
}

int check_keys(XEvent *e)
{
	//Was there input from the keyboard?
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
	switch(key) {

		case XK_a: // Animation ON-OFF
			if(g.animation == 0){ 
				g.animation = 1; 
			}
			else
				g.animation = 0; 
		break;

		case XK_b: // CAPTURE ON-OFF
			if(g.capture == 0){ 
				g.capture = 1; 
			}
			else
				g.capture = 0; 
			break;

		case XK_2:
			g.lesson_num = 2;
			init_opengl();
			break;
		case XK_l:
			//set light position
			//g.lightPosition[0] = rnd() * 200.0 - 100.0;
			//g.lightPosition[1] = rnd() * 100.0 + 20.0;
            		//g.lightPosition[2] = rnd() * 200.0 + 50.0;
			//glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
			cout << g.lightPosition[0] << endl;
			cout << g.lightPosition[1] << endl;
			cout << g.lightPosition[2] << endl;
			//glLightfv(GL_LIGHT1, GL_POSITION, g.lightPosition1);
			
			
			break;
		case XK_m:
			show_menu();
			return 0;
			//break;
		case XK_Escape:
			return 1;
	   }
	}
	return 0;
}

void screen_capture()
{
    //screen shot, capture, snapshot, opengl screenshot
    static int inc = 0;
    int xres = g.xres;
    int yres = g.yres;
    //get pixels
    unsigned char *data = new unsigned char [xres * yres * 3];
    glReadPixels(0, 0, xres, yres, GL_RGB, GL_UNSIGNED_BYTE, data);
    //write ppm file...
    char ts[256];
    sprintf(ts, "img%03i.ppm", inc++);
    FILE *fpo = fopen(ts, "w");
    fprintf(fpo, "P6\n");
    fprintf(fpo, "%i %i\n", xres, yres);
    fprintf(fpo, "255\n");
    //go backwards a row at a time...
    unsigned char *p = data;
    p = p + ((yres-1) * xres * 3);
    unsigned char *start = p;
    for (int i=0; i<yres; i++) {
        for (int j=0; j<xres*3; j++) {
            fprintf(fpo, "%c", *p);
            ++p;
        }
        start = start - (xres*3);
        p = start;
    }
    fclose(fpo);
}


void movement()
{
	static float vel0 = -10.0;
	if(g.animation){
		g.Ypos += g.Yvel;
		g.Yvel += GRAVITY;
  		if (g.Ypos < g.Ymin) {
    			g.Ypos = g.Ymin;
    			if (vel0 == -10.0){
      				vel0 = fabs(g.Yvel);
    			}
    			g.Yvel = vel0;
    		}
    		// for the squish effect
    		if(g.Ypos == g.Ymin){
    		      	g.Xscale = 1.1;
			g.Yscale = 0.9;	
		}
		else{
			g.Xscale = 1.0;
			g.Yscale = 1.0;
		}
			
    }
}

void DrawGLScene2()
{
    const int n = 26;
    static double points[n][2];

	// Vec v1, v2, v3, v4, v5, v6, norm;
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	/************************** CREATE SPHERE ****************************************/
	glLoadIdentity();
	glLightfv(GL_LIGHT0, GL_POSITION, g.lightPosition);
	
	glTranslatef(g.Xpos, g.Ypos, -5.0f);
	//glTranslatef(0.0f, 0.0f, -7.0f);
	glScalef(g.Xscale, g.Yscale, 1.0);
	// Sectors ---> longitude & Stacks ---> latitude
	glColor3f(192.0f/255.0f, 192.0f/255.0f, 192.0f/255.0f); 
	//glColor3f(255.0f, 255.0f, 255.0f); 
	glBindTexture(GL_TEXTURE_2D, g.texID);
	for(int i = 0; i <= n; i++) // longitude 
	{
		double phi1 = PI * ((double) i / n);
		double phi2 = PI * ((double) (i + 1) / n);
		
		double z0 = sin(phi1);
		double z1  = cos(phi1);
		
		double z2 = sin(phi2);
		double z3 = cos(phi2);
		
		glBegin(GL_QUAD_STRIP);
		for(int j=0; j <= n; j++) // latitude
		{
			double theta = 2*PI * (double) (j) / n;
			double x = cos(theta);
			double y = sin(theta);
			
			double s = phi1/(PI); // column
			double s1 = phi2/(PI); 
			double t =  1 -theta/PI; // row
			

			points[j][0] = x * g.radius;
			points[j][1] = y * g.radius;

			glNormal3f(x * z0, y * z0, z1);
			glTexCoord2f(s, t); 
			glVertex3f(points[j][0] * z0, points[j][1] * z0, g.radius * z1);
			
			glNormal3f(x * z2, y * z2, z3);
			glTexCoord2f(s1, t);
			glVertex3f(points[j][0] * z2, points[j][1] * z2, g.radius * z3);
		}
		glEnd();
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void render(void)
{

	if(g.capture && g.snapshot % 4 == 0){
		screen_capture();
	}
	g.snapshot++;

	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45.0f,(GLfloat)g.xres/(GLfloat)g.yres,0.1f,100.0f);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0,5,10,  0,0,0,  0,1,0);
	
	//Enable this so material colors are the same as vert colors.
	glEnable(GL_COLOR_MATERIAL);
	glEnable(GL_LIGHTING);
	//
	switch (g.lesson_num) {
		case 0:
        case 2: DrawGLScene2(); break;
	}
	//Set 2D mode (no perspective)
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_LIGHTING);

	// comment out show_menu when wanting to create GIF
	show_menu();

    usleep(16000);
}


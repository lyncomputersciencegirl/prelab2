//modified by: Yeana Bond		
//date: 8/30/22
//
//author: Gordon Griesel
//date: Spring 2022
//purpose: get openGL working on your personal computer
//
#include <iostream>
using namespace std;
#include <stdio.h>
#include <unistd.h>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>


const int MAX_PARTICLES = 1000;


class Global {
public:
	int xres, yres;
	Global();
} g;



class Box {

public:
    float w;
    float dir;
    // two dimensional movements for water animation 
    float vel[2];
    float pos[2];

    Box() {
	w = 20.0f;
	dir = 25.0f;
	pos[0] = g.xres / 2.0f;
	pos[1] = g.yres / 2.0f;
	vel[0] = vel[1] = 0.0;
    }

     Box(float wid, float d, float p0, float p1) {
	//this->w = w;
	w = wid;
	dir = d;
	pos[0] = p0;
	pos[1] = p1;

	vel[0] = vel[1] = 0.0;
    }




} box, particle(4.0, 0.0, g.xres/2.0, g.yres/4.0*3.0);


// Global array 
Box particles[MAX_PARTICLES];
int n = 0;



class X11_wrapper {
private:
	Display *dpy;
	Window win;
	GLXContext glc;
public:
	~X11_wrapper();
	X11_wrapper();
	void set_title();
	bool getXPending();
	XEvent getXNextEvent();
	void swapBuffers();
	void reshape_window(int width, int height);
	void check_resize(XEvent *e);
	void check_mouse(XEvent *e);
	int check_keys(XEvent *e);
} x11;

//Function prototypes
void init_opengl(void);
void physics(void);
void render(void);



//=====================================
// MAIN FUNCTION IS HERE
//=====================================
int main()
{
	init_opengl();
	//Main loop
	int done = 0;
	while (!done) {
		//Process external events.
		while (x11.getXPending()) {
			XEvent e = x11.getXNextEvent();
			x11.check_resize(&e);
			x11.check_mouse(&e);
			done = x11.check_keys(&e);
		}
		physics();
		render();
		x11.swapBuffers();
		usleep(200);
	}
	return 0;
}

Global::Global()
{
	xres = 400;
	yres = 200;
}

X11_wrapper::~X11_wrapper()
{
	XDestroyWindow(dpy, win);
	XCloseDisplay(dpy);
}

X11_wrapper::X11_wrapper()
{
	GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
	int w = g.xres, h = g.yres;
	dpy = XOpenDisplay(NULL);
	if (dpy == NULL) {
		cout << "\n\tcannot connect to X server\n" << endl;
		exit(EXIT_FAILURE);
	}
	Window root = DefaultRootWindow(dpy);
	XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
	if (vi == NULL) {
		cout << "\n\tno appropriate visual found\n" << endl;
		exit(EXIT_FAILURE);
	} 
	Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
	XSetWindowAttributes swa;
	swa.colormap = cmap;
	swa.event_mask =
		ExposureMask | KeyPressMask | KeyReleaseMask |
		ButtonPress | ButtonReleaseMask |
		PointerMotionMask |
		StructureNotifyMask | SubstructureNotifyMask;
	win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
		InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
	set_title();
	glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
	glXMakeCurrent(dpy, win, glc);
}

void X11_wrapper::set_title()
{
	//Set the window title bar.
	XMapWindow(dpy, win);
	XStoreName(dpy, win, "3350 Lab1");
}

bool X11_wrapper::getXPending()
{
	//See if there are pending events.
	return XPending(dpy);
}

XEvent X11_wrapper::getXNextEvent()
{
	//Get a pending event.
	XEvent e;
	XNextEvent(dpy, &e);
	return e;
}

void X11_wrapper::swapBuffers()
{
	glXSwapBuffers(dpy, win);
}

void X11_wrapper::reshape_window(int width, int height)
{
	//window has been resized.
	g.xres = width;
	g.yres = height;
	//
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
}

void X11_wrapper::check_resize(XEvent *e)
{
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
//-----------------------------------------------------------------------------


void make_particles(int x, int y)
{
    if (n>= MAX_PARTICLES)
	return;

   particles[n].w = 4;
   particles[n].pos[0] = x;
   particles[n].pos[1] = y;
   particles[n].vel[0] = particles[n].vel[1] = 0.0;
   //index should increase
   ++n;
   printf("n : %i\n", n); fflush(stdout);

}


void X11_wrapper::check_mouse(XEvent *e)
{
	static int savex = 0;
	static int savey = 0;

	//Weed out non-mouse events
	if (e->type != ButtonRelease &&
		e->type != ButtonPress &&
		e->type != MotionNotify) {
		//This is not a mouse event that we care about.
		return;
	}
	//
	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed.
			int y = g.yres - e->xbutton.y;
			int x = e->xbutton.x;
			//If left button is pressed, make particles 
			make_particles(x,y);	

			return;
		}
		if (e->xbutton.button==3) {
			//Right button was pressed.
			return;
		}
	}
	if (e->type == MotionNotify) {
		//The mouse moved!
		if (savex != e->xbutton.x || savey != e->xbutton.y) {
			savex = e->xbutton.x;
			savey = e->xbutton.y;
			//Code placed here will execute whenever the mouse moves.


		}
	}
}

int X11_wrapper::check_keys(XEvent *e)
{
	if (e->type != KeyPress && e->type != KeyRelease)
		return 0;
	int key = XLookupKeysym(&e->xkey, 0);
	if (e->type == KeyPress) {
		switch (key) {
			case XK_1:
				//Key 1 was pressed
				break;
			case XK_Escape:
				//Escape key was pressed
				return 1;
		}
	}
	return 0;
}

void init_opengl(void)
{
	//OpenGL initialization
	glViewport(0, 0, g.xres, g.yres);
	//Initialize matrices
	glMatrixMode(GL_PROJECTION); glLoadIdentity();
	glMatrixMode(GL_MODELVIEW); glLoadIdentity();
	//Set 2D mode (no perspective)
	glOrtho(0, g.xres, 0, g.yres, -1, 1);
	//Set the screen background color
	glClearColor(0.1, 0.1, 0.1, 1.0);
}

void physics()
{
    //About 60 frames a second it will run physics()
    //Every frame, gravity should act on
    particle.vel[1] -= 0.01;
    particle.pos[0] += particle.vel[0];
    particle.pos[1] += particle.vel[1];

    //Check for collision
    
    // if y value is less than top of the box
    if ((particle.pos[1] < box.pos[1] + box.w) &&
	    
        (particle.pos[1] > box.pos[1] - box.w) &&

        (particle.pos[0] > box.pos[0] - box.w) &&
	    
        (particle.pos[0] < box.pos[0] + box.w)) {

	    // particle y direction stops
	    particle.vel[1] = 0.0;

	    // particle x direction increases
	    particle.vel[0] += 0.01;
    }

    // TODO: use a while loop to process moved array element

	for (int i = 0; i < n; i++) {

            particles[i].vel[1] -= 0.01;
            particles[i].pos[0] += particles[i].vel[0];
            particles[i].pos[1] += particles[i].vel[1];

	    // check for collision

            if ((particles[i].pos[1] < box.pos[1] + box.w) &&
	    
            (particles[i].pos[1] > box.pos[1] - box.w) &&

            (particles[i].pos[0] > box.pos[0] - box.w) &&
	    
            (particles[i].pos[0] < box.pos[0] + box.w)) {

	        // particles y direction stops
	        particles[i].vel[1] = 0.0;

	        // particle x direction increases
	        particles[i].vel[0] += 0.01;
	    }

	

	    if (particles[i].pos[1] < 0.0) {
// comment #define OPT_1 out if you want the old code to run 

#define OPT_1
#ifdef OPT_1

		// ======================================
		// To fix bounary issue, this code optimized as below: 
		particles[i] = particles[n-1];
		n = n-1;
		// =======================================

#else // OPT_1

		// new code, the optimized one
		particles[i] = particles[--n];

#endif // OPT_1

		// =======================================
	        // if particles go off the screen
	        // the gone away particle is switched with the last element 
	        // so, we can always reference the last element by decrementing n
	        

	    }
	}




}

void render()
{
	//static float w = 20.0f;
	//static float dir = 25.0f;
	//static float pos[2] = {0.0f+w, g.yres/2.0f};
	//
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw box.
	
	glPushMatrix();

	
	glColor3ub(150, 200, 120);
	glTranslatef(box.pos[0], box.pos[1], 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(-box.w, -box.w);
		glVertex2f(-box.w,  box.w);
		glVertex2f( box.w,  box.w);
		glVertex2f( box.w, -box.w);
	glEnd();
	glPopMatrix();

	//Draw particle.
	//glPushMatric() saves the current position 
	glPushMatrix();

	
	glColor3ub(150, 160, 255);
	glTranslatef(particle.pos[0], particle.pos[1], 0.0f);
	glBegin(GL_QUADS);
		glVertex2f(-particle.w, -particle.w);
		glVertex2f(-particle.w,  particle.w);
		glVertex2f( particle.w,  particle.w);
		glVertex2f( particle.w, -particle.w);
	glEnd();
	glPopMatrix(); 

	//Draw all particles.
	//glPushMatric() saves the current position 
	//glPushMatrix();

	
	glColor3ub(150, 160, 255);
	for (int i = 0; i < n; i++) {
	    glPushMatrix();
	    glTranslatef(particles[i].pos[0], particles[i].pos[1], 0.0f);
	    glBegin(GL_QUADS);
		glVertex2f(-particles[i].w, -particles[i].w);
		glVertex2f(-particles[i].w,  particles[i].w);
		glVertex2f( particles[i].w,  particles[i].w);
		glVertex2f( particles[i].w, -particles[i].w);
	    glEnd();
	    glPopMatrix();
	}
	//glPopMatrix();

//	pos[0] += dir;
//	if (pos[0] >= (g.xres-w)) {
//		pos[0] = (g.xres-w);
//		dir = -dir;
//	}
//	if (pos[0] <= w) {
//		pos[0] = w;
//		dir = -dir;
//	}
}







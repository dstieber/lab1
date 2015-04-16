//cs335 Spring 2015 Lab-1
//This program demonstrates the use of OpenGL and XWindows
//
//Assignment is to modify this program.
//You will follow along with your instructor.
//
//Elements to be learned in this lab...
//
//. general animation framework
//. animation loop
//. object definition and movement
//. collision detection
//. mouse/keyboard interaction
//. object constructor
//. coding style
//. defined constants
//. use of static variables
//. dynamic memory allocation
//. simple opengl components
//. git
//
//elements we will add to program...
//. Game constructor
//. multiple particles
//. gravity
//. collision detection
//. more objects
//
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <cstring>
#include <cmath>
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <GL/glx.h>

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 8000
#define GRAVITY 0.1

//X Windows variables
Display *dpy;
Window win;
GLXContext glc;

//Structures

struct Vec {
    float x, y, z;
};

struct Shape {
    float width, height;
    float radius;
    Vec center;
};

struct Particle {
    Shape s;
    Vec velocity;
};

struct Game {
    Shape circle;
    bool bubbler;
    int lastMouse[2];
    Shape box[5];
    Particle *particle;
    int n;
    ~Game() { delete [] particle; }
    Game() {
	bubbler = false;
	particle = new Particle[MAX_PARTICLES];
	n=0;
	//declare a box shape
	for (int i=0; i<5; i++) {
	    box[i].width = 100;
	    box[i].height = 10;
	    box[i].center.x = 120 + i*65;
	    box[i].center.y = 500 - i*60;
	    box[i].center.z = 0;
	}
	circle.radius = 200.0;
	circle.center.x = 550;
	circle.center.y = -25;
    }
};

//Function prototypes
void initXWindows(void);
void init_opengl(void);
void cleanupXWindows(void);
void check_mouse(XEvent *e, Game *game);
int check_keys(XEvent *e, Game *game);
void movement(Game *game);
void render(Game *game);


int main(void)
{
    int done=0;
    srand(time(NULL));
    initXWindows();
    init_opengl();
    //declare game object
    Game game;

    //start animation
    while(!done) {
	while(XPending(dpy)) {
	    XEvent e;
	    XNextEvent(dpy, &e);
	    check_mouse(&e, &game);
	    done = check_keys(&e, &game);
	}
	movement(&game);
	render(&game);
	glXSwapBuffers(dpy, win);
    }
    cleanupXWindows();
    return 0;
}

void set_title(void)
{
    //Set the window title bar.
    XMapWindow(dpy, win);
    XStoreName(dpy, win, "335 Lab1   LMB for particle");
}

void cleanupXWindows(void) {
    //do not change
    XDestroyWindow(dpy, win);
    XCloseDisplay(dpy);
}

void initXWindows(void) {
    //do not change
    GLint att[] = { GLX_RGBA, GLX_DEPTH_SIZE, 24, GLX_DOUBLEBUFFER, None };
    int w=WINDOW_WIDTH, h=WINDOW_HEIGHT;
    dpy = XOpenDisplay(NULL);
    if (dpy == NULL) {
	std::cout << "\n\tcannot connect to X server\n" << std::endl;
	exit(EXIT_FAILURE);
    }
    Window root = DefaultRootWindow(dpy);
    XVisualInfo *vi = glXChooseVisual(dpy, 0, att);
    if(vi == NULL) {
	std::cout << "\n\tno appropriate visual found\n" << std::endl;
	exit(EXIT_FAILURE);
    } 
    Colormap cmap = XCreateColormap(dpy, root, vi->visual, AllocNone);
    XSetWindowAttributes swa;
    swa.colormap = cmap;
    swa.event_mask = ExposureMask | KeyPressMask | KeyReleaseMask |
	ButtonPress | ButtonReleaseMask |
	PointerMotionMask |
	StructureNotifyMask | SubstructureNotifyMask;
    win = XCreateWindow(dpy, root, 0, 0, w, h, 0, vi->depth,
	    InputOutput, vi->visual, CWColormap | CWEventMask, &swa);
    set_title();
    glc = glXCreateContext(dpy, vi, NULL, GL_TRUE);
    glXMakeCurrent(dpy, win, glc);
}

void init_opengl(void)
{
    //OpenGL initialization
    glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    //Initialize matrices
    glMatrixMode(GL_PROJECTION); glLoadIdentity();
    glMatrixMode(GL_MODELVIEW); glLoadIdentity();
    //Set 2D mode (no perspective)
    glOrtho(0, WINDOW_WIDTH, 0, WINDOW_HEIGHT, -1, 1);
    //Set the screen background color
    glClearColor(0.1, 0.1, 0.1, 1.0);
}

#define rnd() (float)rand() / (float)RAND_MAX


void makeParticle(Game *game, int x, int y) {
    if (game->n >= MAX_PARTICLES)
	return;
    std::cout << "makeParticle() " << x << " " << y << std::endl;
    //position of particle
    Particle *p = &game->particle[game->n];
    p->s.center.x = x;
    p->s.center.y = y;
    p->velocity.y = rnd() - 0.5;
    p->velocity.x = rnd() - 0.5;
    game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
    static int savex = 0;
    static int savey = 0;
    static int n = 0;

    if (e->type == ButtonRelease) {
	return;
    }
    if (e->type == ButtonPress) {
	if (e->xbutton.button==1) {
	    //Left button was pressed
	    int y = WINDOW_HEIGHT - e->xbutton.y;
	    for (int i=0; i<10; i++)
		makeParticle(game, e->xbutton.x, y);
	    return;
	}
	if (e->xbutton.button==3) {
	    //Right button was pressed
	    return;
	}
    }
    //Did the mouse move?
    if (savex != e->xbutton.x || savey != e->xbutton.y) {
	savex = e->xbutton.x;
	savey = e->xbutton.y;

	if (++n < 10)
	    return;
	int y = WINDOW_HEIGHT - e->xbutton.y;
	for (int i=0; i<10; i++)
	    makeParticle(game, e->xbutton.x, y);
	game->lastMouse[0] = savex;
	game->lastMouse[1] = WINDOW_HEIGHT - savey;
    }
}

int check_keys(XEvent *e, Game *game)
{
    //Was there input from the keyboard?
    if (e->type == KeyPress) {
	int key = XLookupKeysym(&e->xkey, 0);
	if (key == XK_Escape) {
	    return 1;
	}
	//You may check other keys here.
	if (key == XK_b){
	    game->bubbler = !game->bubbler;
	}
    }
    return 0;
}

void movement(Game *game)
{
    Particle *p;

    if(game->bubbler) {
	for (int i=0; i<10; i++){
	    makeParticle(game, game->lastMouse[0], game->lastMouse[1]);
	}
    }

    if (game->n <= 0)
	return;

    for (int i = 0; i < game->n; i++) {
	p = &game->particle[i];
	p->s.center.x += p->velocity.x;
	p->s.center.y += p->velocity.y;
	p->velocity.y -= GRAVITY;

	//check for collision with shapes...
	//Shape *s;
	for (int j=0; j<5; j++) {
	    if(p->s.center.x >= game->box[j].center.x - game->box[j].width &&
		    p->s.center.x <= game->box[j].center.x + game->box[j].width &&
		    p->s.center.y < game->box[j].center.y + game->box[j].height&&
		    p->s.center.y > game->box[j].center.y - game->box[j].height){
		//collision with box
		p->s.center.y = game->box[j].center.y + game->box[j].height + 0.1;
		p->velocity.y *= rnd() * -0.5;
		p->velocity.x *= 1.01;
	    }

	}

	//collision detection for circle
	float d0,d1,dist;
	d0 = p->s.center.x - game->circle.center.x;
	d1 = p->s.center.y - game->circle.center.y;
	dist = sqrt(d0*d0 + d1*d1);
	if (dist < game->circle.radius) {
	    //move particle to circle edge
	    p->s.center.x = game->circle.center.x + (d0/dist) * game->circle.radius * 1.01;
	    p->s.center.y = game->circle.center.y + (d1/dist) * game->circle.radius * 1.01;
	    //collision! apply a penalty to the particle
	    p->velocity.x += d0/dist * 2.0;
	    p->velocity.y += d1/dist * 2.0;
	}


	//check for off-screen
	if (p->s.center.y < 0.0) {
	    memcpy(&game->particle[i], &game->particle[game->n-1], 
		    sizeof(Particle));
	    std::cout << "off screen" << std::endl;
	    game->n--;
	}
    }
}

void render(Game *game)
{
    float w, h;
    glClear(GL_COLOR_BUFFER_BIT);
    //Draw shapes...

    const int n=40;
    static int firsttime = 1;
    static Vec vert[n];
    if (firsttime) {
	float ang = 0.0, inc = (3.14159 * 2.0) / (float)n;
	for (int i = 0; i < n; i++) {
	    vert[i].x = cos(ang) * game->circle.radius;
	    vert[i].y = sin(ang) * game->circle.radius;
	    ang += inc;
	}
	firsttime = 0;
    }

    //draw circle
    glColor3ub(255,255,255);
    glBegin(GL_LINE_LOOP);
    for (int i = 0; i < n; i++) {
	glVertex2i(
		game->circle.center.x + vert[i].x,
		game->circle.center.y + vert[i].y);
    }
    glEnd();

    //draw box
    Shape *s;
    glColor3ub(90,140,90);
    for (int i=0; i<5; i++) {
	s = &game->box[i];
	glPushMatrix();
	glTranslatef(s->center.x, s->center.y, s->center.z);
	w = s->width;
	h = s->height;
	glBegin(GL_QUADS);
	glVertex2i(-w,-h);
	glVertex2i(-w, h);
	glVertex2i( w, h);
	glVertex2i( w,-h);
	glEnd();
	glPopMatrix();
    }

    //draw all particles here
    glPushMatrix();
    //glColor3ub(150,160,220);
    for (int i = 0; i < game->n; i++) {
	Vec *c = &game->particle[i].s.center;
    	glColor3ub(rand() % 150, rand() % 102 + 102, 255);
	w = 2;
	h = 2;
	glBegin(GL_QUADS);
	glVertex2i(c->x-w, c->y-h);
	glVertex2i(c->x-w, c->y+h);
	glVertex2i(c->x+w, c->y+h);
	glVertex2i(c->x+w, c->y-h);
	glEnd();
    }
    glPopMatrix();
}




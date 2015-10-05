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
extern"C" {
    #include "fonts.h"
}

#define WINDOW_WIDTH  800
#define WINDOW_HEIGHT 600

#define MAX_PARTICLES 8000
#define GRAVITY 0.1
//for check keys
int bubbler=0;
//int keys[65536];

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
	Shape box[5];
	Shape circle;
	Particle particle[MAX_PARTICLES];
	int n;
	int lastMousex;
	int lastMousey;
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
	game.n=0;

	//declare a box shape
	game.box[0].width = 100;
	game.box[0].height = 10;
	game.box[0].center.x = 270 + 5*65;
	game.box[0].center.y = 500 - 5*60;
    game.box[1].width = 100;
    game.box[1].height = 10;
    game.box[1].center.x = 270 + 5*65 - 75;
    game.box[1].center.y = 500 - 5*60 + 45;
    game.box[2].width = 100;
    game.box[2].height = 10;
    game.box[2].center.x = 270 + 5*65 - 150;
    game.box[2].center.y = 500 - 5*60 + 90;
    game.box[3].width = 100;
    game.box[3].height = 10;
    game.box[3].center.x = 270 + 5*65 - 225;
    game.box[3].center.y = 500 - 5*60 + 135;
    game.box[4].width = 100;
    game.box[4].height = 10;
    game.box[4].center.x = 270 + 5*65 - 300;
    game.box[4].center.y = 500 - 5*60 + 180;
	game.circle.center.x = 760;
	game.circle.center.y = -25;
	game.circle.radius = 100;

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
	cleanup_fonts();
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
    //Do this to allow fonts
    glEnable(GL_TEXTURE_2D);
    initialize_fonts();
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
	p->velocity.y = rnd()*0.1 - 0.5;
	p->velocity.x = 0 + rnd()*1;                   /*1.0 + rnd()*0.1;*/
	game->n++;
}

void check_mouse(XEvent *e, Game *game)
{
	static int savex = 0;
	static int savey = 0;
	//static int n = 0; took out

	if (e->type == ButtonRelease) {
		return;
	}
	if (e->type == ButtonPress) {
		if (e->xbutton.button==1) {
			//Left button was pressed
			int y = WINDOW_HEIGHT - e->xbutton.y;

			for(int i=0; i<10; i++)
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
		int y = WINDOW_HEIGHT - e->xbutton.y;
        /*if(bubbler){
		    for (int i=0; i<10; i++)
		 	    makeParticle(game, e->xbutton.x, y); //make particle if mouse moves
        }*/
            //if (n++ < 10;
			//return;
	
			game->lastMousex = e->xbutton.x;  //added
			game->lastMousey = y;    //added
		
	}
}

int check_keys(XEvent *e, Game *game)
{
   /* static int shift=0;
    int key = XLookupKeysym(&e->xkey, 0);
    if(e->type == KeyRelease) {
        if (key == XK_Shift_L || key == XK_Shift_R)
            shift=0;
        return;
    }
    if (e->type == KeyPress) {
        if(key == XK_Shift_L || key == XK_Shift_R) {
            shift=1;
            return;
        }
    }else {
        return;
    }
    switch(key) {
        case XK_Escape:
            return 1;
        case XK_d:
            bubbler ^= 1;
    }
}
*/
	//Was there input from the keyboard?
	if (e->type == KeyPress) {
		int key = XLookupKeysym(&e->xkey, 0);
	
    
        if (key == XK_Escape) {
			return 1;
  	}
		//You may check other keys here
        if (key == XK_b) {
            bubbler ^= 1 ;
           	std::cout << "b works " << bubbler <<  std::endl;


            return 0; 
        }
            
	}
	return 0;
}

void movement(Game *game)
{
	Particle *p;

	if (game->n <= 0)
	    return;

    /*if(bubbler){
	    for(int i=0; i<10; i++)
	         makeParticle(game, game->lastMousex, game->lastMousey);
    }*/ //make constant particles

	for(int i=0; i<game->n; i++){
	    p = &game->particle[i];
		p->s.center.x += p->velocity.x;
		p->s.center.y += p->velocity.y;
		p->velocity.y -= GRAVITY;

	


	//check for collision with shapes...
	//add for loop using j and number of boxes
    for (int j=0; j<5; j++) {
	        Shape *s = &game->box[j];	 //make &game->box[]
	        if (p->s.center.y < s->center.y + s->height && 
		                    p->s.center.y > s->center.y - s->height &&
		                    p->s.center.x >= s->center.x - s->width && 
		                    p->s.center.x <= s->center.x + s->width){
                    p->velocity.y *= -0.45;
                    p->s.center.y = s->center.y + s->height + 0.01;
		    p->velocity.x = 1.0;

	    	       // p->s.center.y = s->center.y + s->height;
                   // p->velocity.y *= -.5;
	               // p->velocity.x = 2;                                 /*1.0 + rnd()*0.1;*/
	        }
    }
	//check circle collison
	        float d0,d1,dist;
	        d0 = p->s.center.x - game->circle.center.x;
	        d1 = p->s.center.y - game->circle.center.y;
	        dist = sqrt(d0*d0 + d1*d1);
	        if(dist <= game->circle.radius) {
	            //p->velocity.y = -p->velocity.y;
                //float v[2];
	            d0 /= dist;
	            d1 /= dist;
	            d0 *= game->circle.radius * 1.01;
	            d1 *= game->circle.radius * 1.01;
	            p->s.center.x = game->circle.center.x + d0;
	            p->s.center.y = game->circle.center.y + d1;
	            p->velocity.x += d0 * 0.02;
	            p->velocity.y += d0 * 0.02;

	        }
	        //end for loop
	        //check for off-screen
	        if (p->s.center.y < 0.0 || p->s.center.y > WINDOW_HEIGHT ) {
		        //std::cout << "off screen" << std::endl;
		            memcpy(&game->particle[i],  &game->particle[game->n-1], sizeof(Particle));
		            game->n--;
	        }
    }
}

void render(Game *game)
{ 
    
	float w, h;
	glClear(GL_COLOR_BUFFER_BIT);
	//Draw shapes...
	//
	//circle
	static int firsttime=1;
	static int verts[60][2];
	static int n = 60;
	if (firsttime) {

            float angle = 0.0;
	        float inc = (3.14159 * 2.0) / (float)n;
	        for (int i=0; i<n; i++) {
	                verts[i][0] = cos(angle) * game->circle.radius + game->circle.center.x;
	                verts[i][1] = sin(angle) * game->circle.radius + game->circle.center.y;
	                angle += inc;
            }
	        firsttime=0;
    }
	glColor3ub(90,140,90);
    glPushMatrix();
	glBegin(GL_TRIANGLE_FAN);
	    for (int i=0; i<n; i++)
		        glVertex2i(verts[i][0], verts[i][1]);  
	glEnd();
	glPopMatrix();
		
    //press b to turn on bubbler
    if(bubbler){
	    for(int i=0; i<10; i++)
	         makeParticle(game, game->lastMousex, game->lastMousey);
    }

	//draw box
	Shape *s;
	glColor3ub(90,140,90);
	for (int j=0; j<5; j++){
	        s = &game->box[j];
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
	for(int i=0; i<game->n; i++){
		    glPushMatrix();
		    glColor3ub(rnd()*150,rnd()*160,220);
		    Vec *c = &game->particle[i].s.center;
		    w = 2;
		    h = 2;
		    glBegin(GL_QUADS);
			glVertex2i(c->x-w, c->y-h);
			glVertex2i(c->x-w, c->y+h);
			glVertex2i(c->x+w, c->y+h);
			glVertex2i(c->x+w, c->y-h);
		    glEnd();
		    glPopMatrix();
	}
    Rect r[4];
    Rect re;
    Rect rec;
    glBindTexture(GL_TEXTURE_2D, 0);   
 
    r[0].bot =  495 - 5 * 60;
    r[0].left = 270 + 5 * 65;
    r[0].center = -100;
    r[1].bot =  495 - 5 * 60 + 45;
    r[1].left = 270 + 5 * 65 - 75;
    r[1].center = -100;      
    r[2].bot =  495 - 5 * 60 + 90;
    r[2].left = 270 + 5 * 65 - 150;
    r[2].center = -100;
    r[3].bot =  495 - 5 * 60 + 135;
    r[3].left = 270 + 5 * 65 - 225;
    r[3].center = -100;      
    re.bot =  495 - 5 * 60 + 180;
    re.left = 270 + 5 * 65 - 300;
    re.center = -100;      
    rec.bot =  495 - 5 * 60 + 225;
    rec.left = 270 + 5 * 65 - 375;
    rec.center = -100;      

    unsigned int cref = 0x00ffff;
    ggprint8b(&r[0], 16, cref, "Maintenance");
    ggprint8b(&r[1], 16, cref, "Testing");
    ggprint8b(&r[2], 16, cref, "Coding");
    ggprint8b(&r[3], 16, cref, "Design");
    ggprint8b(&re, 16, cref, "Requirements");
    ggprint16(&rec, 16, 0x00ffffff, "Waterfall Model");
}




/*HW1-A Simple Solar System
in this homework, you need to modify:
init() function, for texture building.
we provide code that read 24-bit bmp into a char* buffer(inside struct Image)

Display() function, for drawing required scene

and you also need to find the proper place(outside of init func.) to put the setting function of light source position
so that the light source truly lies at world coordinate (0,0,0), where the sun is located
**modifying LightPos[] is forbidden**(you can modify it to see the diffrence, but it should be (0,0,0,1) in your final code)

You can define your own function/datatype for drawing certain object or perform certain transformation...
*/
#include "../GL/glut.h"
#include <GL/glu.h>
#include <GL/gl.h>
#include <stdlib.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>

//IMPORTANT data type for image texture, you need to acesse its member for texture building
struct Image {
	unsigned long sizeX;
	unsigned long sizeY;
	char *data;
};
typedef struct Image Image;

//number of textures desired, you may want to change it to get bonus point
#define TEXTURE_NUM 7
//directories of image files
char* texture_name[TEXTURE_NUM] = {
	"../Resource/sun.bmp",
	"../Resource/mercury.bmp",
	"../Resource/earth.bmp",
	"../Resource/moon.bmp",
	"../Resource/jupiter.bmp",
	"../Resource/europa.bmp",
	"../Resource/sponge.bmp"
	//you may add additional image files here
};
//texture id array
GLuint texture[TEXTURE_NUM];


//time parameter for helping coordinate your animation, you may utilize it to help perform animation
#define deltaTime 100
double time = 0.0;

//light sorce parameter, no need to modify
//actually, modification of these parameter is forbidden in this homework
float LightPos[] = { 0.0f, 0.0f, 0.0f, 1.0f };//Light position		THIS CANNOT BE MODIFY
float LightAmb[] = { 0.0f, 0.0f, 0.0f, 1.0f };//Ambient Light Values
float LightDif[] = { 1.0f, 1.0f, 1.0f, 1.0f };//Diffuse Light Values
float LightSpc[] = { 1.0f, 1.0f, 1.0f, 1.0f };//Specular Light Values

GLUquadric * quad;
GLfloat sunr = 0.0f;
GLfloat earthr = 0.0f;
GLfloat moonr = 0.0f;
GLfloat mercuryr = 0.0f;
GLfloat jupiterr = 0.0f;
GLfloat times = 5.0f;

//24-bit bmp loading function, no need to modify it
int ImageLoad(char *filename, Image *image) {
	FILE *file;
	unsigned long size; // size of the image in bytes.
	unsigned long i; // standard counter.
	unsigned short int planes; // number of planes in image (must be 1)
	unsigned short int bpp; // number of bits per pixel (must be 24)
	char temp; // temporary color storage for bgr-rgb conversion.
			   // make sure the file is there.
	if ((file = fopen(filename, "rb")) == NULL) {
		printf("File Not Found : %s\n", filename);
		return 0;
	}

	// seek through the bmp header, up to the width/height:
	fseek(file, 18, SEEK_CUR);
	// read the width
	if ((i = fread(&image->sizeX, 4, 1, file)) != 1) {
		printf("Error reading width from %s.\n", filename);
		return 0;
	}

	//printf("Width of %s: %lu\n", filename, image->sizeX);
	// read the height
	if ((i = fread(&image->sizeY, 4, 1, file)) != 1) {
		printf("Error reading height from %s.\n", filename);
		return 0;
	}
	//printf("Height of %s: %lu\n", filename, image->sizeY);
	// calculate the size (assuming 24 bits or 3 bytes per pixel).
	size = image->sizeX * image->sizeY * 3;
	// read the planes
	if ((fread(&planes, 2, 1, file)) != 1) {
		printf("Error reading planes from %s.\n", filename);
		return 0;
	}
	if (planes != 1) {
		printf("Planes from %s is not 1: %u\n", filename, planes);
		return 0;
	}
	// read the bitsperpixel
	if ((i = fread(&bpp, 2, 1, file)) != 1) {
		printf("Error reading bpp from %s.\n", filename);
		return 0;
	}
	if (bpp != 24) {
		printf("Bpp from %s is not 24: %u\n", filename, bpp);
		return 0;
	}
	// seek past the rest of the bitmap header.
	fseek(file, 24, SEEK_CUR);
	// read the data.
	image->data = (char *)malloc(size);
	if (image->data == NULL) {
		printf("Error allocating memory for color-corrected image data");
		return 0;
	}
	if ((i = fread(image->data, size, 1, file)) != 1) {
		printf("Error reading image data from %s.\n", filename);
		return 0;
	}
	for (i = 0; i<size; i += 3) { // reverse all of the colors. (bgr -> rgb)
		temp = image->data[i];
		image->data[i] = image->data[i + 2];
		image->data[i + 2] = temp;
	}
	// we're done.
	return 1;
}
//memory allocation and file reading for an Image datatype, no need to modify it
Image * loadTexture(char *filename) {
	Image *image;
	// allocate space for texture
	image = (Image *)malloc(sizeof(Image));
	if (image == NULL) {
		printf("Error allocating space for image");
		getchar();
		exit(0);
	}
	if (!ImageLoad(filename, image)) {
		getchar();
		exit(1);
	}
	return image;
}

/*Closed SolidCylinder*/
GLvoid DrawCircleArea(float cx, float cy, float cz, float r, int num_segments)
{
	GLfloat vertex[4];

	const GLfloat delta_angle = 2.0*M_PI / num_segments;
	glBegin(GL_TRIANGLE_FAN);

	vertex[0] = cx;
	vertex[1] = cy;
	vertex[2] = cz;
	vertex[3] = 1.0;
	glVertex4fv(vertex);

	//draw the vertex on the contour of the circle   
	for (int i = 0; i < num_segments; i++)
	{
		vertex[0] = std::cos(delta_angle*i) * r + cx;
		vertex[1] = std::sin(delta_angle*i) * r + cy;
		vertex[2] = cz;
		vertex[3] = 1.0;
		glVertex4fv(vertex);
	}

	vertex[0] = 1.0 * r + cx;
	vertex[1] = 0.0 * r + cy;
	vertex[2] = cz;
	vertex[3] = 1.0;
	glVertex4fv(vertex);
	glEnd();
}

void mySolidCylinder(GLUquadric*   quad,GLdouble base,GLdouble top,GLdouble height,GLint slices,GLint stacks)
{
	glEnable(GL_COLOR_MATERIAL);//enable this parameter to use glColor() as material of lighting model
	//glColor3f(0.5f, 1.0f, 0.7f);
	glTranslatef(0.0, 0.0, 0.0);			//not neccessary
	glRotatef(0, 0.5, 1.0, 0.0);
	gluCylinder(quad, base, top, height, slices, stacks);
	//top   
	DrawCircleArea(0.0, 0.0, height, top, slices);
	//base   
	DrawCircleArea(0.0, 0.0, 0.0, base, slices);
}

void mySatellite(void)
{
	glPushMatrix();
		glRotatef(210, 0, 1, 0);
		glColor3f(0.5f, 1.0f, 0.7f);
		mySolidCylinder(quad, 0.75f, 0.75f, 2.5f, 32, 32);		//SolidCylinder
		glScalef(0.25, 5.0, 2.5); /*modeling transformation */
		glTranslatef(0.0, 0.0, 0.5);							//square against solidcylinder
		glColor3f(0.5f, 1.0f, 0.7f);
		glRotatef(90, 0, 1, 0);
		glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
		glRotatef(210, 0, 1, 0);
		glColor3f(0.5f, 1.0f, 0.7f);
		glRotatef(270, 0, 1, 0);
		glTranslatef(1.0, -0.5, -1.0);
		gluCylinder(quad, 1.5f, 0, 0.75f, 32, 32);
	glPopMatrix();
}

void mySatellite2(void)
{
	//glDisable(GL_LIGHTING);
	glPushMatrix();
		glRotatef(210, 0, 1, 0);
		glColor3f(1.0, 1.0f, 0.2f);
		mySolidCylinder(quad, 0.75f, 0.75f, 2.5f, 32, 32);		//SolidCylinder
		glScalef(0.25, 5.0, 2.5); /*modeling transformation */
		glTranslatef(0.0, 0.0, 0.5);							//square against solidcylinder
		glColor3f(1.0, 1.0f, 0.2f);
		glRotatef(90, 0, 1, 0);
		glutSolidCube(1.0);
	glPopMatrix();

	glPushMatrix();
		glRotatef(210, 0, 1, 0);
		glColor3f(1.0f, 1.0f, 0.2f);
		glRotatef(270, 0, 1, 0);
		glTranslatef(1.0, -0.5, -1.0);
		gluCylinder(quad, 1.5f, 0, 0.75f, 32, 32);
	glPopMatrix();
}

/*Lighting*/


//callback function for drawing a frame
void Display(void)
{
	glEnable(GL_DEPTH_TEST);//when you draw something without texture, be sure to disable GL_TEXTURE_2D
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(0, 0, 40, 0, 0, 0, 0, 1, 0);//set the view part of modelview matrix

	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);//Set Light1 Position, this setting function should be at another place
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmb);//Set Light1 Ambience
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDif);//Set Light1 Diffuse
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpc);//Set Light1 Specular

	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT1);
	glEnable(GL_TEXTURE_2D);
	
	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[0]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

		glTranslatef(0.0, 0.0, 0.0);		//original
		glRotatef(sunr, 0.2, 1, 0);
		gluQuadricTexture(quad, 1);
		glColor3f(1.0, 0.7, 0.0);
		gluSphere(quad, 4.5, 30, 30);///glutSolidTeapot(0.3);					//The sun 0

	glPopMatrix();

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[1]);
		glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);

		glRotatef(1.25*mercuryr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 7);
		glRotatef(0.25*mercuryr, 0.0, 0.0, 1.0);
		glColor3f(1.5, 1.5, 1.5);
		gluSphere(quad, 1, 20, 20);
	glPopMatrix();													//Mercury 7

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[2]);

		glRotatef(0.75*earthr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 15);
		glRotatef(270, 1, 0, 0);											//Rotate itself
		glRotatef(0.25*earthr, 0.0, 0.0, -1.0);
		glColor3f(1, 1, 1);
		glScalef(1.0f, 1.25f, 1.0f);									//FLATTER
		gluSphere(quad, 2.5, 20, 20);									//Earth 15
		
	glPopMatrix();

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[3]);

		glRotatef(0.75*earthr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 15);
		glRotatef(2 * earthr, 0.5, 10, 0.0);
		glTranslatef(0.0, 0.0, 4.25);
		glRotatef(270, 1, 0, 0);
		glRotatef(moonr, 0.0, 1, 0.0);
		glColor3f(1.4, 1.4, 1.4);
		gluSphere(quad, 1.0, 20, 20);										//Moon 15
	glPopMatrix();

	glPushMatrix();
		glDisable(GL_TEXTURE_2D);
		glRotatef(0.75*earthr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 15);
		glRotatef(0.25*earthr, 0.0, 0.0, 1.0);
		glTranslatef(0.0, 0.0, 4);
		glScalef(0.4f, 0.4f, 0.4f);
		mySatellite();

		glEnable(GL_TEXTURE_2D);
	glPopMatrix();												//Satellite

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[4]);
	
		glRotatef(0.75*jupiterr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 25);
		glRotatef(90, 1, 0, 0);										//Rotate itself
		glRotatef(0.25*jupiterr, 0.0, 0.0, -1.0);
		glColor3f(1, 1, 1);
		gluSphere(quad, 2.5, 20, 20);
	glPopMatrix();													//Jupiter 25

	glPushMatrix();
		glDisable(GL_TEXTURE_2D);

		glRotatef(0.75*jupiterr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 25);
		glRotatef(1.5*jupiterr, 0.0, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 3.5);
		glRotatef(moonr, 0.0, 1.0, 0.0);
		glScalef(0.3f, 0.3f, 0.3f);
		mySatellite2();

		glEnable(GL_TEXTURE_2D);
	glPopMatrix();												//Satellite2

	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[5]);

		glRotatef(0.75*jupiterr, 0.2, 1.0, 0.0);
		glTranslatef(0.0, 0.0, 25);
		glRotatef(2.5*jupiterr, 0.0, 1.0, 0.0);		//europa revolution against Y-axis
		glTranslatef(0.0, 0.0, 4.0);
		glRotatef(moonr, 0.0, 1.0, 0.0);
		glColor3f(1.6, 1.6, 1.6);
		gluSphere(quad, 1, 20, 20);
	glPopMatrix();													//Europa 25


	glPushMatrix();
		glBindTexture(GL_TEXTURE_2D, texture[6]);

		glTranslatef(0.0, 0.0, 0.0);
		glRotatef(0.8*earthr, 0, 0.5, 0.0);
		glTranslatef(0.0, 0.0, 27.5);
		glRotatef(1.5*earthr, 0.5, 3.5, 1.0);
		glColor3f(1.2, 1.2, 1.2);
		gluSphere(quad, 1.5, 20, 20);
	glPopMatrix();													//Sponge	27.5



	glutSwapBuffers();//swap the drawn buffer to the window
}

//callback funtion as a timer, no need to modify it
void Tick(int id){
	double d = deltaTime / 1000.0;
	time += d;

	sunr = sunr + 0.037f * times;
	earthr = earthr + 1.0f * times;
	moonr = moonr + 0.33f * times;
	mercuryr = mercuryr + 0.64 * times;
	jupiterr = jupiterr + 0.59 * times;
	
	glutPostRedisplay();
	glutTimerFunc(deltaTime, Tick, 0); // 100ms for time step size
}

//callback function when the window size is changed, no need to modify it
void WindowSize(int w, int h)
{
	glViewport(0, 0, w, h);							//changing the buffer size the same to the window size
	glMatrixMode(GL_PROJECTION);					//choose the projection matrix
	glLoadIdentity();
	gluPerspective(70, (double)w /(double)h, 1.0, 1000.0);//set the projection matrix as perspective mode
	glMatrixMode(GL_MODELVIEW);						//it is suggested that modelview matrix is chosen if nothing specific being performed
	glLoadIdentity();
}

//initialization for parameters of this program, you must perform something here
void init()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);//set what color should be used when we clean the color buffer
	glEnable(GL_TEXTURE_2D);
	glEnable(GL_LIGHT1);//Enable Light1
	glEnable(GL_LIGHTING);//Enable Lighting
	//***********
	glLightfv(GL_LIGHT1, GL_POSITION, LightPos);//Set Light1 Position, this setting function should be at another place
	//***********
	glLightfv(GL_LIGHT1, GL_AMBIENT, LightAmb);//Set Light1 Ambience
	glLightfv(GL_LIGHT1, GL_DIFFUSE, LightDif);//Set Light1 Diffuse
	glLightfv(GL_LIGHT1, GL_SPECULAR, LightSpc);//Set Light1 Specular
	//since a light source is also an object in the 3D scene, we set its position in the display function after gluLookAt()
	//you should know that straight texture mapping(without modifying shader) may not have shading effect
	//you need to tweak certain parameter(when building texture) to obtain a lit and textured object
	glShadeModel(GL_SMOOTH);//shading model
	
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);//you can choose which part of lighting model should be modified by texture mapping
	glEnable(GL_COLOR_MATERIAL);//enable this parameter to use glColor() as material of lighting model

	//please load all the textures here
	//use Image* loadTexture(file_directory) function to obtain char* data and size info.
	quad = gluNewQuadric();
	Image* img0 = loadTexture(texture_name[0]);
	Image* img1 = loadTexture(texture_name[1]);
	Image* img2 = loadTexture(texture_name[2]);
	Image* img3 = loadTexture(texture_name[3]);
	Image* img4 = loadTexture(texture_name[4]);
	Image* img5 = loadTexture(texture_name[5]);
	Image* img6 = loadTexture(texture_name[6]);
	

	glGenTextures(7, texture);
	glBindTexture(GL_TEXTURE_2D, texture[0]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img0->sizeX, img0->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img0->data);

	glBindTexture(GL_TEXTURE_2D, texture[1]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img1->sizeX, img1->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img1->data);

	glBindTexture(GL_TEXTURE_2D, texture[2]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img2->sizeX, img2->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img2->data);

	glBindTexture(GL_TEXTURE_2D, texture[3]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img3->sizeX, img3->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img3->data);

	glBindTexture(GL_TEXTURE_2D, texture[4]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img4->sizeX, img4->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img4->data);

	glBindTexture(GL_TEXTURE_2D, texture[5]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img5->sizeX, img5->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img5->data);

	glBindTexture(GL_TEXTURE_2D, texture[6]);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR); //scale linearly when image bigger than texture
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR); //scale linearly when image smalled than texture
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, img6->sizeX, img6->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, img6->data);

}
int main(int argc, char *argv[])
{
	glutInit(&argc, argv);//glut function for simplifying OpenGL initialization
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);//demanding: double-framed buffer | RGB colorbuffer | depth buffer
	glutInitWindowPosition(100, 50);//set the initial window position
	glutInitWindowSize(800, 600);//set the initial window size
	//**************************
	glutCreateWindow("CG_HW1_0656154");//IMPORTANT!!重要!! Create the window and set its title, please replace 0123456 with your own student ID
	//**************************
	glutDisplayFunc(Display);//callback funtion for drawing a frame
	glutReshapeFunc(WindowSize);//callback function when the window size is changed
	glutTimerFunc(deltaTime, Tick, 0);//timer function
	//you may want to write your own callback funtion for other events(not demanded nor forbidden)
	init();//self-defined initialization function for the elegance of your code

	glutMainLoop();
	return 0;
}
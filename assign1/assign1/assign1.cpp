// assign1.cpp : Defines the entry point for the console application.
//

/*
  CSCI 480 Computer Graphics
  Assignment 1: Height Fields
  C++ starter code
*/

#include "stdafx.h"
#include <pic.h>
#include <windows.h>
#include <stdlib.h>
#include <GL/glu.h>
#include <GL/glut.h>

int g_iMenuId;

int g_vMousePos[2] = {0, 0};
int g_iLeftMouseButton = 0;    /* 1 if pressed, 0 if not */
int g_iMiddleMouseButton = 0;
int g_iRightMouseButton = 0;

/* Added vals */
GLfloat delta = 10.0f;

GLdouble FOV_Y = 120.0;

GLfloat maxHeight = 30.0f;

GLfloat TRANSLATE_SPEED = 10.0f;

bool g_bWireframeMode = false;

bool g_bTextureMapped = false;

enum MESHMODE { VERTICES = -1, WIREFRAME = -2, SURFACE = -3, WIRE_AND_SURFACE = -4};

MESHMODE g_MeshMode = SURFACE;

typedef enum { ROTATE, TRANSLATE, SCALE } CONTROLSTATE;

CONTROLSTATE g_ControlState = ROTATE;

/* state of the world */
float g_vLandRotate[3] = {0.0, 0.0, 0.0};
float g_vLandTranslate[3] = {0.0, 0.0, 0.0};
float g_vLandScale[3] = {1.0, 1.0, 1.0};
GLfloat theta[3] = {0.0f,0.0f,0.0f};

/* see <your pic directory>/pic.h for type Pic */
Pic * g_pHeightData;
Pic* g_pColorData;

/* Write a screenshot to the specified filename */
void saveScreenshot (char *filename)
{
  int i, j;
  Pic *in = NULL;

  if (filename == NULL)
    return;

  /* Allocate a picture buffer */
  in = pic_alloc(640, 480, 3, NULL);

  printf("File to save to: %s\n", filename);

  for (i=479; i>=0; i--) {
    glReadPixels(0, 479-i, 640, 1, GL_RGB, GL_UNSIGNED_BYTE,
                 &in->pix[i*in->nx*in->bpp]);
  }

  if (jpeg_write(filename, in))
    printf("File saved Successfully\n");
  else
    printf("Error in Saving\n");

  pic_free(in);
}

void myinit()
{
  /* setup gl view here */
  glClearColor(0.0, 0.0, 0.0, 0.0);

  glShadeModel(GL_SMOOTH);

  glClear(GL_COLOR_BUFFER_BIT);

  glutSwapBuffers();
}

void inline makeCube(float color)
{
  if (g_bWireframeMode)
  {
    glBegin(GL_POLYGON);
      glVertex3f(-0.5f, -0.5f, 0.0f - color);
      glVertex3f(-0.5f, 0.5f, 0.0f - color);
      glVertex3f(0.5f, 0.5f, 0.0f - color);
      glVertex3f(0.5f, -0.5f, 0.0f - color);
    glEnd();
  }
  else
  {
    glBegin(GL_POLYGON);
      glColor3f(0.5f + color, 0.5f, 0.5f);
      glVertex3f(-0.5f, -0.5f, 0.0f - color);
      glColor3f(0.0f, 0.0f + color, 1.0f);
      glVertex3f(-0.5f, 0.5f, 0.0f - color);
      glColor3f(0.0f, 0.0f, 0.0f + color);
      glVertex3f(0.5f, 0.5f, 0.0f - color);
      glColor3f(1.0f, 1.0f, 0.0f);
      glVertex3f(0.5f, -0.5f, 0.0f - color);
    glEnd();
  }
}

void spinCube()
{
  GLfloat delta = 2.0f;
  GLint axis = 1;

  theta[axis] += delta;
  
  if (theta[axis] > 360.0f)
  {
    theta[axis] -= 360.0f;
  }
}

inline void makeHeightMap()
{
  int width = 0;
  int height = 0;

  if (g_pHeightData->bpp == 1)
  {
    int height = 0;
    int width = 0;

    width = g_pHeightData->nx;
    height = g_pHeightData->ny;
    
    glBegin(GL_TRIANGLES);
    
    int val[2] = {0, 0};
    glGetIntegerv(GL_POLYGON_MODE, val);
    
    if (g_bWireframeMode)
    {
      /*  Wireframe Mode */

      glColor3f(0.0f, 1.0f, 0.0f);

      for(int i = 0; i < height-1; i++)
      {
        for (int j = 0; j < width-1; j++)
        {
          int xPos = j - width / 2;
          int yPos = i - height / 2;

          float heightValue = float(PIC_PIXEL(g_pHeightData, j, i, 1)) / 255.0f;
          glVertex3f(xPos, heightValue * maxHeight, yPos);
        
          heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i, 1)) / 255.0f;
          glVertex3f(xPos + 1, heightValue * maxHeight, yPos);
        
          heightValue = float(PIC_PIXEL(g_pHeightData, j, i + 1, 1)) / 255.0f;
          glVertex3f(xPos, heightValue * maxHeight, yPos + 1);
        
          if (i > 0)
          {
            int xPos = j - width / 2;
            int yPos = i - height / 2;
          
            float heightValue = float(PIC_PIXEL(g_pHeightData, j, i, 1)) / 255.0f;
            glVertex3f(xPos, heightValue * maxHeight, yPos);

            heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i - 1, 1)) / 255.0f;
            glVertex3f(xPos + 1, heightValue * maxHeight, yPos - 1);

            heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i, 1)) / 255.0f;
            glVertex3f(xPos + 1, heightValue * maxHeight, yPos);
          }
        }
      }
    }
    else
    {
      /* Normal Mesh Mode*/

      for(int i = 0; i < height-1; i++)
      {
        for (int j = 0; j < width-1; j++)
        {
          int xPos = j - width / 2;
          int yPos = i - height / 2;

          float heightValue = float(PIC_PIXEL(g_pHeightData, j, i, 1)) / 255.0f;
          if (g_bTextureMapped)
          {
            float rValue = float(PIC_PIXEL(g_pColorData, j, i, 0)) / 255.0f;
            float gValue = float(PIC_PIXEL(g_pColorData, j, i, 1)) / 255.0f;
            float bValue = float(PIC_PIXEL(g_pColorData, j, i, 2)) / 255.0f;

            glColor3f(rValue, gValue, bValue);
          }
          else
          {
            glColor3f(0.0f, 0.0f, heightValue);
          }
          glVertex3f(xPos, heightValue * maxHeight, yPos);
        
          heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i, 1)) / 255.0f;
          if (g_bTextureMapped)
          {
            float rValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 0)) / 255.0f;
            float gValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 1)) / 255.0f;
            float bValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 2)) / 255.0f;

            glColor3f(rValue, gValue, bValue);
          }
          else
          {
            glColor3f(0.0f, 0.0f, heightValue);
          }
          glVertex3f(xPos + 1, heightValue * maxHeight, yPos);
        
          heightValue = float(PIC_PIXEL(g_pHeightData, j, i + 1, 1)) / 255.0f;
          if (g_bTextureMapped)
          {
            float rValue = float(PIC_PIXEL(g_pColorData, j, i + 1, 0)) / 255.0f;
            float gValue = float(PIC_PIXEL(g_pColorData, j, i + 1, 1)) / 255.0f;
            float bValue = float(PIC_PIXEL(g_pColorData, j, i + 1, 2)) / 255.0f;

            glColor3f(rValue, gValue, bValue);
          }
          else
          {
            glColor3f(0.0f, 0.0f, heightValue);
          }
          glVertex3f(xPos, heightValue * maxHeight, yPos + 1);
        
          if (i > 0)
          {
            int xPos = j - width / 2;
            int yPos = i - height / 2;
          
            float heightValue = float(PIC_PIXEL(g_pHeightData, j, i, 1)) / 255.0f;
            if (g_bTextureMapped)
            {
              float rValue = float(PIC_PIXEL(g_pColorData, j, i, 0)) / 255.0f;
              float gValue = float(PIC_PIXEL(g_pColorData, j, i, 1)) / 255.0f;
              float bValue = float(PIC_PIXEL(g_pColorData, j, i, 2)) / 255.0f;

              glColor3f(rValue, gValue, bValue);
            }
            else
            {
              glColor3f(0.0f, 0.0f, heightValue);
            }
            glVertex3f(xPos, heightValue * maxHeight, yPos);

            heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i - 1, 1)) / 255.0f;
            if (g_bTextureMapped)
            {
              float rValue = float(PIC_PIXEL(g_pColorData, j + 1, i - 1, 0)) / 255.0f;
              float gValue = float(PIC_PIXEL(g_pColorData, j + 1, i - 1, 1)) / 255.0f;
              float bValue = float(PIC_PIXEL(g_pColorData, j + 1, i - 1, 2)) / 255.0f;

              glColor3f(rValue, gValue, bValue);
            }
            else
            {
              glColor3f(0.0f, 0.0f, heightValue);
            }
            glVertex3f(xPos + 1, heightValue * maxHeight, yPos - 1);

            heightValue = float(PIC_PIXEL(g_pHeightData, j + 1, i, 1)) / 255.0f;
            if (g_bTextureMapped)
            {
              float rValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 0)) / 255.0f;
              float gValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 1)) / 255.0f;
              float bValue = float(PIC_PIXEL(g_pColorData, j + 1, i, 2)) / 255.0f;

              glColor3f(rValue, gValue, bValue);
            }
            else
            {
              glColor3f(0.0f, 0.0f, heightValue);
            }
            glVertex3f(xPos + 1, heightValue * maxHeight, yPos);
          }
        }
      }
    }
    
    glEnd();
  }
  else 
  {
    printf("more than 1 byte per pixel in height map");
  }
}

void renderGroundPlane()
{
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glBegin(GL_POLYGON);
	  glColor3f(1.0f,1.0f,1.0f);
	  glVertex3f(-1000.0f,0.0f,-1000.0f);
	  glVertex3f(-1000.0f,0.0f,1000.0f);
	  glVertex3f(1000.0f,0.0f,1000.0f);
	  glVertex3f(1000.0f,0.0f,-1000.0f);
	glEnd();
}

inline void drawObjects()
{  
  int val[2];
  glGetIntegerv(GL_POLYGON_MODE, val);

  if(val[0] == GL_LINE)
  {
    g_bWireframeMode = true;
  }
  else
  {
    g_bWireframeMode = false;
  }

  makeHeightMap();
  /*
  makeCube(0.0f);
  makeCube(0.2f);
  */
}

void display()
{
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

  glLoadIdentity();
  
  /* Scaling, Rotating, then Translating all objects */
  glScalef(g_vLandScale[0], g_vLandScale[1], g_vLandScale[2]); 
  
  glRotatef(g_vLandRotate[0], 1.0f, 0.0f, 0.0f);
  glRotatef(g_vLandRotate[1], 0.0f, 1.0f, 0.0f);
  glRotatef(g_vLandRotate[2], 0.0f, 0.0f, 1.0f);
  
  glTranslatef(g_vLandTranslate[0] * TRANSLATE_SPEED, g_vLandTranslate[1] * TRANSLATE_SPEED, g_vLandTranslate[2] * TRANSLATE_SPEED); 

  /*  Renders a ground plane, but it stays relative to the object */
  //renderGroundPlane();

  /* Determine how to render the objects */
  switch (g_MeshMode)
  {
  case VERTICES:
    glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
    break;
  case WIREFRAME:
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;
  case SURFACE:
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    break;
  case WIRE_AND_SURFACE:
    glPolygonOffset(1.0f,1.0f); // eliminate z-buffer fighting
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    drawObjects();
    glDisable(GL_POLYGON_OFFSET_FILL);
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    break;    
  }

  drawObjects();

  glutSwapBuffers();
}

void menufunc(int value)
{
  switch (value)
  {
    case 0:
      exit(0);
      break;
  }
}

void doIdle()
{
  /* do some stuff... */
  spinCube();

  /* make the screen update */
  glutPostRedisplay();
}

/* converts mouse drags into information about 
rotation/translation/scaling */
void mousedrag(int x, int y)
{
  int vMouseDelta[2] = {x-g_vMousePos[0], y-g_vMousePos[1]};
  
  switch (g_ControlState)
  {
    case TRANSLATE:  
      if (g_iLeftMouseButton)
      {
        g_vLandTranslate[0] += vMouseDelta[0]*0.01;
        g_vLandTranslate[1] -= vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandTranslate[2] += vMouseDelta[1]*0.01;
      }
      break;
    case ROTATE:
      if (g_iLeftMouseButton)
      {
        g_vLandRotate[0] += vMouseDelta[1];
        g_vLandRotate[1] += vMouseDelta[0];
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandRotate[2] += vMouseDelta[1];
      }
      break;
    case SCALE:
      if (g_iLeftMouseButton)
      {
        g_vLandScale[0] *= 1.0+vMouseDelta[0]*0.01;
        g_vLandScale[1] *= 1.0-vMouseDelta[1]*0.01;
      }
      if (g_iMiddleMouseButton)
      {
        g_vLandScale[2] *= 1.0-vMouseDelta[1]*0.01;
      }
      break;
  }
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mouseidle(int x, int y)
{
  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void mousebutton(int button, int state, int x, int y)
{

  switch (button)
  {
    case GLUT_LEFT_BUTTON:
      g_iLeftMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_MIDDLE_BUTTON:
      g_iMiddleMouseButton = (state==GLUT_DOWN);
      break;
    case GLUT_RIGHT_BUTTON:
      g_iRightMouseButton = (state==GLUT_DOWN);
      break;
  }
 
  switch(glutGetModifiers())
  {
    case GLUT_ACTIVE_CTRL:
      g_ControlState = TRANSLATE;
      break;
    case GLUT_ACTIVE_SHIFT:
      g_ControlState = SCALE;
      break;
    default:
      g_ControlState = ROTATE;
      break;
  }

  g_vMousePos[0] = x;
  g_vMousePos[1] = y;
}

void keyboard(unsigned char key, int x, int y)
{
  if(key == 'm' || 'M')
  {
    switch (g_MeshMode)
    {
    case VERTICES:
      g_MeshMode = WIREFRAME;
      break;
    case WIREFRAME:
      g_MeshMode = SURFACE;
      break;
    case SURFACE:
      g_MeshMode = WIRE_AND_SURFACE;
      break;
    case WIRE_AND_SURFACE:
      g_MeshMode = VERTICES;
      break;    
    }
  }
}

void reshape(int width, int height)
{
	GLfloat aspect = (GLfloat) width / (GLfloat) height;
	glViewport(0, 0, width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if (width <= height) /* aspect <= 1 */
  {
    /* Orthographic Projection */
    //glOrtho(-2.0, 2.0, -2.0/aspect, 2.0/aspect, -10.0, 10.0);

    /* Perspective Projection */
    gluPerspective(FOV_Y, aspect, 0.01f, 1000.0f);
  }
  else /* aspect > 1 */
	{
    /* Orthographic Projection */
    //glOrtho(-2.0*aspect, 2.0*aspect, -2.0, 2.0, -10.0, 10.0);

    /* Perspective Projection */
    gluPerspective(FOV_Y, aspect, 0.01f, 1000.0f);
  }

  // rotate camera to make the heightfield visible
  glRotatef(20.0f, 1.0f, 0.0f, 0.0f);

  // move the camera to make the heightfield visible
  glTranslatef(0.0f,-150.0f, -200.0f);

  glMatrixMode(GL_MODELVIEW);
}

int _tmain(int argc, _TCHAR* argv[])
{
	// I've set the argv[1] to spiral.jpg.
	// To change it, on the "Solution Explorer",
	// right click "assign1", choose "Properties",
	// go to "Configuration Properties", click "Debugging",
	// then type your texture name for the "Command Arguments"
	if (argc<2)
	{  
		printf ("usage: %s heightfield.jpg\n", argv[0]);
		exit(1);
	}

	g_pHeightData = jpeg_read((char*)argv[1], NULL);
	
  if (!g_pHeightData)
	{
	    printf ("error reading %s.\n", argv[1]);
	    exit(2);
	}
  
  // if there is a second parameter, it is the texture map
  if (argc > 2)
  {
    g_bTextureMapped = true;
    g_pColorData = jpeg_read((char*)argv[2], NULL);
  }

  if (g_bTextureMapped && !g_pColorData || g_pColorData && g_pColorData->bpp < 3)
  {
      printf ("error reading %s.\n", argv[2]);
	    exit(3);
  }

	glutInit(&argc,(char**)argv);
  
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);

	glutInitWindowSize(640, 480);
	
	glutInitWindowPosition(0,0);

	glutCreateWindow("Cool Window");

	glEnable(GL_DEPTH_TEST); // keeps things from being drawn if "behind" other objects

	/* tells glut to use a particular display function to redraw */
	glutDisplayFunc(display);
  
	/* allow the user to quit using the right mouse button menu */
	g_iMenuId = glutCreateMenu(menufunc);
	glutSetMenu(g_iMenuId);
	glutAddMenuEntry("Quit",0);
	glutAttachMenu(GLUT_RIGHT_BUTTON);
  
	/* replace with any animate code */
	glutIdleFunc(doIdle);

	/* callback for mouse drags */
	glutMotionFunc(mousedrag);
	/* callback for idle mouse movement */
	glutPassiveMotionFunc(mouseidle);
	/* callback for mouse button changes */
	glutMouseFunc(mousebutton);
  /* callback for keyboard buttons */
  glutKeyboardFunc(keyboard);
	/* callback for window resizing */
	glutReshapeFunc(reshape);

	/* do initialization */
	myinit();

	glutMainLoop();
	return 0;
}
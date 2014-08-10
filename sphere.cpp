#include <stdio.h>
#include <stdlib.h>
#include <math.h>
/* Use glew.h instead of gl.h to get all the GL prototypes declared */
#include <GL/glew.h>
/* Using the GLUT library for the base windowing setup */
#include <GL/freeglut.h>
/* Use GLM mathematics library*/
#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
/* Use shader_utils.h for create and link shaders */
#include "shader_utils.h"
#include <iostream>
using namespace std;
/* Use the SOIL library for loading image */
#include <SOIL/SOIL.h>


// Global variables
int screen_width=800, screen_height=600;
int last_mx = 0, last_my = 0, cur_mx = 0, cur_my = 0;
int trackball_on = false;
GLuint spere_vbo;
GLuint program;
GLuint mytexture_id;
GLint attribute_v_coord;
GLint uniform_mytexture;
GLint uniform_m, uniform_v, uniform_p, uniform_rot;

glm::mat4 model = glm::mat4(1.0f);
glm::mat4 view = glm::lookAt(glm::vec3(0.0, 0.0, 2.5), glm::vec3(0.0, 0.0, 0.0), glm::vec3(0.0, 1.0, 0.0));
glm::mat4 project = glm::perspective(45.0f, 1.0f*screen_width/screen_height, 0.1f, 10.0f);
glm::vec4 rotate = glm::vec4(1.0f,glm::vec3(0.0f));


struct filename {
    const char* texture_filename;
    const char* vshader_filename;
    const char* fshader_filename;
};
struct filename files = {"Earthmap.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"BasketballColor.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"SoftballColor.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"SoftballBump.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"NewTennisBallColor.jpg","sphere.v.glsl","sphere.f.glsl"};
//struct filename files = {"Pool Ball Skins/Ball13.jpg","sphere.v.glsl","sphere.f.glsl"};


// Create a 3D sphere.
GLuint sphere(float radius, int slices, int stacks) {
  GLuint vbo;
  int n = 2 * (slices + 1) * stacks;
  int i = 0;
  glm::vec3 points[n];
  
  for (float theta = -M_PI / 2; theta < M_PI / 2 - 0.0001; theta += M_PI / stacks) {
    for (float phi = -M_PI; phi <= M_PI + 0.0001; phi += 2 * M_PI / slices) {
      points[i++] = glm::vec3(cos(theta) * sin(phi), -sin(theta), cos(theta) * cos(phi));
      points[i++] = glm::vec3(cos(theta + M_PI / stacks) * sin(phi), -sin(theta + M_PI / stacks), cos(theta + M_PI / stacks) * cos(phi));
    }
  }
  
  glGenBuffers(1, &vbo);
  glBindBuffer(GL_ARRAY_BUFFER, vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof points, points, GL_STATIC_DRAW);
  
  return vbo;
}


// Initial VBO, shader program, attributes, uniforms, and load texture.
int init_resources()
{
	printf("init_resources: %s, %s, %s\n", files.texture_filename, files.vshader_filename, files.fshader_filename);

	// Create spere VBO//
	spere_vbo = sphere(1, 60, 60);

	// Load texture image using SOIL//
	mytexture_id = SOIL_load_OGL_texture
    (
     files.texture_filename,
     SOIL_LOAD_AUTO,
     SOIL_CREATE_NEW_ID,
     SOIL_FLAG_INVERT_Y | SOIL_FLAG_TEXTURE_REPEATS
     );
  	if (mytexture_id == 0)
    	cerr << "SOIL loading error: '" << SOIL_last_result() << "' (" << files.texture_filename << ")" << endl;
    
    // Create and link shader//
    GLint link_ok = GL_FALSE;
    GLuint vs, fs;
    if ((vs = create_shader(files.vshader_filename, GL_VERTEX_SHADER))   == 0) return 0;
    if ((fs = create_shader(files.fshader_filename, GL_FRAGMENT_SHADER)) == 0) return 0;

    program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &link_ok);
    if (!link_ok) {
      fprintf(stderr, "glLinkProgram:");
      print_log(program);
      return 0;
    }

    const char* attribute_name;
    attribute_name = "v_coord";
    attribute_v_coord = glGetAttribLocation(program, attribute_name);
    if (attribute_v_coord == -1) {
      fprintf(stderr, "Could not bind attribute %s\n", attribute_name);
      return 0;
    }
    
    const char* uniform_name;
    uniform_name = "mytexture";
    uniform_mytexture = glGetUniformLocation(program, uniform_name);
    if (uniform_mytexture == -1) {
      fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
      return 0;
    }

    uniform_name = "m";
    uniform_m = glGetUniformLocation(program, uniform_name);
    if (uniform_m == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }

    uniform_name = "rot";
    uniform_rot = glGetUniformLocation(program, uniform_name);
    if (uniform_rot== -1) {
      fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
      return 0;
    }

    uniform_name = "v";
    uniform_v = glGetUniformLocation(program, uniform_name);
    if (uniform_v == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }

    uniform_name = "p";
    uniform_p = glGetUniformLocation(program, uniform_name);
    if (uniform_p == -1) {
    	fprintf(stderr, "Could not bind uniform %s\n", uniform_name);
    	return 0;
    }


    return 1;
}


// Compute virtual trackball vector.
glm::vec3 get_trackball_vec(int x, int y)
{
	glm::vec3 pvec = glm::vec3(1.0*x/screen_width*2 -1.0, 1.0*y/screen_height*2 -1.0, 0);
	pvec.y = -pvec.y;
	float plength = pvec.x*pvec.x + pvec.y*pvec.y;
	if (plength <= 1.0)
	{
		pvec.z = sqrt(1-plength);
	}				
	else pvec = glm::normalize(pvec);
	return pvec;
}


// Quaternions multiplication.
glm::vec4 quaMul(glm::vec4 a, glm::vec4 b)
{
  float angA = a.x;
  float angB = b.x;
  glm::vec3 axisA = glm::vec3(a.y, a.z, a.w);
  glm::vec3 axisB = glm::vec3(b.y, b.z, b.w);
  return glm::vec4(angA*angB-glm::dot(axisA,axisB), glm::vec3(angA*axisB+angB*axisA+glm::cross(axisA, axisB)));
}


// Rotation and model-view-projection
void mvp()
{
	// Trackball rotation //
  glm::vec4 rot = glm::vec4(1.0f,glm::vec3(0.0f));

  printf("last_mx: %d, cur_mx: %d,\n", last_mx, cur_mx);

	if (cur_mx != last_mx || last_my != cur_my)
	{
		glm::vec3 va = get_trackball_vec(last_mx, last_my);
		glm::vec3 vb = get_trackball_vec(cur_mx, cur_my);
		float rotAng = acos(min(1.0f, glm::dot(va, vb)));
		glm::vec3 rotAxis = glm::normalize(glm::cross(va, vb));
    float ca = cos(rotAng/2.0);
    float sa = sin(rotAng/2.0);
    rot = glm::vec4(ca, rotAxis*sa);
		last_mx = cur_mx;
		last_my = cur_my;
	}
  
  rotate = quaMul(rot, rotate);

	glUseProgram(program);
	glUniformMatrix4fv(uniform_m, 1, GL_FALSE, glm::value_ptr(model));
	glUniformMatrix4fv(uniform_v, 1, GL_FALSE, glm::value_ptr(view));
	glUniformMatrix4fv(uniform_p, 1, GL_FALSE, glm::value_ptr(project));
  glUniform4fv(uniform_rot, 1, glm::value_ptr(rotate));

	glutPostRedisplay();

}


void draw()
{
	glClearColor(0.0, 0.0, 0.0, 1.0);
	//glClearColor(1.0, 1.0, 1.0, 1.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

	glUseProgram(program);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, mytexture_id);
	glUniform1i(uniform_mytexture, /*GL_TEXTURE*/0);

	glEnableVertexAttribArray(attribute_v_coord);
	glBindBuffer(GL_ARRAY_BUFFER, spere_vbo);
	glVertexAttribPointer(attribute_v_coord, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 2 * 61 * 60);

}


void onDisplay()
{
	mvp();
	draw();
	glutSwapBuffers();
}


void onReshape(int width, int height)
{
	screen_width = width;
	screen_height = height;
	glViewport(0, 0, screen_width, screen_height);
}


void onMouse(int button, int state, int x, int y)
{
	if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		last_mx = cur_mx = x;
		last_my = cur_my = y;
		trackball_on = true;
	}
	else trackball_on = false;
}


void onMotion(int x, int y)
{
	if (trackball_on)
	{
		cur_mx = x;
		cur_my = y;
	}
}


// Free memory after closing windows//
void free_resources()
{
	glDeleteProgram(program);
	glDeleteTextures(1, &mytexture_id);
}


int main(int argc, char* argv[]) {
  glutInit(&argc, argv);
  glutInitDisplayMode(GLUT_RGBA|GLUT_ALPHA|GLUT_DOUBLE|GLUT_DEPTH);
  glutInitWindowSize(screen_width, screen_height);
  glutCreateWindow("Textured Spheres");

  GLenum glew_status = glewInit();
  if (glew_status != GLEW_OK) {
    fprintf(stderr, "Error: %s\n", glewGetErrorString(glew_status));
    return EXIT_FAILURE;
  }

  if (!GLEW_VERSION_2_0) {
    fprintf(stderr, "Error: your graphic card does not support OpenGL 2.0\n");
    return EXIT_FAILURE;
  }

  if (init_resources()) {
    glutDisplayFunc(onDisplay);
    glutReshapeFunc(onReshape);
 	  glutMouseFunc(onMouse);
 	  glutMotionFunc(onMotion);
    glEnable(GL_CULL_FACE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glutMainLoop();
  }

  free_resources();
  return EXIT_SUCCESS;
}
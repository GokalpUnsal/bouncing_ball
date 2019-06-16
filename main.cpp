// Gokalp Unsal
// 54040
//


// Include glew.h here or in Angel.h (rather do it in Angel.h)
#include "Angel.h"
#define PI 3.14159265
#define g 9.81
;
//Animation global variables
GLfloat initxPos = 15.6f;
GLfloat	inityPos = 15.6f;
GLfloat xPos = -initxPos;
GLfloat yPos = inityPos;
GLfloat yAcc = -g;
GLfloat xSpeed = 0.1f;
GLfloat ySpeed = 0.0f;
GLfloat scaleParam = 0.6;
bool falling = true;

//Global variables for color change
float color[3] = { 0.0, 0.0 ,0.0 };
GLuint color_location;

typedef vec4  color4;
typedef vec4  point4;

//Global variables for VAOs
GLuint abuffer[2];

const int NumVertices = 36; //(6 faces)(2 triangles/face)(3 vertices/triangle)
point4 points[NumVertices];

const int NumSphereVertices = 492;
point4 spherePoints[NumSphereVertices];
point4 sphereVertices[492];




// Vertices of a unit cube centered at origin, sides aligned with axes
GLfloat pointSize = 0.8f; //point size of the cube
point4 vertices[8] = {
	point4(-pointSize, -pointSize,  pointSize, 1.0),
	point4(-pointSize,  pointSize,  pointSize, 1.0),
	point4(pointSize,  pointSize,  pointSize, 1.0),
	point4(pointSize, -pointSize,  pointSize, 1.0),
	point4(-pointSize, -pointSize, -pointSize, 1.0),
	point4(-pointSize,  pointSize, -pointSize, 1.0),
	point4(pointSize,  pointSize, -pointSize, 1.0),
	point4(pointSize, -pointSize, -pointSize, 1.0)
};


// Array of rotation angles (in degrees) for each coordinate axis
enum { Xaxis = 0, Yaxis = 1, Zaxis = 2, NumAxes = 3 };
int      Axis = Yaxis;
GLfloat  Theta[NumAxes] = { 0.0, 0.0, 0.0 };

// Model-view and projection matrices uniform location
GLuint  ModelView, Projection;


// fragment shading of sphere model
#include "Angel.h"
const int NumTimesToSubdivide = 3;
const int NumTriangles = 4096;
// (4 faces)^(NumTimesToSubdivide + 1)
const int NumVert = 3 * NumTriangles;
typedef Angel::vec4 point4;
typedef Angel::vec4 color4;
point4 points2[NumVert];
vec3   normals[NumVert];
// Model-view and projection matrices uniform location
//----------------------------------------------------------------------
int ind = 0;
void
triangle(const point4& a, const point4& b, const point4& c)

{

	vec3  normal = normalize(cross(b - a, c - b));
	normals[ind] = normal;  points2[ind] = a;  ind++;
	normals[ind] = normal;  points2[ind] = b;  ind++;
	normals[ind] = normal;  points2[ind] = c;  ind++;

}

//----------------------------------------------------------------------
point4
unit(const point4& p)
{

	float len = p.x*p.x + p.y*p.y + p.z*p.z;
	point4 t;

	if (len > DivideByZeroTolerance) {

		t = p / sqrt(len);
		t.w = 1.0;
	}
	return t;
}

//----------------------------------------------------------------------

void
divide_triangle(const point4& a, const point4& b,

	const point4& c, int count)

{

	if (count > 0) {
		point4 v1 = unit(a + b);
		point4 v2 = unit(a + c);
		point4 v3 = unit(b + c);
		divide_triangle(a, v1, v2, count - 1);
		divide_triangle(c, v2, v3, count - 1);
		divide_triangle(b, v3, v1, count - 1);
		divide_triangle(v1, v3, v2, count - 1);
	}
	else {
		triangle(a, b, c);
	}
}

//----------------------------------------------------------------------
void
tetrahedron(int count)
{
	point4 v[4] = {
		vec4(0.0, 0.0, 1.0, 1.0),
		vec4(0.0, 0.942809, -0.333333, 1.0),
		vec4(-0.816497, -0.471405, -0.333333, 1.0),
		vec4(0.816497, -0.471405, -0.333333, 1.0)
	};

	divide_triangle(v[0], v[1], v[2], count);
	divide_triangle(v[3], v[2], v[1], count);
	divide_triangle(v[0], v[3], v[1], count);
	divide_triangle(v[0], v[2], v[3], count);

}

//----------------------------------------------------------------------------


// quad generates two triangles for each face and assigns colors
//    to the vertices
int Index = 0;
void
quad(int a, int b, int c, int d)
{
	points[Index] = vertices[a]; Index++;
	points[Index] = vertices[b]; Index++;
	points[Index] = vertices[c]; Index++;
	points[Index] = vertices[a]; Index++;
	points[Index] = vertices[c]; Index++;
	points[Index] = vertices[d]; Index++;
}

//----------------------------------------------------------------------------

// generate 12 triangles: 36 vertices and 36 colors
void
colorcube()
{
	quad(1, 0, 3, 2);
	quad(2, 3, 7, 6);
	quad(3, 0, 4, 7);
	quad(6, 5, 1, 2);
	quad(4, 5, 6, 7);
	quad(5, 4, 0, 1);
}

//----------------------------------------------------------------------------
//creates a sphere
void
createSphere(GLint radius, GLint stackCount, GLint sectorCount)
{
	float x, y, z, xy;                              // vertex position
	float nx, ny, nz, lengthInv = 1.0f / radius;    // vertex normal
	float s, t;

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	for (int i = 0; i <= stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cosf(stackAngle);             // r * cos(u)
		z = radius * sinf(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for (int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cosf(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sinf(sectorAngle);             // r * cos(u) * sin(v)
			sphereVertices[i*j][0] = x;
			sphereVertices[i][1] = y;
			sphereVertices[i][2] = z;

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;


			// vertex tex coord (s, t) range between [0, 1]
			s = (float)j / sectorCount;
			t = (float)i / stackCount;

		}
	}
}


// OpenGL initialization
void
init()
{

	// Create a vertex array object
	glGenVertexArrays(2, abuffer);
	glBindVertexArray(abuffer[0]);

	colorcube();
	tetrahedron(4);

	// Create and initialize a buffer object
	GLuint buffer;
	glGenBuffers(1, &buffer);
	glBindBuffer(GL_ARRAY_BUFFER, buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points),
		points, GL_STATIC_DRAW);

	// Load shaders and use the resulting shader program
	GLuint program = InitShader("vshader.glsl", "fshader.glsl");
	glUseProgram(program);
	color_location = glGetUniformLocation(program, "color");

	// set up vertex arrays
	GLuint vPosition = glGetAttribLocation(program, "vPosition");
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0,
		BUFFER_OFFSET(0));

	glBindVertexArray(abuffer[1]);

	//create the second buffer object
	GLuint buffer2;
	glGenBuffers(1, &buffer2);
	glBindBuffer(GL_ARRAY_BUFFER, buffer2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(points2), points2, GL_STATIC_DRAW);
	glEnableVertexAttribArray(vPosition);
	glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, BUFFER_OFFSET(0));
	glUseProgram(program);

	// Retrieve transformation uniform variable locations
	ModelView = glGetUniformLocation(program, "ModelView");
	Projection = glGetUniformLocation(program, "Projection");

	// Set projection matrix
	mat4  projection;
	projection = Perspective(45.0, 1.0, 0.5, 3.0);
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	glBindVertexArray(abuffer[1]);

	glEnable(GL_DEPTH_TEST);
	glClearColor(0.5, 0.5, 0.5, 1.0);
}

//----------------------------------------------------------------------------

void
display(void)
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//  Generate tha model-view matrix
	const vec3 displacement(xPos, yPos, 0.0);
	mat4  model_view = (Scale(.06,.06, .06) * Translate(displacement) *
		RotateX(Theta[Xaxis]) *
		RotateY(Theta[Yaxis]) *
		RotateZ(Theta[Zaxis]));  // Scale(), Translate(), RotateX(), RotateY(), RotateZ(): user-defined functions in mat.h

	glUniformMatrix4fv(ModelView, 1, GL_TRUE, model_view);
	glUniform4f(color_location, color[0], color[1], color[2], 1.0);

	glDrawArrays(GL_TRIANGLES, 0, NumVert);


	glutSwapBuffers();

}
//----------------------------------------------------------------------------

void
keyboard(unsigned char key, int x, int y)
{
	switch (key) {
	case 033:  // Escape key
	case 'q': case 'Q':
		exit(EXIT_SUCCESS);
		break;
	case 'i':
		xPos = -initxPos;
		yPos = inityPos;
		ySpeed = 0.0f;
		break;
	case 'h':
		printf("Help\n");
		printf("Press q to quit.\n");
		printf("Press i to reset the object.\n");
		printf("Right click to open the menu and select between, object, color, and drawing mode.\n");
	}
}

//----------------------------------------------------------------------------

void
mouse(int button, int state, int x, int y)
{
	if (state == GLUT_DOWN) {
		switch (button) {
		case GLUT_LEFT_BUTTON:    break;
		case GLUT_MIDDLE_BUTTON:  break;
		case GLUT_RIGHT_BUTTON:   break;
		}
	}
}

//----------------------------------------------------------------------------
//movevemnt is implemented below, velocity and acc increases with each callback
//when it hits a border, velocity reverses
void
idle(void)
{
	xPos += xSpeed;

	if (yPos > -inityPos) {
		ySpeed += yAcc / 10;
		falling = true;
	}
	if (yPos <= -inityPos && falling) {
		ySpeed = -ySpeed * 0.9;
		falling = false;

	}
	if (xPos > initxPos || xPos < -initxPos) {
		xSpeed = -xSpeed;
	}
	if (ySpeed <= 0.3 && !falling) {
		ySpeed = 0;
	}
	yPos += ySpeed /10;
	glutPostRedisplay();
}

//----------------------------------------------------------------------------

void reshape(int w, int h)
{
	glViewport(0, 0, w, h);
	mat4 projection;
	if (w <= h) {
	projection = Ortho(-1.0, 1.0, -1.0 * (GLfloat)h /
		(GLfloat)w, 1.0 * (GLfloat)h / (GLfloat)w, -1.0, 1.0);

}
	else {
	projection = Ortho(-1.0* (GLfloat)w / (GLfloat)h, 1.0 *
		(GLfloat)w / (GLfloat)h, -1.0, 1.0, -1.0, 1.0);

		}
	glUniformMatrix4fv(Projection, 1, GL_TRUE, projection);
	
}

//----------------------------------------------------------------------------
//color picking menu
void color_menu(int id)
{
	if (id == 1) {// black
		color[0] = 0.0;
		color[1] = 0.0;
		color[2] = 0.0;
	}
	else if (id == 2) {// red
		color[0] = 1.0;
		color[1] = 0.0;
		color[2] = 0.0;
	}
	else if (id == 3) {// yellow
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 0.0;
	}
	else if (id == 4) {// green
		color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 0.0;
	}
	else if (id == 5) {// blue
		color[0] = 0.0;
		color[1] = 0.0;
		color[2] = 1.0;
	}
	else if (id == 6) {// magenta
		color[0] = 1.0;
		color[1] = 0.0;
		color[2] = 1.0;
	}
	else if (id == 7) {// white
		color[0] = 1.0;
		color[1] = 1.0;
		color[2] = 1.0;
	}
	else if (id == 8) {// cyan
		color[0] = 0.0;
		color[1] = 1.0;
		color[2] = 1.0;
	}
	glutPostRedisplay();
}

//----------------------------------------------------------------------------
//object selection menu
void object_menu(int id)
{
	if (id == 1) {
		glBindVertexArray(abuffer[0]);
	}
	else if (id == 2) {
		glBindVertexArray(abuffer[1]);
	}
	else if (id == 3) {
		glBindVertexArray(abuffer[1]);
	}
}

//----------------------------------------------------------------------------
//polygon mode selection
void drawing_menu(int id) {
	if (id == 1) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}
	else if (id == 2) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	}
	else if (id == 3) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_POINT);
	}

}


//----------------------------------------------------------------------------
void right_menu(int id)
{
}
//----------------------------------------------------------------------------

GLuint windowWidth = 700;
GLuint windowHeight = 700;
int
main(int argc, char **argv)
{
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(windowWidth, windowHeight);
	glutInitWindowPosition(50, 50);
	glutCreateWindow("54040");

	glewExperimental = GL_TRUE;
	glewInit();

	init();

	int c_menu = glutCreateMenu(color_menu);
	glutAddMenuEntry("Black", 1);
	glutAddMenuEntry("Red", 2);
	glutAddMenuEntry("Yellow", 3);
	glutAddMenuEntry("Green", 4);
	glutAddMenuEntry("Blue", 5);
	glutAddMenuEntry("Magenta", 6);
	glutAddMenuEntry("White", 7);
	glutAddMenuEntry("Cyan", 8);

	int o_menu = glutCreateMenu(object_menu);
	glutAddMenuEntry("Cube", 1);
	glutAddMenuEntry("Sphere", 2);
	glutAddMenuEntry("Bunny", 3);

	int d_menu = glutCreateMenu(drawing_menu);
	glutAddMenuEntry("Solid", 1);
	glutAddMenuEntry("Wireframe", 2);
	glutAddMenuEntry("Point", 3);

	glutCreateMenu(right_menu);
	glutAddSubMenu("Color", c_menu);
	glutAddSubMenu("Object Type", o_menu);
	glutAddSubMenu("Drawing Mode", d_menu);
	glutAttachMenu(GLUT_RIGHT_BUTTON);


	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutIdleFunc(idle);
	glutReshapeFunc(reshape);

	glutMainLoop();

	return 0;
}

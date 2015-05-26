#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <iostream>

#include <GL/glew.h>
#include <GL/GLAUX.H>
#include <GL/glut.h>
#include "textfile.h"
using namespace std;

#define  PI 3.1415926

GLuint v,f,f2,p;

/*******��ͼģ�ͱ任����***************
*camera[3]:�������
*center[3]:ģ������
*celength:ģ�Ͱ뾶
*yrot:������ϵ����y��н�
*xrot:������ϵ��ͶӰ��z��н�
***/
GLfloat camera[3] = {0.0f,0.0f,1.0f};
GLfloat center[3] = {0.0f,0.0f,0.0f};
GLfloat celength = 1.5f;
GLfloat yrot = 0.01;
GLfloat xrot = 0.0;
long start = 0;    //��¼�û�������ʱ���

static GLfloat cSpeed = 0.05; /// center move speed
static GLfloat xSpeed = PI/90.0; /// rotate speed
static GLfloat ySpeed = PI/90.0; /// reserved

int width = 400; //��¼��ǰ���ڵĴ�С
int height = 400;

int mx = 0;//��¼�����������
int my = 0;

GLuint	texture[4];								// 3������Ĵ���ռ�
GLuint    buffers;                //VBO���������������
GLuint vao;
GLuint vbo;
GLuint texCoordVbo;
int N = 64;                  //ˮ�صĿ����������������

typedef struct _VEC3
{
	float x,y,z;
}VEC3;
typedef struct VEC2
{
	float x,y;
}VEC2;
 
VEC3 obj;            //�����������ά�ռ��е�����
//�����������������
void CalCamPosition()
{
	if(yrot > PI)  yrot = PI;
	if(yrot < 0) yrot = 0.001;
	if(xrot > 2*PI) xrot = 2*PI;
	if(xrot < 0) xrot = 0;
	if(celength > 20) celength = 20;
	if(celength < 0) celength = 0;

	camera[0] = celength*sin(yrot)*sin(xrot);
	camera[1] = celength*cos(yrot);
	camera[2] = celength*sin(yrot)*cos(xrot);
}

AUX_RGBImageRec * LoadBMP(char * filename)
{
	FILE * file = NULL;
	if(!filename) return NULL;

	file = fopen(filename,"r");
	if(file) {fclose(file);return auxDIBImageLoad(filename);}

	return NULL;
}

int LoadGLTextures()
{
	int status = FALSE;
	AUX_RGBImageRec * TextureImage[3];
	memset(TextureImage,0,sizeof(void *)*1);

	if(TextureImage[0]=LoadBMP("wall.bmp")) 
	{
		status=TRUE;
		glGenTextures(3,&texture[0]);

		glBindTexture(GL_TEXTURE_2D,texture[0]);
		//��������
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);

		glBindTexture(GL_TEXTURE_2D,texture[1]);
		//��������
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// �����˲�
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// �����˲�

		// ���� MipMapped ����
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); 			
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data); 

		if (TextureImage[0])							// �����Ƿ����
		{
			if (TextureImage[0]->data)					// ����ͼ���Ƿ����
			{
				free(TextureImage[0]->data);				// �ͷ�����ͼ��ռ�õ��ڴ�
			}
			free(TextureImage[0]);						// �ͷ�ͼ��ṹ
		}

	}

	//����ˮ����ͼ
	if(TextureImage[1]=LoadBMP("water.bmp")) 
	{
		glGenTextures(1,&texture[3]);

		glBindTexture(GL_TEXTURE_2D,texture[3]);
		//��������
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[1]->sizeX, TextureImage[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[1]->data);

		if (TextureImage[1])							// �����Ƿ����
		{
			if (TextureImage[1]->data)					// ����ͼ���Ƿ����
			{
				free(TextureImage[1]->data);				// �ͷ�����ͼ��ռ�õ��ڴ�
			}
			free(TextureImage[1]);						// �ͷ�ͼ��ṹ
		}

	}

	//��󷵻�״̬���������һ��OK������ Status ��ֵΪ TRUE ������Ϊ FALSE ��	
	return status;								// ���� Status
}

bool Initial()
{
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!LoadGLTextures())							// ������������������
	{
		MessageBox(NULL,"��������ʧ�ܡ�","�ر�",MB_OK | MB_ICONINFORMATION);
		return FALSE;							// ���δ�����룬����FALSE
	}
	glEnable(GL_TEXTURE_2D);						// ��������ӳ��

	return true;
}

#define printOpenGLError() printOglError(__FILE__, __LINE__)

int printOglError(char *file, int line)
{
	//
	// Returns 1 if an OpenGL error occurred, 0 otherwise.
	//
	GLenum glErr;
	int    retCode = 0;

	glErr = glGetError();
	while (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n", file, line, gluErrorString(glErr));
		retCode = 1;
		glErr = glGetError();
	}
	return retCode;
}


void printShaderInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetShaderiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetShaderInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}

void printProgramInfoLog(GLuint obj)
{
	int infologLength = 0;
	int charsWritten  = 0;
	char *infoLog;

	glGetProgramiv(obj, GL_INFO_LOG_LENGTH,&infologLength);

	if (infologLength > 0)
	{
		infoLog = (char *)malloc(infologLength);
		glGetProgramInfoLog(obj, infologLength, &charsWritten, infoLog);
		printf("%s\n",infoLog);
		free(infoLog);
	}
}


void DrawCube()
{	
	glUseProgram(0);
	glColor4f(0.5f,0.5f,0.5f,0.7f);			// ȫ���ȣ� 50% Alpha ���
	glEnable(GL_BLEND);		// ���_���
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);		// ���Դ����alphaͨ��ֵ�İ�͸����Ϻ���
	glDisable(GL_DEPTH_TEST);	// �P�]��Ȝyԇ

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);				// ѡ������
	glBegin(GL_QUADS);							//  ����������
	//ǰ
	glNormal3f( 0.0f, 0.0f, -1.0f);					// ����ָ��۲���
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f, -0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,  0.3f, 0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f,  0.3f, 0.5f);

	//��
	glNormal3f( -1.0f, 0.0f, 0.0f);					// ���߳���
	glTexCoord2f(0.0f, 0.0f);glVertex3f( 0.5f, 0.3f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.5f, 0.3f,-0.5f);

	//��
	glNormal3f( 0.0f, 0.0f,1.0f);					// ���߱���۲���
	glTexCoord2f(0.0f, 0.0f);glVertex3f( 0.5f,0.3f,-0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f, 0.3f,-0.5f);		

	//��
	glNormal3f(1.0f, 0.0f, 0.0f);					// ���߳���
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f, 0.3f,-0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f, 0.3f, 0.5f);

	//��
	glNormal3f( 0.0f,1.0f, 0.0f);					// ���߳���
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f,-0.5f,-0.5f);

	glEnd();								// �����λ��ƽ���
	glFinish();
	glUseProgram(p);
}

void MakePlaneVBO()
{
	VEC3 * vert = (VEC3 *)malloc(sizeof(VEC3) * (N+1) * (N+1) );
	float step = 1.0 / (N*1.0);        //step��ʾ�����ࡣˮ�س�����1.0,Ԥ������128��128������:)
	int x,y;
	for (y = 0; y <=  N; y++)
		for (x = 0; x <=  N; x++) 
		{
			vert[x + y + y * N].z = (FLOAT)y * step - 0.5;
			vert[x + y + y * N].x = (FLOAT)x * step - 0.5;
			vert[x + y + y * N].y = 0;
		}

	VEC3 * indx = (VEC3 *)malloc(sizeof(VEC3) * N * N * 6);
	int i ,j;
	j = 0;
	for(i = 0; i < (N+1) * (N+1); i++)
	{
		if(i != 0 && ((i+1) % (N+1) == 0))
			continue;
		if(i > (N-1) * (N+1) + N -1)
			continue;
		indx[j++] = vert[i];
		indx[j++] = vert[i + 1 + N + 1];
		indx[j++] = vert[i + N + 1];
		indx[j++] = vert[i];
		indx[j++] = vert[i + 1];
		indx[j++] = vert[i + 1 + N + 1];
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, N * N * 6 * sizeof(VEC3), indx, GL_STATIC_DRAW);
	
	glGenVertexArrays(1,&vao);
	glBindVertexArray(vao);

	free(vert);
	free(indx);
}

void setShaders() {

	char *vs = NULL,*fs = NULL ,*fs2 = NULL;

	v = glCreateShader(GL_VERTEX_SHADER);
	f = glCreateShader(GL_FRAGMENT_SHADER);
	f2 = glCreateShader(GL_FRAGMENT_SHADER);


	vs = textFileRead("vshader.vert");
	fs = textFileRead("fshader.frag");
	const char * vv = vs;
	const char * ff = fs;

	glShaderSource(v, 1, &vv,NULL);
	glShaderSource(f, 1, &ff,NULL);

	free(vs);free(fs);

	glCompileShader(v);
	glCompileShader(f);

	printShaderInfoLog(v);
	printShaderInfoLog(f);
	printShaderInfoLog(f2);

	p = glCreateProgram();
	glAttachShader(p,v);
	glAttachShader(p,f);

	glLinkProgram(p);
	printProgramInfoLog(p);

	GLint loc1 = glGetUniformLocation(p,"on_off");  //�ʼˮ��Ӧ���Ǿ�ֹ��:)
	glUniform1i(loc1,1);
}

void DrawVBO()
{
	glUseProgram(p);
	glEnable(GL_TEXTURE_2D); //��ʼʹ��������
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);				// ѡ������
	glBindSampler(3,texture[3]);

	/*****************����ɫ��������ֵ**************/
	float myTime = (clock() - start)/1000.0 ;
	GLint loc2 = glGetUniformLocation(p,"Time");
	//cout<<"period="<<period/1000.0<<endl;
	glUniform1f(loc2,myTime);
	GLint loc1;
	float water[4] = {2.0,0.2,0.0,-0.02};//���� ���� ��λ ���
	loc1 = glGetUniformLocation(p,"WaveData");
	glUniform4f(loc1,water[0],water[1],water[2],water[3]);

	GLint loc3 = glGetUniformLocation(p,"water");
	glUniform1i(loc3,3);

	if(myTime > 2.0)                 //ˮ���˶��������ھ�ֹͣ.����������ʧ�ĺ�ͻأ:(
	{
		GLint loc4 = glGetUniformLocation(p,"on_off");
		glUniform1i(loc4,0);
	}

	glBindVertexArray(vao);

	//�󶨶�������
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES,0,N*N*6);

	glUseProgram(0);
}

//����ˮ��
void DrawWater()
{
	DrawVBO();					//����VBO
}

void Resharp(int w,int h)
{
	width = w;height = h;
	glViewport(0,0,(GLsizei )w,(GLsizei) h); //����viewport
	glMatrixMode(GL_PROJECTION);                                                                                    // Select The Projection Matrix
	glLoadIdentity();                                                                                            
	gluPerspective(45.0f,(GLfloat)w/(GLfloat)h,0.5f,200.0f);                // Calculate The Aspect Ratio Of The Window

	glMatrixMode(GL_MODELVIEW);                                                                                             // Select The Modelview Matrix
	glLoadIdentity(); 

}

void SpecialKeys(unsigned char key, int x, int y)
{

	if (key == 27) 
		exit(0);
	//����������ƶ�
	switch(key)
	{
	case 'a':
	case'A':
		xrot -= xSpeed;
		break;
	case 'W':
	case 'w':
		celength -= cSpeed;
		break;
	case 's':
	case 'S':
		celength += cSpeed;
		break;
	case 'D':
	case 'd':
		xrot += xSpeed;
		break;
	case 'q':
	case 'Q':
		yrot += ySpeed;
		break;
	case 'e':
	case 'E':
		yrot -= ySpeed;
		break;
	default:
		break;
	}

	CalCamPosition();
	glutPostRedisplay();

}

void Display()
{
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glLineWidth(1.0f);
	CalCamPosition();
	gluLookAt(camera[0],camera[1],camera[2],0, 0, 0,0.0f,1.0f,0.0f);
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	DrawWater();

	DrawCube();
	glFlush();
	glutSwapBuffers();
}


void Idle()
{
	Display();
}

void MouseFun(int button,int state,int x,int y)
{
	int viewport[4];
	double mvmatrix[16];
	double projmatrix[16];

	glGetIntegerv(GL_VIEWPORT, viewport);
	glGetDoublev(GL_MODELVIEW_MATRIX, mvmatrix);
	glGetDoublev(GL_PROJECTION_MATRIX, projmatrix);

	VEC3 winPos;
	winPos.x =x;
	winPos.y = viewport[3] - y -1;
	winPos.z = 0;

	float wz = 0;
	glReadPixels((int)winPos.x, (int)winPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &wz); 
	winPos.z = wz;
	GLdouble xt ;GLdouble yt; GLdouble zt ;
	if (winPos.z < 1.0 && winPos.z > 0)
	{
		gluUnProject(winPos.x, winPos.y, winPos.z, mvmatrix, projmatrix, viewport, &xt, &yt, &zt);
	} 
	obj.x = xt;obj.y = yt; obj.z = zt;
	if(button==GLUT_LEFT_BUTTON && state == GLUT_DOWN)
	{
		start = clock();
		GLint loc3 = glGetUniformLocation(p,"pos");
		//cout<<"obj.x = "<<obj.x<<" obj.y = "<<obj.y<<" obj.z = "<<obj.z<<endl;
		//glUniform3f(loc3,obj.x,obj.y,obj.z);
		glUniform3f(loc3,0.0,0.0,0.0);
		GLint loc1 = glGetUniformLocation(p,"on_off");      //��������Ժ�����ö�����ɫ��:)
		glUniform1i(loc1,1);
	}
}
int main(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DEPTH | GLUT_DOUBLE | GLUT_RGBA);
	glutInitWindowSize(400,400);
	glutInitWindowPosition(300,300);
	glutCreateWindow("water wave");

	glewInit();
	if (glewIsSupported("GL_VERSION_2_0"))
		printf("Ready for OpenGL 2.0\n");
	else {
		printf("OpenGL 2.0 not supported\n");
		exit(1);
	}
	if(!Initial())
		return 1;

	setShaders();
	MakePlaneVBO();          //�����ɶ�������

	glutDisplayFunc(Display);
	glutIdleFunc(Idle);
	glutReshapeFunc(Resharp);
	glutKeyboardFunc(SpecialKeys);
	glutMouseFunc(MouseFun);

	glEnable(GL_DEPTH_TEST);
	glClearColor(1.0,1.0,1.0,1.0);
	glutMainLoop();

	return 0;
}


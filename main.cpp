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

/*******视图模型变换参数***************
*camera[3]:相机坐标
*center[3]:模型坐标
*celength:模型半径
*yrot:球坐标系中与y轴夹角
*xrot:球坐标系中投影与z轴夹角
***/
GLfloat camera[3] = {0.0f,0.0f,1.0f};
GLfloat center[3] = {0.0f,0.0f,0.0f};
GLfloat celength = 1.5f;
GLfloat yrot = 0.01;
GLfloat xrot = 0.0;
long start = 0;    //记录用户交互的时间点

static GLfloat cSpeed = 0.05; /// center move speed
static GLfloat xSpeed = PI/90.0; /// rotate speed
static GLfloat ySpeed = PI/90.0; /// reserved

int width = 400; //记录当前窗口的大小
int height = 400;

int mx = 0;//记录鼠标点击的坐标
int my = 0;

GLuint	texture[4];								// 3种纹理的储存空间
GLuint    buffers;                //VBO缓冲区对象的名称
GLuint vao;
GLuint vbo;
GLuint texCoordVbo;
int N = 64;                  //水池的宽度所包含的网格数

typedef struct _VEC3
{
	float x,y,z;
}VEC3;
typedef struct VEC2
{
	float x,y;
}VEC2;
 
VEC3 obj;            //鼠标点击后在三维空间中的坐标
//计算相机的世界坐标
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
		//生成纹理
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);

		glBindTexture(GL_TEXTURE_2D,texture[1]);
		//生成纹理
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data);	
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);	// 线形滤波
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);	// 线形滤波

		// 创建 MipMapped 纹理
		glBindTexture(GL_TEXTURE_2D, texture[2]);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_NEAREST); 			
		gluBuild2DMipmaps(GL_TEXTURE_2D, 3, TextureImage[0]->sizeX, TextureImage[0]->sizeY, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[0]->data); 

		if (TextureImage[0])							// 纹理是否存在
		{
			if (TextureImage[0]->data)					// 纹理图像是否存在
			{
				free(TextureImage[0]->data);				// 释放纹理图像占用的内存
			}
			free(TextureImage[0]);						// 释放图像结构
		}

	}

	//载入水面贴图
	if(TextureImage[1]=LoadBMP("water.bmp")) 
	{
		glGenTextures(1,&texture[3]);

		glBindTexture(GL_TEXTURE_2D,texture[3]);
		//生成纹理
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST); 
		glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST); 
		glTexImage2D(GL_TEXTURE_2D, 0, 3, TextureImage[1]->sizeX, TextureImage[1]->sizeY, 0, GL_RGB, GL_UNSIGNED_BYTE, TextureImage[1]->data);

		if (TextureImage[1])							// 纹理是否存在
		{
			if (TextureImage[1]->data)					// 纹理图像是否存在
			{
				free(TextureImage[1]->data);				// 释放纹理图像占用的内存
			}
			free(TextureImage[1]);						// 释放图像结构
		}

	}

	//最后返回状态变量。如果一切OK，变量 Status 的值为 TRUE 。否则为 FALSE 。	
	return status;								// 返回 Status
}

bool Initial()
{
	glClearColor(0.0f,0.0f,0.0f,0.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	if (!LoadGLTextures())							// 调用纹理载入子例程
	{
		MessageBox(NULL,"纹理载入失败。","关闭",MB_OK | MB_ICONINFORMATION);
		return FALSE;							// 如果未能载入，返回FALSE
	}
	glEnable(GL_TEXTURE_2D);						// 启用纹理映射

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
	glColor4f(0.5f,0.5f,0.5f,0.7f);			// 全亮度， 50% Alpha 混合
	glEnable(GL_BLEND);		// 打開混合
	glBlendFunc(GL_SRC_ALPHA,GL_ONE);		// 基於源象素alpha通道值的半透明混合函數
	glDisable(GL_DEPTH_TEST);	// 關閉深度測試

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, texture[0]);				// 选择纹理
	glBegin(GL_QUADS);							//  绘制正方形
	//前
	glNormal3f( 0.0f, 0.0f, -1.0f);					// 法线指向观察者
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f, -0.5f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f, -0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,  0.3f, 0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f,  0.3f, 0.5f);

	//右
	glNormal3f( -1.0f, 0.0f, 0.0f);					// 法线朝右
	glTexCoord2f(0.0f, 0.0f);glVertex3f( 0.5f, 0.3f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f( 0.5f, 0.3f,-0.5f);

	//后
	glNormal3f( 0.0f, 0.0f,1.0f);					// 法线背向观察者
	glTexCoord2f(0.0f, 0.0f);glVertex3f( 0.5f,0.3f,-0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f, 0.3f,-0.5f);		

	//左
	glNormal3f(1.0f, 0.0f, 0.0f);					// 法线朝左
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f, 0.3f,-0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f(-0.5f,-0.5f,-0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f, 0.3f, 0.5f);

	//下
	glNormal3f( 0.0f,1.0f, 0.0f);					// 法线朝下
	glTexCoord2f(0.0f, 0.0f);glVertex3f(-0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 0.0f);glVertex3f( 0.5f,-0.5f, 0.5f);
	glTexCoord2f(1.0f, 1.0f);glVertex3f( 0.5f,-0.5f,-0.5f);
	glTexCoord2f(0.0f, 1.0f);glVertex3f(-0.5f,-0.5f,-0.5f);

	glEnd();								// 正方形绘制结束
	glFinish();
	glUseProgram(p);
}

void MakePlaneVBO()
{
	VEC3 * vert = (VEC3 *)malloc(sizeof(VEC3) * (N+1) * (N+1) );
	float step = 1.0 / (N*1.0);        //step表示网格间距。水池长宽都是1.0,预备绘制128×128的网格。:)
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

	GLint loc1 = glGetUniformLocation(p,"on_off");  //最开始水面应该是静止的:)
	glUniform1i(loc1,1);
}

void DrawVBO()
{
	glUseProgram(p);
	glEnable(GL_TEXTURE_2D); //开始使用纹理缓冲
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, texture[3]);				// 选择纹理
	glBindSampler(3,texture[3]);

	/*****************给着色器参数赋值**************/
	float myTime = (clock() - start)/1000.0 ;
	GLint loc2 = glGetUniformLocation(p,"Time");
	//cout<<"period="<<period/1000.0<<endl;
	glUniform1f(loc2,myTime);
	GLint loc1;
	float water[4] = {2.0,0.2,0.0,-0.02};//周期 波长 相位 振幅
	loc1 = glGetUniformLocation(p,"WaveData");
	glUniform4f(loc1,water[0],water[1],water[2],water[3]);

	GLint loc3 = glGetUniformLocation(p,"water");
	glUniform1i(loc3,3);

	if(myTime > 2.0)                 //水波运动两个周期就停止.但是这样消失的很突兀:(
	{
		GLint loc4 = glGetUniformLocation(p,"on_off");
		glUniform1i(loc4,0);
	}

	glBindVertexArray(vao);

	//绑定顶点数据
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glDrawArrays(GL_TRIANGLES,0,N*N*6);

	glUseProgram(0);
}

//绘制水面
void DrawWater()
{
	DrawVBO();					//绘制VBO
}

void Resharp(int w,int h)
{
	width = w;height = h;
	glViewport(0,0,(GLsizei )w,(GLsizei) h); //设置viewport
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
	//控制照相机移动
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
		GLint loc1 = glGetUniformLocation(p,"on_off");      //鼠标点击了以后才启用顶点着色器:)
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
	MakePlaneVBO();          //先生成顶点数组

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


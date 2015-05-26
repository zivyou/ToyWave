
#define PI 3.1415926
//顶点着色器属性变量：
uniform vec3  pos;  // 点波源位置
uniform vec4  WaveData;  //波相关数据
uniform float  Time ;        //时间参数
uniform int  on_off;         //水波总有要静止的时候，这个参数控制什么时候启用顶点着色器:)

vec3 lightPos = vec3(-0.3,1.0,-0.3); //光源位置
const float speCon = 0.3; //光的镜面反射比例
const float difCon = 1.0 - speCon;   //漫反射比例

 varying float lightInt; //光强
 varying vec2 mcPos;
 varying vec4 finColor;

 varying vec3 vertexPos;

void main()
{
	vertexPos = vec3(gl_Vertex.x,gl_Vertex.y,gl_Vertex.z);
	 //其中WaterData为：
	float m_T  =  WaveData.x;  // 周期
	float m_wavelengh = WaveData.y; //波长
	float m_pha = WaveData.z; // 相位
	float m_A = WaveData.w;  // 振幅
	
	vec3 ecPos = vec3(gl_ModelViewMatrix * gl_Vertex);
	vec3 tnorm = normalize(gl_NormalMatrix * gl_Normal);
	vec3 lightVec = normalize(lightPos - ecPos);
	vec3 reflectVec = reflect(-lightVec,tnorm);
	vec3 viewVec = normalize(-ecPos);
	float diffuse = max(dot(lightVec,tnorm),0.0);
	float spec = 0.0;

	if(on_off != 0)
	{
		float m_length = sqrt((gl_Vertex.x - pos.x)*(gl_Vertex.x - pos.x) + (gl_Vertex.z-pos.z)*(gl_Vertex.z - pos.z));
		float factor = 1.0 + 0.07 * Time * Time + 10.0 * m_length * m_length;//振幅应该随时间进行衰减，不然能量不守恒啊
										//振幅还要随距离进行衰减 -_-||
		gl_Vertex.y = gl_Vertex.y + m_A*cos( 2* PI /m_T* Time-2* PI /m_wavelengh* m_length+ m_pha) ;
		float m_noise= noise1(vec2( gl_Vertex.x, gl_Vertex.z) );
		gl_Vertex.y += m_noise;
		gl_Vertex.y /=factor;//需要考虑振幅随时间、距离衰减
		if(abs(gl_Vertex.y) < 0.003) gl_Vertex.y = 0.0;
	}

	if(diffuse > 0.0)
	{
		spec = max(dot(reflectVec,viewVec),0.0);
		spec = pow(spec,16.0);
	}

	lightInt = difCon * diffuse + speCon * spec;


	gl_TexCoord[0] = gl_MultiTexCoord0;  
	gl_Position = ftransform();
}

#define PI 3.1415926
//������ɫ�����Ա�����
uniform vec3  pos;  // �㲨Դλ��
uniform vec4  WaveData;  //���������
uniform float  Time ;        //ʱ�����
uniform int  on_off;         //ˮ������Ҫ��ֹ��ʱ�������������ʲôʱ�����ö�����ɫ��:)

vec3 lightPos = vec3(-0.3,1.0,-0.3); //��Դλ��
const float speCon = 0.3; //��ľ��淴�����
const float difCon = 1.0 - speCon;   //���������

 varying float lightInt; //��ǿ
 varying vec2 mcPos;
 varying vec4 finColor;

 varying vec3 vertexPos;

void main()
{
	vertexPos = vec3(gl_Vertex.x,gl_Vertex.y,gl_Vertex.z);
	 //����WaterDataΪ��
	float m_T  =  WaveData.x;  // ����
	float m_wavelengh = WaveData.y; //����
	float m_pha = WaveData.z; // ��λ
	float m_A = WaveData.w;  // ���
	
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
		float factor = 1.0 + 0.07 * Time * Time + 10.0 * m_length * m_length;//���Ӧ����ʱ�����˥������Ȼ�������غ㰡
										//�����Ҫ��������˥�� -_-||
		gl_Vertex.y = gl_Vertex.y + m_A*cos( 2* PI /m_T* Time-2* PI /m_wavelengh* m_length+ m_pha) ;
		float m_noise= noise1(vec2( gl_Vertex.x, gl_Vertex.z) );
		gl_Vertex.y += m_noise;
		gl_Vertex.y /=factor;//��Ҫ���������ʱ�䡢����˥��
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
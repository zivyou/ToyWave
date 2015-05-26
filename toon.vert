

varying vec3 normal, lightDir;

void main()
{	
	vec4 p;
	lightDir = normalize(vec3(gl_LightSource[0].position)); //normalize():向量单位化函数
	normal = normalize(gl_NormalMatrix * gl_Normal);
		
	gl_Position = ftransform();
}

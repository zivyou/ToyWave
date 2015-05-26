uniform sampler2D water;
 varying float lightInt; //π‚«ø
varying vec4 finColor;
varying vec3 vertexPos;

void main()
{
	vec4 oriColor = vec4(0.0,0.2,0.4,1.0);
	vec4 recColor = texture2D(water,vec2(vertexPos.z+0.5,vertexPos.x+0.5));
	finColor = mix(vec4(vec3(lightInt),1.0),oriColor,0.7);
	finColor = mix(recColor,finColor,0.01);
	gl_FragColor = finColor;

}
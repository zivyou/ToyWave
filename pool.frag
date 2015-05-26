uniform sample2D poolTextrue;

void main()
{
	vec3 texelColor =vec3(texture2D (poolTexture,gl_TexCoord[0].st ) );
	gl_FragColor = vec4( texelColor, 0.7) ;
}
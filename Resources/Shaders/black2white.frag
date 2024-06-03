uniform float time;

uniform sampler2D image;

void main()
{

    float x = gl_TexCoord[0].x;
    float y = gl_TexCoord[0].y;
    //gl_FragColor = texture2D(image, vec2(x,y));
    //if(texture2D(image, vec2(x,y)).a < 0.1){
        gl_FragColor.rgb = vec3(1) - texture2D(image, vec2(x,y)).rgb;
        gl_FragColor.a = texture2D(image, vec2(x,y)).a;
   // }

    //gl_FragColor = vec4(1) - texture2D(image, vec2(x,y));
    
}
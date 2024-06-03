uniform float time;


float cubicPulse( float c, float w, float x ){
    x = abs(x - c);
    if( x>w ) return 0.0;
    x /= w;
    return 1.0 - x*x*(3.0-2.0*x);
}

float plot(vec2 st, float pct){
  return  smoothstep( pct-0.02, pct, st.y) -
          smoothstep( pct, pct+0.02, st.y);
}

void main()
{

    float x = gl_TexCoord[0].x/600.;
    float y = gl_TexCoord[0].y/600.;

    vec2 r = vec2(x,y);

    vec4 c = vec4(0,0,0,1);
    float d = distance(r, vec2(0.5, 0.5));
    float d1 = 1. - smoothstep(0., 0.1 + 0.001*time*time, d);
    float d2 = smoothstep(0., 0.05 + 0.001*time*time, d);
    float dx = cubicPulse(0.1 + 0.001*time, 0.05, d);
    c.g += sqrt(dx);
    c.a *= dx;

    // multiply it by the color
    gl_FragColor = c; // vec4(0, .,0,1);
}
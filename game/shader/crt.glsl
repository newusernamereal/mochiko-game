#version 100

#define ksize 5
#define middle ((ksize - 1)/2)
precision mediump float;

// Input vertex attributes (from vertex shader)
varying vec2 fragTexCoord;
varying vec4 fragColor;

// Input uniform values
uniform sampler2D texture0;
uniform vec4 colDiffuse;

float multiplier(in int x, in int y){
    float inp = distance(vec2(x,y),vec2(0,0));
    return (float(middle * middle) - (inp * inp))/float(1666.66);
}

void main()
{
    vec4 final = vec4(0,0,0,0);
    for(int i = (middle - ksize); i < (ksize - middle); i++){
        for(int j = (middle - ksize); j < (ksize - middle); j++){
            final += multiplier(i,j) * texture2D(texture0, vec2(fragTexCoord.x + float(i), fragTexCoord.y + float(j)));
        }
    }
    gl_FragColor = final*fragColor;
}
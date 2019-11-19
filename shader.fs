#version 330 core

out vec4 color;
in vec2 TexCoord;

uniform sampler2D image;
uniform vec3 spriteColor;

void main(){
     color = vec4(spriteColor, 1.0f) * texture(image, TexCoord);
}
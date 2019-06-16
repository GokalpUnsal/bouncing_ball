#version 410

uniform vec4 color;
out vec4 fragColor;

void main()
{
     fragColor = vec4(color.r,color.g,color.b,1.0);
}


#version 330 core

in vec3 position;
uniform mat4 MVP;

void main()
{
	gl_Position = position;
}
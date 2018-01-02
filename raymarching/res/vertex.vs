#version 420 core

layout (location = 0) in vec4 a_Vertex;
layout (location = 1) in vec3 a_Normal;

uniform mat4  u_ModelViewMatrix;
uniform mat4  u_ProjectionMatrix;
uniform vec3  u_Translate;
uniform vec4  u_Color;
uniform int   u_UserVariableInt;
uniform float u_UserVariableFloat;
uniform int   u_NumModels;


out vec3 v_Normal;
out vec3 v_WorldPos;
out vec4 v_Color;
out vec4 v_ViewPos;

void main(void) {
	v_ViewPos = u_ModelViewMatrix * a_Vertex;
	gl_Position = u_ProjectionMatrix * v_ViewPos;
	v_Normal = (vec4(a_Normal, 0.0)).xyz;
	v_WorldPos = a_Vertex.xyz;
    v_Color     = u_Color;

///* TODO: Modify vertex shader for instance rendering
//    int column = int(mod(gl_InstanceID, u_NumModels));
//    int row    = gl_InstanceID / u_NumModels;
//    vec3 translate = vec3(1.0 - u_NumModels + 2.0*column,0.0, 1.0 - u_NumModels + 2.0*row);
// */
//    // Read input attributes
//    vec4 myVertex = a_Vertex + vec4(u_Translate, 0.0);
//    vec3 myNormal = a_Normal;
//
//    // Transform vertex into view-space
//    myVertex = u_ModelViewMatrix * myVertex;
//
//    // Set output attributes
//    v_Color     = u_Color;
//    v_Normal    = mat3(u_ModelViewMatrix) * myNormal;
//    v_Vertex    = myVertex.xyz;
//    gl_Position = u_ProjectionMatrix * myVertex;
}
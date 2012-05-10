uniform float size_value;
uniform float time;

void main(void)
{
   vec3 normal = gl_NormalMatrix * gl_Normal;
   vec3 Pos = gl_Vertex.xyz;
   Pos += 0.3 * normal;
   gl_Position = gl_ModelViewProjectionMatrix * vec4(Pos,1.0);
}

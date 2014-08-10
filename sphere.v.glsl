attribute vec3 v_coord;
attribute vec3 v_normal;
varying vec4 texCoords;
uniform mat4 m, v, p;
uniform vec4 rot;


vec4 quaMul(vec4 a, vec4 b)
{
	float angA = a.x;
	float angB = b.x;
	vec3 axisA = a.yzw;
	vec3 axisB = b.yzw;
	return vec4(angA*angB-dot(axisA,axisB), vec3(angA*axisB+angB*axisA+cross(axisA, axisB)));
}

vec4 quaInv(vec4 a)
{
	float magsqr = dot(a, a);
	return vec4(a.x, a.yzw*(-1.0))/magsqr;
}

void main()
{
	vec4 point = vec4(0.0, v_coord);
	vec4 newp = quaMul(rot, quaMul(point, quaInv(rot)));
	mat4 mvp = p*v*m;
	vec4 pos = mvp*vec4(newp.yzw, 1.0);
	gl_Position = pos;
	texCoords = vec4(v_coord, 1.0);
}

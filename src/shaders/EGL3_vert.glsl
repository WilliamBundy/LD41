#version 300 es
precision mediump float;

layout(location=0) in float vFlags; 
layout(location=1) in vec4 vColor;
layout(location=2) in vec3 vPos; 
layout(location=3) in float vAngle;
layout(location=4) in vec2 vSize; 
layout(location=5) in vec2 vCenter; 
layout(location=6) in vec4 vTexture; 

out vec2 fPos;
out vec2 fTexture; 
out vec2 fTextureScale;
out vec4 fColor;
flat out float fIsCircle;
flat out float fIsSDF;
flat out float fHasTexture;
flat out float fHasAA;

uniform vec2 uOffset;
uniform vec2 uViewport;
uniform float uScale;

float[4] corners = float[4](-0.5, -0.5, 0.5, 0.5); 
float[9] offsetX = float[9](0.0, 0.5, 0.0, -0.5, -0.5, -0.5,  0.0,  0.5, 0.5); 
float[9] offsetY = float[9](0.0, 0.5, 0.5,  0.5,  0.0, -0.5, -0.5, -0.5, 0.0); 

void main() 
{ 
	/*
	if((vFlags & 0x10) > 0) {
		gl_Position = vec4(0, 0, 0, 0); 
		fTexture = vec2(0, 0); 
		fColor = vec4(0, 0, 0, 0);
		fFlags = 0x20;
		return; 
	} 
	*/
	
	fColor = vColor.wzyx;
	//fFlags = vFlags;
	fIsCircle = 0.0;
	fIsSDF = 0.0;
	fHasTexture = 1.0;
	fHasAA = 1.0;

	if(vFlags >= 10.0 && vFlags < 40.0) fIsCircle = 1.0;
	if(vFlags >= 20.0 && vFlags < 30.0) fHasTexture = 0.0;
	else if(vFlags >= 40.0 && vFlags < 50.0) fHasTexture = 0.0;
	if(vFlags >= 30.0 && vFlags < 40.0) fHasAA = 0.0;
	else if(vFlags >= 50.0 && vFlags < 60.0) fHasAA = 0.0;
	if(vFlags >= 60.0 && vFlags < 70.0) fIsSDF = 1.0;

	int vx = gl_VertexID & 2; 
	int vy = ((gl_VertexID & 1) << 1) ^ 3; 
	float lastDigit = vFlags - float(int(vFlags / 10.0) * 10);
	int anchor = int(lastDigit);

	vec2 size = vSize; 
	vec2 pos = vec2(corners[vx], corners[vy]);
	fPos = pos + vec2(0.5, 0.5);
	pos += vec2(offsetX[anchor], offsetY[anchor]);
	
	/*
	if((vFlags & (1<<6)) > 1) {
		pos = vec2(-pos.y, pos.x); 
		size.xy = size.yx;
	} 
	if((vFlags & (1<<7)) > 1) {
		pos = vec2(pos.y, -pos.x); 
		size.xy = size.yx;
	} 
	*/

	pos *= size; 

	vec2 rot = vec2(cos(vAngle), sin(vAngle));
	mat2 rotmat = mat2(
			rot.x, rot.y,
			-rot.y, rot.x);
	pos -= vCenter;
	pos = rotmat * pos;
	pos += vCenter;

	pos += vPos.xy;
	pos.y -= vPos.z;
	pos -= uOffset;
	pos *= uScale; 

	vec2 normalPos = pos * vec2(2, -2) / uViewport - vec2(1, -1);
	gl_Position = vec4(normalPos, 0, 1);
	//gl_Position = vec4(corners[vx], corners[vy], 0, 1);

	vec4 texVec = vec4(vTexture.xy, vTexture.xy + vTexture.zw); 

	/*
	if((vFlags & (1<<8)) > 1) {
		texVec.xyzw = texVec.zyxw; 
	} 
	if((vFlags & (1<<9)) > 1) {
		texVec.xyzw = texVec.xwzy; 
	} 
	*/

	float[4] texCoords = float[4](
			texVec.x, texVec.y,
			texVec.z, texVec.w); 
	fTexture = vec2(texCoords[vx], texCoords[vy]); 
	fTextureScale = (uScale * vSize) / vTexture.zw;
} 

#version 300 es
precision mediump float;

in vec2 fPos;
in vec2 fTexture;
in vec2 fTextureScale;
in vec4 fColor;
//flat in float fFlags;
flat in float fIsCircle;
flat in float fIsSDF;
flat in float fHasTexture;
flat in float fHasAA;

out vec4 gColor;

uniform sampler2D uTexture; 
uniform vec4 uTint; 
uniform vec2 uInvTextureSize;


float median(float a, float b, float c)
{
	return max(min(a, b), min(max(a, b), c));
}

vec2 subpixelAA(vec2 pixel, vec2 zoom)
{
	vec2 uv = floor(pixel) + 0.5;
    uv += 1.0 - clamp((1.0 - fract(pixel)) * zoom, 0.0, 1.0);
	return uv;
}

void main()
{
	//gColor = vec4(1,1,1,1);
	//return;
	vec4 baseColor = fColor;
	if(fIsSDF > 0.5) {
		vec2 msdfUnit = vec2(8.0) * uInvTextureSize;
		vec2 uv = subpixelAA(fTexture, fTextureScale) * uInvTextureSize;
		vec4 sdfVal = texture(uTexture, uv);
		float sigDist = median(sdfVal.r, sdfVal.g, sdfVal.b) - 0.5;
		sigDist *= dot(msdfUnit, 0.5/fwidth(uv));
		float opacity = clamp(sigDist + 0.5, 0.0, 1.0);
		baseColor *= vec4(1, 1, 1, opacity) * uTint;
		//baseColor = vec4(0.5/fwidth(uv), 1, 1);
	} else if(fHasTexture > 0.5) {
		vec2 uv;
		if(fHasAA > 0.5) {
			uv = subpixelAA(fTexture, fTextureScale);
		} else {
			uv = floor(fTexture) + 0.5;
		}
		baseColor *= texture(uTexture, uv * uInvTextureSize);
	}
	gColor = baseColor;

	if(fIsCircle > 0.5) {
		vec2 dl = fPos - vec2(0.5, 0.5);
		//dist^2 = mag^2 - (0.5)^2
		float dist2 = dot(dl, dl) - 0.25;

		if(dist2 > 0.0) {
			gColor = vec4(0, 0, 0, 0);
		}
	}

}

#version 300 es

uniform  sampler2D     sTexture;
uniform  mediump sampler3D		sColourLUT;

in mediump vec2 texCoords;
layout(location = 0) out lowp vec4 oFragColour;

void main()
{
const highp float array[16] = float[](
	0.820844,
	-0.419144,
	-0.977806,
	0.625848,
	-0.879658,
	0.886325,
	0.920155,
	-0.916329,
	0.844853,
	-0.818481,
	-0.430844,
	-0.102321,
	-0.549422,
	0.468418,
	0.091143,
	-0.391751);

    highp vec3 vCol = texture(sTexture, texCoords).rgb;
	lowp vec3 vAlteredCol = texture(sColourLUT, vCol.rgb).rgb;
    oFragColour = vec4(vAlteredCol.xy, array[int(vAlteredCol.z*15.0)], 1.0);
}

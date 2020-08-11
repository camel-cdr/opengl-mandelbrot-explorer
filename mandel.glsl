#version 330 core
#extension GL_ARB_gpu_shader_fp64 : enable
out vec4 fragColor;
in vec2 pos;
uniform dvec2 dim;
uniform dvec2 center;
uniform dvec2 zoom;
uniform uint maxIter;
uniform float rotation;
uniform sampler1D palet;

dvec2
rot(dvec2 p, dvec2 pivot, float angle)
{
	double s = sin(angle);
	double c = cos(angle);
	p -= pivot;
	return dvec2(p.x*c - p.y*s, p.x*s + p.y*c) + pivot;
}

void
main()
{
	dvec2 C = rot(dvec2(pos * zoom + center), center, rotation);
	dvec2 Z = dvec2(0.0, 0.0);
	uint iter = 0u;
	dvec2 Zsqr = Z;
	while (Zsqr.x + Zsqr.y <= 4.0 && iter < maxIter) {
		Z = dvec2(Zsqr.x - Zsqr.y + C.x, 2.0 * Z.x * Z.y + C.y);
		Zsqr = Z*Z;
		++iter;
	}

	if (iter < maxIter) {
		fragColor = texture(palet, float(iter) / 100.0);
	} else {
		fragColor = vec4(0,0,0,1);
	}
}


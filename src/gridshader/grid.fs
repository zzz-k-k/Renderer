#version 330 core
out vec4 FragColor;

in vec3 WorldPos;

uniform vec3 colorThin;    // 细线颜色
uniform vec3 colorThick;   // 粗线颜色
uniform float spacing;     // 细线间距
uniform float thickEvery;  // 每几格画一条粗线
uniform float fadeDist;    // 远处渐隐

// 抗锯齿线条
float gridLine(float coord, float spacing) {
    float x = coord / spacing;
    float w = fwidth(x);
    float a = abs(fract(x - 0.5) - 0.5) / w;
    return 1.0 - clamp(a, 0.0, 1.0);
}

void main()
{
    float lineX = gridLine(WorldPos.x, spacing);
    float lineZ = gridLine(WorldPos.z, spacing);
    float line = max(lineX, lineZ);

    float thick = max(
        gridLine(WorldPos.x, spacing * thickEvery),
        gridLine(WorldPos.z, spacing * thickEvery)
    );

    vec3 color = mix(colorThin, colorThick, thick);

    // 距离渐隐
    float dist = length(WorldPos.xz);
    float fade = clamp(1.0 - dist / fadeDist, 0.0, 1.0);

    FragColor = vec4(color * line, line * fade);
}

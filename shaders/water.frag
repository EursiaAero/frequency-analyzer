// SmoothWater.frag — Qt 6.10 compatible
// Réduction du banding, reflets doux, rendu d’eau plus fluide

layout(set = 1, binding = 0) uniform sampler2D waterNormalTex;
layout(set = 1, binding = 1) uniform sampler2D waterDiffuseTex;

void MAIN()
{
    vec3 n = normalize(NORMAL);
    float t = waterTime;

    // Coordonnées UV stables et animation croisée
    vec2 uv = normalize(n.xz) * 0.5 + 0.5;
    vec2 uv1 = uv + vec2(t * 0.03, t * 0.02);
    vec2 uv2 = uv - vec2(t * 0.025, t * 0.04);

    // Normales adoucies
    vec3 n1 = texture(waterNormalTex, uv1 * 4.0).rgb * 2.0 - 1.0;
    vec3 n2 = texture(waterNormalTex, uv2 * 3.0).rgb * 2.0 - 1.0;
    vec3 mapN = normalize(mix(n1, n2, 0.5));
    vec3 perturbed = normalize(n + mapN * 0.25);

    // Couleur diffuse animée
    vec3 texA = texture(waterDiffuseTex, uv1 * 2.0).rgb;
    vec3 texB = texture(waterDiffuseTex, uv2 * 2.0).rgb;
    vec3 baseTex = mix(texA, texB, 0.5);

    // Profondeur colorée : turquoise sur bords, bleu foncé au centre
    float depth = clamp(1.0 - abs(n.y), 0.0, 1.0);
    vec3 deepColor = mix(vec3(0.05, 0.25, 0.6), vec3(0.0, 0.05, 0.25), depth);

    // Fresnel plus doux
    float fresnel = pow(1.0 - abs(n.y), 3.0) * 0.8;

    // Spéculaire adoucie avec courbe filmique
    float spec = pow(max(dot(perturbed, normalize(vec3(0.3, 1.0, 0.2))), 0.0), 12.0);
    spec = pow(spec, 1.3);

    // Composition finale
    vec3 color = mix(baseTex, deepColor, 0.5);
    color = color * 1.15 + fresnel * 0.25 + spec * vec3(1.0, 0.95, 0.8);

    // Dithering pour casser le banding
    color += (fract(sin(dot(gl_FragCoord.xy, vec2(12.9898,78.233))) * 43758.5453) - 0.5) * 0.002;

    BASE_COLOR = vec4(color, 0.9);
    ROUGHNESS = 0.08;
    METALNESS = 0.0;
}

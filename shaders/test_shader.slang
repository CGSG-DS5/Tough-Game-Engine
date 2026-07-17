/***
 * Copyright (c) 2026 Tough Game Engine Contributors
 * SPDX-License-Identifier: MIT
 **/

struct PushConstants {
    float time;
}
[[vk::push_constant]] ConstantBuffer<PushConstants> pc;

struct UniformBuffer {
    float color;
};
ConstantBuffer<UniformBuffer> ubo;

float2 rotate(float2 v) {
    float c = cos(pc.time);
    float s = sin(pc.time);
    return float2(
        v.x * c - v.y * s,
        v.x * s + v.y * c
    );
}

float2 rotate(float x, float y) {
    return rotate(float2(x, y));
}

[shader("vertex")]
float4 vertMain() : SV_Position {
    return float4(0, 0, 0, 1);
}

struct GSInput {
    float4 position : SV_Position;
};

struct GSOutput {
    float4 position : SV_Position;
    float3 color;
    float2 tex_coord;
};

[shader("geometry")]
[maxvertexcount(4)]
void geomMain(
    point GSInput input[1],
    inout TriangleStream<GSOutput> outputStream
) {
    GSOutput v;
    v.position = float4(rotate(-0.5, 0.5), 0, 1);
    v.color = float3(1, 0, 0) * ubo.color;
    v.tex_coord = float2(0, 0);
    outputStream.Append(v);

    v.position = float4(rotate(-0.5, -0.5), 0, 1);
    v.color = float3(0, 1, 0) * ubo.color;
    v.tex_coord = float2(0, 1);
    outputStream.Append(v);

    v.position = float4(rotate(0.5, 0.5), 0, 1);
    v.color = float3(0, 0, 1) * ubo.color;
    v.tex_coord = float2(1, 0);
    outputStream.Append(v);

    v.position = float4(rotate(0.5, -0.5), 0, 1);
    v.color = float3(0, 0, 1) * ubo.color;
    v.tex_coord = float2(1, 1);
    outputStream.Append(v);

    outputStream.RestartStrip();
}

layout(set = 1) Sampler2D texture;

struct FSInput {
    float3 color;
    float2 tex_coord;
};

[shader("fragment")]
float4 fragMain(FSInput input) : SV_Target0 {
    // return float4(input.color, 1);
    return texture.Sample(input.tex_coord) * float4(input.color, 1);
}
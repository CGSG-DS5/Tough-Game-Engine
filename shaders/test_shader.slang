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
    [[vk::location(0)]] float3 color : COLOR;
};

[shader("geometry")]
[maxvertexcount(3)]
void geomMain(
    point GSInput input[1],
    inout TriangleStream<GSOutput> outputStream
) {
    GSOutput v0;
    v0.position = float4(rotate(0, -0.5), 0, 1);
    v0.color = float3(1, 0, 0) * ubo.color;
    outputStream.Append(v0);

    GSOutput v1;
    v1.position = float4(rotate(-0.5, 0.5), 0, 1);
    v1.color = float3(0, 1, 0) * ubo.color;
    outputStream.Append(v1);

    GSOutput v2;
    v2.position = float4(rotate(0.5, 0.5), 0, 1);
    v2.color = float3(0, 0, 1) * ubo.color;
    outputStream.Append(v2);

    outputStream.RestartStrip();
}

struct FSInput {
    [[vk::location(0)]] float3 color : COLOR;
};

[shader("fragment")]
float4 fragMain(FSInput input) : SV_Target0 {
    return float4(input.color, 1);
}
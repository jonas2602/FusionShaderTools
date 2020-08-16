{
    "name": "ColorShader",
    "stages": [
        {
            "inputs": [
                {
                    "name": "inPosition",
                    "slot": 0,
                    "type": "float2"
                },
                {
                    "name": "inColor",
                    "slot": 1,
                    "type": "float3"
                }
            ],
            "outputs": [
                {
                    "name": "fragColor",
                    "slot": 0,
                    "type": "float3"
                }
            ],
            "source": "compiled/ColorShader.vert",
            "type": "vertex",
            "uniform_blocks": [
                {
                    "elements": [
                        {
                            "name": "model",
                            "slot": 0,
                            "type": "mat4"
                        },
                        {
                            "name": "view",
                            "slot": 1,
                            "type": "mat4"
                        },
                        {
                            "name": "proj",
                            "slot": 2,
                            "type": "mat4"
                        }
                    ],
                    "layout": "std140",
                    "name": "ubo",
                    "slot": 0,
                    "type": "UniformBufferObject"
                }
            ]
        },
        {
            "inputs": [
                {
                    "name": "fragColor",
                    "slot": 0,
                    "type": "float3"
                }
            ],
            "outputs": [
                {
                    "name": "outColor",
                    "slot": 0,
                    "type": "float4"
                }
            ],
            "source": "compiled/ColorShader.frag",
            "type": "fragment",
            "uniform_blocks": []
        }
    ]
}
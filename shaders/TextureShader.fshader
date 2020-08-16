{
    "name": "TextureShader",
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
                },
                {
                    "name": "inTexCoord",
                    "slot": 2,
                    "type": "float2"
                }
            ],
            "outputs": [
                {
                    "name": "fragColor",
                    "slot": 0,
                    "type": "float3"
                },
                {
                    "name": "fragTexCoord",
                    "slot": 1,
                    "type": "float2"
                }
            ],
            "source": "compiled/TextureShader.vert",
            "textures": [],
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
                },
                {
                    "name": "fragTexCoord",
                    "slot": 1,
                    "type": "float2"
                }
            ],
            "outputs": [
                {
                    "name": "outColor",
                    "slot": 0,
                    "type": "float4"
                }
            ],
            "source": "compiled/TextureShader.frag",
            "textures": [
                {
                    "name": "texSampler",
                    "slot": 1
                }
            ],
            "type": "fragment",
            "uniform_blocks": []
        }
    ]
}
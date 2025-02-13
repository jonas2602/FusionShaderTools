{
    "name": "TextureShader",
    "stages": [
        {
            "inputs": [
                {
                    "name": "a_Position",
                    "slot": 0,
                    "type": "float3"
                },
                {
                    "name": "a_Color",
                    "slot": 1,
                    "type": "float3"
                },
                {
                    "name": "a_TexCoord",
                    "slot": 2,
                    "type": "float2"
                }
            ],
            "outputs": [
                {
                    "name": "v_FragColor",
                    "slot": 0,
                    "type": "float3"
                },
                {
                    "name": "v_FragTexCoord",
                    "slot": 1,
                    "type": "float2"
                }
            ],
            "source": "compiled/TextureShader.vert",
            "textures": [],
            "type": "vertex",
            "uniform_blocks": [
                {
                    "binding": 0,
                    "elements": [
                        {
                            "name": "View",
                            "slot": 0,
                            "type": "mat4"
                        },
                        {
                            "name": "Projection",
                            "slot": 1,
                            "type": "mat4"
                        },
                        {
                            "name": "ViewProjection",
                            "slot": 2,
                            "type": "mat4"
                        }
                    ],
                    "layout": "std140",
                    "name": "u_Camera",
                    "set": 0,
                    "type": "CameraBuffer"
                },
                {
                    "binding": 2,
                    "elements": [
                        {
                            "name": "Transform",
                            "slot": 0,
                            "type": "mat4"
                        }
                    ],
                    "layout": "std140",
                    "name": "u_Model",
                    "set": 0,
                    "type": "ModelBuffer"
                }
            ]
        },
        {
            "inputs": [
                {
                    "name": "v_FragColor",
                    "slot": 0,
                    "type": "float3"
                },
                {
                    "name": "v_FragTexCoord",
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
                    "binding": 3,
                    "name": "u_TexSampler",
                    "set": 0
                },
                {
                    "binding": 1,
                    "name": "u_TexSampler1",
                    "set": 1
                },
                {
                    "binding": 2,
                    "name": "u_TexSampler2",
                    "set": 2
                }
            ],
            "type": "fragment",
            "uniform_blocks": []
        }
    ]
}
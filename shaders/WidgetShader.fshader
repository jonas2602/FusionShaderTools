{
    "name": "WidgetShader",
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
            "source": "compiled/WidgetShader.vert",
            "textures": [],
            "type": "vertex",
            "uniform_blocks": [
                {
                    "binding": 0,
                    "count": 0,
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
            "source": "compiled/WidgetShader.frag",
            "textures": [
                {
                    "binding": 0,
                    "count": 1,
                    "name": "u_TexSampler",
                    "set": 1
                }
            ],
            "type": "fragment",
            "uniform_blocks": []
        }
    ]
}
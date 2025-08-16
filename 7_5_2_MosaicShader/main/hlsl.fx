// -------------------------------------------------------------
// モザイク
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
texture tMask;
sampler MaskSamp = sampler_state
{
    Texture = <tMask>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
texture tSrc;
sampler SrcSamp = sampler_state
{
    Texture = <tSrc>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos            : POSITION;
    float2 Tex0           : TEXCOORD0;// マスクテクスチャ
    float2 Tex1           : TEXCOORD1;// レンダリング画像
};

// -------------------------------------------------------------
// ピクセルシェーダプログラム
// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
    float4 Color;
    const float grids = 1.0f; // 元サイズ256だとほぼモザイク無し、徐々に画面分割数をあげるとモザイクが強くなる
    
    // floorを使って、グリッドサイズでテクスチャ座標を切り捨てる
    float2 tex_coord = floor( grids * In.Tex1 + 0.5f)/grids;
    Color.rgb = tex2D( SrcSamp, tex_coord );

    Color.a = tex2D( MaskSamp, In.Tex0 );// 透明度
    
    return Color;
}

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        PixelShader  = compile ps_2_0 PS();
    }
}

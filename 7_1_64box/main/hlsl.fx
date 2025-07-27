// ------------------------------------------------------------
// 64ボックスフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float MAP_WIDTH;
float MAP_HEIGHT;

// ------------------------------------------------------------
// テクスチャ
// ------------------------------------------------------------
texture SrcMap;
sampler SrcSamp = sampler_state
{
    Texture = <SrcMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// ------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// ------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
	float2 Tex0			: TEXCOORD0;
	float2 Tex1			: TEXCOORD1;
	float2 Tex2			: TEXCOORD2;
	float2 Tex3			: TEXCOORD3;
	float2 Tex4			: TEXCOORD4;
	float2 Tex5			: TEXCOORD5;
	float2 Tex6			: TEXCOORD6;
	float2 Tex7			: TEXCOORD7;
};

// ------------------------------------------------------------
// 頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS (
      float4 Pos    : POSITION           // モデルの頂点
     ,float4 Tex    : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(3.0f/MAP_WIDTH, -3.0f/MAP_HEIGHT);
    Out.Tex1 = Tex + float2(3.0f/MAP_WIDTH, -1.0f/MAP_HEIGHT);
    Out.Tex2 = Tex + float2(3.0f/MAP_WIDTH, +1.0f/MAP_HEIGHT);
    Out.Tex3 = Tex + float2(3.0f/MAP_WIDTH, +3.0f/MAP_HEIGHT);
    Out.Tex4 = Tex + float2(1.0f/MAP_WIDTH, -3.0f/MAP_HEIGHT);
    Out.Tex5 = Tex + float2(1.0f/MAP_WIDTH, -1.0f/MAP_HEIGHT);
    Out.Tex6 = Tex + float2(1.0f/MAP_WIDTH, +1.0f/MAP_HEIGHT);
    Out.Tex7 = Tex + float2(1.0f/MAP_WIDTH, +3.0f/MAP_HEIGHT);
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS ( VS_OUTPUT In ) : COLOR0
{
	float4 t0 = tex2D(SrcSamp, In.Tex0);
	float4 t1 = tex2D(SrcSamp, In.Tex1);
	float4 t2 = tex2D(SrcSamp, In.Tex2);
	float4 t3 = tex2D(SrcSamp, In.Tex3);
	
	float4 t4 = tex2D(SrcSamp, In.Tex4);
	float4 t5 = tex2D(SrcSamp, In.Tex5);
	float4 t6 = tex2D(SrcSamp, In.Tex6);
	float4 t7 = tex2D(SrcSamp, In.Tex7);
	
	float4 t8 = tex2D(SrcSamp, In.Tex0 + float2(-4.0f/MAP_WIDTH, 0));
	float4 t9 = tex2D(SrcSamp, In.Tex1 + float2(-4.0f/MAP_WIDTH, 0));
	float4 ta = tex2D(SrcSamp, In.Tex2 + float2(-4.0f/MAP_WIDTH, 0));
	float4 tb = tex2D(SrcSamp, In.Tex3 + float2(-4.0f/MAP_WIDTH, 0));
	
	float4 tc = tex2D(SrcSamp, In.Tex4 + float2(-4.0f/MAP_WIDTH, 0));
	float4 td = tex2D(SrcSamp, In.Tex5 + float2(-4.0f/MAP_WIDTH, 0));
	float4 te = tex2D(SrcSamp, In.Tex6 + float2(-4.0f/MAP_WIDTH, 0));
	float4 tf = tex2D(SrcSamp, In.Tex7 + float2(-4.0f/MAP_WIDTH, 0));
	
	return ((t0+t1+t2+t3)
		   +(t4+t5+t6+t7)
		   +(t8+t9+ta+tb)
		   +(tc+td+te+tf))/16;
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

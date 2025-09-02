// ------------------------------------------------------------
// 輪郭抽出
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float4x4 mWVP0;
float4x4 mWVP1;

float4   vCol;
float4	 vLightDir;	// ライトの方向

// ------------------------------------------------------------
// テクスチャ
// ------------------------------------------------------------
texture SrcTex;
sampler SrcSamp = sampler_state
{
    Texture = <SrcTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};

// ------------------------------------------------------------
texture FloorTex;
sampler FloorSamp = sampler_state
{
    Texture = <FloorTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

// ------------------------------------------------------------
texture OriginalTex;
sampler OriginalSamp = sampler_state
{
    Texture = <OriginalTex>;
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
    float4 Color		: COLOR0;
	float2 Tex0			: TEXCOORD0;
};

// ------------------------------------------------------------
// 照明計算なし頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass0 (
      float4 Pos    : POSITION           // モデルの頂点
     ,float4 Normal : NORMAL             // 法線ベクトル
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP0);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算なしピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass0 (VS_OUTPUT In) : COLOR
{
	float4 Out;
	
	// 色
    Out = tex2D(FloorSamp, In.Tex0);
	
	return Out;
}


// ------------------------------------------------------------
// 照明計算あり頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass1 (
      float4 Pos    : POSITION           // モデルの頂点
    , float4 Normal : NORMAL             // 法線ベクトル
     ,float4 Tex0   : TEXCOORD0	         // テクスチャ座標
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP1);
	
	// 色
	float diffuse = max(dot(vLightDir.xyz, Normal.xyz), 0);//拡散色
	float ambient = vLightDir.w;						   //環境色
	Out.Color = vCol * ( diffuse + ambient );
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}
// ------------------------------------------------------------
// 照明計算ありピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_pass1 (VS_OUTPUT In) : COLOR
{
	float4 Out;
	
	// 色
    Out = In.Color * tex2D(SrcSamp, In.Tex0);
	
	return Out;
}


// ------------------------------------------------------------
// 照明計算なし頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS_pass2(
      float4 Pos : POSITION // モデルの頂点
     , float4 Normal : NORMAL // 法線ベクトル
     , float4 Tex0 : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
	// 位置座標
    Out.Pos = mul(Pos, mWVP1);
	
	// テクスチャ座標
    Out.Tex0 = Tex0;

    return Out;
}


// ------------------------------------------------------------
// 法線エッジピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS_NormalEdge(VS_OUTPUT In) : COLOR0
{
    float2 uv = In.Tex0;
    float2 off = 1/512;

    // 4近傍サンプル（左上・右下・左下・右上）
    float4 tl = tex2D(OriginalSamp, uv + float2(-off.x, -off.y));
    float4 br = tex2D(OriginalSamp, uv + float2(off.x, off.y));
    float4 bl = tex2D(OriginalSamp, uv + float2(-off.x, off.y));
    float4 tr = tex2D(OriginalSamp, uv + float2(off.x, -off.y));

    // 差分の二乗（輝度差の近似）
    float diff1 = dot(tl - br, tl - br);
    float diff2 = dot(bl - tr, bl - tr);

    // 1 - (diff1 + diff2) → エッジ部分だけ黒く、それ以外は白
    float edge = saturate(1.0 - diff1 - diff2);

    return float4(edge, edge, edge, 1.0);
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0 // 照明計算なし
    {
        VertexShader = compile vs_1_1 VS_pass0();
        PixelShader  = compile ps_2_0 PS_pass0();
    }

    pass P1 // 照明計算あり
    {
        VertexShader = compile vs_1_1 VS_pass1();
        PixelShader = compile ps_2_0 PS_pass1();
    }

    pass P2 // 法線エッジ
    {
        PixelShader = compile ps_2_0 PS_NormalEdge();
    }
}

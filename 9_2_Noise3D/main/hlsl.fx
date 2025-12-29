// -------------------------------------------------------------
// Perlin Noise
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP; // 座標変換の行列
float4 LightDir; // ライトベクトル
float time;

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
// ランダムのテクスチャ
texture Tex;
sampler Samp = sampler_state
{
    Texture = <Tex>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

texture WoodTex;
sampler WoodSamp = sampler_state
{
    Texture = <WoodTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Wrap;
    AddressV = Wrap;
};

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos : POSITION;
    float4 Col : COLOR0;
    float3 Tex : TEXCOORD0; // デカールテクスチャ座標
};

// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos : POSITION, // ローカル位置座標
      float3 Normal : NORMAL // 法線ベクトル
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
	
	// 座標変換
    Out.Pos = mul(Pos, mWVP);
	
	// ライティング
    Out.Col = max(0, dot(Normal, LightDir.xyz)) + LightDir.w;

	// 位置を適当に変換して
    Out.Tex = 0.5f * Pos + 0.5;

    return Out;
}
// -------------------------------------------------------------
// １つのオクターブのノイズ関数
// -------------------------------------------------------------
float noise(float gap, float3 pos)
{
    float3 x = gap * pos;
    float3 ix = floor(x) / 32.0f;
    float3 fx = frac(x);
	
    float4 x0, x1;

    x0.x = tex3D(Samp, ix + float3(0.f / 32.f, 0.f / 32.f, 0.f / 32.f)).x;
    x0.y = tex3D(Samp, ix + float3(1.f / 32.f, 0.f / 32.f, 0.f / 32.f)).x;
    x0.z = tex3D(Samp, ix + float3(0.f / 32.f, 1.f / 32.f, 0.f / 32.f)).x;
    x0.w = tex3D(Samp, ix + float3(1.f / 32.f, 1.f / 32.f, 0.f / 32.f)).x;
	
    x1.x = tex3D(Samp, ix + float3(0.f / 32.f, 0.f / 32.f, 1.f / 32.f)).x;
    x1.y = tex3D(Samp, ix + float3(1.f / 32.f, 0.f / 32.f, 1.f / 32.f)).x;
    x1.z = tex3D(Samp, ix + float3(0.f / 32.f, 1.f / 32.f, 1.f / 32.f)).x;
    x1.w = tex3D(Samp, ix + float3(1.f / 32.f, 1.f / 32.f, 1.f / 32.f)).x;
	
    float4 x01 = lerp(x0.xyzw, x1.xyzw, fx.z); // x0とx1をz方向に線形補間(x01のz成分を持つ座標を取得)
    x01.xy = lerp(x01.xy, x01.zw, fx.y);
    return lerp(x01.x, x01.y, fx.x);
}

// -------------------------------------------------------------
// ピクセルシェーダ
// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{
    float4 color = { 167.f / 256.f, 105.f / 256.f, 61.f / 256.f, 0 };

    float n = 0.5000f * noise(12.f, In.Tex)
			+ 0.2500f * noise(24.f, In.Tex);

    return In.Col * tex2D(WoodSamp, n + time);
//	return color * (0.8f*fmod(10*n,1)+0.2);
}

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 PS();
    }
}

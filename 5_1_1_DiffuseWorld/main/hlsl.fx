// -------------------------------------------------------------
// 拡散光
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// 参考: https://note.com/cgm_cyuiragi/n/nc780a8bf6767
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------

float4x4 mWVP;
float4x4 mWIT;

float3 vLightDir;							// ライトの方向

// 光の強さ
float4 I_a = { 0.3f, 0.3f, 0.3f, 0.0f }; // ambient
float4 I_d = { 0.7f, 0.7f, 0.7f, 0.0f };    // diffuse

// 反射率
float4 k_a = { 1.0f, 1.0f, 1.0f, 1.0f };    // ambient
float4 k_d = { 1.0f, 1.0f, 1.0f, 1.0f };    // diffuse

texture vTex;
sampler Samp = sampler_state
{
    Texture = <vTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = LINEAR;
    AddressU = WRAP;
    AddressV = WRAP;
};

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;
    float2 Tex : TEXCOORD0;
};

// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos    : POSITION,          // ローカル位置座標
      float3 Normal : NORMAL,            // 法線ベクトル
      float2 Texcoord : TEXCOORD0 // テクスチャ座標
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(Pos, mWVP);
	
	// 頂点色
	float3 L = -vLightDir;
	float3 N = normalize(mul(Normal, (float3x3)mWIT)); // ワールド座標系での法線
    
    float4 diffuse = I_d * k_d * max(0, dot(N, L)); // 拡散光
    float4 ambient = I_a * k_a; // 環境光
	
    //Out.Color = diffuse; // 拡散光
    //Out.Color = ambient;// 環境光
    Out.Color = ambient + diffuse; // 環境光 + 拡散光
    Out.Tex = Texcoord; // テクスチャ座標
    
	return Out;
}

// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
    return tex2D(Samp, In.Tex);
}

// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        VertexShader = compile vs_2_0 VS(); // vs_1_1 -> ps_2_0
        PixelShader = compile ps_2_0 PS(); // ps_1_1 -> ps_2_0
    }
}

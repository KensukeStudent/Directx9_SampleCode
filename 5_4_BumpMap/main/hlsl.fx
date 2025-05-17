// -------------------------------------------------------------
// バンプマップ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------

float4x4 mWVP;		// 座標変換の行列

float4 vLightDir;	// ライトの方向
float4 vColor;		// ライト＊メッシュの色
float3 vEyePos;		// カメラの位置（ローカル座標系）

// -------------------------------------------------------------
// テクスチャ
// -------------------------------------------------------------
// 模様のテクスチャ
texture DecaleTex;
sampler DecaleSamp = sampler_state
{
    Texture = <DecaleTex>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// 法線マップ
texture NormalMap;
sampler NormalSamp = sampler_state
{
    Texture = <NormalMap>;
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
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;		// 頂点色
    float2 Tex			: TEXCOORD0;	// デカールテクスチャ座標
    float3 L			: TEXCOORD1;	// ライトベクトル
    float3 E			: TEXCOORD2;	// 視線ベクトル
};
// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos      : POSITION,          // ローカル位置座標
      float3 Normal   : NORMAL,            // 法線ベクトル
      float3 Tangent  : TANGENT0,          // 接ベクトル
      float2 Texcoord : TEXCOORD0          // 法線ベクトル
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(Pos, mWVP);
	
	// メッシュの色
	Out.Color = vColor;
	
	// デカール用のテクスチャ座標
	Out.Tex = Texcoord;

	// 座標系の変換基底
	float3 N = Normal;
	float3 T = Tangent;
	float3 B = cross(N,T);

	// 鏡面反射用のベクトル
	float3 E = vEyePos - Pos.xyz;	// 視線ベクトル
	Out.E.x = dot(E,T);
	Out.E.y = dot(E,B);
	Out.E.z = dot(E,N);

	float3 L = -vLightDir.xyz;		// ライトベクトル
	Out.L.x = dot(L,T);
	Out.L.y = dot(L,B);
	Out.L.z = dot(L,N);
	
	return Out;
}
// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
	float3 N = 2.0f*tex2D( NormalSamp, In.Tex ).xyz-1.0;// 法線マップからの法線
	float3 L = normalize(In.L);						// ライトベクトル
	float3 R = reflect(-normalize(In.E), N);		// 反射ベクトル
	float amb = -vLightDir.w;						// 環境光の強さ
	
    //return In.Color * tex2D(DecaleSamp, In.Tex) // 拡散光と環境光には、
			 //  * (max(0, dot(N, L)) + amb) // 頂点色とテクスチャの色を合成する
			 //+ 0.3f * pow(max(0, dot(R, L)), 8); // Phong 鏡面反射光
    
    //return tex2D(DecaleSamp, float2(0.5, 1)); u:0.7~0.9の時に黒くなる
	
    return tex2D(DecaleSamp, saturate(In.Tex));
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
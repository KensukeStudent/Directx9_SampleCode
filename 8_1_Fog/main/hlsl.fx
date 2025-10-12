// -------------------------------------------------------------
// フォグ
// 
// Copyright (c) 2002,2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// ローカルから射影空間への座標変換
float4	 vLightDir;	// ライトの位置
float4   vCol;		// メッシュの色
float4   vFog;		// (Far/(Far-Near), -1/(Far-Near))

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: POSITION;
	float4 Col		: COLOR0;
	float2 Tex		: TEXCOORD0;
	float  Fog		: FOG;
};

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS (
	float4 Pos		: POSITION,			// モデルの頂点
	float4 Normal	: NORMAL,			// モデルの法線
	float2 Tex		: TEXCOORD0			// テクスチャ座標
){
	VS_OUTPUT Out = (VS_OUTPUT)0;		// 出力データ
	
	float4 pos = mul( Pos, mWVP );		// 座標変換
	
	Out.Pos = pos;						// 位置座標
	
	Out.Col = vCol * max( dot(vLightDir, Normal), 0);	// 照明計算
	
	Out.Tex = Tex;						// テクスチャ座標
	
	// Far = 100, Near = 1 の場合
	// vFog.x = 100/(100-1) = 1.01
	// vFog.y = -1/(100-1) = -0.0101
	
	// フォグの計算検証(max100はfarとnearに応じて変わる)
	// pos.w = 1
	// -> Out.Fog = 1.01 + 1 * -0.0101 = 1.0 (フォグなし)
	// pos.w = 50
	// -> Out.Fog = 1.01 + 50 * -0.0101 = 0.505 (フォグ半分)
	// pos.w = 100
	// -> Out.Fog = 1.01 + 100 * -0.0101 = 0.0 (フォグ最大)
	
	// far-pos.w/(far-near): フォグの計算式
	Out.Fog = vFog.x + pos.w * vFog.y;	// フォグ
	
	return Out;
}
// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        
		FogEnable = true;			// フォグを有効にする
        FogVertexMode = Linear;		// 線形フォグ
        FogColor = 0xd8e3fe;		// フォグの色
    }
}

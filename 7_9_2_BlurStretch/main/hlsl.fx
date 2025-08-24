// ------------------------------------------------------------
// モーションブラー
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWV;
float4x4 mLastWV;
float4x4 mVP;
float3 vEyePos;
float3 vLightDir;
float4 vCol;

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;
};
// -------------------------------------------------------------
struct OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;
};
// -------------------------------------------------------------
OUTPUT VS(
      float4 Pos    : POSITION,          // モデルの頂点
      float3 Normal : NORMAL             // モデルの法線
){
	OUTPUT Out = (OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(mul(Pos, mWV), mVP);
	
	// 色
	Out.Color = vCol * (0.7*max( dot(vLightDir, Normal), 0)// 拡散色
						+0.3);// 環境色
	
	return Out;
}
// -------------------------------------------------------------
OUTPUT VS_Blur(
      float4 Pos    : POSITION,          // モデルの頂点
      float3 Normal : NORMAL             // モデルの法線
){
	OUTPUT Out = (OUTPUT)0;        // 出力データ
	
	// 座標変換
	float4 x1 = mul(Pos, mWV);			// 今回のビュー座標
	float4 x0 = mul(Pos, mLastWV);		// １フレーム前のビュー座標
	float4 v = x1-x0;					// 速度
	float3 n = mul(Normal, mWV);		// ビュー座標系での法線
	
	bool bFront = (0<=dot(n, v.xyz));	// 速度方向を向いてる？
	float4 x = bFront ? x1 : x0;		// 向きによって、位置を決める
	
	Out.Pos = mul(x, mVP);				// 射影空間に変換
	
	// 色
	Out.Color = vCol * (0.7*max( dot(vLightDir, Normal), 0)// 拡散色
						+0.3);// 環境色
	
	// 進行方向を向いていれば不透明、反対なら透明
	Out.Color.a = bFront ? 1 : 0;
	
	return Out;
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // 通常描画
        VertexShader = compile vs_1_1 VS();
    }
    pass P1
    {
        // モーションブラー
        VertexShader = compile vs_1_1 VS_Blur();
    }
}

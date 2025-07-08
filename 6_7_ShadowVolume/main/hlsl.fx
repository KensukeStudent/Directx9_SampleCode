// -------------------------------------------------------------
// シャドウボリュームによる影
// 
// Copyright (c) 2002,2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP_SmallBox;		// ローカルから射影空間への座標変換
float4x4 mWVP_LargeBox;		// ローカルから射影空間への座標変換
float4	 vLightPos_smallBox;	// ライトの位置
float4	 vLightPos_largeBox;	// ライトの位置

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
	float4 Pos		: POSITION;
};

// -------------------------------------------------------------
// 頂点シェーダプログラム
// -------------------------------------------------------------
VS_OUTPUT VS_SmallBox (
      float4 Pos    : POSITION,          // モデルの頂点
      float4 Normal : NORMAL	         // モデルの法線
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
    // 光の裏になっている面を後ろに引き伸ばす
    float4 dir = vLightPos_smallBox - Pos;
    float LN = dot( Normal, dir ); // 光の方向と逆を向いていれば引き延ばす
    float scale = (0<LN) ? 0.0f : 1.0f;
    
    // 座標変換
    Pos.xyz -= 0.001f*Pos;// 縞がおきないように少し縮める
    Out.Pos = mul(Pos - scale * dir, mWVP_SmallBox); // 引き延ばした座標を射影空間に変換
    
    return Out;
}

VS_OUTPUT VS_LargeBox(
      float4 Pos : POSITION, // モデルの頂点
      float4 Normal : NORMAL // モデルの法線
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
	
    // 光の裏になっている面を後ろに引き伸ばす
    float4 dir = vLightPos_largeBox - Pos;
    float LN = dot(Normal, dir);
    float scale = (0 < LN) ? 0.0f : 1.0f;
    
    // 座標変換
    Pos.xyz -= 0.001f * Pos; // 縞がおきないように少し縮める
    Out.Pos = mul(Pos - scale * dir, mWVP_LargeBox);
    
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
        VertexShader = compile vs_1_1 VS_SmallBox();
        PixelShader  = NULL;
    }
    pass P1
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS_LargeBox();
        PixelShader = NULL;
    }
}

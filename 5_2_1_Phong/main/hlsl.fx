// -------------------------------------------------------------
// 鏡面反射光
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------

float4x4 mWVP;

float4 vLightDir;	// ライトの方向
float4 vColor;		// ライト＊メッシュの色
float3 vEyePos;		// カメラの位置（ローカル座標系）

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Color		: COLOR0;
};
// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos    : POSITION,          // ローカル位置座標
      float4 Normal : NORMAL            // 法線ベクトル
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(Pos, mWVP);
	
	// 頂点色
	float ambient = -vLightDir.w;	// 環境光の強さ
	
	float3 eye = normalize(vEyePos - Pos.xyz);
	float3 L = -vLightDir; // ローカル座標系でのライトベクトル
	float3 N = Normal.xyz;
	float3 R = -eye + 2.0f*dot(N,eye)*N;	// 反射ベクトル 公式: R = -E + 2.0 * dot(N・E) * N
	
	// 環境光 + 拡散反射 + 鏡面反射
    float4 ambientAndDiffuse = vColor * max(ambient, dot(Normal, -vLightDir));
    float4 phong = pow(max(0, dot(L, R)), 10);
    Out.Color = ambientAndDiffuse + phong;
	
	return Out;
}

// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
    return In.Color;
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

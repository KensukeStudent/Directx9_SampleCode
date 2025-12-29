// -------------------------------------------------------------
// Perlin Noise
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// グローバル変数
// -------------------------------------------------------------
float4x4 mWVP;		// 座標変換の行列
float4 LightDir;		// ライトベクトル

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
    AddressV = Clamp;
};

// -------------------------------------------------------------
// 頂点シェーダからピクセルシェーダに渡すデータ
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
    float4 Col			: COLOR0;
    float3 Tex			: TEXCOORD0;	// デカールテクスチャ座標
};
// -------------------------------------------------------------
// 座標変換
// -------------------------------------------------------------

// 長さ1の3Dベクトルを2D座標空間に変換する
/// v: 頂点位置のベクトル方向
float2 Q(float3 v)
{
	float2 Out = (float2)0;
	
	// pvなし
	// |・-------------・|
	// 0                 1
	
	// pvあり
    // |--・---------・--| clamp的な役割ぽい(ただし、こちらは現象を弱めるための倍率計算)
	// 0                 1
	float pv = 7.0f/8.0f; // 端に振れないように縮める倍率
	
	// y方向を見る角度によって、x・zの広がり具合を調整する係数(下方向を強調する投影)
    //v.y = +1    ar = 0.25　←すごく縮む
    //v.y = 0     ar = 0.5   ←基準
    //v.y = -0.5  ar = 1.0   ←等倍
    //v.y = -0.9  ar = 5.0   ←激しく拡大
	float ar = 1.0/(2.0*(1.0+v.y));

	// 最低：-0.5, 最大：1.5,ただし、warpで周期的に折り返される
	Out.x = pv * ar * v.x + 0.5;
	Out.y = pv * ar * v.z + 0.5;

	return Out;
}

// -------------------------------------------------------------
// シーンの描画
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos      : POSITION,         // ローカル位置座標
      float3 Normal   : NORMAL            // 法線ベクトル
){
	VS_OUTPUT Out = (VS_OUTPUT)0;        // 出力データ
	
	// 座標変換
	Out.Pos = mul(Pos, mWVP);
	
	// ライティング
	Out.Col = max(0,dot(Normal, LightDir.xyz)) - LightDir.w;

	// 位置を適当に変換して
	Out.Tex.xy = Q(normalize(Pos));

	return Out;
}
// -------------------------------------------------------------
// １つのオクターブのノイズ関数
// -------------------------------------------------------------
float noise( float gap, float3 pos )
{
	// gap間隔でサンプリング（gapが大きいほど振動が高くなり、高周波数になる）
	float2 x = gap * pos;
	// 128画像サイズ, 128内のどこに位置するか
	float2 ix = floor(x)/128.0f;
	float2 fx = frac(x); // 各4成分を線形補完するための変数

    float x00 = tex2D(Samp, ix + float2(0.f / 128.f, 0.f / 128.f)).x; // 左上
    float x10 = tex2D(Samp, ix + float2(1.f / 128.f, 0.f / 128.f)).x; // 右上
    float x01 = tex2D(Samp, ix + float2(0.f / 128.f, 1.f / 128.f)).x; // 左下
    float x11 = tex2D(Samp, ix + float2(1.f / 128.f, 1.f / 128.f)).x; // 右下
	
	// 以下のような特定のマス内から4点をサンプリングして4点間で補完するイメージ
	// ■■■■■■■■■■
	// ■■■■□□■■■■
	// ■■■■□□■■■■
	// ■■■■■■■■■■
	
	return lerp(
		lerp(x00, x01, fx.y), // 左側テクセル縦方向
		lerp(x10, x11, fx.y), // 右側テクセル縦方向 
		fx.x // 横方向
	);
}
// -------------------------------------------------------------
// ピクセルシェーダ
// -------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR
{   
	// 4オクターブのノイズを合成してざらざらとした質感を表現(1つのみだとのっぺりとしたものとなりやすい)
	// ふり幅 * ノイズ(周波数, 位置)
    float n = 0.5000f * noise(8.f, In.Tex)
			+ 0.2500f * noise(16.f, In.Tex);
			+ 0.1250f * noise(32.f, In.Tex)
			+ 0.0625f * noise(64.f, In.Tex);

	return In.Col * tex2D( WoodSamp, float2(0.5, n) );

	//float4 color = {167.f/256.f, 105.f/256.f, 61.f/256.f, 0};
    //return color * (0.8f * fmod(10 * n, 1) + 0.2);
}




// -------------------------------------------------------------
// テクニック
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        VertexShader = compile vs_1_1 VS();
        PixelShader  = compile ps_2_0 PS();
    }
}

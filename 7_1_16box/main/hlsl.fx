// ------------------------------------------------------------
// 16ボックスフィルタ
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// ------------------------------------------------------------


//MinFilterやMagFilterが自前のフィルター処理と何が違うのか
//
//MinFilterやMagFilter
//画面に出力されるときまたはhlslに渡すときに施される処理のこと
//
//自前のフィルター処理
//渡されたものをどうカスタマイズするかが違いである
//
// AI生成の補足： 両者の連携
//HLSLシェーダ内の tex2D(SrcSamp, In.Tex0)
//のような呼び出しでは、まずSrcSamp に設定されたMinFilter やMagFilter のルールに基づいてテクスチャから色がサンプリングされます。
//その後、 そのサンプリングされた色を、あなたの書いたシェーダコードがさらに加工する、 という流れになります。
//したがって、 両者は排他的なものではなく、連携して最終的なレンダリング結果を生成します。 
//MinFilter/MagFilter は基本的なフィルタリングを提供し、自前のシェーダコードはそれに加えてより複雑なフィルタリングや効果を実装するために使用されます。


// ------------------------------------------------------------
// グローバル変数
// ------------------------------------------------------------
float WIDTH;
float HEIGHT;

// ------------------------------------------------------------
// テクスチャ
// ------------------------------------------------------------
texture SrcMap;
sampler SrcSamp = sampler_state
{
    Texture = <SrcMap>;
    MinFilter = LINEAR; // LINEARをバイリニアサンプリングを設定, 受け取るテクスチャカラーの中心4点の平均化されたカラーを返すため
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
    float4 Pos : POSITION;
    float2 Tex0 : TEXCOORD0;
    float2 Tex1 : TEXCOORD1;
    float2 Tex2 : TEXCOORD2;
    float2 Tex3 : TEXCOORD3;
};

// ------------------------------------------------------------
// 頂点シェーダプログラム
// ------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos : POSITION // モデルの頂点
     , float2 Tex : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    float texPoint = 1.0f;
    Out.Tex0 = Tex + float2(-texPoint / WIDTH, -texPoint / HEIGHT);
    Out.Tex1 = Tex + float2(+texPoint / WIDTH, -texPoint / HEIGHT);
    Out.Tex2 = Tex + float2(-texPoint / WIDTH, +texPoint / HEIGHT);
    Out.Tex3 = Tex + float2(+texPoint / WIDTH, +texPoint / HEIGHT);
    
    return Out;
}

// ------------------------------------------------------------
// ピクセルシェーダプログラム
// ------------------------------------------------------------
float4 PS(VS_OUTPUT In) : COLOR0
{
    // 見た目上は4つのテクスチャサンプルだが、MinFilterやMagFilterの設定により、実際には合計16点の平均を取っている
    
    // 4つのテクスチャサンプルを取得
    float4 color0 = tex2D(SrcSamp, In.Tex0); // 受け取るカラーは4点の平均化されたカラー
    float4 color1 = tex2D(SrcSamp, In.Tex1);
    float4 color2 = tex2D(SrcSamp, In.Tex2);
    float4 color3 = tex2D(SrcSamp, In.Tex3);
    
    // 4つのサンプルの平均を計算（ボックスフィルター）
    return (color0 + color1 + color2 + color3) * 0.25f;
}

// ------------------------------------------------------------
// ９コーンフィルタサンプリング
// ------------------------------------------------------------
VS_OUTPUT VS9(
      float4 Pos : POSITION // モデルの頂点
     , float2 Tex : TEXCOORD0 // テクスチャ座標
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // 出力データ
    
    // 位置座標
    Out.Pos = Pos;
    
    Out.Tex0 = Tex + float2(0.0f / WIDTH, 0.0f / HEIGHT);
    Out.Tex1 = Tex + float2(+1.0f / WIDTH, 0.0f / HEIGHT);
    Out.Tex2 = Tex + float2(0.0f / WIDTH, +1.0f / HEIGHT);
    Out.Tex3 = Tex + float2(+1.0f / WIDTH, +1.0f / HEIGHT);
    
    return Out;
}

// ------------------------------------------------------------
// テクニック
// ------------------------------------------------------------
technique TShader
{
    pass P0 // １６ボックスフィルタサンプリング
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 PS();
        
        // サンプラ
        Sampler[0] = (SrcSamp);
        Sampler[1] = (SrcSamp);
        Sampler[2] = (SrcSamp);
        Sampler[3] = (SrcSamp);
    }
    pass P1 // ９コーンフィルタサンプリング
    {
        // シェーダ
        VertexShader = compile vs_1_1 VS9();
        PixelShader = compile ps_2_0 PS();
        
        // サンプラ
        Sampler[0] = (SrcSamp);
        Sampler[1] = (SrcSamp);
        Sampler[2] = (SrcSamp);
        Sampler[3] = (SrcSamp);
    }
}
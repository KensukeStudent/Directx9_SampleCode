// -------------------------------------------------------------
// �v���C�I���e�B�o�b�t�@�V���h�E
// 
// Copyright (c) 2003 IMAGIRE Takashi. All rights reserved.
// -------------------------------------------------------------

// -------------------------------------------------------------
// �O���[�o���ϐ�
// -------------------------------------------------------------
float4x4 mWVP;		// ���[�J������ˉe��Ԃւ̍��W�ϊ�
float4x4 mWLP;		// ���[�J������ˉe��Ԃւ̍��W�ϊ�
float4x4 mWVPT;		// �e�N�X�`�����W�n�ւ̎ˉe
float4   vCol;		// ���b�V���̐F
float4   vId;		// �v���C�I���e�B�ԍ�
float4	 vLightDir;	// ���C�g�̕���

// -------------------------------------------------------------
// �e�N�X�`��
// -------------------------------------------------------------
texture IdMap;
sampler IdMapSamp = sampler_state
{
    Texture = <IdMap>;
    MinFilter = POINT;
    MagFilter = POINT;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
texture DecaleMap;
sampler DecaleMapSamp = sampler_state
{
    Texture = <DecaleMap>;
    MinFilter = LINEAR;
    MagFilter = LINEAR;
    MipFilter = NONE;

    AddressU = Clamp;
    AddressV = Clamp;
};
// -------------------------------------------------------------
// ���_�V�F�[�_����s�N�Z���V�F�[�_�ɓn���f�[�^
// -------------------------------------------------------------
struct VS_OUTPUT
{
    float4 Pos			: POSITION;
	float4 Diffuse		: COLOR0;
	float4 Ambient		: COLOR1;
	float4 ShadowMapUV	: TEXCOORD0;
	float4 ID			: TEXCOORD1;
	float2 TexDecale	: TEXCOORD2;
};

// -------------------------------------------------------------
// 1�p�X�ځF���_�V�F�[�_�v���O����
// -------------------------------------------------------------
VS_OUTPUT VS_pass0(
      float4 Pos    : POSITION,          // ���f���̒��_
      float3 Normal : NORMAL	         // ���f���̖@��
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // �o�̓f�[�^
    
    // �ʒu���W
    Out.Pos =  mul( Pos, mWLP );
    
    // ID��F�Ƃ��ďo�͂���
    Out.Diffuse = vId;

    return Out;
}
// -------------------------------------------------------------
// 1�p�X�ځF�s�N�Z���V�F�[�_�v���O����
// -------------------------------------------------------------
//PIXELSHADER PS_pass0 = asm
//{
//    ps.1.1
    
//    mov r0, v0	// �F��ID�Ƃ��ďo�͂���
//};
float4 PS_pass0(VS_OUTPUT In) : COLOR
{
    return In.Diffuse; // ID�l��F�Ƃ��ďo��
}


// -------------------------------------------------------------
// ���_�V�F�[�_�v���O����
// -------------------------------------------------------------
VS_OUTPUT VS(
      float4 Pos    : POSITION,          // ���f���̒��_
      float4 Normal : NORMAL,	         // ���f���̖@��
      float2 Tex    : TEXCOORD0	         // �e�N�X�`�����W
){
    VS_OUTPUT Out = (VS_OUTPUT)0;        // �o�̓f�[�^
	float4	uv;
	
	// ���W�ϊ�
    Out.Pos = mul(Pos, mWVP);
	// �F
	Out.Diffuse = vCol * max( dot(vLightDir, Normal), 0);// �g�U�F
	Out.Ambient = vCol * 0.3f;							 // ���F
	
	// �e�N�X�`�����W
	uv = mul(Pos, mWVPT);
	Out.ShadowMapUV = uv;

	// ID �l
	Out.ID = vId;
	
	// �f�J�[���e�N�X�`��
	Out.TexDecale = Tex;
		
    return Out;
}



// -------------------------------------------------------------
// 2�p�X�ځF�s�N�Z���V�F�[�_�v���O����(�e�N�X�`������)
// -------------------------------------------------------------
float4 PS_pass1(VS_OUTPUT In) : COLOR
{   
    float4 Color = In.Ambient;
    float4 zero = { 0, 0, 0, 0 };
    float ofset = 1.0f / 256.0f;
    
    float id_map = tex2Dproj(IdMapSamp, In.ShadowMapUV);
    float4 decale = tex2D(DecaleMapSamp, In.TexDecale);
    
    Color += (id_map.x < In.ID.x + ofset && In.ID.x - ofset < id_map.x)
				 ? In.Diffuse : zero;

    return decale * Color;
}  
// -------------------------------------------------------------
// 2�p�X�ځF�s�N�Z���V�F�[�_�v���O����(�e�N�X�`���Ȃ�)
// -------------------------------------------------------------
float4 PS_pass2(VS_OUTPUT In) : COLOR
{   
    float4 Color = In.Ambient;
    float4 zero = { 0, 0, 0, 0 };
    float ofset = 0.01f;
    
    float id_map = tex2Dproj(IdMapSamp, In.ShadowMapUV);
    float4 decale = tex2D(DecaleMapSamp, In.TexDecale);
    
    Color += (id_map.x < In.ID.x + ofset && In.ID.x - ofset < id_map.x)
				 ? In.Diffuse : zero;

    return Color;
}  
// -------------------------------------------------------------
// �e�N�j�b�N
// -------------------------------------------------------------
technique TShader
{
    pass P0
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS_pass0();
        PixelShader = compile ps_2_0 PS_pass0(); // �� HLSL �łɐ؂�ւ�
    }
    pass P1
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 PS_pass1();
    }
    pass P2
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 PS_pass2();
    }
}
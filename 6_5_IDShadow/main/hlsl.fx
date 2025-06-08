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

float4x4 mWVP_ufo; // ���[�J������ˉe��Ԃւ̍��W�ϊ�
float4x4 mWLP_ufo; // ���[�J������ˉe��Ԃւ̍��W�ϊ�
float4x4 mWVPT_ufo; // �e�N�X�`�����W�n�ւ̎ˉe
float4 vCol_ufo; // ���b�V���̐F
float4 vId_ufo; // �v���C�I���e�B�ԍ�
float4 vLightDir_ufo; // ���C�g�̕���

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
// 1�p�X�ځF���_�V�F�[�_�v���O���� �V���h�[�}�b�v UFO
// -------------------------------------------------------------
VS_OUTPUT VS_01(
      float4 Pos : POSITION, // ���f���̒��_
      float3 Normal : NORMAL // ���f���̖@��
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // �o�̓f�[�^
    
    // �ʒu���W
    Out.Pos = mul(Pos, mWLP_ufo);
    
    // ID��F�Ƃ��ďo�͂���
    Out.Diffuse = vId;

    return Out;
}

// -------------------------------------------------------------
// 1�p�X�ځF���_�V�F�[�_�v���O���� �V���h�[�}�b�v �n��
// -------------------------------------------------------------
VS_OUTPUT VS_02(
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
float4 PS_pass0(VS_OUTPUT In) : COLOR
{
    return In.Diffuse; // ID�l��F�Ƃ��ďo��
}



// �n��


// -------------------------------------------------------------
// ���_�V�F�[�_�v���O����
// -------------------------------------------------------------
VS_OUTPUT VS_1(
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
// 1�p�X�ځF�s�N�Z���V�F�[�_�v���O����(�e�N�X�`������) �n��
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


// UFO

// -------------------------------------------------------------
// ���_�V�F�[�_�v���O����
// -------------------------------------------------------------
VS_OUTPUT VS_2(
      float4 Pos : POSITION, // ���f���̒��_
      float4 Normal : NORMAL, // ���f���̖@��
      float2 Tex : TEXCOORD0 // �e�N�X�`�����W
)
{
    VS_OUTPUT Out = (VS_OUTPUT) 0; // �o�̓f�[�^
    float4 uv;
	
	// ���W�ϊ�
    Out.Pos = mul(Pos, mWVP_ufo);
	// �F
    Out.Diffuse = vCol_ufo * max(dot(vLightDir_ufo, Normal), 0); // �g�U�F
    Out.Ambient = vCol_ufo * 0.3f; // ���F
	
	// �e�N�X�`�����W
    uv = mul(Pos, mWVPT_ufo);
    Out.ShadowMapUV = uv;

	// ID �l
    Out.ID = vId_ufo;
	
	// �f�J�[���e�N�X�`��
    Out.TexDecale = Tex;
		
    return Out;
}

// -------------------------------------------------------------
// 2�p�X�ځF�s�N�Z���V�F�[�_�v���O����(�e�N�X�`���Ȃ�) UFO
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
    pass P0 // ufo �V���h�[�}�b�v
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS_01();
        PixelShader = compile ps_2_0 PS_pass0(); // �� HLSL �łɐ؂�ւ�
    }
    pass P1 // �n�� �V���h�[�}�b�v
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS_02();
        PixelShader = compile ps_2_0 PS_pass0(); // �� HLSL �łɐ؂�ւ�
    }
    pass P2 // �n��
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS_1();
        PixelShader = compile ps_2_0 PS_pass1();
    }
    pass P3 // UFO
    {
        // �V�F�[�_
        VertexShader = compile vs_1_1 VS_2();
        PixelShader = compile ps_2_0 PS_pass2();
    }
}
//-------------------------------------------------------------
// File: main.h
//
// Desc: �v���C�I���e�B�o�b�t�@�V���h�E
//-------------------------------------------------------------
#pragma once




//-------------------------------------------------------------
// ��`��萔
//-------------------------------------------------------------
// ���݂̓��̓f�[�^��ۑ�����\����
struct UserInput
{
	BOOL bRotateUp;
	BOOL bRotateDown;
	BOOL bRotateLeft;
	BOOL bRotateRight;
	BOOL bZoomIn;
	BOOL bZoomOut;

	BOOL arrowW;
	BOOL arrowS;
	BOOL arrowD;
	BOOL arrowA;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	CD3DMesh* m_pMesh;		// UFO
	CD3DMesh* m_pMeshBg;		// �n��
	D3DXVECTOR3				m_pos;			// UFO �̈ʒu

	// �V�F�[�_
	LPD3DXEFFECT		    m_pEffect;		// �G�t�F�N�g
	D3DXHANDLE				m_hTechnique;	// �e�N�j�b�N
	D3DXHANDLE       m_hmWVP;	// ���[���h�~�r���[�~�ˉe�s��
	D3DXHANDLE       m_hmWLP;	// ���C�g��������̕ϊ��s��
	D3DXHANDLE       m_hmWVPT;	// ���C�g��������̕ϊ��s��
	D3DXHANDLE       m_hvCol;	// ���b�V���̐F
	D3DXHANDLE       m_hvId;	// �v���C�I���e�B�ԍ�
	D3DXHANDLE       m_hvDir;	// ���C�g�̕���
	D3DXHANDLE       m_htIdMap;// �e�N�X�`��

	D3DXHANDLE       m_hmWVP_ufo;	// ���[���h�~�r���[�~�ˉe�s��
	D3DXHANDLE       m_hmWLP_ufo;	// ���C�g��������̕ϊ��s��
	D3DXHANDLE       m_hmWVPT_ufo;	// ���C�g��������̕ϊ��s��
	D3DXHANDLE       m_hvCol_ufo;	// ���b�V���̐F
	D3DXHANDLE       m_hvId_ufo;	// �v���C�I���e�B�ԍ�
	D3DXHANDLE       m_hvDir_ufo;	// ���C�g�̕���
	D3DXHANDLE       m_htIdMap_ufo;// �e�N�X�`��

	LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// ���_�錾

	// �V���h�E�}�b�v
	LPDIRECT3DTEXTURE9		m_pShadowMap;		// �e�N�X�`��
	LPDIRECT3DSURFACE9		m_pShadowMapSurf;	// �T�[�t�F�X
	LPDIRECT3DSURFACE9		m_pShadowMapZ;		// �[�x�o�b�t�@

	// �ʏ�̍��W�ϊ��s��
	D3DXMATRIX				m_mWorld;
	D3DXMATRIX				m_mView;
	D3DXMATRIX				m_mProj;
	D3DXMATRIX				m_mLightVP;

	D3DXVECTOR3				m_LighPos;		// �����̕���

	BOOL					m_bLoadingApp;	// ���[�h���H
	CD3DFont* m_pFont;		// �t�H���g
	UserInput				m_UserInput;	// ���̓f�[�^

	FLOAT                   m_fWorldRotX;   // �w����]
	FLOAT                   m_fWorldRotY;   // �x����]
	FLOAT                   m_fViewZoom;    // ���_�̋���

	VOID DrawModel(int pass);
	void DrawUfo(int pass, const D3DXMATRIX& mScaleBias);
	void DrawGround(int pass, const D3DXMATRIX& mScaleBias);

protected:
	virtual HRESULT OneTimeSceneInit();
	virtual HRESULT InitDeviceObjects();
	virtual HRESULT RestoreDeviceObjects();
	virtual HRESULT InvalidateDeviceObjects();
	virtual HRESULT DeleteDeviceObjects();
	virtual HRESULT Render();
	virtual HRESULT FrameMove();
	virtual HRESULT FinalCleanup();
	virtual HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT);

	HRESULT RenderText();

	void    UpdateInput(UserInput* pUserInput);
public:
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CMyD3DApplication();
	virtual ~CMyD3DApplication();
};


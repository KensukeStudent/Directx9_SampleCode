//-------------------------------------------------------------
// File: main.h
//
// Desc: ���e�e�N�X�`��
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
    BOOL bZ;
    BOOL bX;

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
    CD3DMesh* m_pMeshBg;		// �w�i���f��
    D3DXVECTOR3					m_positions[3];			// UFO �̈ʒu
    D3DXVECTOR4					m_light;		// ���C�g�̈ʒu

    LPD3DXEFFECT				m_pEffect;		// �V�F�[�_
    D3DXHANDLE					m_hTechnique;	// �e�N�j�b�N
    D3DXHANDLE					m_hmWVP;		// ���[���h�`�ˉe�s��
    D3DXHANDLE					m_hmWVPT;		// ���[���h�`�e�N�X�`���s��
    D3DXHANDLE					m_hvLightPos;	// ���C�g�̕���
    D3DXHANDLE					m_hDecaleMap;	// �f�J�[���}�b�v
    D3DXHANDLE					m_hShadowMap;	// �e�e�N�X�`��
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// ���_�錾

    // �e�̃e�N�X�`��
    LPDIRECT3DTEXTURE9			m_pShadowTex;
    LPDIRECT3DSURFACE9			m_pShadowSurf;
    LPDIRECT3DSURFACE9			m_pShadowTexZ;

    CD3DMesh* m_pMeshs[3];		// ��s���f��

    D3DXMATRIX					m_mWorld;		// ���[���h�s��
    D3DXMATRIX					m_mView;		// �r���[�s��
    D3DXMATRIX					m_mProj;		// �ˉe�s��

    FLOAT						m_zoom;			// �Y�[��
    FLOAT						m_fWorldRotX;	// �w����]
    FLOAT						m_fWorldRotY;	// �x����]

    BOOL						m_bLoadingApp;	// ���[�h���H
    CD3DFont* m_pFont;		// �t�H���g
    UserInput					m_UserInput;	// ���̓f�[�^

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
    LRESULT MsgProc(HWND hWnd, UINT msg
        , WPARAM wParam, LPARAM lParam);
    CMyD3DApplication();
    virtual ~CMyD3DApplication();
};

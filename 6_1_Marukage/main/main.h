//-------------------------------------------------------------
// File: main.h
//
// Desc: �ۉe
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
};


//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    D3DXVECTOR3					m_pos;			// UFO �̈ʒu
    FLOAT						m_shadowH;		// �e�̍���

    LPDIRECT3DTEXTURE9			m_pTex;			// �e�̃e�N�X�`��
    CD3DMesh* m_pMeshBG;		// �w�i���f��
    CD3DMesh* m_pMesh;		// ��s���f��

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

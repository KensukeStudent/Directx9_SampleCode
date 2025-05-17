//-------------------------------------------------------------
// File: main.h
//
// Desc: ���e�V���h�E
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
    BOOL bA; // ��
    BOOL bD; // ��
};


//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: �A�v���P�[�V�����̃N���X
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    D3DXVECTOR3					m_pos;			// UFO �̈ʒu
    D3DXVECTOR4					m_light;		// ���C�g�̈ʒu
    CD3DMesh* m_pMesh;		// ��s���f��

    LPDIRECT3DTEXTURE9			m_pTex;			// �n��

    D3DXMATRIX					m_mWorld;		// ���[���h�s��
    D3DXMATRIX					m_mView;		// �r���[�s��
    D3DXMATRIX					m_mProj;		// �ˉe�s��

    FLOAT						m_zoom;			// �Y�[��
    FLOAT						m_fWorldRotX;	// �w����]
    FLOAT						m_fWorldRotY;	// �x����]

	FLOAT                       m_ufoRot;	    // UFO �̉�]

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

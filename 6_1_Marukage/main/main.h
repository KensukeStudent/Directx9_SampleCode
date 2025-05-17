//-------------------------------------------------------------
// File: main.h
//
// Desc: 丸影
//-------------------------------------------------------------
#pragma once




//-------------------------------------------------------------
// 定義や定数
//-------------------------------------------------------------
// 現在の入力データを保存する構造体
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
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    D3DXVECTOR3					m_pos;			// UFO の位置
    FLOAT						m_shadowH;		// 影の高さ

    LPDIRECT3DTEXTURE9			m_pTex;			// 影のテクスチャ
    CD3DMesh* m_pMeshBG;		// 背景モデル
    CD3DMesh* m_pMesh;		// 飛行モデル

    D3DXMATRIX					m_mWorld;		// ワールド行列
    D3DXMATRIX					m_mView;		// ビュー行列
    D3DXMATRIX					m_mProj;		// 射影行列

    FLOAT						m_zoom;			// ズーム
    FLOAT						m_fWorldRotX;	// Ｘ軸回転
    FLOAT						m_fWorldRotY;	// Ｙ軸回転

    BOOL						m_bLoadingApp;	// ロード中？
    CD3DFont* m_pFont;		// フォント
    UserInput					m_UserInput;	// 入力データ

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

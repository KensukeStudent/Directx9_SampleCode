//-------------------------------------------------------------
// File: main.h
//
// Desc: サンプルのヘッダー
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
    BOOL bA;
    BOOL bS;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DMesh* m_pMesh;	// モデル
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言
    LPD3DXEFFECT					m_pEffect;	// シェーダ
    D3DXHANDLE						m_hmWVP;	// ワールド～射影行列
    D3DXHANDLE						m_hvCol;	// メッシュの色
    D3DXHANDLE						m_hvDir;	// ライトの方向
    D3DXHANDLE						m_hvFog;	// フォグのnear/far

    D3DXMATRIX						m_mWorld;	// ワールド行列
    D3DXMATRIX						m_mView;	// ビュー行列
    D3DXMATRIX						m_mProj;	// 射影行列
    D3DXVECTOR4						m_LightDir;	// 光源の方向

    FLOAT						m_near;			// フォグのかかり始め
    FLOAT						m_far;			// フォグか完全にかかる
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

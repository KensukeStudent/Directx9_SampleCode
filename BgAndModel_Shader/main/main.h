//-------------------------------------------------------------
// File: main.h
//
// Desc: HLSLサンプルのヘッダー
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

    // UFO移動
    BOOL bW;
    BOOL bS;
    BOOL bD;
    BOOL bA;

    // ビュー行列
    BOOL bviewL; // 上
    BOOL bviewJ; // 右
    BOOL bviewI; // 左
    BOOL bviewK; // 下
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    LPD3DXEFFECT					m_pEffect;	// ★シェーダ
    D3DXHANDLE						m_hTechnique;// ★テクニック
    D3DXHANDLE						m_hmWVP;	// ★ワールド～射影行列

    D3DXHANDLE						m_hmWVP_Ufo;	// ★ワールド～射影行列
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言

    CD3DMesh* m_pMesh;	// モデル
    CD3DMesh* m_pMeshUfo;	// モデル

    D3DXMATRIX						m_mWorld;	// ワールド行列
    D3DXMATRIX						m_mView;	// ビュー行列
    D3DXMATRIX						m_mProj;	// 射影行列

    FLOAT						m_fWorldRotX;	// Ｘ軸回転
    FLOAT						m_fWorldRotY;	// Ｙ軸回転

    FLOAT						m_viewX;	// 視点X
    FLOAT						m_viewY;	// 視点Y

    FLOAT						m_ufoX;	// UFO X
    FLOAT						m_ufoZ;	// UFO Y

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

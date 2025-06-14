//-------------------------------------------------------------
// File: main.h
//
// Desc: 投影テクスチャ
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

    BOOL arrowW;
    BOOL arrowS;
    BOOL arrowD;
    BOOL arrowA;
};


//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    CD3DMesh* m_pMeshBg;		// 背景モデル
    D3DXVECTOR3					m_positions[3];			// UFO の位置
    D3DXVECTOR4					m_light;		// ライトの位置

    LPD3DXEFFECT				m_pEffect;		// シェーダ
    D3DXHANDLE					m_hTechnique;	// テクニック
    D3DXHANDLE					m_hmWVP;		// ワールド～射影行列
    D3DXHANDLE					m_hmWVPT;		// ワールド～テクスチャ行列
    D3DXHANDLE					m_hvLightPos;	// ライトの方向
    D3DXHANDLE					m_hDecaleMap;	// デカールマップ
    D3DXHANDLE					m_hShadowMap;	// 影テクスチャ
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言

    // 影のテクスチャ
    LPDIRECT3DTEXTURE9			m_pShadowTex;
    LPDIRECT3DSURFACE9			m_pShadowSurf;
    LPDIRECT3DSURFACE9			m_pShadowTexZ;

    CD3DMesh* m_pMeshs[3];		// 飛行モデル

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

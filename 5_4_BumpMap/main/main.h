//-------------------------------------------------------------
// File: main.h
//
// Desc:バンプマップ
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
    BOOL bZoomIn;
    BOOL bZoomOut;
};




//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    LPDIRECT3DTEXTURE9		m_pNormalMap;	// 法線マップ
    LPDIRECT3DVERTEXDECLARATION9	m_pDecl;// 頂点宣言
    CD3DMesh* m_pMesh;		// ティーポットのメッシュ
    CD3DMesh* m_pMeshBg;		// 背景のメッシュ
    LPDIRECT3DTEXTURE9		m_pTexture;	// ★追加:テクスチャ

    // シェーダ
    LPD3DXEFFECT		    m_pEffect;		// エフェクト
    D3DXHANDLE				m_hTechnique;	// テクニック
    D3DXHANDLE				m_hmWVP;		// ローカル-射影変換行列
    D3DXHANDLE				m_hvLightDir;	// ライトの方向
    D3DXHANDLE				m_hvColor;		// 頂点色
    D3DXHANDLE				m_hvEyePos;		// 視点の位置
    D3DXHANDLE				m_htDecaleTex;	// 模様のテクスチャ
    D3DXHANDLE				m_htNormalMap;	// 法線マップ


    // 通常の座標変換行列
    D3DXVECTOR4				m_vFromPt;
    D3DXMATRIX				m_mView;
    D3DXMATRIX				m_mProj;

    BOOL					m_bLoadingApp;	// ロード中？
    CD3DFont* m_pFont;		// フォント
    UserInput				m_UserInput;	// 入力データ

    FLOAT                   m_fWorldRotX;   // Ｘ軸回転
    FLOAT                   m_fWorldRotY;   // Ｙ軸回転
    FLOAT                   m_fViewZoom;    // 視点の距離

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


//-------------------------------------------------------------
// File: main.h
//
// Desc: フォーカスフィルタ
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
    // シェーダ
    LPD3DXEFFECT		    m_pEffect;		// エフェクト
    D3DXHANDLE				m_hTechnique;	// テクニック
    D3DXHANDLE				m_hafWeight;	// 重みの配列
    D3DXHANDLE				m_htSrcTex;		// テクスチャ
    D3DXHANDLE				m_htBlurTex;	// テクスチャ
    D3DXHANDLE				m_htBlendTex;	// テクスチャ

    // レンダリングターゲット
    LPDIRECT3DSURFACE9		m_pMapZ;		// 深度バッファ
    LPDIRECT3DTEXTURE9		m_pOriginalTex;	// テクスチャ
    LPDIRECT3DSURFACE9		m_pOriginalSurf;// サーフェス
    LPDIRECT3DTEXTURE9		m_pPostTex[2];	// テクスチャ
    LPDIRECT3DSURFACE9		m_pPostSurf[2];	// サーフェス

    // メッシュ
    CD3DMesh* m_pMesh;
    CD3DMesh* m_pMeshBg;
    LPDIRECT3DTEXTURE9		m_pTex;

    // 通常の座標変換行列
    D3DXMATRIX				m_mWorld;
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


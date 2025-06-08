//-------------------------------------------------------------
// File: main.h
//
// Desc: プライオリティバッファシャドウ
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
	CD3DMesh* m_pMesh;		// UFO
	CD3DMesh* m_pMeshBg;		// 地面
	D3DXVECTOR3				m_pos;			// UFO の位置

	// シェーダ
	LPD3DXEFFECT		    m_pEffect;		// エフェクト
	D3DXHANDLE				m_hTechnique;	// テクニック
	D3DXHANDLE       m_hmWVP;	// ワールド×ビュー×射影行列
	D3DXHANDLE       m_hmWLP;	// ライト方向からの変換行列
	D3DXHANDLE       m_hmWVPT;	// ライト方向からの変換行列
	D3DXHANDLE       m_hvCol;	// メッシュの色
	D3DXHANDLE       m_hvId;	// プライオリティ番号
	D3DXHANDLE       m_hvDir;	// ライトの方向
	D3DXHANDLE       m_htIdMap;// テクスチャ

	D3DXHANDLE       m_hmWVP_ufo;	// ワールド×ビュー×射影行列
	D3DXHANDLE       m_hmWLP_ufo;	// ライト方向からの変換行列
	D3DXHANDLE       m_hmWVPT_ufo;	// ライト方向からの変換行列
	D3DXHANDLE       m_hvCol_ufo;	// メッシュの色
	D3DXHANDLE       m_hvId_ufo;	// プライオリティ番号
	D3DXHANDLE       m_hvDir_ufo;	// ライトの方向
	D3DXHANDLE       m_htIdMap_ufo;// テクスチャ

	LPDIRECT3DVERTEXDECLARATION9	m_pDecl;	// 頂点宣言

	// シャドウマップ
	LPDIRECT3DTEXTURE9		m_pShadowMap;		// テクスチャ
	LPDIRECT3DSURFACE9		m_pShadowMapSurf;	// サーフェス
	LPDIRECT3DSURFACE9		m_pShadowMapZ;		// 深度バッファ

	// 通常の座標変換行列
	D3DXMATRIX				m_mWorld;
	D3DXMATRIX				m_mView;
	D3DXMATRIX				m_mProj;
	D3DXMATRIX				m_mLightVP;

	D3DXVECTOR3				m_LighPos;		// 光源の方向

	BOOL					m_bLoadingApp;	// ロード中？
	CD3DFont* m_pFont;		// フォント
	UserInput				m_UserInput;	// 入力データ

	FLOAT                   m_fWorldRotX;   // Ｘ軸回転
	FLOAT                   m_fWorldRotY;   // Ｙ軸回転
	FLOAT                   m_fViewZoom;    // 視点の距離

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


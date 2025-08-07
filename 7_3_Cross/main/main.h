//-------------------------------------------------------------
// File: main.h
//
// Desc:クロスフィルタ
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
// テクスチャ座標用の矩形
struct CoordRect
{
	float u0, v0;
	float u1, v1;
};



//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
	enum {
		NUM_STAR_TEXTURES = 8, // 光芒のテクスチャの枚数
		MAX_SAMPLES = 16, // サンプラの最大数
	};

	DWORD m_dwCropWidth;
	DWORD m_dwCropHeight;

	PDIRECT3DTEXTURE9		m_pTexScene;	    // HDR の画面の描画先
	PDIRECT3DSURFACE9		m_pSurfScene;
	PDIRECT3DTEXTURE9		m_pTexSceneScaled;	// 縮小バッファ
	PDIRECT3DSURFACE9		m_pSurfSceneScaled;
	PDIRECT3DTEXTURE9		m_pTexBrightPass;	// 輝度の抽出
	PDIRECT3DSURFACE9		m_pSurfBrightPass;
	PDIRECT3DTEXTURE9		m_pTexStarSource;   // 星の元になる画像
	PDIRECT3DSURFACE9		m_pSurfStarSource;
	PDIRECT3DTEXTURE9		m_apTexStar[NUM_STAR_TEXTURES];// 光芒効果の中間レンダリングバッファ
	PDIRECT3DSURFACE9		m_apSurfStar[NUM_STAR_TEXTURES];

	// シーンの描画用
	LPDIRECT3DTEXTURE9		m_pNormalMap;	// 法線マップ
	LPDIRECT3DVERTEXDECLARATION9	m_pDecl;// 頂点宣言
	CD3DMesh* m_pMesh;		// ティーポットのメッシュ
	CD3DMesh* m_pMeshBg;		// 背景のメッシュ

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

	// 追加した関数
	void    RenderScene();				// ティーポットや地面の描画
	HRESULT Scene_To_SceneScaled();		// 縮小バッファへコピー
	HRESULT SceneScaled_To_BrightPass();// 明るい部分を抽出
	HRESULT BrightPass_To_StarSource();	// すこしぼかす
	HRESULT RenderStar();				// 光芒の作成
	// 全画面にポリゴンの描画
	void DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV);
	// ガウス型のぼかしの係数を計算する
	HRESULT GetGaussBlur5x5(DWORD dwD3DTexWidth, DWORD dwD3DTexHeight,
		D3DXVECTOR2* avTexCoordOffset, D3DXVECTOR4* avSampleWeight);
	// 描画元、先の大きさから、テクスチャ座標を補正する
	HRESULT GetTextureCoords(PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc,
		PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords);

public:
	LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	CMyD3DApplication();
	virtual ~CMyD3DApplication();
};


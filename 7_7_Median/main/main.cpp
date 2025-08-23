//-------------------------------------------------------------
// File: main.cpp
//
// Desc: メディアンフィルタ
//-------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <comdef.h>  // _com_error
#include <tchar.h>
#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
#include "D3DApp.h"
#include "D3DFont.h"
#include "D3DFile.h"
#include "D3DUtil.h"
#include "resource.h"
#include "main.h"

// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState


//-------------------------------------------------------------
// 頂点の構造体
//-------------------------------------------------------------
typedef struct {
	FLOAT       p[4];
	FLOAT       tu, tv;
} TVERTEX;

//-------------------------------------------------------------
// グローバル変数
//-------------------------------------------------------------
CMyD3DApplication* g_pApp = NULL;
HINSTANCE          g_hInst = NULL;

// 頂点宣言
D3DVERTEXELEMENT9 decl[] =
{
	{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
	{0, 12, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
	D3DDECL_END()
};


//-------------------------------------------------------------
// Name: WinMain()
// Desc: メイン関数
//-------------------------------------------------------------
INT WINAPI WinMain(HINSTANCE hInst, HINSTANCE, LPSTR, INT)
{
	CMyD3DApplication d3dApp;

	g_pApp = &d3dApp;
	g_hInst = hInst;

	InitCommonControls();
	if (FAILED(d3dApp.Create(hInst)))
		return 0;

	return d3dApp.Run();
}




//-------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: アプリケーションのコンストラクタ
//-------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
	m_pTex = NULL;

	m_pEffect = NULL;
	m_hTechnique = NULL;
	m_htSrcMap = NULL;
	m_pDecl = NULL;

	m_dwCreationWidth = 512;
	m_dwCreationHeight = 512;
	m_strWindowTitle = TEXT("main");
	m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
	m_bStartFullscreen = false;
	m_bShowCursorWhenFullscreen = false;

	m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
	m_bLoadingApp = TRUE;

	ZeroMemory(&m_UserInput, sizeof(m_UserInput));
}




//-------------------------------------------------------------
// Name: ~CMyD3DApplication()
// Desc: デストラクタ
//-------------------------------------------------------------
CMyD3DApplication::~CMyD3DApplication()
{
}




//-------------------------------------------------------------
// Name: OneTimeSceneInit()
// Desc: 一度だけ行う初期化
//		ウィンドウの初期化やIDirect3D9の初期化は終わってます。
//		ただ、LPDIRECT3DDEVICE9 の初期化は終わっていません。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::OneTimeSceneInit()
{
	// ローディングメッセージを表示する
	SendMessage(m_hWnd, WM_PAINT, 0, 0);

	m_bLoadingApp = FALSE;

	return S_OK;
}




//-------------------------------------------------------------
// Name: ConfirmDevice()
// Desc: 初期化の時に呼ばれます。必要な能力をチェックします。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps,
	DWORD dwBehavior, D3DFORMAT Format)
{
	UNREFERENCED_PARAMETER(Format);
	UNREFERENCED_PARAMETER(dwBehavior);
	UNREFERENCED_PARAMETER(pCaps);


	// ピクセルシェーダバージョンチェック
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		return E_FAIL;

	// 頂点シェーダバージョンが上位かソフトウェア頂点処理
	if (pCaps->VertexShaderVersion < D3DVS_VERSION(1, 1)
		&& 0 == (dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING))
		return E_FAIL;

	return S_OK;
}




//-------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: デバイスが生成された後の初期化をします。
//		フレームバッファフォーマットやデバイスの種類が変わった
//		後に通過します。
//		ここで確保したメモリはDeleteDeviceObjects()で開放します
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
	HRESULT hr;
	LPDIRECT3DSURFACE9		pSurf;
	D3DSURFACE_DESC			desc;

	// テクスチャの読み込み
	D3DXCreateTextureFromFile(m_pd3dDevice, _T("fujiyama.bmp"), &m_pTex);
	m_pTex->GetSurfaceLevel(0, &pSurf);
	pSurf->GetDesc(&desc);
	m_TexWidth = desc.Width;
	m_TexHeight = desc.Height;
	SAFE_RELEASE(pSurf);

	// シェーダの読み込み
	LPD3DXBUFFER pErr;
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
		// シェーダの読み込みの失敗
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
	}
	else {
		m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
		m_htSrcMap = m_pEffect->GetParameterByName(NULL, "SrcMap");
	}

	// 頂点宣言のオブジェクトの生成
	if (FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(decl, &m_pDecl)))
	{
		MessageBoxA(NULL, "Vertex Declaration Create ERROR", "Error", MB_OK);
		return hr;
	}

	// フォント
	m_pFont->InitDeviceObjects(m_pd3dDevice);

	return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//		確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	// レンダリング状態の設定
	RS(D3DRS_DITHERENABLE, FALSE);
	RS(D3DRS_SPECULARENABLE, FALSE);
	RS(D3DRS_ZENABLE, TRUE);

	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// フォント
	m_pFont->RestoreDeviceObjects();

	m_pEffect->OnResetDevice();

	return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	// 入力データの更新
	UpdateInput(&m_UserInput);

	return S_OK;
}
//-------------------------------------------------------------
// Name: UpdateInput()
// Desc: 入力データを更新する
//-------------------------------------------------------------
void CMyD3DApplication::UpdateInput(UserInput* pUserInput)
{
	pUserInput->bRotateUp = (m_bActive && (GetAsyncKeyState(VK_UP) & 0x8000) == 0x8000);
	pUserInput->bRotateDown = (m_bActive && (GetAsyncKeyState(VK_DOWN) & 0x8000) == 0x8000);
	pUserInput->bRotateLeft = (m_bActive && (GetAsyncKeyState(VK_LEFT) & 0x8000) == 0x8000);
	pUserInput->bRotateRight = (m_bActive && (GetAsyncKeyState(VK_RIGHT) & 0x8000) == 0x8000);
}


//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	D3DXMATRIX m, mT, mR, mView, mProj;

	//---------------------------------------------------------
	// 描画
	//---------------------------------------------------------
	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		// クリア
		m_pd3dDevice->Clear(0L, NULL
			, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
			, 0xffffffff, 1.0f, 0L);
		if (m_pEffect != NULL)
		{
			//-------------------------------------------------
			// シェーダの設定
			//-------------------------------------------------
			m_pEffect->SetTechnique(m_hTechnique);
			m_pEffect->Begin(NULL, 0);
			m_pEffect->BeginPass(0);

			//-------------------------------------------------
			// フィルタリング
			//-------------------------------------------------

			RS(D3DRS_ZENABLE, FALSE);
			RS(D3DRS_LIGHTING, FALSE);
			TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			TSS(1, D3DTSS_COLOROP, D3DTOP_DISABLE);

			TVERTEX Vertex1[4] = {
				// x     y     z     tu tv
				{-1.0f, 1.0f, 0.1f,   0, 0,},
				{ 1.0f, 1.0f, 0.1f,   1, 0,},
				{ 1.0f,-1.0f, 0.1f,   1, 1,},
				{-1.0f,-1.0f, 0.1f,   0, 1,},
			};
			m_pd3dDevice->SetVertexDeclaration(m_pDecl);
			m_pEffect->SetTexture(m_htSrcMap, m_pTex);
			m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
				, 2, Vertex1, sizeof(TVERTEX));

			m_pEffect->EndPass();
			m_pEffect->End();
		}

		RS(D3DRS_ZENABLE, TRUE);
		RS(D3DRS_LIGHTING, TRUE);

		// ヘルプの表示
		RenderText();

#if 1 // デバッグ用にテクスチャを表示する
		{
			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
			m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
			m_pd3dDevice->SetVertexShader(NULL);
			m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
			m_pd3dDevice->SetPixelShader(0);
			float scale = 128.0f;
			for (DWORD i = 0; i < 1; i++) {
				TVERTEX Vertex[4] = {
					// x  y  z rhw tu tv
					{    0,(i + 0) * scale,0, 1, 0, 0,},
					{scale,(i + 0) * scale,0, 1, 1, 0,},
					{scale,(i + 1) * scale,0, 1, 1, 1,},
					{    0,(i + 1) * scale,0, 1, 0, 1,},
				};
				if (0 == i) m_pd3dDevice->SetTexture(0, m_pTex);
				m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
			}
		}
#endif		

		// 描画の終了
		m_pd3dDevice->EndScene();
	}

	return S_OK;
}




//-------------------------------------------------------------
// Name: RenderText()
// Desc: 状態やヘルプを画面に表示する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RenderText()
{
	D3DCOLOR fontColor = D3DCOLOR_ARGB(255, 255, 255, 0);
	TCHAR szMsg[MAX_PATH] = TEXT("");

	FLOAT fNextLine = 40.0f; // 表示する高さ

	// 操作法やパラメータを表示する
	fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
	lstrcpy(szMsg, TEXT("Press 'F2' to configure display"));
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	lstrcpy(szMsg, m_strDeviceStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, m_strFrameStats);
	fNextLine -= 20.0f;
	m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd, UINT msg,
	WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
	case WM_PAINT:
	{
		if (m_bLoadingApp)
		{
			// ロード中
			HDC hDC = GetDC(hWnd);
			TCHAR strMsg[MAX_PATH];
			wsprintf(strMsg, TEXT("Loading... Please wait"));
			RECT rct;
			GetClientRect(hWnd, &rct);
			DrawText(hDC, strMsg, -1, &rct
				, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
			ReleaseDC(hWnd, hDC);
		}
		break;
	}

	}

	return CD3DApplication::MsgProc(hWnd, msg, wParam, lParam);
}




//-------------------------------------------------------------
// Name: InvalidateDeviceObjects()
// Desc: RestoreDeviceObjects() で作成したオブジェクトの開放
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InvalidateDeviceObjects()
{
	// シェーダ
	if (m_pEffect != NULL) m_pEffect->OnLostDevice();

	m_pFont->InvalidateDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	// シェーダ
	SAFE_RELEASE(m_pTex);

	// シェーダ
	SAFE_RELEASE(m_pEffect);
	SAFE_RELEASE(m_pDecl);

	// フォント
	m_pFont->DeleteDeviceObjects();

	return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: 終了する直前に呼ばれる
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
	SAFE_DELETE(m_pFont);	// フォント

	return S_OK;
}





//-------------------------------------------------------------
// File: main.cpp
//
// Desc: エリア総和テーブル
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

#define MAP_WIDTH	512
#define MAP_HEIGHT	512

// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState

typedef struct {
	FLOAT p[3];
	FLOAT tu, tv;
} VERTEX;

//-------------------------------------------------------------
// グローバル変数
//-------------------------------------------------------------
CMyD3DApplication* g_pApp = NULL;
HINSTANCE          g_hInst = NULL;


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
// Name: UpdateParam()
// Desc: ぼかす量の調整
//-------------------------------------------------------------
void CMyD3DApplication::UpdateParam()
{
	float size = 1.0f + (float)(int)m_size;// 少数を落とす
	m_pEffect->SetFloat("SIZE", size);
}

//-------------------------------------------------------------
// Name: CMyD3DApplication()
// Desc: アプリケーションのコンストラクタ
//-------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
	m_pMesh = new CD3DMesh();
	m_pMeshBg = new CD3DMesh();
	m_size = 40.0f;

	m_pMapZ = NULL;
	m_pSatTex = NULL;
	m_pSatSurf = NULL;

	m_pEffect = NULL;
	m_hTechnique = NULL;
	m_htSrcMap = NULL;

	m_fWorldRotX = -0.41271535f;
	m_fWorldRotY = 0.0f;
	m_fViewZoom = 5.0f;

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
	if (pCaps->PixelShaderVersion < D3DPS_VERSION(1, 1))
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
	D3DXVECTOR4 offset;

	// シェーダの読み込み
	LPD3DXBUFFER pErr;
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
		// シェーダの読み込みの失敗
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
	}
	else
	{
		m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
		m_htSrcMap = m_pEffect->GetParameterByName(NULL, "SrcMap");
		m_pEffect->SetFloat("MAP_WIDTH", MAP_WIDTH);	// 幅の設定
		m_pEffect->SetFloat("MAP_HEIGHT", MAP_HEIGHT);	// 高さの設定
	}

	this->UpdateParam();

	// UFOの読み込み
	if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("ufo.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("ufo.x load Error"), MB_OK);
		return hr;
	}
	
	// 地形の読み込み
	if (FAILED(hr = m_pMeshBg->Create(m_pd3dDevice, _T("map.x"))))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("map.x load Error"), MB_OK);
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
	// レンダリングターゲットの生成
	if (FAILED(m_pd3dDevice->CreateDepthStencilSurface(
		MAP_WIDTH, MAP_HEIGHT, D3DFMT_D16,
		D3DMULTISAMPLE_NONE, 0, TRUE, &m_pMapZ, NULL)))
		return E_FAIL;
	// エリア総和テーブル
	if (FAILED(m_pd3dDevice->CreateTexture(MAP_WIDTH, MAP_HEIGHT, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_A32B32G32R32F,
		D3DPOOL_DEFAULT, &m_pSatTex, NULL)))
		return E_FAIL;
	if (FAILED(m_pSatTex->GetSurfaceLevel(0, &m_pSatSurf)))
		return E_FAIL;

	// エフェクト
	m_pEffect->OnResetDevice();

	// メッシュ
	m_pMesh->RestoreDeviceObjects(m_pd3dDevice);
	m_pMeshBg->RestoreDeviceObjects(m_pd3dDevice);

	// 質感の設定
	D3DMATERIAL9 mtrl;
	D3DUtil_InitMaterial(mtrl, 1.0f, 0.0f, 0.0f);
	m_pd3dDevice->SetMaterial(&mtrl);


	// レンダリング状態の設定
	RS(D3DRS_DITHERENABLE, FALSE);
	RS(D3DRS_SPECULARENABLE, FALSE);
	RS(D3DRS_ZENABLE, TRUE);
	RS(D3DRS_AMBIENT, 0x000F0F0F);

	TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
	TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
	TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
	TSS(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
	TSS(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);
	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);

	// ワールド行列
	D3DXMATRIX matIdentity;
	D3DXMatrixIdentity(&m_mWorld);

	// ビュー行列
	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -5.0f);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);

	// 射影行列
	FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width) / m_d3dsdBackBuffer.Height;
	D3DXMatrixPerspectiveFovLH(&m_mProj, D3DX_PI / 4, fAspect, 1.0f, 100.0f);

	// フォント
	m_pFont->RestoreDeviceObjects();

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

	//---------------------------------------------------------
	// 入力に応じて座標系を更新する
	//---------------------------------------------------------
	// 回転
	D3DXMATRIX matRotY;
	D3DXMATRIX matRotX;

	if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
		m_fWorldRotY += m_fElapsedTime;
	else
		if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
			m_fWorldRotY -= m_fElapsedTime;

	if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
		m_fWorldRotX += m_fElapsedTime;
	else
		if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
			m_fWorldRotX -= m_fElapsedTime;

	D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
	D3DXMatrixRotationY(&matRotY, m_fWorldRotY);

	D3DXMatrixMultiply(&m_mWorld, &matRotY, &matRotX);

	//---------------------------------------------------------
	// ビュー行列の設定
	//---------------------------------------------------------
	// ズーム
	if (m_UserInput.bZoomIn && !m_UserInput.bZoomOut)
		m_fViewZoom += m_fElapsedTime;
	else if (m_UserInput.bZoomOut && !m_UserInput.bZoomIn)
		m_fViewZoom -= m_fElapsedTime;

	D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -m_fViewZoom);
	D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFromPt, &vLookatPt, &vUpVec);

	//---------------------------------------------------------
	// ぼかすパラメータの設定
	//---------------------------------------------------------
	if (m_UserInput.bSizeUp && !m_UserInput.bSizeDown)
		m_size += 10.0f * m_fElapsedTime;
	else if (m_UserInput.bSizeDown && !m_UserInput.bSizeUp)
		m_size -= 10.0f * m_fElapsedTime;
	this->UpdateParam();

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

	pUserInput->bZoomIn = (m_bActive && (GetAsyncKeyState('Z') & 0x8000) == 0x8000);
	pUserInput->bZoomOut = (m_bActive && (GetAsyncKeyState('X') & 0x8000) == 0x8000);
	pUserInput->bSizeUp = (m_bActive && (GetAsyncKeyState('Q') & 0x8000) == 0x8000);
	pUserInput->bSizeDown = (m_bActive && (GetAsyncKeyState('W') & 0x8000) == 0x8000);
}


//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	D3DXMATRIX m, mT, mR, mView, mProj;
	LPDIRECT3DSURFACE9 pOldBackBuffer, pOldZBuffer;
	D3DVIEWPORT9 oldViewport;
	DWORD i;

	//-----------------------------------------------------
	// レンダリングターゲットの保存
	//-----------------------------------------------------
	m_pd3dDevice->GetRenderTarget(0, &pOldBackBuffer);
	m_pd3dDevice->GetDepthStencilSurface(&pOldZBuffer);
	m_pd3dDevice->GetViewport(&oldViewport);

	//---------------------------------------------------------
	// 描画
	//---------------------------------------------------------
	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
	{
		//-----------------------------------------------------
		// レンダリングターゲットの変更
		//-----------------------------------------------------
		{
			//m_pd3dDevice->SetRenderTarget(0, m_pSatSurf);
			//m_pd3dDevice->SetDepthStencilSurface(m_pMapZ);
			//// ビューポートの変更
			//D3DVIEWPORT9 viewport = { 0,0 // 左上の座標
			//				, MAP_WIDTH  // 幅
			//				, MAP_HEIGHT // 高さ
			//				, 0.0f,1.0f };// 前面、後面
			//m_pd3dDevice->SetViewport(&viewport);
		}

		// レンダリングターゲットのクリア
		m_pd3dDevice->Clear(0L, NULL
			, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
			, 0, 1.0f, 0L);

		//-----------------------------------------------------
		// 1パス目:レンダリングターゲットの作成
		//-----------------------------------------------------
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_mWorld);
		m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_mView);
		m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mProj);

		TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pMeshBg->Render(m_pd3dDevice);

		// 飛行モデルの描画
		TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		TSS(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
		RS(D3DRS_LIGHTING, TRUE);
		RS(D3DRS_AMBIENT, 0);
		m_pd3dDevice->LightEnable(0, TRUE);
		D3DXMatrixTranslation(&m, 1.0f, 0.0f, 0.0f);
		D3DXMatrixRotationY(&mR, m_fTime);
		D3DXMatrixTranslation(&mT, 1.0f, 1.0f, 0.0f);
		m = m * mR * mT * m_mWorld;
		m_pd3dDevice->SetTransform(D3DTS_WORLD, &m);
		m_pMesh->Render(m_pd3dDevice);

		m_pd3dDevice->EndScene();

		//// ----------------------------------------------------
		//// SAT 計算
		//// ----------------------------------------------------
		//RS(D3DRS_ZENABLE, FALSE);
		//RS(D3DRS_LIGHTING, FALSE);

		//TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		//TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		//TSS(1, D3DTSS_COLOROP, D3DTOP_ADD);
		//TSS(1, D3DTSS_COLORARG1, D3DTA_CURRENT);
		//TSS(1, D3DTSS_COLORARG2, D3DTA_TEXTURE);
		//TSS(2, D3DTSS_COLOROP, D3DTOP_DISABLE);

		//m_pEffect->SetTechnique(m_hTechnique);
		//m_pEffect->Begin(NULL, 0);
		//m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
		//m_pEffect->SetTexture(m_htSrcMap, m_pSatTex);

		//// 縦と横の合計を計算
		//// 横の計算では画面解像度分縦に1本の線を引く、それを解像度回数分（今回で言うと512回）
		//// 縦の計算では画面解像度分横に1本の線を引く、それを解像度回数分（今回で言うと512回）

		//// 横方向に合計
		//m_pEffect->BeginPass(0);
		//for (i = 0; i < MAP_WIDTH; i++) {
		//	// -1 + 2.0f * x....としているのは、ndc座標系に変換するため
		//	// -1 ~ +1 の範囲にするため
		//	FLOAT dx = (1.0f / MAP_WIDTH);
		//	VERTEX Vertex[4] = {
		//		//       x                y     z        tu       tv
		//		{{ -1 + 2.0f * dx * (FLOAT)i, +1.0f, 0.1f}, dx * (FLOAT)i, 0,},
		//		{{ -1 + 2.0f * dx * (FLOAT)i, -1.0f, 0.1f}, dx * (FLOAT)i, 1,},
		//	};
		//	m_pd3dDevice->BeginScene();
		//	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vertex, sizeof(VERTEX));
		//	m_pd3dDevice->EndScene();
		//}
		//m_pEffect->EndPass();


		//// 縦方向に合計
		//m_pEffect->BeginPass(1);
		//for (i = 0; i < MAP_HEIGHT; i++) {
		//	FLOAT dy = (1.0f / MAP_HEIGHT);
		//	VERTEX Vertex[4] = {
		//		//   x            y              z     tu    tv
		//		{{ -1.0f,  +1 - 2.0f * dy * (FLOAT)i, 0.1f}, 0, dy * (FLOAT)i },
		//		{{ +1.0f,  +1 - 2.0f * dy * (FLOAT)i, 0.1f}, 1, dy * (FLOAT)i },
		//	};
		//	m_pd3dDevice->BeginScene();
		//	m_pd3dDevice->DrawPrimitiveUP(D3DPT_LINELIST, 1, Vertex, sizeof(VERTEX));
		//	m_pd3dDevice->EndScene();
		//}
		//m_pEffect->EndPass();
		//m_pEffect->End();

		////-----------------------------------------------------
		//// レンダリングターゲットを元に戻す
		////-----------------------------------------------------
		//m_pd3dDevice->SetRenderTarget(0, pOldBackBuffer);
		//m_pd3dDevice->SetDepthStencilSurface(pOldZBuffer);
		//m_pd3dDevice->SetViewport(&oldViewport);
	}

//	if (SUCCEEDED(m_pd3dDevice->BeginScene()))
//	{
//		//// バッファのクリア
//		//m_pd3dDevice->Clear(0L, NULL
//		//	, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
//		//	, 0x00404080, 1.0f, 0L);
//
//		//if (m_pEffect != NULL)
//		//{
//		//	//-------------------------------------------------
//		//	// テクスチャをぼかしつつ張る
//		//	//-------------------------------------------------
//		//	m_pEffect->BeginPass(2);
//
//		//	TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
//		//	TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
//		//	TSS(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
//
//		//	VERTEX Vertex[4] = {
//		//		//   x      y     z      tu tv
//		//		{{  1.0f, -1.0f, 0.1f},   1, 1,},
//		//		{{ -1.0f, -1.0f, 0.1f},   0, 1,},
//		//		{{ -1.0f,  1.0f, 0.1f},   0, 0,},
//		//		{{  1.0f,  1.0f, 0.1f},   1, 0,},
//		//	};
//		//	m_pd3dDevice->SetFVF(D3DFVF_XYZ | D3DFVF_TEX1);
//		//	m_pEffect->SetTexture(m_htSrcMap, m_pSatTex);
//		//	m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
//		//		, 2, Vertex, sizeof(VERTEX));
//
//		//	m_pEffect->EndPass();
//		//	m_pEffect->End();
//		//}
//
//		//RS(D3DRS_ZENABLE, TRUE);
//		//RS(D3DRS_LIGHTING, TRUE);
//
////#if 1 // デバッグ用にテクスチャを表示する
////		{
////			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
////			m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
////			m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
////			m_pd3dDevice->SetVertexShader(NULL);
////			m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
////			m_pd3dDevice->SetPixelShader(0);
////			float scale = 512.0f / 1;
////			typedef struct { FLOAT p[4]; FLOAT tu, tv; } TVERTEX;
////			for (DWORD i = 0; i < 1; i++) {
////				TVERTEX Vertex[4] = {
////					// x  y  z rhw tu tv
////					{    0,(i + 0) * scale,0, 1, 0, 0,},
////					{scale,(i + 0) * scale,0, 1, 1, 0,},
////					{scale,(i + 1) * scale,0, 1, 1, 1,},
////					{    0,(i + 1) * scale,0, 1, 0, 1,},
////				};
////				if (0 == i) m_pd3dDevice->SetTexture(0, m_pSatTex);
////				m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
////			}
////		}
////
////		// ヘルプの表示
////		RenderText();
////
////#endif		
//
//		// 描画の終了
//		m_pd3dDevice->EndScene();
//	}

	pOldBackBuffer->Release();
	pOldZBuffer->Release();

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
    _stprintf_s(szMsg, MAX_PATH, TEXT("'Q' or 'w' keys change the bluring size, now %f"), m_size);
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
	// SAT
	SAFE_RELEASE(m_pSatSurf);
	SAFE_RELEASE(m_pSatTex);
	SAFE_RELEASE(m_pMapZ);

	m_pMesh->InvalidateDeviceObjects(); // メッシュ
	m_pMeshBg->InvalidateDeviceObjects();

	m_pFont->InvalidateDeviceObjects();	// フォント

	// シェーダ
	if (m_pEffect != NULL) m_pEffect->OnLostDevice();

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	// シェーダ
	SAFE_RELEASE(m_pEffect);

	// メッシュ
	m_pMesh->Destroy();
	m_pMeshBg->Destroy();

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
	SAFE_DELETE(m_pMeshBg); // メッシュ
	SAFE_DELETE(m_pMesh);

	SAFE_DELETE(m_pFont);	// フォント

	return S_OK;
}





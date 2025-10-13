//-------------------------------------------------------------
// File: main.cpp
//
// Desc: ボリュームフォグ
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
// Name: CMyD3DApplication()
// Desc: アプリケーションのコンストラクタ
//-------------------------------------------------------------
CMyD3DApplication::CMyD3DApplication()
{
	m_pMesh = new CD3DMesh();
	m_pMeshBg = new CD3DMesh();
	m_pDecl = NULL;
	m_pEffect = NULL;
	m_hmWVP = NULL;
	m_hmWVPT = NULL;
	m_hvCol = NULL;
	m_hvDir = NULL;
	m_hvDecaleTex = NULL;
	m_hvDepthTex = NULL;
	m_hvFrameBufferTex = NULL;

	m_pMapZ = NULL;
	m_pColorMap = NULL;
	m_pColorMapSurf = NULL;
	m_pDepthMap = NULL;
	m_pDepthMapSurf = NULL;
	m_pFogMap = NULL;
	m_pFogMapSurf = NULL;

	m_zoom = 5.0f;
	m_fWorldRotX = -0.8f;
	m_fWorldRotY = 0;

	m_dwCreationWidth = 500;
	m_dwCreationHeight = 375;
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
HRESULT CMyD3DApplication::ConfirmDevice(D3DCAPS9* pCaps
	, DWORD dwBehavior, D3DFORMAT Format)
{
	UNREFERENCED_PARAMETER(Format);
	UNREFERENCED_PARAMETER(dwBehavior);
	UNREFERENCED_PARAMETER(pCaps);

	// シェーダのチェック
	if (pCaps->VertexShaderVersion < D3DVS_VERSION(1, 1) &&
		!(dwBehavior & D3DCREATE_SOFTWARE_VERTEXPROCESSING))
		return E_FAIL;	// 頂点シェーダ

	if (pCaps->PixelShaderVersion < D3DPS_VERSION(2, 0))
		return E_FAIL;	// ピクセルシェーダ

	// MRT を２枚使う
	if (pCaps->NumSimultaneousRTs < 2) return E_FAIL;

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

	// メッシュの読み込み
	if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("ufo.x")))) {
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("ufo.x load Error"), MB_OK);
		return hr;
	}
	m_pMesh->UseMeshMaterials(FALSE);// テクスチャは自分で設定
	m_pMesh->SetFVF(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_NORMAL);

	// メッシュの読み込み
	if (FAILED(hr = m_pMeshBg->Create(m_pd3dDevice, _T("map.x")))) {
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("map.x load Error"), MB_OK);
		return hr;
	}
	m_pMeshBg->UseMeshMaterials(FALSE);// テクスチャは自分で設定
	m_pMeshBg->SetFVF(m_pd3dDevice, D3DFVF_XYZ | D3DFVF_NORMAL | D3DFVF_TEX1);

	// 頂点宣言のオブジェクトの生成(地形用)
	D3DVERTEXELEMENT9 decl[] =
	{
		{0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	0},
		{0, 24, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		D3DDECL_END()
	};
	if (FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(
		decl, &m_pDecl)))
	{
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("CreateVertexDeclaration"), MB_OK);
		return hr;
	}


	// シェーダの読み込み
	LPD3DXBUFFER pErr;
	if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
		// シェーダの読み込みの失敗
		MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
	}
	else {
		m_hmWVP = m_pEffect->GetParameterByName(NULL, "mWVP");
		m_hmWVPT = m_pEffect->GetParameterByName(NULL, "mWVPT");
		m_hvCol = m_pEffect->GetParameterByName(NULL, "vCol");
		m_hvDir = m_pEffect->GetParameterByName(NULL, "vLightDir");
		m_hvDecaleTex = m_pEffect->GetParameterByName(NULL, "DecaleTex");
		m_hvDepthTex = m_pEffect->GetParameterByName(NULL, "DepthTex");
		m_hvFrameBufferTex = m_pEffect->GetParameterByName(NULL, "FrameBufferTex");
	}

	m_pFont->InitDeviceObjects(m_pd3dDevice);// フォント

	return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//		確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
	D3DVIEWPORT9 viewport;

	// 新しく設定されたビューポートからテクスチャを再構成
	m_pd3dDevice->GetViewport(&viewport);
	m_Width = viewport.Width;
	m_Height = viewport.Height;
	// テクスチャサイズはビューポート以上の 2^n
	for (m_MapW = 1; m_MapW < m_Width; m_MapW *= 2);
	for (m_MapH = 1; m_MapH < m_Height; m_MapH *= 2);

	// レンダリングターゲット深度の生成
	if (FAILED(m_pd3dDevice->CreateDepthStencilSurface(m_MapW, m_MapH,
		D3DFMT_D16, D3DMULTISAMPLE_NONE, 0, TRUE, &m_pMapZ, NULL)))
		return E_FAIL;
	// カラーバッファ
	if (FAILED(m_pd3dDevice->CreateTexture(m_MapW, m_MapH, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8, D3DPOOL_DEFAULT, &m_pColorMap, NULL)))
		return E_FAIL;
	if (FAILED(m_pColorMap->GetSurfaceLevel(0, &m_pColorMapSurf)))
		return E_FAIL;
	// 深度バッファ
	if (FAILED(m_pd3dDevice->CreateTexture(m_MapW, m_MapH, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_pDepthMap, NULL)))
		return E_FAIL;
	if (FAILED(m_pDepthMap->GetSurfaceLevel(0, &m_pDepthMapSurf)))
		return E_FAIL;
	// フォグバッファ
	if (FAILED(m_pd3dDevice->CreateTexture(m_MapW, m_MapH, 1,
		D3DUSAGE_RENDERTARGET, D3DFMT_R32F, D3DPOOL_DEFAULT, &m_pFogMap, NULL)))
		return E_FAIL;
	if (FAILED(m_pFogMap->GetSurfaceLevel(0, &m_pFogMapSurf)))
		return E_FAIL;


	// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define SAMP m_pd3dDevice->SetSamplerState

// レンダリング状態の設定
	RS(D3DRS_ZENABLE, TRUE);
	RS(D3DRS_LIGHTING, FALSE);

	// テクスチャの設定
	SAMP(0, D3DSAMP_ADDRESSU, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_ADDRESSV, D3DTADDRESS_CLAMP);
	SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

	// ワールド行列
	D3DXMatrixIdentity(&m_mWorld);

	// ビュー行列
	D3DXVECTOR3 vFrom = D3DXVECTOR3(0.0f, 0.0f, -m_zoom);
	D3DXVECTOR3 vLookat = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFrom, &vLookat, &vUp);

	// 射影行列の設定
	FLOAT fAspect = ((FLOAT)m_d3dsdBackBuffer.Width)
		/ ((FLOAT)m_d3dsdBackBuffer.Height);
	D3DXMatrixPerspectiveFovLH(&m_mProj, D3DX_PI / 4, fAspect
		, 1.0f, 100.0f);

	// ライトの方向の設定
	m_LightDir = D3DXVECTOR4(-0.6f, 0.6f, -0.6f, 0.3f);

	m_pMesh->RestoreDeviceObjects(m_pd3dDevice);
	m_pMeshBg->RestoreDeviceObjects(m_pd3dDevice);
	if (m_pEffect != NULL) m_pEffect->OnResetDevice();// シェーダ

	m_pFont->RestoreDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: FrameMove()
// Desc: 毎フレーム呼ばれます。アニメの処理などを行います。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FrameMove()
{
	UpdateInput(&m_UserInput); // 入力データの更新

	//---------------------------------------------------------
	// 入力に応じて座標系を更新する
	//---------------------------------------------------------
	// 回転
	if (m_UserInput.bRotateLeft && !m_UserInput.bRotateRight)
		m_fWorldRotY += m_fElapsedTime;
	else if (m_UserInput.bRotateRight && !m_UserInput.bRotateLeft)
		m_fWorldRotY -= m_fElapsedTime;

	if (m_UserInput.bRotateUp && !m_UserInput.bRotateDown)
		m_fWorldRotX += m_fElapsedTime;
	else if (m_UserInput.bRotateDown && !m_UserInput.bRotateUp)
		m_fWorldRotX -= m_fElapsedTime;
	// ズーム
	if (m_UserInput.bZ && !m_UserInput.bX)
		m_zoom += 0.01f;
	else if (m_UserInput.bX && !m_UserInput.bZ)
		m_zoom -= 0.01f;


	//---------------------------------------------------------
	// 行列の更新
	//---------------------------------------------------------
	D3DXMATRIX matRotX, matRotY;
	D3DXMatrixRotationX(&matRotX, m_fWorldRotX);
	D3DXMatrixRotationY(&matRotY, m_fWorldRotY);
	D3DXMatrixMultiply(&m_mWorld, &matRotY, &matRotX);

	D3DXVECTOR3 vFrom = D3DXVECTOR3(0.0f, 0.0f, -m_zoom);
	D3DXVECTOR3 vLookat = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
	D3DXVECTOR3 vUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
	D3DXMatrixLookAtLH(&m_mView, &vFrom, &vLookat, &vUp);

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
	pUserInput->bZ = (m_bActive && (GetAsyncKeyState('Z') & 0x8000) == 0x8000);
	pUserInput->bX = (m_bActive && (GetAsyncKeyState('X') & 0x8000) == 0x8000);
}




//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
	LPDIRECT3DSURFACE9 pOldBackBuffer, pOldZBuffer;
	D3DVIEWPORT9 oldViewport;
	D3DXHANDLE hTechnique;
	D3DXMATRIX m, mL, mWT;
	D3DXVECTOR4 v;
	DWORD i;
	FLOAT ds = 0.5f / (FLOAT)m_MapW;// テクセル中心移動用
	FLOAT dt = 0.5f / (FLOAT)m_MapH;
	FLOAT s = (FLOAT)m_Width / (FLOAT)m_MapW + ds;
	FLOAT t = (FLOAT)m_Height / (FLOAT)m_MapH + dt;
	// 射影空間からテクスチャ座標に移行する変換行列
	D3DXMATRIX	mT = D3DXMATRIX(
		0.5f * s, 0.0f, 0.0f, 0.0f,
		0.0f, -0.5f * t, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f,
		0.5f * s, 0.5f * t, 0.0f, 1.0f);
	// 変更用ビューポート    x y   width   height  minz maxz
	D3DVIEWPORT9 viewport = { 0,0, m_Width,m_Height,0.0f,1.0f };

	if (SUCCEEDED(m_pd3dDevice->BeginScene()))	// 描画の開始
	{
		if (m_pEffect != NULL)
		{
			//-------------------------------------------------
			// レンダリングターゲットの保存
			//-------------------------------------------------
			m_pd3dDevice->GetRenderTarget(0, &pOldBackBuffer);
			m_pd3dDevice->GetDepthStencilSurface(&pOldZBuffer);
			m_pd3dDevice->GetViewport(&oldViewport);

			//-------------------------------------------------
			// レンダリングターゲットの変更
			//-------------------------------------------------
			m_pd3dDevice->SetRenderTarget(0, m_pColorMapSurf);
			m_pd3dDevice->SetRenderTarget(1, m_pDepthMapSurf);
			m_pd3dDevice->SetDepthStencilSurface(m_pMapZ);
			m_pd3dDevice->SetViewport(&viewport);

			// レンダリングターゲットのクリア
			m_pd3dDevice->Clear(0L, NULL,
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				0xffffff, 1.0f, 0L);

			//-------------------------------------------------
			// シェーダの設定
			//-------------------------------------------------
			hTechnique = m_pEffect->GetTechniqueByName("TShader");
			m_pEffect->SetTechnique(hTechnique);
			m_pEffect->Begin(NULL, 0);
			m_pEffect->BeginPass(0);

			//-------------------------------------------------
			// シェーダ定数の設定
			//-------------------------------------------------
			// 座標変換
			m = m_mWorld * m_mView * m_mProj;
			m_pEffect->SetMatrix(m_hmWVP, &m);

			// ライト
			D3DXMatrixInverse(&m, NULL, &m_mWorld);
			D3DXVec4Transform(&v, &m_LightDir, &m);
			D3DXVec4Normalize(&v, &v); v.w = 0.3f;
			m_pEffect->SetVector(m_hvDir, &v);

			//-------------------------------------------------
			// 描画
			//-------------------------------------------------
			D3DMATERIAL9* pMtrl = m_pMeshBg->m_pMaterials;
			for (i = 0; i < m_pMeshBg->m_dwNumMaterials; i++) {
				// メッシュの色
				v.x = pMtrl->Diffuse.r;
				v.y = pMtrl->Diffuse.g;
				v.z = pMtrl->Diffuse.b;
				v.w = pMtrl->Diffuse.a;
				m_pEffect->SetVector(m_hvCol, &v);
				// テクスチャ
				m_pEffect->SetTexture(m_hvDecaleTex, m_pMeshBg->m_pTextures[i]);

				m_pEffect->CommitChanges();
				m_pMeshBg->m_pLocalMesh->DrawSubset(i); // 描画
				pMtrl++;
			}

			m_pEffect->EndPass();
			m_pEffect->End();

			//-------------------------------------------------
			//-------------------------------------------------
			// パス2,3 フォグマップの作成
			//-------------------------------------------------
			//-------------------------------------------------

			//-------------------------------------------------
			// レンダリングターゲットの変更
			//-------------------------------------------------
			m_pd3dDevice->SetRenderTarget(0, m_pFogMapSurf);
			m_pd3dDevice->SetRenderTarget(1, NULL);
			m_pd3dDevice->SetViewport(&viewport);
			// レンダリングターゲットのクリア
			m_pd3dDevice->Clear(0L, NULL,
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				0x0, 1.0f, 0L);

			//-------------------------------------------------
			// シェーダの設定
			//-------------------------------------------------
			hTechnique = m_pEffect->GetTechniqueByName("TVolume");
			m_pEffect->SetTechnique(hTechnique);
			m_pEffect->SetTexture(m_hvDepthTex, m_pDepthMap);
			m_pEffect->SetTexture(m_hvFrameBufferTex, m_pFogMap);

			D3DXMatrixTranslation(&mL, 0, 1.0f, 0);
			m = mL * m_mWorld * m_mView * m_mProj;
			m_pEffect->SetMatrix(m_hmWVP, &m);

			mWT = m * mT;
			m_pEffect->SetMatrix(m_hmWVPT, &mWT);

			for (i = 0; i < 2; i++) {
				for (DWORD j = 0; j < m_pMesh->m_dwNumMaterials; j++) {
					// 一度描画を終わらせて再スタート
					m_pd3dDevice->EndScene();
					m_pd3dDevice->BeginScene();
					m_pEffect->Begin(NULL, 0);
					m_pEffect->BeginPass(i);

					m_pEffect->CommitChanges();
					m_pMesh->m_pLocalMesh->DrawSubset(j);

					m_pEffect->EndPass();
					m_pEffect->End();
				}
			}

			//-------------------------------------------------
			//-------------------------------------------------
			// 4パス目:シーンの描画
			//-------------------------------------------------
			//-------------------------------------------------

			//-------------------------------------------------
			// レンダリングターゲットを元に戻す
			//-------------------------------------------------
			m_pd3dDevice->SetRenderTarget(0, pOldBackBuffer);
			m_pd3dDevice->SetDepthStencilSurface(pOldZBuffer);
			m_pd3dDevice->SetViewport(&oldViewport);
			pOldBackBuffer->Release();
			pOldZBuffer->Release();

			//-------------------------------------------------
			// シェーダの設定
			//-------------------------------------------------
			hTechnique = m_pEffect->GetTechniqueByName("TFinal");
			m_pEffect->SetTechnique(hTechnique);
			m_pEffect->Begin(NULL, 0);
			m_pEffect->BeginPass(0);
			m_pEffect->SetTexture("FogMap", m_pFogMap);
			m_pEffect->SetTexture("ColorMap", m_pColorMap);

			// バッファのクリア
			m_pd3dDevice->Clear(0L, NULL,
				D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
				0x00404080, 1.0f, 0L);

			m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
			typedef struct { FLOAT p[4]; FLOAT tu, tv; } TVERTEX;
			TVERTEX Vertex[4] = {
				//         x               y     z rhw tu  tv
				{             0,              0, 0, 1, ds, dt},
				{(FLOAT)m_Width,              0, 0, 1,  s, dt},
				{(FLOAT)m_Width,(FLOAT)m_Height, 0, 1,  s,  t},
				{             0,(FLOAT)m_Height, 0, 1, ds,  t},
			};
			m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
				, 2, Vertex, sizeof(TVERTEX));

			m_pEffect->End();
		}

#if 1 // デバッグ用にテクスチャを表示する
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
		m_pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
		m_pd3dDevice->SetTextureStageState(1, D3DTSS_COLOROP, D3DTOP_DISABLE);
		m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
		m_pd3dDevice->SetVertexShader(NULL);
		m_pd3dDevice->SetPixelShader(0);
		for (DWORD loop = 0; loop < 3; loop++) {
			const float scale = 200.0f;
			typedef struct { FLOAT p[4]; FLOAT tu, tv; } TVERTEX;

			TVERTEX Vertex[4] = {
				{(loop)*scale,    0,0, 1, ds, dt,},
				{(loop + 1) * scale,    0,0, 1,  s, dt,},
				{(loop + 1) * scale,scale,0, 1,  s,  t,},
				{(loop)*scale,scale,0, 1, ds,  t,},
			};
			D3DXHANDLE str = loop == 1 ? "DepthTex" : "FrameBufferTex";
			if ((loop == 1 || loop == 2) && m_pEffect) {
				// 深度はエフェクトで .x を RGB にコピーして描画（灰色）
				m_pEffect->SetTechnique(m_pEffect->GetTechniqueByName("TDebugDepth"));
				m_pEffect->SetTexture(str, m_pDepthMap);
				m_pEffect->Begin(NULL, 0);
				m_pEffect->BeginPass(loop-1);
				m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
				m_pEffect->EndPass();
				m_pEffect->End();
			}
			else {
				// 通常のテクスチャ描画 ColorMap
				m_pd3dDevice->SetTexture(0, m_pColorMap);
				m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, 2, Vertex, sizeof(TVERTEX));
			}
		}
#endif      
		RenderText();				// ヘルプ等の表示

		m_pd3dDevice->EndScene();	// 描画の終了
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

	FLOAT fNextLine = 40.0f;

	// 操作法やパラメータを表示する
	fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
	swprintf_s(szMsg, TEXT("Zoom: %f"), m_zoom);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Use arrow keys to rotate object"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, TEXT("Press 'z' or 'x' to change zoom"));
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	// ディスプレイの状態を表示する
	lstrcpy(szMsg, m_strFrameStats);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
	lstrcpy(szMsg, m_strDeviceStats);
	fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

	return S_OK;
}




//-------------------------------------------------------------
// Name: MsgProc()
// Desc: WndProc をオーバーライドしたもの
//-------------------------------------------------------------
LRESULT CMyD3DApplication::MsgProc(HWND hWnd,
	UINT msg, WPARAM wParam, LPARAM lParam)
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
			DrawText(hDC, strMsg, -1, &rct,
				DT_CENTER | DT_VCENTER | DT_SINGLELINE);
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
	// レンダリングターゲット
	SAFE_RELEASE(m_pFogMapSurf);
	SAFE_RELEASE(m_pFogMap);
	SAFE_RELEASE(m_pDepthMapSurf);
	SAFE_RELEASE(m_pDepthMap);
	SAFE_RELEASE(m_pColorMapSurf);
	SAFE_RELEASE(m_pColorMap);
	SAFE_RELEASE(m_pMapZ);

	m_pMesh->InvalidateDeviceObjects();				// メッシュ
	m_pMeshBg->InvalidateDeviceObjects();				// メッシュ
	if (m_pEffect != NULL) m_pEffect->OnLostDevice();	// シェーダ

	m_pFont->InvalidateDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
	SAFE_RELEASE(m_pEffect);		// シェーダ
	SAFE_RELEASE(m_pDecl);		// 頂点宣言

	m_pMesh->Destroy();				// メッシュ
	m_pMeshBg->Destroy();				// メッシュ

	m_pFont->DeleteDeviceObjects();	// フォント

	return S_OK;
}




//-------------------------------------------------------------
// Name: FinalCleanup()
// Desc: 終了する直前に呼ばれる
//-------------------------------------------------------------
HRESULT CMyD3DApplication::FinalCleanup()
{
	SAFE_DELETE(m_pMesh);	// メッシュ
	SAFE_DELETE(m_pMeshBg);	// メッシュ

	SAFE_DELETE(m_pFont);	// フォント

	return S_OK;
}





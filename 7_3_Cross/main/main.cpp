//-------------------------------------------------------------
// File: main.cpp
//
// Desc: クロスフィルタ
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
#include "framework/DXUtil.h"
#include "framework/D3DEnumeration.h"
#include "framework/D3DSettings.h"
#include "framework/D3DApp.h"
#include "framework/D3DFont.h"
#include "framework/D3DFile.h"
#include "framework/D3DUtil.h"
#include "resource.h"
#include "main.h"

// 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState


//-------------------------------------------------------------
// 頂点宣言
//-------------------------------------------------------------
D3DVERTEXELEMENT9 decl[] =
{
    {0,  0, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
    {0, 12, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,	0},
    {0, 24, D3DDECLTYPE_FLOAT3,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TANGENT,	0},
    {0, 36, D3DDECLTYPE_FLOAT2,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
    D3DDECL_END()
};

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
    m_pTexScene = NULL;
    m_pSurfScene = NULL;
    m_pTexSceneScaled = NULL;
    m_pSurfSceneScaled = NULL;
    m_pTexBrightPass = NULL;
    m_pSurfBrightPass = NULL;
    m_pTexStarSource = NULL;
    m_pSurfStarSource = NULL;
    ZeroMemory(m_apTexStar, sizeof(m_apTexStar));
    ZeroMemory(m_apSurfStar, sizeof(m_apSurfStar));

    m_pMesh = new CD3DMesh();
    m_pMeshBg = new CD3DMesh();
    m_pDecl = NULL;
    m_pNormalMap = NULL;

    m_pEffect = NULL;
    m_hTechnique = NULL;
    m_hmWVP = NULL;
    m_hvLightDir = NULL;
    m_hvColor = NULL;
    m_hvEyePos = NULL;
    m_htDecaleTex = NULL;
    m_htNormalMap = NULL;

    m_fWorldRotX = -D3DX_PI / 10;
    m_fWorldRotY = D3DX_PI / 2;
    m_fViewZoom = 5.0f;

    m_dwCreationWidth = 640;
    m_dwCreationHeight = 480;
    m_strWindowTitle = TEXT("main");
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
    m_bStartFullscreen = false;
    m_bShowCursorWhenFullscreen = false;

    m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
    m_bLoadingApp = TRUE;

    ZeroMemory(&m_UserInput, sizeof(m_UserInput));
}



//-----------------------------------------------------------------------------
// Name: DrawFullScreenQuad
// Desc: 四角形を全画面に描画する
//-----------------------------------------------------------------------------
void CMyD3DApplication::DrawFullScreenQuad(float fLeftU, float fTopV, float fRightU, float fBottomV)
{
    D3DSURFACE_DESC desc;
    PDIRECT3DSURFACE9 pSurf;

    // レンダリングターゲットの情報(幅と高さ)を所得する
    m_pd3dDevice->GetRenderTarget(0, &pSurf);
    pSurf->GetDesc(&desc);
    pSurf->Release();
    FLOAT w = (FLOAT)desc.Width;
    FLOAT h = (FLOAT)desc.Height;

    typedef struct {
        float p[4]; // 位置座標
        float t[2]; // テクスチャ座標
    } ScreenVertex;
    ScreenVertex svQuad[4] = {
        // x       y      z     w       u       v
        {0 - 0.5f, 0 - 0.5f, 0.5f, 1.0f, fLeftU,  fTopV,},
        {w - 0.5f, 0 - 0.5f, 0.5f, 1.0f, fRightU, fTopV,},
        {0 - 0.5f, h - 0.5f, 0.5f, 1.0f, fLeftU,  fBottomV,},
        {w - 0.5f, h - 0.5f, 0.5f, 1.0f, fRightU, fBottomV,},
    };

    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);	// 深度バッファは使わない
    m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
    m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLESTRIP, 2, svQuad, sizeof(ScreenVertex));
    m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);	// 深度バッファを復活
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
    LPDIRECT3DTEXTURE9	pHeightTexture;
    D3DSURFACE_DESC desc;

    // 法線マップの作成
    D3DUtil_CreateTexture(m_pd3dDevice,// 高さマップの読み込み
        _T("height.bmp"), &pHeightTexture);
    pHeightTexture->GetLevelDesc(0, &desc);// テクスチャ情報の入手
    D3DXCreateTexture(m_pd3dDevice, desc.Width, desc.Height, 0, 0,
        D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, &m_pNormalMap);// テクスチャ生成
    D3DXComputeNormalMap(m_pNormalMap,	// 法線マップの生成
        pHeightTexture, NULL, 0, D3DX_CHANNEL_RED, 5.0f);
    SAFE_RELEASE(pHeightTexture);		// 使わないリソースの開放

    // 頂点宣言のオブジェクトの生成
    if (FAILED(hr = m_pd3dDevice->CreateVertexDeclaration(
        decl, &m_pDecl)))
    {
		_com_error err(hr);
		LPCTSTR errMsg = err.ErrorMessage();
		MessageBox(nullptr, errMsg, _T("Vertex Declaration Error"), MB_OK);
		return hr;
    }

    // ティーポットの読み込み
    if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("t-pot.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("t-pot.x load Error"), MB_OK);
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

    // シェーダの読み込み
    LPD3DXBUFFER pErr = NULL;
    if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
        // シェーダの読み込みの失敗
        MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
        return hr;
    }
    else {
        m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
        m_hmWVP = m_pEffect->GetParameterByName(NULL, "mWVP");
        m_hvLightDir = m_pEffect->GetParameterByName(NULL, "vLightDir");
        m_hvColor = m_pEffect->GetParameterByName(NULL, "vColor");
        m_hvEyePos = m_pEffect->GetParameterByName(NULL, "vEyePos");
        m_htDecaleTex = m_pEffect->GetParameterByName(NULL, "DecaleTex");
        m_htNormalMap = m_pEffect->GetParameterByName(NULL, "NormalMap");
    }
    SAFE_RELEASE(pErr);

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
    HRESULT hr;

    //---------------------------------------------------------
    // シーンを描画するHDR フォーマットのレンダリングターゲット
    //---------------------------------------------------------
    hr = m_pd3dDevice->CreateTexture(
        m_d3dsdBackBuffer.Width, m_d3dsdBackBuffer.Height,
        1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
        D3DPOOL_DEFAULT, &m_pTexScene, NULL);
    if (FAILED(hr)) return hr;
    hr = m_pTexScene->GetSurfaceLevel(0, &m_pSurfScene);
    if (FAILED(hr)) return hr;

    //---------------------------------------------------------
    // 縮小バッファ
    //---------------------------------------------------------
    // 縮小バッファの基本サイズ（FBを切捨てで４の倍数の大きさ）
    m_dwCropWidth = m_d3dsdBackBuffer.Width - m_d3dsdBackBuffer.Width % 4;
    m_dwCropHeight = m_d3dsdBackBuffer.Height - m_d3dsdBackBuffer.Height % 4;

    hr = m_pd3dDevice->CreateTexture(
        m_dwCropWidth / 4, m_dwCropHeight / 4,
        1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
        D3DPOOL_DEFAULT, &m_pTexSceneScaled, NULL);
    if (FAILED(hr)) return hr;
    hr = m_pTexSceneScaled->GetSurfaceLevel(0, &m_pSurfSceneScaled);
    if (FAILED(hr)) return hr;

    //---------------------------------------------------------
    // 輝度の抽出
    //---------------------------------------------------------
    hr = m_pd3dDevice->CreateTexture(
        m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2,
        1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT, &m_pTexBrightPass, NULL);
    if (FAILED(hr)) return hr;
    hr = m_pTexBrightPass->GetSurfaceLevel(0, &m_pSurfBrightPass);
    if (FAILED(hr)) return hr;
    // ふちを黒く塗りつぶしておく
    m_pd3dDevice->ColorFill(m_pSurfBrightPass, NULL
        , D3DCOLOR_ARGB(0, 0, 0, 0));

    //---------------------------------------------------------
    // ちらつかないようにぼかすためのテクスチャ
    //---------------------------------------------------------
    hr = m_pd3dDevice->CreateTexture(
        m_dwCropWidth / 4 + 2, m_dwCropHeight / 4 + 2,
        1, D3DUSAGE_RENDERTARGET, D3DFMT_A8R8G8B8,
        D3DPOOL_DEFAULT, &m_pTexStarSource, NULL);
    if (FAILED(hr)) return hr;
    hr = m_pTexStarSource->GetSurfaceLevel(0, &m_pSurfStarSource);
    if (FAILED(hr)) return hr;
    // ふちを黒く塗りつぶしておく
    m_pd3dDevice->ColorFill(m_pSurfStarSource, NULL
        , D3DCOLOR_ARGB(0, 0, 0, 0));

    //---------------------------------------------------------
    // 光芒のためのテクスチャ
    //---------------------------------------------------------
    for (int i = 0; i < NUM_STAR_TEXTURES; i++) {
        hr = m_pd3dDevice->CreateTexture(
            m_dwCropWidth / 4, m_dwCropHeight / 4,
            1, D3DUSAGE_RENDERTARGET, D3DFMT_A16B16G16R16F,
            D3DPOOL_DEFAULT, &m_apTexStar[i], NULL);
        if (FAILED(hr)) return hr;
        hr = m_apTexStar[i]->GetSurfaceLevel(0, &m_apSurfStar[i]);
        if (FAILED(hr)) return hr;
    }

    // エフェクト
    if (m_pEffect) m_pEffect->OnResetDevice();

    // メッシュ
    m_pMeshBg->RestoreDeviceObjects(m_pd3dDevice);

    //---------------------------------------------------------
    // FVF で処理しきれない頂点宣言のときはMeshを自分で処理
    //---------------------------------------------------------
    if (m_pMesh && m_pMesh->GetSysMemMesh()) {
        LPD3DXMESH pMesh;

        m_pMesh->GetSysMemMesh()->CloneMesh(
            m_pMesh->GetSysMemMesh()->GetOptions(), decl,
            m_pd3dDevice, &pMesh);
        D3DXComputeNormals(pMesh, NULL);
        D3DXComputeTangent(pMesh, 0, 0, 0, TRUE, NULL);

        SAFE_RELEASE(m_pMesh->m_pLocalMesh);
        m_pMesh->m_pLocalMesh = pMesh;
    }

    // レンダリング状態の設定
    RS(D3DRS_ZENABLE, TRUE);
    RS(D3DRS_LIGHTING, FALSE);

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
    D3DXMATRIX mCamera;

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

    D3DXMatrixMultiply(&mCamera, &matRotY, &matRotX);

    //---------------------------------------------------------
    // ビュー行列の設定
    //---------------------------------------------------------
    // ズーム
    if (m_UserInput.bZoomIn && !m_UserInput.bZoomOut)
        m_fViewZoom += m_fElapsedTime;
    else if (m_UserInput.bZoomOut && !m_UserInput.bZoomIn)
        m_fViewZoom -= m_fElapsedTime;

    m_vFromPt = D3DXVECTOR4(0.0f, 0.f, -m_fViewZoom, 1);
    D3DXVECTOR3 vLookatPt = D3DXVECTOR3(0.0f, 0.0f, 0.0f);
    D3DXVECTOR3 vUpVec = D3DXVECTOR3(0.0f, 1.0f, 0.0f);
    D3DXMatrixLookAtLH(&m_mView, (D3DXVECTOR3*)&m_vFromPt, &vLookatPt, &vUpVec);

    m_mView = mCamera * m_mView;

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
}


//-------------------------------------------------------------
// Name: RenderScene()
// Desc: 画面をに表示されるオブジェクトを描画する
//-------------------------------------------------------------
void CMyD3DApplication::RenderScene()
{
    D3DXMATRIX m, mT, mR, mView, mProj;
    D3DXMATRIX mWorld;
    D3DXVECTOR4 v, light_pos, eye_pos;
    DWORD i;

    //-----------------------------------------------------
    // 地形の描画
    //-----------------------------------------------------
    // ワールド行列
    D3DXMatrixScaling(&m, 3.0f, 3.0f, 3.0f);
    D3DXMatrixRotationY(&mR, D3DX_PI);
    D3DXMatrixTranslation(&mT, 0.0f, -2.0f, 0.0f);
    mWorld = m * mR * mT;

    // 行列の設定
    m_pd3dDevice->SetTransform(D3DTS_WORLD, &mWorld);
    m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_mView);
    m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mProj);

    TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
    TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
    SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
    m_pMeshBg->Render(m_pd3dDevice);

    if (m_pEffect != NULL)
    {
        //-------------------------------------------------
        // シェーダの設定
        //-------------------------------------------------
        m_pEffect->SetTechnique(m_hTechnique);
        m_pEffect->Begin(NULL, 0);
        m_pEffect->BeginPass(0);

        //-------------------------------------------------
        // ティーポットの描画
        //-------------------------------------------------

        // ワールド行列（回転）
        D3DXMatrixRotationY(&mWorld, m_fTime);

        // ローカル-射影変換行列
        m = mWorld * m_mView * m_mProj;
        m_pEffect->SetMatrix(m_hmWVP, &m);

        // ライトの方向（ローカル座標系）
        light_pos = D3DXVECTOR4(-0.577f, -0.577f, -0.577f, 0);
        D3DXMatrixInverse(&m, NULL, &mWorld);
        D3DXVec4Transform(&v, &light_pos, &m);
        D3DXVec3Normalize((D3DXVECTOR3*)&v, (D3DXVECTOR3*)&v);
        v.w = -0.5f;		// 環境光の強さ
        m_pEffect->SetVector(m_hvLightDir, &v);

        // 視点（ローカル座標系）
        m = mWorld * m_mView;
        D3DXMatrixInverse(&m, NULL, &m);
        v = D3DXVECTOR4(0, 0, 0, 1);
        D3DXVec4Transform(&v, &v, &m);
        m_pEffect->SetVector(m_hvEyePos, &v);

        // 法線マップ
        m_pEffect->SetTexture(m_htNormalMap, m_pNormalMap);
        // 頂点宣言
        m_pd3dDevice->SetVertexDeclaration(m_pDecl);

        D3DMATERIAL9* pMtrl = m_pMesh->m_pMaterials;
        for (i = 0; i < m_pMesh->m_dwNumMaterials; i++) {
            v.x = pMtrl->Diffuse.r;
            v.y = pMtrl->Diffuse.g;
            v.z = pMtrl->Diffuse.b;
            m_pEffect->SetVector(m_hvColor, &v);
            m_pEffect->SetTexture(m_htDecaleTex, m_pMesh->m_pTextures[i]);
            m_pMesh->m_pLocalMesh->DrawSubset(i);  // 描画
            pMtrl++;
        }

		m_pEffect->EndPass();
        m_pEffect->End();
    }
}


//-----------------------------------------------------------------------------
// Name: GetTextureCoords()
// Desc: 入力元と出力先のテクスチャからテクスチャ座標を計算する
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::GetTextureCoords(PDIRECT3DTEXTURE9 pTexSrc, RECT* pRectSrc,
    PDIRECT3DTEXTURE9 pTexDest, RECT* pRectDest, CoordRect* pCoords)
{
    D3DSURFACE_DESC desc;

    // 妥当性の検証
    if (pTexSrc == NULL || pTexDest == NULL || pCoords == NULL)
        return E_INVALIDARG;

    // 基本的には、テクスチャ座標をそのまま出力する
    pCoords->u0 = 0.0f;
    pCoords->v0 = 0.0f;
    pCoords->u1 = 1.0f;
    pCoords->v1 = 1.0f;

    // 入力元のサーフェスに関して補正する
    if (pRectSrc != NULL) {
        pTexSrc->GetLevelDesc(0, &desc);// テクスチャの情報を所得する
        // 転送元の矩形に応じてテクスチャ座標をあわせる
        pCoords->u0 += pRectSrc->left / desc.Width;
        pCoords->v0 += pRectSrc->top / desc.Height;
        pCoords->u1 -= (desc.Width - pRectSrc->right) / desc.Width;
        pCoords->v1 -= (desc.Height - pRectSrc->bottom) / desc.Height;
    }

    // 出力先のサーフェスに関して補正する
    if (pRectDest != NULL) {
        pTexDest->GetLevelDesc(0, &desc);// テクスチャの情報を所得する
        // 出力先の矩形に応じてテクスチャ座標をあわせる
        pCoords->u0 -= pRectDest->left / desc.Width;
        pCoords->v0 -= pRectDest->top / desc.Height;
        pCoords->u1 += (desc.Width - pRectDest->right) / desc.Width;
        pCoords->v1 += (desc.Height - pRectDest->bottom) / desc.Height;
    }

    return S_OK;
}


//-----------------------------------------------------------------------------
// Name: Scene_To_SceneScaled()
// Desc: m_pTexScene を 1/4 にして m_pTexSceneScaled に入れる
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::Scene_To_SceneScaled()
{
    // はみ出した場合に中心部分をコピーする
    CoordRect coords;
    RECT rectSrc;
    rectSrc.left = (m_d3dsdBackBuffer.Width - m_dwCropWidth) / 2;
    rectSrc.top = (m_d3dsdBackBuffer.Height - m_dwCropHeight) / 2;
    rectSrc.right = rectSrc.left + m_dwCropWidth;
    rectSrc.bottom = rectSrc.top + m_dwCropHeight;
    // レンダリングターゲットにあったテクスチャ座標を計算する
    GetTextureCoords(m_pTexScene, &rectSrc, m_pTexSceneScaled, NULL, &coords);

    // 周辺の１６テクセルをサンプリング点として
    // 計算する0.5は中心に合わせるための補正
    int index = 0;
    D3DXVECTOR2 offsets[MAX_SAMPLES];

    for (int y = 0; y < 4; y++) {
        for (int x = 0; x < 4; x++) {
            offsets[index].x = (x - 1.5f) / m_d3dsdBackBuffer.Width;
            offsets[index].y = (y - 1.5f) / m_d3dsdBackBuffer.Height;
            index++;
        }
    }
    m_pEffect->SetValue("g_avSampleOffsets", offsets, sizeof(offsets));

    // １６テクセルをサンプリングしてその平均を縮小バッファに出力する
    m_pd3dDevice->SetRenderTarget(0, m_pSurfSceneScaled);
    m_pEffect->SetTechnique("DownScale4x4");
    m_pEffect->Begin(NULL, 0);
    m_pEffect->BeginPass(0);
    m_pd3dDevice->SetTexture(0, m_pTexScene);
    DrawFullScreenQuad(coords.u0, coords.v0, coords.u1, coords.v1);

    m_pEffect->EndPass();
    m_pEffect->End();

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: SceneScaled_To_BrightPass
// Desc: 縮小バッファにコピーされたうち、明るい部分だけを抽出する
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::SceneScaled_To_BrightPass()
{
    // 出力先の情報から描画先のサイズを設定する
    D3DSURFACE_DESC desc;
    m_pTexBrightPass->GetLevelDesc(0, &desc);
    RECT rectDest = { 0,0,desc.Width,desc.Height };
    InflateRect(&rectDest, -1, -1);// 出力先の大きさを一回り小さくする
    m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    m_pd3dDevice->SetScissorRect(&rectDest);

    // 全画面コピー
    m_pd3dDevice->SetRenderTarget(0, m_pSurfBrightPass);
    m_pEffect->SetTechnique("BrightPassFilter");
    m_pEffect->Begin(NULL, 0);
    m_pEffect->BeginPass(0);
    m_pd3dDevice->SetTexture(0, m_pTexSceneScaled);
    DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);
    m_pEffect->EndPass();
    m_pEffect->End();

    m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: GetGaussBlur5x5
// Desc: ブラーをかけたときにガウス型のぼかしになるように係数を計算する
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::GetGaussBlur5x5(
    DWORD dwD3DTexWidth, DWORD dwD3DTexHeight,
    D3DXVECTOR2* avTexCoordOffset, D3DXVECTOR4* avSampleWeight)
{
    float tu = 1.0f / (float)dwD3DTexWidth;
    float tv = 1.0f / (float)dwD3DTexHeight;

    float totalWeight = 0.0f;
    int index = 0;
    for (int x = -2; x <= 2; x++) {
        for (int y = -2; y <= 2; y++) {
            // 係数が小さくなる部分は除去
            if (2 < abs(x) + abs(y)) continue;

            avTexCoordOffset[index] = D3DXVECTOR2(x * tu, y * tv);
            float fx = (FLOAT)x;
            float fy = (FLOAT)y;
            avSampleWeight[index].x = avSampleWeight[index].y =
                avSampleWeight[index].z = avSampleWeight[index].w
                = expf(-(fx * fx + fy * fy) / (2 * 1.0f));
            totalWeight += avSampleWeight[index].x;

            index++;
        }
    }

    // 重みの合計を 1.0f にする
    for (int i = 0; i < index; i++) avSampleWeight[i] *= 1.0f / totalWeight;

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: BrightPass_To_StarSource
// Desc: 縮小バッファへコピーしたことによるエイリアシングを防ぐために、
//       ガウス型のぼかしをかける
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::BrightPass_To_StarSource()
{
    // 出力先の情報から描画先のサイズを設定する
    D3DSURFACE_DESC desc;
    m_pTexStarSource->GetLevelDesc(0, &desc);
    RECT rectDest = { 0,0,desc.Width,desc.Height };
    InflateRect(&rectDest, -1, -1);// 出力先の大きさを一回り小さくする
    m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, TRUE);
    m_pd3dDevice->SetScissorRect(&rectDest);

    // テクスチャ座標を算出する
    CoordRect coords;
    GetTextureCoords(m_pTexBrightPass, NULL, m_pTexStarSource,
        &rectDest, &coords);

    // 元画像の大きさからガウス分布の係数を計算する
    D3DXVECTOR2 offsets[MAX_SAMPLES];
    D3DXVECTOR4 weights[MAX_SAMPLES];
    m_pTexBrightPass->GetLevelDesc(0, &desc);
    GetGaussBlur5x5(desc.Width, desc.Height, offsets, weights);
    m_pEffect->SetValue("g_avSampleOffsets", offsets, sizeof(offsets));
    m_pEffect->SetValue("g_avSampleWeights", weights, sizeof(weights));

    // ガウスぼかしをおこなう
    m_pEffect->SetTechnique("GaussBlur5x5");
    m_pd3dDevice->SetRenderTarget(0, m_pSurfStarSource);
    m_pEffect->Begin(NULL, 0);
    m_pEffect->BeginPass(0);
    m_pd3dDevice->SetTexture(0, m_pTexBrightPass);
    DrawFullScreenQuad(coords.u0, coords.v0, coords.u1, coords.v1);
    m_pEffect->End();

    m_pd3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);

    return S_OK;
}




//-----------------------------------------------------------------------------
// Name: RenderStar()
// Desc: 光芒を作成する
//-----------------------------------------------------------------------------
HRESULT CMyD3DApplication::RenderStar()
{
    // エフェクトで使う定数の設定
    static const int s_maxPasses = 3;
    static const int nSamples = 8;
    // 光芒の色
    static const D3DXCOLOR s_colorWhite(0.63f, 0.63f, 0.63f, 0.0f);
    static const D3DXCOLOR s_ChromaticAberrationColor[8] = {
        D3DXCOLOR(0.5f, 0.5f, 0.5f,  0.0f),	// 白
        D3DXCOLOR(0.8f, 0.3f, 0.3f,  0.0f), // 赤
        D3DXCOLOR(1.0f, 0.2f, 0.2f,  0.0f),	// 赤
        D3DXCOLOR(0.5f, 0.2f, 0.6f,  0.0f), // 紫
        D3DXCOLOR(0.2f, 0.2f, 1.0f,  0.0f),	// 青
        D3DXCOLOR(0.2f, 0.3f, 0.7f,  0.0f), // 青
        D3DXCOLOR(0.2f, 0.6f, 0.2f,  0.0f),	// 緑
        D3DXCOLOR(0.3f, 0.5f, 0.3f,  0.0f), // 緑
    };

    static D3DXVECTOR4 s_aaColor[s_maxPasses][nSamples];

    for (int p = 0; p < s_maxPasses; p++) {
        // 中心からの距離に応じて光芒の色をつける
        float ratio = (float)(p + 1) / (float)s_maxPasses;
        // それぞれのサンプリングで適当に色をつける
        for (int s = 0; s < nSamples; s++) {
            D3DXCOLOR chromaticAberrColor;
            D3DXColorLerp(&chromaticAberrColor,
                &(s_ChromaticAberrationColor[s]),
                &s_colorWhite, ratio);
            // 全体的な色の変化を調整する
            D3DXColorLerp((D3DXCOLOR*)&(s_aaColor[p][s]),
                &s_colorWhite, &chromaticAberrColor, 0.7f);
        }
    }

    float radOffset = m_fWorldRotY / 5;// 視点に応じて回転する

    // 元画像の幅と高さをしらべる
    D3DSURFACE_DESC desc;
    m_pSurfStarSource->GetDesc(&desc);
    float srcW = (FLOAT)desc.Width;
    float srcH = (FLOAT)desc.Height;

    int nStarLines = 6;// 光芒の本数
    for (int d = 0; d < nStarLines; d++) {    // 方向に応じたループ
        PDIRECT3DTEXTURE9 pTexSource = m_pTexStarSource;
        float rad = radOffset + 2 * d * D3DX_PI / (FLOAT)nStarLines;// 角度
        float sn = sinf(rad);
        float cs = cosf(rad);
        D3DXVECTOR2 vtStepUV = D3DXVECTOR2(0.3f * sn / srcW,
            0.3f * cs / srcH);

        float attnPowScale = (atanf(D3DX_PI / 4) + 0.1f) *
            (160.0f + 120.0f) / (srcW + srcH);

        int iWorkTexture = 0;
        for (int p = 0; p < s_maxPasses; p++) {
            // 描画先の決定
            PDIRECT3DSURFACE9 pSurfDest = NULL;
            if (p == s_maxPasses - 1) {
                // 最後のパスは、保存用のバッファに確保する
                pSurfDest = m_apSurfStar[d + 2];
            }
            else {
                pSurfDest = m_apSurfStar[iWorkTexture];
            }
            m_pd3dDevice->SetRenderTarget(0, pSurfDest);

            // テクスチャ座標や合成するときの重みを計算する
            D3DXVECTOR4 avSampleWeights[MAX_SAMPLES];
            D3DXVECTOR2 avSampleOffsets[MAX_SAMPLES];

            for (int i = 0; i < nSamples; i++) {
                // それぞれの重み
                float lum = powf(0.95f, attnPowScale * i);
                avSampleWeights[i] = s_aaColor[s_maxPasses - 1 - p][i]
                    * lum * (p + 1.0f) * 0.5f;

                    // テクスチャ座標をずらす量
                    avSampleOffsets[i].x = vtStepUV.x * i;
                    avSampleOffsets[i].y = vtStepUV.y * i;
                    if (0.9f <= fabs(avSampleOffsets[i].x) ||
                        0.9f <= fabs(avSampleOffsets[i].y)) {
                        avSampleOffsets[i].x = 0.0f;
                        avSampleOffsets[i].y = 0.0f;
                        avSampleWeights[i] *= 0.0f;
                    }
            }
            m_pEffect->SetValue("g_avSampleOffsets", avSampleOffsets, sizeof(avSampleOffsets));
            m_pEffect->SetVectorArray("g_avSampleWeights", avSampleWeights, nSamples);

            // 全画面コピー
            m_pEffect->SetTechnique("Star");
            m_pEffect->Begin(NULL, 0);
            m_pEffect->BeginPass(0);
            m_pd3dDevice->SetTexture(0, pTexSource);
            DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

            m_pEffect->EndPass();
            m_pEffect->End();

            // 次のパスのためにパラメータを設定する
            vtStepUV *= nSamples;
            attnPowScale *= nSamples;

            // レンダリングした出力を次のテクスチャにする
            pTexSource = m_apTexStar[iWorkTexture];

            iWorkTexture ^= 1;
        }
    }


    // 全ての光芒を合成する
    m_pd3dDevice->SetRenderTarget(0, m_apSurfStar[0]);

    m_pEffect->SetTechnique("MergeTextures");
    m_pEffect->Begin(NULL, 0);
    m_pEffect->BeginPass(0);
    m_pd3dDevice->SetTexture(0, m_apTexStar[0 + 2]);
    m_pd3dDevice->SetTexture(1, m_apTexStar[1 + 2]);
    m_pd3dDevice->SetTexture(2, m_apTexStar[2 + 2]);
    m_pd3dDevice->SetTexture(3, m_apTexStar[3 + 2]);
    m_pd3dDevice->SetTexture(4, m_apTexStar[4 + 2]);
    m_pd3dDevice->SetTexture(5, m_apTexStar[5 + 2]);

    DrawFullScreenQuad(0.0f, 0.0f, 1.0f, 1.0f);

    m_pEffect->EndPass();
    m_pEffect->End();

    return S_OK;
}
//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{

    PDIRECT3DSURFACE9 pBackBuffer;

    // HDR のレンダリングターゲットを設定する
    m_pd3dDevice->GetRenderTarget(0, &pBackBuffer);// バックアップ
    m_pd3dDevice->SetRenderTarget(0, m_pSurfScene);// 描画先の切り替え

    //---------------------------------------------------------
    // 描画
    //---------------------------------------------------------
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
        // レンダリングターゲットのクリア
        m_pd3dDevice->Clear(0L, NULL
            , D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
            , 0x0060c0, 1.0f, 0L);

        // ティーポットなどを画面に描画
        this->RenderScene();

        this->Scene_To_SceneScaled();     // 縮小バッファへシーンをコピー
        this->SceneScaled_To_BrightPass();// 明るい部分を抽出する
        this->BrightPass_To_StarSource(); // 光芒のためにぼかす
        this->RenderStar();               // 光芒の作成

        // レンダリングターゲットを元に戻す
        m_pd3dDevice->SetRenderTarget(0, pBackBuffer);
        m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);

        // 最終的にLDR に落として描画する
        float w = (float)this->m_d3dsdBackBuffer.Width;
        float h = (float)this->m_d3dsdBackBuffer.Height;
        typedef struct { float x, y, z, w, u, v; }TVERTEX;
        TVERTEX VertexFinal[4] = {
            //x  y  z rhw    tu        tv
            { 0, 0, 0, 1, 0 + 0.5f / w, 0 + 0.5f / h,},
            { w, 0, 0, 1, 1 + 0.5f / w, 0 + 0.5f / h,},
            { w, h, 0, 1, 1 + 0.5f / w, 1 + 0.5f / h,},
            { 0, h, 0, 1, 0 + 0.5f / w, 1 + 0.5f / h,},
        };
        m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
        m_pd3dDevice->SetTexture(0, m_pTexScene);
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
            , 2, VertexFinal, sizeof(TVERTEX));

        // 光芒の加算合成
        m_pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
        m_pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);
        m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
        m_pd3dDevice->SetTexture(0, m_apTexStar[0]);
        m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
            , 2, VertexFinal, sizeof(TVERTEX));
        m_pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);


#if 0// デバッグ用にテクスチャを見る
        for (int i = 0; i < 4; i++) {
            TVERTEX VertexFinal2[4] = {
                //x  y  z rhw    tu        tv
                { 0,     0 + h * (FLOAT)i / 8, 0, 1, 0 + 0.5f / w, 0 + 0.5f / h,},
                { w / 8,   0 + h * (FLOAT)i / 8, 0, 1, 1 + 0.5f / w, 0 + 0.5f / h,},
                { w / 8, h / 8 + h * (FLOAT)i / 8, 0, 1, 1 + 0.5f / w, 1 + 0.5f / h,},
                { 0,   h / 8 + h * (FLOAT)i / 8, 0, 1, 0 + 0.5f / w, 1 + 0.5f / h,},
            };
            m_pd3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_TEX1);
            switch (i) {
            case 0:m_pd3dDevice->SetTexture(0, m_pTexSceneScaled); break;
            case 1:m_pd3dDevice->SetTexture(0, m_pTexBrightPass); break;
            case 2:m_pd3dDevice->SetTexture(0, m_pTexStarSource); break;
            case 3:m_pd3dDevice->SetTexture(0, m_apTexStar[0]); break;
            }
            m_pd3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN
                , 2, VertexFinal2, sizeof(TVERTEX));
        }
#endif

        // 適当に元に戻す
        m_pd3dDevice->SetRenderState(D3DRS_ZENABLE, TRUE);

        // ヘルプの表示
        RenderText();

        // 描画の終了
        m_pd3dDevice->EndScene();
    }

    // サーフェスを解放する
    SAFE_RELEASE(pBackBuffer);

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
    SAFE_RELEASE(m_pSurfScene);
    SAFE_RELEASE(m_pTexScene);
    SAFE_RELEASE(m_pSurfSceneScaled);
    SAFE_RELEASE(m_pTexSceneScaled);
    SAFE_RELEASE(m_pSurfBrightPass);
    SAFE_RELEASE(m_pTexBrightPass);
    SAFE_RELEASE(m_pSurfStarSource);
    SAFE_RELEASE(m_pTexStarSource);
    for (int i = 0; i < NUM_STAR_TEXTURES; i++) {
        SAFE_RELEASE(m_apSurfStar[i]);
        SAFE_RELEASE(m_apTexStar[i]);
    }

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
    SAFE_RELEASE(m_pEffect);      // シェーダ
    SAFE_RELEASE(m_pDecl);		// 頂点宣言
    SAFE_RELEASE(m_pNormalMap);

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





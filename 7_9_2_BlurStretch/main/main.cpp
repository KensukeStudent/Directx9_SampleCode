//-------------------------------------------------------------
// File: main.cpp
//
// Desc: モーションブラー
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

#include <iostream>
#include <algorithm>

#define MAP_WIDTH	512
#define MAP_HEIGHT	512


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

    m_pEffect = NULL;
    m_hTechnique = NULL;
    m_hmWV = NULL;
    m_hmVP = NULL;
    m_hmLastWV = NULL;
    m_hvLightDir = NULL;
    m_hvEyePos = NULL;
    m_hvCol = NULL;

    m_fWorldRotX = -0.54f;
    m_fWorldRotY = 1.41f;
    m_fViewZoom = 4.0f;

    m_dwCreationWidth = 512;
    m_dwCreationHeight = 512;
    m_strWindowTitle = TEXT("main");
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
    m_bStartFullscreen = false;
    m_bShowCursorWhenFullscreen = false;

    m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
    m_bLoadingApp = TRUE;

    ZeroMemory(&m_UserInput, sizeof(m_UserInput));

    m_fUfoPos = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    m_fUfoRot = D3DXVECTOR3(0.0f, 3.0f * m_fTime, 0.0f);
    m_fUfoPos2 = D3DXVECTOR3(1.0f, 1.0f, 0.0f);

    m_fUfoPos_lerp = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
    m_fUfoRot_lerp = D3DXVECTOR3(0.0f, 3.0f * m_fTime, 0.0f);
    m_fUfoPos2_lerp = D3DXVECTOR3(1.0f, 1.0f, 0.0f);
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

    // 車の読み込み
    if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("ufo.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("ufo.x load Error"), MB_OK);
        return hr;
    }

    // 地面の読み込み
    if (FAILED(hr = m_pMeshBg->Create(m_pd3dDevice, _T("map.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("map.x load Error"), MB_OK);
        return hr;
    }

    // シェーダの読み込み
    LPD3DXBUFFER pErr;
    if (FAILED(hr = D3DXCreateEffectFromFile(m_pd3dDevice, _T("hlsl.fx"), NULL, NULL, D3DXSHADER_DEBUG, NULL, &m_pEffect, &pErr))) {
        // シェーダの読み込みの失敗
        MessageBoxA(NULL, (LPCSTR)pErr->GetBufferPointer(), "Shader Load ERROR", MB_OK);
    }
    else {
        m_hTechnique = m_pEffect->GetTechniqueByName("TShader");
        m_hmWV = m_pEffect->GetParameterByName(NULL, "mWV");
        m_hmWV2 = m_pEffect->GetParameterByName(NULL, "mWV2");
        m_hmVP = m_pEffect->GetParameterByName(NULL, "mVP");
        m_hmLastWV = m_pEffect->GetParameterByName(NULL, "mLastWV");
        m_hvLightDir = m_pEffect->GetParameterByName(NULL, "vLightDir");
        m_hvEyePos = m_pEffect->GetParameterByName(NULL, "vEyePos");
        m_hvCol = m_pEffect->GetParameterByName(NULL, "vCol");
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
    // メッシュ
    m_pMesh->RestoreDeviceObjects(m_pd3dDevice);
    m_pMeshBg->RestoreDeviceObjects(m_pd3dDevice);

    // シェーダ
    m_pEffect->OnResetDevice();

    // 質感の設定
    D3DMATERIAL9 mtrl;
    D3DUtil_InitMaterial(mtrl, 1.0f, 0.0f, 0.0f);
    m_pd3dDevice->SetMaterial(&mtrl);


    // レンダリング状態の設定
    RS(D3DRS_DITHERENABLE, FALSE);
    RS(D3DRS_SPECULARENABLE, FALSE);
    RS(D3DRS_ZENABLE, TRUE);
    RS(D3DRS_AMBIENT, 0x000F0F0F);
    RS(D3DRS_LIGHTING, TRUE);

    TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);
    TSS(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
    TSS(0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE);
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

	m_sleepTime += m_fElapsedTime;

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
    // UFO座標の設定
    //---------------------------------------------------------
	m_fUfoPos  = D3DXVECTOR3(1.0f, 0.0f, 0.0f);
	m_fUfoRot  = D3DXVECTOR3(0.0f, 3.0f * m_fTime, 0.0f);
	m_fUfoPos2 = D3DXVECTOR3(1.0f, 1.0f, 0.0f);

    //---------------------------------------------------------
    // 回転を m_fUfoRot_lerp → m_fUfoRot に向かって補間
    //---------------------------------------------------------
    // 現在の差分
    float oldDiff = m_fUfoRot.y - m_fUfoRot_lerp.y;
    // 目標の差分
    const float targetDiff = 0.3f;

    // 0除算防止 + α を [0,1] にクランプ
    float alpha = 0.0f;
    if (fabsf(oldDiff) > targetDiff && fabsf(oldDiff) > 1e-6f) {
        // lerpを使用して目標地点との差分を計算: 公式
        // newDiff = end − (start + α·(end − start))
        // newDiff = (end − start) − α·(end − start)
		// newDiff = (1 − α)·(end − start) 
		// targetDiff = (1 - alpha)·oldDiff
		// alpha = 1 - targetDiff / oldDiff
        alpha = 1.0f - targetDiff / fabsf(oldDiff);
        alpha = std::clamp(alpha, 0.0f, 1.0f);
    }

    // 補間実行
    D3DXVec3Lerp(&m_fUfoRot_lerp,
        &m_fUfoRot_lerp,
        &m_fUfoRot,
        alpha);

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

    pUserInput->bDispersionUp = (m_bActive && (GetAsyncKeyState(VK_PRIOR) & 0x8000) == 0x8000);
    pUserInput->bDispersionDown = (m_bActive && (GetAsyncKeyState(VK_NEXT) & 0x8000) == 0x8000);
}


//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    D3DXMATRIX m, mT, mR, mL, mL2, mView, mProj;
    D3DXVECTOR4 v, light_pos;

    //---------------------------------------------------------
    // 描画
    //---------------------------------------------------------
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
        m_pd3dDevice->Clear(0L, NULL
            , D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER
            , D3DCOLOR_RGBA(0, 0, 0, 0), 1.0f, 0L);

        //-------------------------------------------------
        // 普通のシーンの描画
        //-------------------------------------------------
        m_pd3dDevice->SetTransform(D3DTS_WORLD, &m_mWorld);
        m_pd3dDevice->SetTransform(D3DTS_VIEW, &m_mView);
        m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &m_mProj);

        TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        m_pMeshBg->Render(m_pd3dDevice);

        // 飛行モデルの描画
        TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        TSS(0, D3DTSS_COLORARG1, D3DTA_DIFFUSE);
        TSS(0, D3DTSS_ALPHAOP, D3DTOP_SELECTARG1);
        TSS(0, D3DTSS_ALPHAARG1, D3DTA_DIFFUSE);

        //-------------------------------------------------
        // 引き伸ばしたモデルの表示
        //-------------------------------------------------
        if (m_pEffect != NULL)
        {
            m_pEffect->SetTechnique(m_hTechnique);
            m_pEffect->Begin(NULL, 0);

            // ローカル-ワールド行列: ufo
            D3DXMatrixTranslation(&m, m_fUfoPos.x, m_fUfoPos.y, m_fUfoPos.z);
            D3DXMatrixRotationY(&mR, m_fUfoRot.y);
            D3DXMatrixTranslation(&mT, m_fUfoPos2.x, m_fUfoPos2.y, m_fUfoPos2.z);
            mL = m * mR * mT * m_mWorld;

            // ローカル-ワールド行列: lerp ufo
            D3DXMatrixRotationY(&mR, m_fUfoRot_lerp.y);
			mL2 = m * mR * mT * m_mWorld;

            // ライトの方向
            D3DXMatrixInverse(&m, NULL, &mL);
            light_pos = D3DXVECTOR4(1, 1, -1, 0);
            D3DXVec4Transform(&v, &light_pos, &m);
            v.w = 0;
            D3DXVec4Normalize(&v, &v);
            m_pEffect->SetVector(m_hvLightDir, &v);
            // 視点
            D3DXVECTOR3 vFromPt = D3DXVECTOR3(0.0f, 0.0f, -m_fViewZoom);
            D3DXVec3Transform(&v, &vFromPt, &m);
            m_pEffect->SetVector(m_hvEyePos, &v);
            // ローカル-射影変換行列
            m = mL * m_mView;
            m_pEffect->SetMatrix(m_hmWV, &m);
            m_pEffect->SetMatrix(m_hmVP, &m_mProj);

            // 現在位置のモデル描画
            DrawSubset(0);
            m_mLastWV = mL2 * m_mView;
            m_pEffect->SetMatrix(m_hmWV2, &m_mLastWV);
            DrawSubset(2);

            // モーションブラーの描画
            /*RS(D3DRS_ZWRITEENABLE, FALSE);
            RS(D3DRS_ALPHABLENDENABLE, TRUE);
            RS(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
            RS(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);
            m_mLastWV = mL2 * m_mView;
            m_pEffect->SetMatrix(m_hmLastWV, &m_mLastWV);
            DrawSubset(1);*/

            //if (m_sleepTime > 0.2f) {
            //    m_sleepTime = 0.0f;
            //    m_mLastWV = m;
            //}

            // m_mLastWV = m; Sleep()関数をオンにして上のif分をオフにすると遅延したブラーが確認可能

            m_pEffect->End();

            RS(D3DRS_ALPHABLENDENABLE, FALSE);
            RS(D3DRS_ZWRITEENABLE, TRUE);
        }
        // ヘルプの表示
        RenderText();

        // 描画の終了
        m_pd3dDevice->EndScene();
    }
    //Sleep(100);

    return S_OK;
}


void CMyD3DApplication::DrawSubset(int pass)
{
    D3DMATERIAL9* pMtrl;
    D3DXVECTOR4 v;
    DWORD i;

    m_pEffect->BeginPass(pass);// パスを変えて表示
    pMtrl = m_pMesh->m_pMaterials;
    for (i = 0; i < m_pMesh->m_dwNumMaterials; i++) {
        v.x = pMtrl->Diffuse.r;
        v.y = pMtrl->Diffuse.g;
        v.z = pMtrl->Diffuse.b;
        v.w = pMtrl->Diffuse.a;
        m_pEffect->SetVector(m_hvCol, &v);
        m_pMesh->m_pLocalMesh->DrawSubset(i);  // 描画
        pMtrl++;
    }

    m_pEffect->EndPass();
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
    fNextLine -= 20.0f;
    _stprintf_s(szMsg, MAX_PATH, _T("ufo rotY: %.2f  ufo_lerpRotY: %.2f diff:%.2f"), m_fUfoRot.y, m_fUfoRot_lerp.y, m_fUfoRot.y - m_fUfoRot_lerp.y);
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





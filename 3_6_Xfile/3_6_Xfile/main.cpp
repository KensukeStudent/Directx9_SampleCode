//-------------------------------------------------------------
// File: main.cpp
//
// Desc: Xファイル
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
    m_pMesh = new CD3DMesh();// ★追加

    m_dwCreationWidth = 300;
    m_dwCreationHeight = 300;
    m_strWindowTitle = TEXT("main");
    m_d3dEnumeration.AppUsesDepthBuffer = TRUE;
    m_bStartFullscreen = false;
    m_bShowCursorWhenFullscreen = false;

    m_pFont = new CD3DFont(_T("Arial"), 12, D3DFONT_BOLD);
    m_bLoadingApp = TRUE;
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
//        ウィンドウの初期化やIDirect3D9の初期化は終わってます。
//        ただ、LPDIRECT3DDEVICE9 の初期化は終わっていません。
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

    BOOL bCapsAcceptable;

    // グラフィックスボードがプログラムを実行する能力があるか確認する
    bCapsAcceptable = TRUE;

    if (bCapsAcceptable)
        return S_OK;
    else
        return E_FAIL;
}

//-------------------------------------------------------------
// Name: InitDeviceObjects()
// Desc: デバイスが生成された後の初期化をします。
//        フレームバッファフォーマットやデバイスの種類が変わった
//        後に通過します。
//        ここで確保したメモリはDeleteDeviceObjects()で開放します
//-------------------------------------------------------------
HRESULT CMyD3DApplication::InitDeviceObjects()
{
    HRESULT hr;

    // ★★★追加：Xファイルの読み込み
    if (FAILED(hr = m_pMesh->Create(m_pd3dDevice, _T("map.x"))))
    {
        _com_error err(hr);
        LPCTSTR errMsg = err.ErrorMessage();
        MessageBox(nullptr, errMsg, _T("Error"), MB_OK);
        return hr;
    }

    // フォント
    m_pFont->InitDeviceObjects(m_pd3dDevice);

    return S_OK;
}

//-------------------------------------------------------------
// Name: RestoreDeviceObjects()
// Desc: 画面のサイズが変更された時等に呼ばれます。
//        確保したメモリはInvalidateDeviceObjects()で開放します。
//-------------------------------------------------------------
HRESULT CMyD3DApplication::RestoreDeviceObjects()
{
    m_pMesh->RestoreDeviceObjects(m_pd3dDevice);// ★★★追加

    // 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState

// レンダリング状態の設定
    RS(D3DRS_DITHERENABLE, FALSE);
    RS(D3DRS_SPECULARENABLE, FALSE);
    RS(D3DRS_ZENABLE, TRUE);
    RS(D3DRS_LIGHTING, FALSE);

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
    return S_OK;
}




//-------------------------------------------------------------
// Name: Render()
// Desc: 画面を描画する.
//-------------------------------------------------------------
HRESULT CMyD3DApplication::Render()
{
    D3DXMATRIXA16 mWorld, mView, mProj;

    // 画面をクリアする
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0x00), 1.0f, 0L);

    //---------------------------------------------------------
    // 描画
    //---------------------------------------------------------
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
        // ---------------------------------------------------------
        // ワールド行列を設定する
        // ---------------------------------------------------------
        D3DXMATRIX mWorld;
        D3DXMatrixIdentity(&mWorld);
        m_pd3dDevice->SetTransform(D3DTS_WORLD, &mWorld);

        // ---------------------------------------------------------
        // ビュー行列を設定する
        // ---------------------------------------------------------
        D3DXVECTOR3 vEyePt(0.0f, 5.0f, -10.0f); // 注目点
        D3DXVECTOR3 vLookatPt(0.0f, 0.0f, 0.0f); // カメラの位置
        D3DXVECTOR3 vUp(0.0f, 1.0f, 0.0f); // 上向き方向
        D3DXMatrixLookAtLH(&mView, &vEyePt, &vLookatPt, &vUp);
        m_pd3dDevice->SetTransform(D3DTS_VIEW, &mView);

        // ---------------------------------------------------------
        // 射影行列を設定する
        // ---------------------------------------------------------
        D3DXMatrixPerspectiveFovLH(&mProj
            , D3DX_PI / 4      // 視野角
            , 1.0f           // アスペクト比
            , 1.0f, 100.0f);// 前方、後方クリップ面
        m_pd3dDevice->SetTransform(D3DTS_PROJECTION, &mProj);

        // ---------------------------------------------------------
        // ★★★追加:モデルを描画する
        // ---------------------------------------------------------
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState
        TSS(0, D3DTSS_COLOROP, D3DTOP_SELECTARG1);
        TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
        SAMP(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
        SAMP(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);

        m_pMesh->Render(m_pd3dDevice);

        RenderText();    // 画面の状態やヘルプを表示する

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

    // 画面の状態を表示する
    FLOAT fNextLine = 40.0f;

    lstrcpy(szMsg, m_strDeviceStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    lstrcpy(szMsg, m_strFrameStats);
    fNextLine -= 20.0f;
    m_pFont->DrawText(2, fNextLine, fontColor, szMsg);

    // 操作法やパラメータを表示する
    fNextLine = (FLOAT)m_d3dsdBackBuffer.Height;
    lstrcpy(szMsg, TEXT("Press 'F2' to configure display"));
    fNextLine -= 20.0f; m_pFont->DrawText(2, fNextLine, fontColor, szMsg);
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
    m_pMesh->InvalidateDeviceObjects();	// ★★★追加

    m_pFont->InvalidateDeviceObjects();    // フォント

    return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    m_pMesh->Destroy();	// ★★★追加

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
    SAFE_DELETE(m_pMesh);// ★★★追加

    SAFE_DELETE(m_pFont);    // フォント

    return S_OK;
}





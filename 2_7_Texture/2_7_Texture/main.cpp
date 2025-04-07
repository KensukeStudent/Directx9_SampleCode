//-------------------------------------------------------------
// File: main.cpp
//
// Desc: テクスチャを張る
//-------------------------------------------------------------
#define STRICT
#include <windows.h>
#include <commctrl.h>
#include <commdlg.h>
#include <basetsd.h>
#include <math.h>
#include <stdio.h>
#include <d3dx9.h>
#include <dxerr.h> // DXErr9.h -> DXErr.h に変更
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
// 頂点フォーマット
//-------------------------------------------------------------
#define D3DFVF_CUSTOMVERTEX (D3DFVF_XYZRHW|D3DFVF_DIFFUSE|D3DFVF_TEX1)// ★★★ 変更

typedef struct {
    FLOAT x, y, z, rhw;    // スクリーン座標での位置
    DWORD color;           // 頂点色
    FLOAT tu, tv;          // ★★★ 追加：テクスチャー座標
} CUSTOMVERTEX;



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
    m_pTexture = NULL;
    m_pVB = NULL;

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
	// HRESULT hr = D3DUtil_CreateTexture(m_pd3dDevice, TEXT("texture.bmp"), &m_pTexture);

    //IDirect3DTexture9* pTexture;
    //D3DXIMAGE_INFO info;
    //// ファイルからテクスチャーを生成する
    //HRESULT hr = D3DXCreateTextureFromFileExA(
    //    m_pd3dDevice,	// Direct3DDevice
    //    "earth.bmp",		// ファイル名
    //    D3DX_DEFAULT,		// 横幅(D3DX_DEFAULTでファイルから判定)
    //    D3DX_DEFAULT,		// 高さ(D3DX_DEFAULTでファイルから判定)
    //    1,			// ミップマップの数
    //    0,			// 使用用途
    //    D3DFMT_A8R8G8B8,	// フォーマット
    //    D3DPOOL_MANAGED,	// メモリの管理設定
    //    D3DX_FILTER_NONE,	// フィルター設定
    //    D3DX_DEFAULT,		// ミップマップフィルターの設定
    //    0x00000000,		// カラーキー
    //    &info,			// 画像情報
    //    NULL,			// パレットデータ
    //    &pTexture);		// 生成したテクスチャーの格納先


    HRESULT hr = NULL;

    // ★★★ 追加：テクスチャーを読み込む
    if (FAILED(hr))    // テクスチャオブジェクト
        return E_FAIL;

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
    // 頂点と色情報
    CUSTOMVERTEX vertices[] = {
        //   x,     y,      z,  rhw,   頂点の色   ( 赤    緑   青   α )  U  V
        {  50.0f,  50.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0xff), 0, 0},
        { 250.0f,  50.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0xff), 1, 0},
        {  50.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0xff), 0, 1},
        { 250.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_RGBA(0xff,0xff,0xff,0xff), 1, 1},
    };

    // 頂点バッファを生成する
    if (FAILED(m_pd3dDevice->CreateVertexBuffer(
        4 * sizeof(CUSTOMVERTEX),        // 頂点バッファのサイズ
        0, D3DFVF_CUSTOMVERTEX,        // 使用法、頂点フォーマット
        D3DPOOL_DEFAULT,            // メモリクラス
        &m_pVB, NULL)))            // 頂点バッファリソース 予約済 
        return E_FAIL;

    // 頂点バッファに情報を格納する
    VOID* pVertices;
    if (FAILED(m_pVB->Lock(0, sizeof(vertices), (void**)&pVertices, 0)))
        return E_FAIL;
    memcpy(pVertices, vertices, sizeof(vertices));
    m_pVB->Unlock();

    // 長いから短縮形を作ってみた
#define RS   m_pd3dDevice->SetRenderState
#define TSS  m_pd3dDevice->SetTextureStageState
#define SAMP m_pd3dDevice->SetSamplerState

// レンダリング状態の設定
    RS(D3DRS_DITHERENABLE, FALSE);
    RS(D3DRS_SPECULARENABLE, FALSE);
    RS(D3DRS_ZENABLE, TRUE);
    RS(D3DRS_AMBIENT, 0x000F0F0F);

    // ★★★ 追加：テクスチャーに関する設定
    TSS(0, D3DTSS_COLOROP, D3DTOP_MODULATE);
    TSS(0, D3DTSS_COLORARG1, D3DTA_TEXTURE);
    TSS(0, D3DTSS_COLORARG2, D3DTA_DIFFUSE);

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
    // 画面をクリアする
    m_pd3dDevice->Clear(0L, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER,
        0x000000ff, 1.0f, 0L);

    //---------------------------------------------------------
    // 描画
    //---------------------------------------------------------
    if (SUCCEEDED(m_pd3dDevice->BeginScene()))
    {
        m_pd3dDevice->SetTexture(0, m_pTexture);// ★追加:テクスチャの設定
        m_pd3dDevice->SetStreamSource(0, m_pVB, 0, sizeof(CUSTOMVERTEX));
        m_pd3dDevice->SetFVF(D3DFVF_CUSTOMVERTEX);
        m_pd3dDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP, 0, 2);

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
    SAFE_RELEASE(m_pVB);        // 頂点バッファ

    m_pFont->InvalidateDeviceObjects();    // フォント

    return S_OK;
}




//-------------------------------------------------------------
// Name: DeleteDeviceObjects()
// Desc: InitDeviceObjects() で作成したオブジェクトを開放する
//-------------------------------------------------------------
HRESULT CMyD3DApplication::DeleteDeviceObjects()
{
    SAFE_RELEASE(m_pTexture);   // ★追加:テクスチャの開放

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
    SAFE_DELETE(m_pFont);    // フォント

    return S_OK;
}





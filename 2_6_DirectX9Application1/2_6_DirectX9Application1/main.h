//-----------------------------------------------------------------------------
// File: DirectX9Application1.h
//
// Desc: DirectX AppWizard によって生成されたウィンドウアプリケーション
//-----------------------------------------------------------------------------
#pragma once




//-----------------------------------------------------------------------------
// 定義や定数
//-----------------------------------------------------------------------------
// TODO: "DirectX AppWizard Apps" をあなたの名前や会社名にしてください。
#define DXAPP_KEY        TEXT("Software\\DirectX AppWizard Apps\\DirectX9Application1")

// 現在の入力データを保存する構造体
struct UserInput
{
    // TODO: 必要に応じて書きかえる
    BOOL bRotateUp;
    BOOL bRotateDown;
    BOOL bRotateLeft;
    BOOL bRotateRight;
};



//-------------------------------------------------------------
// Name: class CMyD3DApplication
// Desc: アプリケーションのクラス
//-------------------------------------------------------------
class CMyD3DApplication : public CD3DApplication
{
    BOOL                    m_bLoadingApp;          // ロード中？
    CD3DFont* m_pFont;                // フォント
    ID3DXMesh* m_pD3DXMesh;            // ティーポットのメッシュ
    UserInput               m_UserInput;            // 入力データ

    FLOAT                   m_fWorldRotX;           // Ｘ軸回転
    FLOAT                   m_fWorldRotY;           // Ｙ軸回転

protected:
    // 一度だけ行われる初期化処理
    virtual HRESULT OneTimeSceneInit();
    // LPDIRECT3DDEVICE9 が変更されたときなどに呼ばれる初期化処理
    virtual HRESULT InitDeviceObjects();
    // 画面のサイズが変更されたときなどに呼ばれる初期化処理
    virtual HRESULT RestoreDeviceObjects();
    // RestoreDeviceObjects に対応した後片付けの処理
    virtual HRESULT InvalidateDeviceObjects();
    // InitDeviceObjects に対応した後片付けの処理
    virtual HRESULT DeleteDeviceObjects();
    // 画面を描画する
    virtual HRESULT Render();
    // キー入力を処理したり、動きの制御をする
    virtual HRESULT FrameMove();
    // 一番最後に後片付けをする
    virtual HRESULT FinalCleanup();
    // サポートされている機能を調べてプログラムが動くか確認する
    virtual HRESULT ConfirmDevice(D3DCAPS9*, DWORD, D3DFORMAT);

    // デバイスの情報を表示するデバッグ用のメッセージ関数
    HRESULT RenderText();

    // キー入力を更新する
    void    UpdateInput(UserInput* pUserInput);
public:
    // 各種イベントを処理する
    LRESULT MsgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    CMyD3DApplication();           // コンストラクタ
    virtual ~CMyD3DApplication();  // デストラクタ
};


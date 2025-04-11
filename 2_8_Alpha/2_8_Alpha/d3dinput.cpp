#include "d3dinput.h"
#include <Windows.h>
#include <dinput.h>

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

// キー情報構造体
struct INPUTSTATE
{
	DWORD now;
	DWORD trg;
	DWORD ntrg;
};

// DIRECTINPUT8のポインタ
static LPDIRECTINPUT8 g_pInputInterface;

// DIRECTINPUTDEVICE8のポインタ
static LPDIRECTINPUTDEVICE8 g_pKeyDevice;

// キー情報
static INPUTSTATE g_InputState;

bool InitDirectInput(HINSTANCE instance_handle, HWND window_handle)
{
	// IDirectInput8インターフェイスの取得
	HRESULT hr = DirectInput8Create(instance_handle,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&g_pInputInterface,
		NULL);
	if (FAILED(hr))
	{
		MessageBoxW(window_handle, L"error", L"エラーA", MB_OK);
		return false;
	}

	// IDirectInputDevice8インターフェイスの取得
	hr = g_pInputInterface->CreateDevice(GUID_SysKeyboard, &g_pKeyDevice, NULL);
	if (FAILED(hr))
	{
		MessageBoxW(window_handle, L"error", L"エラーB", MB_OK);
		return false;
	}

	// デバイスのフォーマットの設定
	hr = g_pKeyDevice->SetDataFormat(&c_dfDIKeyboard);
	if (FAILED(hr))
	{
		MessageBoxW(window_handle, L"error", L"エラーC", MB_OK);
		return false;
	}

	// 協調モードの設定
	hr = g_pKeyDevice->SetCooperativeLevel(window_handle, DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
	if (FAILED(hr))
	{
		MessageBoxW(window_handle, L"error", L"エラーD", MB_OK);
		return false;
	}

	// デバイスの取得開始
	g_pKeyDevice->Acquire();

	return true;
}

void ReleaseDirectInput()
{
	g_pKeyDevice->Unacquire();
	g_pKeyDevice->Release();
	g_pInputInterface->Release();
}

void KeyUpdate()
{
	// キー情報取格納用
	BYTE KeyState[256];
	HRESULT hr;

	// キーボードデバイスのゲッター
	hr = g_pKeyDevice->GetDeviceState(256, KeyState);
	if (SUCCEEDED(hr))
	{
		// 1フレーム前のキー情報の確保
		DWORD old = g_InputState.now;

		// キー情報クリア
		g_InputState.now = CLEAR_KEY;

		// 上キー
		if (KeyState[DIK_UP] & 0x80)
		{
			g_InputState.now |= UP_KEY;
		}

		// 下キー
		if (KeyState[DIK_DOWN] & 0x80)
		{
			g_InputState.now |= DOWN_KEY;
		}

		// 左キー
		if (KeyState[DIK_LEFT] & 0x80)
		{
			g_InputState.now |= LEFT_KEY;
		}

		// 右キー
		if (KeyState[DIK_RIGHT] & 0x80)
		{
			g_InputState.now |= RIGHT_KEY;
		}

		// リターンキー
		if (KeyState[DIK_RETURN] & 0x80)
		{
			g_InputState.now |= RETURN_KEY;
		}

		g_InputState.trg = (g_InputState.now & (~old));	// トリガー情報取得
		g_InputState.ntrg = (~g_InputState.now) & old;	// 逆トリガー情報取得

	}
	else if (hr == DIERR_INPUTLOST) {
		g_pKeyDevice->Acquire();
	}
}

bool GetKey(DWORD key_code)
{
	return g_InputState.now & key_code;
}

bool GetKeyDown(DWORD key_code)
{
	return g_InputState.trg & key_code;
}

bool GetKeyUp(DWORD key_code)
{
	return g_InputState.ntrg & key_code;
}

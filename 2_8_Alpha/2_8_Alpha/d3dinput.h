#ifndef DIRECT_INPUT_H_
#define DIRECT_INPUT_H_

#include <Windows.h>

// キー情報GetKeyシリーズの関数で使用する
#define CLEAR_KEY	0x0000		// キークリア用
#define UP_KEY		0x0001		// 上キー
#define DOWN_KEY	0x0002		// 下キー
#define LEFT_KEY	0x0004		// 左キー
#define RIGHT_KEY	0x0008		// 右キー
#define RETURN_KEY	0x0010		// リターンキー

/*
	DirectInputの初期化
		戻り値：
			成功 => true
			失敗 => false

		引数：
			HINSTANCE instance_handle
				対象のアプリのインスタンスハンドル

			HWND window_handle
				ウィンドウハンドル

		内容：
			DirectInputを初期化する
			成功したらtrue、失敗したらfalse
*/
bool InitDirectInput(HINSTANCE instance_handle, HWND window_handle);

/*
	DirectInputの解放
		戻り値：
			なし

		引数：
			なし

		内容：
			DirectInputを解放する
*/
void ReleaseDirectInput();

/*
	キー情報の更新
		戻り値：
			なし

		引数：
			なし

		内容：
			キーの情報を更新してGetKeyシリーズの関数の取得情報を最新にする
			この処理は毎フレームの開始時に呼び出すこと
*/
void KeyUpdate();

/*
	キーを押してるかを判定
		戻り値：
			押されてる => true
			押されてない => false

		引数：
			DWORD key_code
				判断したいキーコード

		内容
			キーが押されているならtrue、押されていないならfalseを返す
*/
bool GetKey(DWORD key_code);

/*
	キーを押した瞬間を判定
		戻り値：
			押した瞬間 => true
			押した瞬間じゃない => false

		引数：
			DWORD key_code
				判断したいキーコード

		内容
			キーが押された瞬間ならtrue、押されてない瞬間ならfalse
*/
bool GetKeyDown(DWORD key_code);

/*
	キーを離した瞬間を判定
		戻り値：
			離した瞬間 => true
			離した瞬間じゃない => false

		引数：
			DWORD key_code
				判断したいキーコード

		内容
			キーが離した瞬間ならtrue、離した瞬間じゃないならfalse
*/
bool GetKeyUp(DWORD key_code);

#endif

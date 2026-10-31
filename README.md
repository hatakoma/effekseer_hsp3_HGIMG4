# Effekseer for HGIMG4

本プロジェクトは、Effekseerの非公式HSP3連携ライブラリです。Effekseer Projectによる公式配布・公認を示すものではありません。

Effekseer 1.80.7を、Windows版HGIMG4／HGIMG4DXの画面に直接描画する32bit DLLです。公式ランタイムを静的リンクし、HSPから呼べるC ABIを追加しています。HGIMG4本体の変更は不要です。OpenGL用とANGLE用の2つのDLLを同梱しています。

## 起動

**床・球・箱との粒子衝突に対応しました。** OpenGL版は`dist/run_collision.cmd`、ANGLE版は`dist/run_collision_dx.cmd`で試せます。[衝突API・使い方・制限](dist/COLLISION.md)を参照してください。GPUパーティクル対応は保留です。

使用するランタイムに合わせて以下を選びます。HSPの配置先は既定で`C:\hsp37`です。異なる場合は環境変数`HSP_DIR`を設定します。

| 実行環境 | DLL / HSPヘッダー | 起動 / サンプル |
|---|---|---|
| OpenGL: `hgimg4.as` / `hsp3gp.exe` | `effekseer_hgimg4.dll` / `effekseer_hgimg4.as` | `dist/run.cmd` / `dist/sample.hsp` |
| ANGLE: `hgimg4dx.as` / `hsp3gpdx.exe` | `effekseer_hgimg4dx.dll` / `effekseer_hgimg4dx.as` | `dist/run_dx.cmd` / `dist/sample_dx.hsp` |

`.hsp`をHSP 3.7エディタで開いてF5でも起動できます。`sample_dx.hsp`は共通サンプルの`sample.hsp`をincludeするため、両方を同じフォルダーに置いてください。

Spaceで再生、Escで終了します。左右の立方体はHGIMG4、中央のレーザーはEffekseerで描画しています。`dist/preview.png`と`dist/preview_dx.png`は、それぞれOpenGL／ANGLEで取得した実際の描画結果です。

必要環境: Windows、HSP 3.7の32bitランタイム。OpenGL版はOpenGL 3.1以上、ANGLE版はHSP 3.7に付属するANGLE/OpenGL ES 2.0を使用します。64bit HSP、Android、iOS、Web向けDLLではありません。

ANGLE版はHGIMG4DXが読み込んだ`libEGL.dll`と`libGLESv2.dll`をそのまま共有します。別のANGLEを同梱・自動ダウンロードする処理はありません。実行ファイルとして配布する場合は、通常のHGIMG4DXと同様にHSP付属の`libEGL.dll`、`libGLESv2.dll`、`d3dcompiler_47.dll`も用意してください。このZIPにはそれらを含めていません。ランタイムと異なる版のANGLEを混在させないでください。

## 自分のプログラムへの組み込み

選択したDLLとHSPヘッダーをスクリプトと同じフォルダーに配置します。ANGLE用ヘッダーは共通の`effekseer_hgimg4.as`もincludeするため、両方の`.as`を配置してください。DLLは使用する側の1つだけで動きます。実行ファイルへまとめる場合もDLLは外部ファイルとして配置してください。エフェクトと参照テクスチャ・モデル等も、相対フォルダー構造を保って通常のファイルとして配置します。DPM内ファイルの読み込みは対応していません。

ANGLE版では冒頭を次の2行にします。以降の`efk_*`命令・引数・戻り値はOpenGL版と同じです。1つのスクリプトで両方のヘッダーをincludeしないでください。対応する描画コンテキストがない場合、`efk_init`はエラーを返します。

```hsp
#include "hgimg4dx.as"
#include "effekseer_hgimg4dx.as"
```

以下はOpenGL版の呼び出し順です。

```hsp
#include "hgimg4.as"
#include "effekseer_hgimg4.as"

gpreset
efk_init 8000
; 各命令のstatが負ならefk_last_errorで取得して処理を中断する
efk_load "assets/Laser01.efkefc",0.5
effect_id=stat
efk_play effect_id,0.0,0.0,0.0
playback_id=stat

; 毎フレーム
efk_update 1.0
redraw 0
gpdraw
efk_draw
; HUD等はこの後に描画
redraw 1

; 実際にはメインループ終了時・ウィンドウ終了時に呼ぶ
efk_stop_all
efk_release effect_id
efk_shutdown
```

これは呼び出し順の抜粋です。ループ・終了処理・エラー判定は`sample.hsp`を参照してください。`efk_init`は画面とOpenGLコンテキストの作成後に呼び、`efk_shutdown`は画面を破棄する前に呼びます。全APIを初期化時と同じスレッド／コンテキストで使用してください。別スレッドからの同時呼び出しは非対応です。

カメラは自動追従しません。HGIMG4と同じ位置・注視点を`efk_camera`に、同じ視野角・縦横比・near/farを`efk_perspective`に渡します。座標は右手系、Y上向き、回転はラジアンです。両者のカメラが一致すれば、現在の描画先・深度バッファを共有できます。半透明物体と粒子をまたぐ自動ソートは行いません。

## API

戻り値は`stat`。原則0が成功、-1が失敗です。失敗後、次のAPIを呼ぶ前に以下で理由を取得します。

```hsp
sdim message,1024
efk_last_error message,1024
```

| 命令 | 引数と動作 |
|---|---|
| `efk_init capacity` | 最大粒子数1～65536。標準8000。二重初期化はエラー |
| `efk_load path,scale` | `.efkefc` / `.efk`読込。正のエフェクトIDを返す。scaleは正数 |
| `efk_release effect` | IDの参照を解放。再生中インスタンスは継続 |
| `efk_play effect,x,y,z` | 正の再生IDを返す。エフェクトIDとは別物 |
| `efk_exists playback` | 再生中なら1、停止済み・未知IDなら0 |
| `efk_stop playback` | 指定再生を停止。完了済みIDにはエラー |
| `efk_stop_all` | 全再生を停止 |
| `efk_location playback,x,y,z` | 再生位置 |
| `efk_rotation playback,x,y,z` | XYZ回転、ラジアン |
| `efk_scale playback,x,y,z` | XYZ拡大率 |
| `efk_speed playback,speed` | 再生速度、0以上 |
| `efk_pause playback,paused` | 1で停止、0で再開 |
| `efk_color playback,r,g,b,a` | 全体の色、各0～255 |
| `efk_update frames` | 60Hz換算の経過フレーム数。1.0で1/60秒、0～600 |
| `efk_camera x,y,z,tx,ty,tz` | 視点と注視点。上向きはY固定 |
| `efk_perspective fov,aspect,near,far` | 垂直FOVは度単位、aspectは幅/高さ |
| `efk_matrices view,projection` | 各`ddim ...,16`の行列。Effekseerの行ベクトル／行優先形式。OpenGLのZ範囲-1～1。LOD用の視点は先に`efk_camera`で設定 |
| `efk_draw` | 現在の描画先に描く。`redraw 0`～`redraw 1`内、通常`gpdraw`の後 |
| `efk_shutdown` | 全リソース解放。重複呼び出し可 |
| `efk_version` | 1807を返す |
| `efk_last_error buffer,bytes` | 直近エラーをコピー。bytesは実際のバッファ長以下にする |

小数引数は`double`で渡してください。行列ポインターや文字列バッファは、宣言されたサイズ以上を確保する必要があります。IDは再初期化後も使い回さず、古いIDから別のエフェクトを誤操作しない構造です。

## 対応範囲と制限

- 公式のスプライト、リボン、リング、トラック、モデル描画と、テクスチャ／モデル／カーブ／マテリアルのローダーを接続しています。
- 同梱レーザーで画像付き再生を確認。あらゆるマテリアル・モデル・旧バージョン素材の互換性を保証する検証はしていません。
- 通常粒子と床・球・軸平行ボックスの外部衝突、反発・摩擦に対応。形状はHSPから登録・更新します。詳細は`dist/COLLISION.md`。
- サウンド、GPUパーティクル、背景取り込みを必要とする歪み、シーン深度を必要とするソフトパーティクルは未対応です。該当機能を持つ素材を完全再現できません。
- 音声・GPU粒子等の未対応機能や参照画像の欠落を、ロード時に網羅的に拒否する検査はありません。ロード成功だけでは素材の完全表示を保証しません。
- パスはWindows ANSIコードページ（日本語WindowsではCP932）。UTF-8版HSP用の文字列インターフェースではありません。
- HGIMG4のOpenGL 3.1ではEffekseerのGLSL 1.20描画経路、3.3以上ではGLSL 3.30描画経路を選択します。ANGLE版はEffekseerのOpenGLES2／GLSL ES 1.00描画経路です。実機確認済みなのはOpenGL 3.1とHSP 3.7付属ANGLEのDirect3D9Ex経路です。OpenGL ES 3専用機能への対応ではありません。
- 画面コンテキストの交換・デバイスロスト復旧は未検証。`screen`や`gpreset`で描画環境を作り直す場合は先に終了処理してください。

## ビルドと検証

Visual Studio 2022のC++ x86ツールとWindows SDK、CMake 3.20以上、Gitを使用します。

```powershell
.\build.ps1
.\test.ps1 -HspDir C:\hsp37
```

`build.ps1`は不足時に公式ソースを取得し、タグ1807のコミット`b87d1a2e3ff3731ba25cad34f8d8abeae382b21f`を確認し、2つのDLLを生成します。CMakeがPATHにない場合は`-CMake "C:\...\cmake.exe"`を指定できます。DLLにEffekseerとC++ランタイムを静的リンクし、追加のVCランタイムDLLは不要です。ANGLE SDK／インポートライブラリのインストールも不要です。

ANGLE対応では、上流のWindows向けWGL関数ローダーを、ビルドフォルダー内の生成コピーでEGL/ANGLE向けに切り替えます。上流チェックアウトは編集しません。ES2で使えないVAO・バッファマップ等の機能を無効にし、ホストの頂点属性を保存・復元します。基本GL呼び出しの転送コードは`src/angle_dispatch.cpp`として同梱しているので、通常ビルドにPythonは不要です。C++向けには`src/effekseer_hgimg4.h`と、各DLLに対応する`build/Release/*.lib`を利用できます。

2026-10-31にWindows/HSP 3.7で両方のRelease DLLをビルドし、各643回のAPIチェック、描画状態復元15項目、無効ID・再初期化・自然終了・異なるバックエンドの誤使用拒否、各180フレームの混在描画を確認しました。ANGLEでは全頂点属性の有効状態・型・サイズ・正規化・ストライド・バッファ・ポインターの復元も比較しています。

確認環境はNVIDIA GeForce RTX 3060。OpenGL側は`3.1.0 NVIDIA 617.42`、ANGLE側は`OpenGL ES 2.0 (ANGLE 2.1.unknown hash)` / `Direct3D9Ex vs_3_0 ps_3_0`です。`dist/backend-info-gl.txt`と`dist/backend-info-dx.txt`に実行時の値を保存しています。

`test.ps1`は両方のランタイムで前回の結果を消して検証し、失敗時は例外で終了します。結果は`dist/api-result-gl.txt`、`api-result-dx.txt`、`smoke-result-gl.txt`、`smoke-result-dx.txt`。画像の生データは各`smoke-*.rgba`（800×600、RGBA、上下反転前）です。`preview*.png`はGPUのフレームバッファから取得した画像です。対話操作や全GPU機種を網羅した試験ではありません。

## 出典

独自ブリッジ、HSPヘッダー、サンプルコード、自作の雨粒素材、ビルド・テスト・生成スクリプト、ドキュメントは **[MIT License](LICENSE)** で公開します。

**Copyright © 2026 HAta/komatsuna-ya**

Effekseer、公式サンプル素材、HGIMG4標準シェーダー等の第三者部分には、各権利者のライセンスと著作権表記が適用されます。再配布時には`LICENSE`および`dist/licenses/`と`dist/THIRD_PARTY_NOTICES.md`を保持してください。

- [Effekseer公式](https://github.com/effekseer/Effekseer/tree/1807)
- [HGIMG4公式ガイド](https://www.onionsoft.net/hsp/v37/doclib/hgimg4.html)
- [ANGLE公式開発ガイド](https://github.com/google/angle/blob/main/doc/DevSetup.md)
- `dist/licenses/`と`dist/THIRD_PARTY_NOTICES.md`に同梱物のライセンスを収録しています。

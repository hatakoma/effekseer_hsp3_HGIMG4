# 粒子と床・球・箱の衝突

Windows 32bitのOpenGL版／ANGLE版で、通常のEffekseer粒子に外部衝突を追加しました。GPUパーティクル対応は今回保留です。

## 試す

- OpenGL: `run_collision.cmd` / `collision_sample.hsp`
- ANGLE: `run_collision_dx.cmd` / `collision_sample_dx.hsp`

Spaceで再スタート、Cで衝突をON/OFF、Escで終了。画面下部に形状ごとの接触回数を表示します。反発係数は0.7、摩擦は0.2です。自作の`assets/collision_rain.efk`は外部画像を必要としません。

## 組み込み

```hsp
#include "hgimg4dx.as"
#include "effekseer_hgimg4dx.as"
gpreset
efk_init 4000

; ID=0は新規作成。statに正の衝突形状IDが返る
efk_collider_floor 0,-2.0
floor_id=stat
efk_collider_sphere 0,2.0,0.0,0.0,1.25
sphere_id=stat
efk_collider_box 0,-2.0,0.0,0.0,2.0,2.0,2.0
box_id=stat

efk_load "assets/collision_rain.efk",1.0
rain=stat
; 読込後、再生前に設定。反発0～1、摩擦0～1
efk_collision_configure rain,1,0.7,0.2
efk_play rain,0.0,0.0,0.0

; 毎フレーム、必要なら同じIDで位置・大きさを更新
efk_collider_sphere sphere_id,2.0,0.0,0.0,1.25
efk_update 1.0
efk_collider_hits sphere_id
hits_this_frame=stat
; redraw 0 → gpdraw → efk_draw → redraw 1
; 終了時にはefk_shutdown
```

抜粋ではエラーチェックを省略しています。実際には各命令の`stat<0`を確認してください。完全な処理はサンプルを参照してください。HSPの表示用オブジェクトIDとDLLの衝突形状IDは別物です。`gpbox`等の物体は自動登録されません。表示用の位置を変える場合は、`efk_update`前に対応する衝突形状も更新します。

## API

| 命令 | 内容 |
|---|---|
| `efk_collider_floor id,height` | 上向きの無限水平面。上側から下へ入る粒子に反応 |
| `efk_collider_sphere id,x,y,z,radius` | 中心と半径 |
| `efk_collider_box id,x,y,z,sx,sy,sz` | 中心と各軸の**全幅**。回転なしの軸平行ボックス |
| `efk_collider_enable id,enabled` | 1で有効、0で無効。再生中にも切替可能 |
| `efk_collider_remove id` | 指定形状を削除。削除済みIDは再使用不可 |
| `efk_collider_clear` | 全形状を削除し、接触カウントをリセット |
| `efk_collision_configure effect,enabled,bounce,friction` | 全ノードの外部衝突と反発・摩擦を設定。再生中はエラー |
| `efk_collision_count` | 直近の`efk_update`で実粒子から発生した全接触回数 |
| `efk_collider_hits id` | 直近の`efk_update`における指定形状への接触回数 |
| `efk_collision_cast sx,sy,sz,ex,ey,ez,result` | 線分と形状の交差判定。`ddim result,7`を渡す |

作成／更新APIはIDを返します。既存IDを渡すと同じ種類の形状を更新し、有効・無効の設定を維持します。種類変更はエラーです。最大256形状、座標の絶対値は100万以下。半径と各全幅は0.0001以上。通常の成功値は0、失敗は-1。詳細は`efk_last_error`で取得できます。

`efk_collision_cast`は最も手前の衝突形状IDを返し、交差しなければ0。配列は`接触点X,Y,Z / 外向き法線X,Y,Z / 線分上の比率t（0～1）`です。交差しない場合は全要素を0にします。問い合わせでは接触カウントを増やしません。同距離なら小さいIDを優先します。

## 反応と設定

前回位置から次回位置への線分で衝突を調べ、接触点と法線をEffekseer公式の衝突コールバックに返します。跳ね返りと摩擦には公式の衝突処理を使用しています。

- **反発0**は法線方向の跳ね返りなし、**反発1**は法線方向の速さをほぼ維持して反転します。
- **摩擦0**は接線方向の速さを維持、**摩擦1**は接触時の接線方向の速さを除去します。空気抵抗ではありません。
- 設定値は粒子生成時に取り込まれます。同じエフェクトを再生中に設定変更することは拒否します。`efk_stop_all`等で停止してから設定し直してください。
- `efk_collision_configure`はファイル内の地面衝突を無効化し、この外部形状を使います。衝突による寿命短縮も0にします。`enabled=0`は衝突を無効にしますが、元のファイル設定を復元しません。元に戻すには再ロードします。
- エディタで外部衝突を有効にした素材は、上書きAPIを呼ばずに素材内の反発・摩擦設定を使うこともできます。
- 形状はワールド座標です。エフェクトの倍率・再生位置・回転とは独立して登録します。接触回数には同じ粒子の繰り返し接触も含みます。

## 制限

- 通常粒子の中心を点として扱います。画像やメッシュの大きさは衝突半径にしません。
- 球・箱の内部、床より下に最初からある粒子は、外へ出られるよう衝突を報告しません。生成位置を形状の外に置いてください。
- 1回の粒子更新で返す接触は最も手前の1件です。複数回の反射を1フレーム内で完全積分する物理エンジンではありません。大きな時間差は1.0フレーム以下に分割して`efk_update`してください。
- 形状の更新は移動先へ直接配置する方式です。動く形状自体の連続判定や、形状の速度を粒子に加える処理はありません。
- 回転ボックス、任意メッシュ、粒子同士、HGIMG4のBullet剛体との自動連携、GPU粒子の衝突は対象外です。
- 全有効形状を順に調べます。粒子数と形状数を同時に増やすとCPU負荷が増えます。

## 検証と素材生成

`build.ps1` → `test.ps1`で既存機能の回帰テスト、衝突形状の数学テスト、実粒子による接触・無効化テストを実行します。結果は`collision-math-result.txt`、`collision-result-gl.txt`、`collision-result-dx.txt`、`collision-smoke-result-*.txt`に出力します。

2026-10-31、HSP 3.7 / RTX 3060のOpenGL 3.1とANGLE Direct3D9Exで、各739回の衝突APIチェック、各300フレームの混在描画、床552回・球63回・箱181回の接触を確認しました。反発・摩擦の数値テストにはEffekseer本体の衝突処理を使っています。

雨粒はこのプロジェクト用の自作素材です。生成コードは`tools/CollisionFixture/Program.cs`。再生成する場合のみ.NET 9 SDKと公式エクスポーターの依存パッケージが必要です。

```powershell
dotnet run --project tools/CollisionFixture/CollisionFixture.csproj --configuration Release -- dist/assets/collision_rain.efk
```

通常のDLLビルド・実行に.NETは不要です。球のメッシュもHSP内で生成します。衝突設定の上書きには、コミットを固定したEffekseer内部のノード構造を使用しています。上流バージョンを変更するときは衝突APIも再検証してください。

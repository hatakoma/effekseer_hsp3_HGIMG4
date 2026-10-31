# 同梱物の出典

Effekseer公式リポジトリ https://github.com/effekseer/Effekseer のタグ1807、コミット`b87d1a2e3ff3731ba25cad34f8d8abeae382b21f`を使用しています。

- DLL内のEffekseerランタイム、OpenGLレンダラー、共通レンダラー、マテリアルコンパイラー: `Dev/Cpp`由来。MIT、Copyright (c) 2011 Effekseer Project。`licenses/Effekseer-LICENSE.txt`。
- `assets/Laser01.efkefc`と`assets/Texture/*`: 同じ公式リポジトリの`Examples/Resources`から無改変でコピー。リポジトリのMIT表記を同梱しています。
- DLL内のPNGローダー: Effekseer同梱の`stb_effekseer/stb_image_effekseer.h`。MITの選択肢を適用。Copyright (c) 2017 Sean Barrett。`licenses/stb-image-LICENSE.txt`。
- FlatBuffers: 上流ソースのビルド依存ヘッダー。Apache License 2.0の原文を`licenses/flatbuffers-LICENSE.txt`に同梱。ネットワーク機能は無効でビルドしています。
- `res/shaders/*`: インストール済みHSP 3.7の`sample/hgimg4/res/shaders`から無改変でコピーした、立方体サンプル用HGIMG4/GamePlay3D標準シェーダー。`licenses/HSP37-HSPLicense.txt`および`licenses/HGIMG4-NOTICES.txt`を同梱。

ANGLE対応版はEffekseerのOpenGLES2描画経路を使用します。CMakeで上流のWindows関数ローダーのコピーを生成し、WGL参照をANGLE/EGLへの動的参照に変更、ES3専用エントリーポイントを任意扱いにしています。上流ソース自体は無改変です。ホストの頂点属性の保存・復元とGL転送処理は独自ブリッジ側の追加実装です。

HSP実行本体、ANGLEのDLL、OpenAL、EffekseerのDirectXレンダラー、Effekseerエディタは同梱していません。ANGLE対応DLLは実行時にHGIMG4DXがロード済みのlibEGL/libGLESv2を共有します。HSP付属ANGLE DLLを別途再配布する場合は、HSPの配布条件と同梱`licenses/HGIMG4-NOTICES.txt`のANGLE表記を確認してください。zlib/libpngのソースはローカル調査時に取得していますが、これらのDLLにはリンクしていません。

独自ブリッジ、HSPヘッダー、サンプルコード、自作の雨粒素材、ビルド・テスト・生成スクリプト、ドキュメントはMIT Licenseで公開します。著作権表記は **Copyright © 2026 HAta/komatsuna-ya** です。許諾条件の全文は`LICENSE`に収録しています。第三者コード・素材のライセンスと著作権表記は各原文に従い、この独自部分の表記で置き換えません。

追加の`assets/collision_rain.efk`は、このプロジェクト用に自作したパラメーターを公式Effekseerエクスポーターで書き出した素材です。外部画像は参照しません。生成コードは`tools/CollisionFixture/Program.cs`。衝突サンプルの球メッシュも自作コードから生成します。調査に取得した別リポジトリのTestData素材は配布物に含めていません。.NETエクスポーターとNuGetパッケージのバイナリも配布物には含めていません。

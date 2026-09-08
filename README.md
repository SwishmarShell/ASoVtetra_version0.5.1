# ASoVtetra_v0.5.1 REDME

- English version is available here: [README.en.md](http://readme.en.md/)
- Download
You can download the latest version from the Releases page:
[https://github.com/SwishmarShell/ASoVtetra_version0.5.1/releases/latest](https://github.com/SwishmarShell/ASoVtetra_version0.5.1/releases/latest)<br><br>
<br>

# ASoVtetra_version0.5.1<br>

## Version<br>

- 現在のバージョン： v0.5.1<br>
<br>

## 本バージョンの機能

- PNG画像を、読み込み、Windows のデスクトップの画面に描画します。<br>
（PNG画像のビットの深さ：３２bit 推奨）<br>

- レイヤー式に、複数のPNG画像を、読み込むことが出来ます。<br>
<br>

## **使用方法**<br>
- **Ｆ２ キー** で、Settings ウィンドウを開いてください。<br>
> ※ 設定後、閉じることを推奨。本プログラムを選択すると、開けます。<br>
     <br>
- 左下の **”Add Layer”** ボタンで、レイヤーを追加してください。<br>
> ####    　 ※ 以下の設定項目が、現れます。<br>
> + レイヤー名<br>
> + Load PNG ボタンで、PNG画像の読込<br>
> + ”エイリアスON” の、チェックボックス<br>
> + 前面・背面の移動（レイヤー追加される方向が、最前面。）<br>
> + レイヤーの削除<br>
     <br>
-  描画ウィンドウの移動・表示する範囲を変更可能。<br>
> ※ 各画像の大きさは、変わらない。<br>
> 　 始点は固定式。<br><br>
- **Ｆ１キー** で、描画ウィンドウを固定する。<br>
> + デスクトップの画面の最前面（Top）で、描画されます。<br>
> + 描画はされますが、白色背景のWindow が消えます。<br>
> <br>
> ◎ポイント：透明度の画像編集をしてあるものは、背面が透けます。<br>
> <br>
> ※ Direct2Dのレイヤード描画に切り替わっています。<br>
> <br>
> ・ 描画ウィンドウを、固定化すると、<br>
> ・ マウス操作は、画像の上を、クリックしても、<br>
> ・ 背面にある Window等の操作になります。<br>
> <br>
>   ※ つまり、何も受け付けない画像になる。<br>
> 　　 本プログラムを選択していても、同様。<br>
> <br>
-  **”Save Project” ボタン** で、全てをフォルダに収納できる。<br>
>    !! あらかじめ、保存先のフォルダを用意してください。<br>
> <br>
>     　※ お手数かけまして、すみません。後のバージョンで改善します。<br>
> <br>
>         ◎ポイント：別のPC環境に、持ち運べる。<br><br>
>             - フォルダ構成<br>
>             /（任意フォルダ）<br>
>             　/ config.ini<br>
>             　/ bitmaps<br>
>             　/ layers<br>
>             　　/ layer(PNG_FileName).png<br>
>             　※ 参照先の各画像は、ひとつのフォルダに、コピーされます。<br>
> <br>
- **”Load Project” ボタン** で、保存した、任意フォルダを指定し、読み込む。<br>
> + すべての、レイヤーの構成と、各PNG画像が再現される。<br>
> + "フォルダ読み込みが完了しました。" と表示する、小さな 'Load Folder' ダイヤログが、画面中央に現れます。<br>
> + このダイヤログを見つけ、'OK' を押して、ご利用ください。<br>
> <br> 
## v0.5.1 の主な改善点

- プロジェクト保存方式を 「フォルダ保存方式」。
    - `config.ini` / `layers/` / `bitmaps/` の構成で保存されます。
- 読み込み直後に画像が表示されない問題を修正。
- Direct2D 描画初期化タイミングを改善し、描画の安定性が向上。
- 古いキャッシュが残留する問題を解消。
- 内部コード構造を整理し、今後の拡張が容易に。
<br>

## 本バージョンの利用するポイント（例）

- デスクトップ上をキャプチャするときに、オリジナルロゴなどを常に、最前面に固定し、かつ、作業には影響しないように利用できる。
- OBSの画面にて、任意の位置への移動と固定が自在に。
- デスクトップ作業中に、モチベーションあがるキャラクター画像が、居てくれます。
- 透明度の強度を編集した画像を、パーツごとに重ね合わせれば、繊細に、モチベーションを支援します。
- Projectフォルダと、本アプリケーションを、持ち運んで使える。
<br>

## 動作環境 & 起動方法

＜推奨環境＞

- Windows 11 64bit (x64)
- Windows 11 32bit (x86)

＜起動方法＞

- それぞれ、対応する、Zipファイルをダウンロードし、解凍して、フォルダの中に在る、”ASoVtetra_version0.5.1.exe” を起動。<br><br>
 ※ フォルダ内にある、テスト用のプロジェクトフォルダ をお試しください。
<br>

## ライセンス

- このプロジェクトは MIT ライセンスのもとで公開しています。
- 再配布・改変・商用利用を行なう場合は、以下の著作権表示と、MIT ライセンス文を保持することが条件となります。

Copyright (c) 2026 Swishmar.Shell
<br>

## CREDIT

 If you use this project, a credit such as “Based on work by Swishmar.Shell” would be appreciated.

　本プロジェクトを利用する際、「Swishmar.Shell による作品を元にしています」等の、クレジット表記をしていただけると嬉しいです。

## 注意

※ Windows 11 64bit (x64)での開発です。

　Windows 10 (x64)等では、動作未確認の為、推奨環境以外の動作の保障はありません。

※ 今後のバージョンアップで、ONNX Rantimeを利用した、機能を実装したときは、64bit (x64) OS 専用になります。

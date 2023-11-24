# MiniMenuForEveryone
This program allows for easy implementation of MiniMenu in CTRPF.
Please place these folders into the source code of CTRPF.
An implementation example is provided below.
The license information is available in the LICENSE.md file.

このプログラムを使うとCTRPFでのMiniMenuの実装が簡単にできます。
CTRPFのソースコードにこれらのフォルダを入れてください。
下に実装例を記載しています。
ライセンスはLISENCE.mdをご覧ください。

Implementation method / 実装方法
1.CTRPFのソースにダウンロードしたファイルを入れる
2.cheat.cppに実装例を参考に記述しcheat.hppに関数のプロトタイプ宣言をする
3.main.cppにエントリーする

Implementation example / 実装例 (In cheat.cpp... / cheat.cppにて)
void MiniMenu() {
  Implementation_MiniMenu("名前", エントリータイプ, メジャーバージョン, マイナーバージョン, リビジョンアップ, ボタン, boolで軽量化にするか);
    new_entry("名前", エントリータイプ(0,1,2), u32のアドレス, u32の値, "ノート");
}

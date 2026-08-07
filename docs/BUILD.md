# ビルド手順

仕様書：[SPEC.md](SPEC.md)

## 必要なもの

| 道具 | 用途 | 最低版 |
| --- | --- | --- |
| CMake | ビルド定義の生成 | 3.20 |
| C++ コンパイラ | 本体のコンパイル | GCC 12 / Clang 15 / MSVC 2022 |
| Ninja または Make | 実際のビルド実行 | — |
| Git | GoogleTest の取得 | — |

テストを無効にすればネットワーク接続は不要になる。

## 二段階の仕組み

CMake 自体はコンパイルを行わない。
`CMakeLists.txt` を読んで、Ninja や MSBuild が理解できる形式のビルドファイルを生成する道具である。
したがって作業は必ず二段階になる。

```
① configure : CMakeLists.txt を読み、build/ に生成物を作る
② build      : 生成されたファイルに従って実際にコンパイルする
```

生成物はすべて `build/` に隔離する（out-of-source ビルド）。
`build/` を丸ごと削除しても、① からやり直せば復元できる。
ソースツリーには何も残らない。

## 手順

### 初回

```
cmake -S . -B build -G Ninja
```

`-S .` はソースの位置、`-B build` は生成先、`-G Ninja` は生成するビルドファイルの種類を指す。
このとき GoogleTest のダウンロードが走るため、初回だけ時間がかかる。

`-G` を省略すると環境ごとの既定（Windows では Visual Studio）が選ばれる。

### ビルド

```
cmake --build build
```

ソースを変更した後はこのコマンドだけでよい。
`CMakeLists.txt` を変更した場合も、必要な再生成は自動で行われる。

### テスト

```
ctest --test-dir build --output-on-failure
```

`--output-on-failure` は、失敗したテストの標準出力を表示する。
これがないと「落ちた」という事実しか分からない。

テストを絞り込むときは `-R` に正規表現を渡す。

```
ctest --test-dir build -R Types
```

### CLI の実行

```
build/dobutsu_cli
```

Windows では `build\dobutsu_cli.exe`。

## オプション

| オプション | 既定 | 意味 |
| --- | --- | --- |
| `DOBUTSU_BUILD_TESTS` | ON | テストをビルドするか。OFF にすると GoogleTest を取得しない |
| `CMAKE_BUILD_TYPE` | 空 | `Release` / `Debug` / `RelWithDebInfo` |

configure 時に `-D` で指定する。

```
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Release
```

`CMAKE_BUILD_TYPE` を指定しないと最適化が一切かからない。
SPEC 10 節の性能を測る際は必ず `Release` を指定する。

## 別のコンパイラで確認する

Windows で MSVC を使う場合は、Visual Studio の開発者コマンドプロンプトから次を実行する。

```
cmake -S . -B build-msvc
cmake --build build-msvc --config Release
```

`build` とは別のディレクトリを使うこと。
同じディレクトリに異なるコンパイラの生成物を混ぜるとキャッシュが衝突する。

MSVC ではランタイムライブラリの選択が GoogleTest と一致している必要がある。
`CMakeLists.txt` の `gtest_force_shared_crt` がこれを揃えており、`FetchContent_MakeAvailable()` より前に置く必要がある。

## うまくいかないとき

**設定を変えたのに反映されない**：`build/CMakeCache.txt` に前回の設定が残っている。
`build/` ごと削除して configure からやり直す。

**GoogleTest の取得に失敗する**：Git が PATH にあるか確認する。
オフライン環境では `-DDOBUTSU_BUILD_TESTS=OFF` を指定する。

**リンクエラーで関数が見つからない**：ヘッダの宣言と `.cpp` の定義が食い違っている。
名前空間の綴り、引数の型、`const` の有無を照合する。
翻訳単位ごとに独立してコンパイルされるため、この種の不一致はコンパイル時には検出されない。

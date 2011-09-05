==================================
Kinect Hackers Manual Support Page
==================================

補足説明と正誤表はこちら `http://hagino3000.github.com/kinectbook/ <http://hagino3000.github.com/kinectbook/>`_

現在は次のOSで動作確認を行なっています。

- MacOS X 10.6 (Snow leoperd) + Xcode3.2.5
- MacOS X 10.7 (Lion) + Xcode 4.1
- Windows 7 (32bit)



注意:

  Visual Studioで実行した時に `main.cpp : warning C4819: ファイルは、現在のコードページ(932)で表示できない文字を含んでいます。データの損失を防ぐために、ファイルをUnicode 形式で保存してください。` という警告がでて実行時にエラーが発生した場合、main.cppを「Unicode(UTF-8 シグネチャ付き)-コードページ65001」で保存しなおしてから実行してください。


ライブラリのバージョン
----------------------

以下のバージョンで動作確認を行なっています。

OpenNI

- OpenNI 1.1.0.39 Unstable version - Apr 11th 2011
- SensorKinect 5.0.1.32 Unstable version - Apr 11th 2011
- NITE 1.3.1

OpenCV

- 2.2 or 2.3

openFrameworks

- 0.062

Kinect SDK for Windows

- Beta1

libfreenect

- commit 4a159f86ea91331c8e0f0532f688deebfbe18096 - Apr 28 2011

OpenNIがバージョンアップしているので動作確認ができ次第更新します。


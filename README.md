QT Box Editor
=============

Project homepage: [zdenop.github.com/qt-box-editor](http://zdenop.github.com/qt-box-editor/)

Screenshots: [wiki](https://github.com/zdenop/qt-box-editor/wiki)

Download current (devel) source: [tar.gz](https://github.com/zdenop/qt-box-editor/tarball/master) or [zip](https://github.com/zdenop/qt-box-editor/zipball/master).

The latest released source is [qt-box-editor-v1.11](https://github.com/zdenop/qt-box-editor/tarball/master/tree/v1.11).

Download [win32 binary build](http://sourceforge.net/projects/qtboxeditor/?source=dlp) from [sourceforge.net](http://sourceforge.net/projects/qtboxeditor/)

Licence: [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)


DESCRIPTION
-----------

QT Box Editor is tool for adjusting [tesseract-ocr](http://code.google.com/p/tesseract-ocr/) box files. Aim of this project is to provide easy and efficient way for editing regardless file size.

Release information can be found in CHANGELOG file. Code and artwork contribution is welcomed.

QT box editor is a successor of [tesseract-gui project](https://github.com/mk219533/tesseract-gui) that is not developed anymore. Name of application was changed due to name collision with project http://tesseract-gui.sourceforge.net.


LICENSE
-------

License was changed from "THE BEER-WARE LICENSE" to ["Apache License, Version 2.0"](http://www.apache.org/licenses/LICENSE-2.0) based on agreement with Marcel Kołodziejczyk ([original author](https://github.com/mk219533/tesseract-gui)). Anyway you are welcome to buy him a free beer if you meet him. He deserves it.

[Faenza Icons](http://tiheum.deviantart.com/art/Faenza-Icons-173323228) are under GNU/GPL license.
[Tango Icons](http://tango.freedesktop.org/) are Public Domain.

ARTWORK
-------

These icons are Faenza Icons:
    exit.png
    filesave.png
    fileopen.png
    gtk-jump-to-ltr.png
    help-about.png
    next.png
    previous.png
    text_bold.png
    text_italic.png
    text_under.png
    window-close.png
    zoom-in.png
    zoom-fit.png
    zoom-original.png
    zoom-out.png
    zoom-selection.png (is modified zoom-fit.png)

import.svg is modified icon of Faenza fileopen icon.

joinRow.svg and splitRow.svg are adapted icons of lmproulx icons [Convergent](http://www.openclipart.org/detail/convergent-by-lmproulx) and [Divergent](http://www.openclipart.org/detail/divergent-by-lmproulx)

gnome-edit-find.svg is from [Wikipedia](http://en.wikipedia.org/wiki/File:Gnome-edit-find.svg).

Other icons/artwork were created for Qt Box Editor and they are released under Apache License, Version 2.0.


Distribution
------------

For Windows users there is binary a version in [sourceforge.net](http://sourceforge.net/projects/qtboxeditor/?source=dlp) section. From version 1.09 distribution is split to 2 part:
* qt-box-editor executable (e.g. qt-box-editor-1.11.exe) - share build
* qt-box-editor dependecies (e.g. qt-box-editor-1.11-dependecies.zip) - needed 3rd party libraries to run qt-box editor.
There is a hope that qt-box-editor dependecies can be used for next few qt-box-editor releases.

On other platforms you need to build qt-box-editor from source. You will need QT4 (v1.11 is compatible with QT5), leptonica and tesseract.
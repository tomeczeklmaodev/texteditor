texteditor - a simple text editor written using C++ with Win32 API.

Version: 0.1 (dd-mm-rrrr)
Developer: tomeczeklmaodev
Description: ---

Compiling:
* Resource files: windres resources.rc -O coff -o resources.res
* Everything: g++ main.cpp resources.res -lgdi32 -lcomdlg32 -o texteditor.exe

Known issues:
* erasing does not work as intended
* text field does not appear until resizing the window

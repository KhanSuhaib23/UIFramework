@echo off


set CommonCompilerFlags=/nologo /MD /EHsc /Od /Zi /W4 /wd4100 /wd4099 /wd4204 /wd4996 /we4702
set Defines=/D_CRT_SECURE_NO_WARNINGS
set FilesToCompile=..\code\Main.c
set LinkLibraries=/NODEFAULTLIB:LIBCMT opengl32.lib kernel32.lib user32.lib gdi32.lib shell32.lib glfw3.lib glew32s.lib
set AdditionalIncDir=..\dependencies\include\
set AdditionalLibDir=/LIBPATH:..\dependencies\lib\


IF EXIST build GOTO DIRECTORY_EXISTS
mkdir build

:DIRECTORY_EXISTS
pushd build
del /Q *.*


cl /I %AdditionalIncDir% %CommonCompilerFlags% %Defines% %FilesToCompile% /link %AdditionalLibDir% %LinkLibraries%
popd

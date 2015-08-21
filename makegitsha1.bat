@echo off
set cmd=git rev-parse HEAD
echo Writing to git_sha1.h

echo #ifndef git_sha1H > git_sha1.h
echo #define git_sha1H >> git_sha1.h
set git_sha1=#define GIT_SHA1
for /f %%i in ('%cmd%') do set "git_sha1=%git_sha1% "%%i""
echo %git_sha1% >> git_sha1.h
echo #endif >> git_sha1.h
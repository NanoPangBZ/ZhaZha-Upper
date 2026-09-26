@echo off
chcp 65001 >nul
setlocal

set "ROOT=%~dp0.."
set "PROTOCOL_DIR=%ROOT%\protocol"
set "OUTPUT=%ROOT%\docs\应用层接口协议文档.html"
set "TOOL=%ROOT%\protocol\tool\protocol_doc_make.py"

cd /d "%ROOT%"

python "%TOOL%" "%PROTOCOL_DIR%" "%OUTPUT%"
set "ERR=%ERRORLEVEL%"

if %ERR% neq 0 (
    echo.
    echo 文档生成失败，错误码: %ERR%
    endlocal
    exit /b %ERR%
)

echo.
echo 文档已输出: %OUTPUT%
start "" "%OUTPUT%"
endlocal
exit /b 0

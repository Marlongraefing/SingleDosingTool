@echo off
setlocal

setlocal enabledelayedexpansion
REM Define the path to the c_cpp_properties.json file
set JSON_FILE=%~dp0.vscode\c_cpp_properties.json

REM Define the lines to add
set LINE1="C:\\Users\\Marlon\\AppData\\Local\\Arduino15\\packages\\esp32\\tools\\esp32-arduino-libs\\idf-release_v5.5-9bb7aa84-v2\\esp32\\include/**"
set LINE2="C:\\Users\\Marlon\\AppData\\Local\\Arduino15\\packages\\esp32\\tools\\esp32-arduino-libs\\idf-release_v5.5-9bb7aa84-v2\\esp32\\include\\freertos\\FreeRTOS-Kernel\\portable\\xtensa\\include\\freertos"

REM Create a temporary file
set TEMP_FILE=%JSON_FILE%.tmp

REM Add the lines to the includePath array
(for /f "usebackq delims=" %%A in ("%JSON_FILE%") do (
    set LINE=%%A
    echo !LINE! | findstr /c:"\"includePath\": [" >nul
    if not errorlevel 1 (
        echo !LINE!>>"%TEMP_FILE%"
        echo                %LINE1%,>>"%TEMP_FILE%"
        echo                %LINE2%,>>"%TEMP_FILE%"
    ) else (
        echo !LINE!>>"%TEMP_FILE%"
    )
)) || (
    echo Fallback method failed. Ensure %JSON_FILE% exists and is readable.
    popd
    exit /b 1
)

REM Replace the original file with the modified file
move /y "%TEMP_FILE%" "%JSON_FILE%" >nul

echo c_cpp_properties.json updated successfully.
popd
endlocal
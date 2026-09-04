:: Pass parameter: _DEBUG or _RELEASE for compilation target.

@echo off
:: Output help info for user...
if %1 == ? (echo Compile Parameters: _DEBUG or _RELEASE)

:: Compiles shaders for debug...
if %1 == _DEBUG (
	echo Compiling SPIR-V shaders via GLSL-C...
	if not exist "%CD%\x64\DEBUG\Shaders\" (mkdir "%CD%\x64\DEBUG\Shaders\")
	echo _DEBUG From: 		"%CD%\Shaders\"
	echo _DEBUG To: 		"%CD%\x64\DEBUG\Shaders\"
	
	for %%f in (%CD%\Shaders\*.frag %CD%\Shaders\*.vert %CD%\Shaders\*.comp) do (
		if %%~xf == .vert (echo 	Vertex Stage:		%%~nf.spv)
		"%VULKAN%\Bin\glslc.exe" %%f -o "%CD%\x64\DEBUG\Shaders\%%~nf.spv"
		
		if %%~xf == .frag (echo 	Fragment Stage:		%%~nf.spv)
		"%VULKAN%\Bin\glslc.exe" %%f -o "%CD%\x64\DEBUG\Shaders\%%~nf.spv"
	)
)

:: Compiles shaders for release...
if %1 == _RELEASE (
	echo Compiling SPIR-V shaders via GLSL-C...
	if not exist "%CD%\x64\RELEASE\Shaders\" (mkdir "%CD%\x64\RELEASE\Shaders\")
	echo _RELEASE From: 	"%CD%\Shaders\"
	echo _RELEASE To: 		"%CD%\x64\RELEASE\Shaders\"
	
	for %%f in (%CD%\Shaders\*.frag %CD%\Shaders\*.vert %CD%\Shaders\*.comp) do (
		if %%~xf == .vert (echo 	Vertex Stage:		%%~nf.spv)
		"%VULKAN%\Bin\glslc.exe" %%f -o "%CD%\x64\RELEASE\Shaders\%%~nf.spv"
		
		if %%~xf == .frag (echo 	Fragment Stage:		%%~nf.spv)
		"%VULKAN%\Bin\glslc.exe" %%f -o "%CD%\x64\RELEASE\Shaders\%%~nf.spv"
	)
)

echo.
pause
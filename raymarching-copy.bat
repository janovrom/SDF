echo "Copying raymarching files.."
xcopy ".\raymarching\res\raymarching.vs" ".\bin\Release" /S /Y
xcopy ".\raymarching\res\raymarching.fs" ".\bin\Release" /S /Y
xcopy ".\raymarching\res\raymarching.vs" ".\bin\Debug" /S /Y
xcopy ".\raymarching\res\raymarching.fs" ".\bin\Debug" /S /Y

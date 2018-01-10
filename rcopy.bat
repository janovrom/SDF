echo "Copying resource files.."
xcopy ".\raymarching\res" ".\bin\Release" /S /Y
xcopy ".\raymarching\res" ".\bin\Debug" /S /Y

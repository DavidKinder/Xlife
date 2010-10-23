@echo off
pushd ..\..
"\Program Files\Zip\zip" \Temp\xlife35.zip Xlife35\*.*
"\Program Files\Zip\zip" \Temp\xlife35.zip Xlife35\doc\*.*
"\Program Files\Zip\zip" \Temp\xlife35.zip Xlife35\patterns\*.life
"\Program Files\Zip\zip" -r \Temp\xlife35src.zip include X11Lib
"\Program Files\Zip\zip" -r \Temp\xlife35src.zip Xlife35\source
popd


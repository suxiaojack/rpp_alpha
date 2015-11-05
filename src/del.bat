
del /F /Q ..\bin\*.ilk
del /F /Q ..\bin\*.pdb

del /F /Q .\*.ncb
attrib -R -S -A -H .\*.suo
del /F /Q .\*.suo
del /F /Q .\*.user

del /F /Q .\proj\rpp_win\*.vsp
rem del /F /Q .\proj\rpp_win\*.filters
del /F /Q .\proj\rpp_win\*.psess
del /F /Q .\proj\rpp_win\*.sdf
del /F /Q .\proj\rpp_win\*.ncb
attrib -R -S -A -H .\proj\rpp_win\*.suo
del /F /Q .\proj\rpp_win\*.suo
del /F /Q .\proj\rpp_win\*.user

rd /S /Q .\proj\rpp_win\debug
rd /S /Q .\proj\rpp_win\Release

del /F /Q .\proj\*.vsp
rem del /F /Q .\proj\*.filters
del /F /Q .\proj\*.psess
del /F /Q .\proj\*.sdf
del /F /Q .\proj\*.ncb
attrib -R -S -A -H .\proj\*.suo
del /F /Q .\proj\*.suo
del /F /Q .\proj\*.user

rd /S /Q .\proj\debug
rd /S /Q .\proj\Release

del /F /Q .\rpp.asm
del /F /Q .\rpp.cpp
del /F /Q .\rpp.exe

del /F /Q .\example\test\*.exe
del /F /Q .\example\test\*.cpp
del /F /Q ..\bin\rpp_gpp.asm
del /F /Q ..\bin\rpp_nasm.asm
del /F /Q ..\bin\rpp_gpp.exe
del /F /Q ..\bin\rpp_nasm.exe

pause
"C:\Program Files (x86)\Windows Kits\8.1\bin\x86\inf2cat.exe" /driver:. /os:8_X86,8_X64,7_X86,7_X64,Vista_X86,Vista_X64,XP_X86,XP_X64

signtool.exe sign /v /ac "../../VeriSign Class 3 Public Primary Certification Authority - G5.cer" /s MY /n "Seek Thermal, Inc." /t http://timestamp.verisign.com/scripts/timstamp.dll /d "Seek Thermal USB Driver" SeekWare.cat
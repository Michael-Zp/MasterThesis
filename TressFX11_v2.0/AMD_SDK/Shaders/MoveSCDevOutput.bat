copy /Y "%2\Shaders\ScDev\%3\%4\*dump.txt" "%2\Shaders\Cache\ISA\%~1.asm.%3.dump.isa"
copy /Y "%2"\Shaders\ScDev\%3\%4\*"%~1.asm" "%2\Shaders\Cache\ISA"
rem del "%2\*dump.txt"
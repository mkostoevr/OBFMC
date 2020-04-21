.PHONY: debug release

debug:
	tcc src/main.c -Iinclude -DONE_SOURCE -Wall -Wextra -Werror -o obfmc.exe
	obfmc.exe test/99-bottles.b       -o test/99-bottles       --c --kos32 --dos16 --win32
	obfmc.exe test/hanoi.b            -o test/hanoi            --c --kos32 --dos16 --win32
	obfmc.exe test/hello-world-long.b -o test/hello-world-long --c --kos32 --dos16 --win32
	obfmc.exe test/hello-world.b      -o test/hello-world      --c --kos32 --dos16 --win32
	obfmc.exe test/rot13.b            -o test/rot13            --c --kos32 --dos16 --win32

release:
	clang src/main.c -Iinclude -DONE_SOURCE -Wall -Wextra -o obfmc.exe
	obfmc test/hello-world.b -o hello-world --c --kos32 --dos16

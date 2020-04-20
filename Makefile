.PHONY: debug release

debug:
    tcc src/main.c -Iinclude -DONE_SOURCE -Wall -Wextra -Werror -o obfmc.exe
    obfmc test/hello-world.b -o hello-world --c --kos32 --dos16

release:
    clang src/main.c -Iinclude -DONE_SOURCE -Wall -Wextra -o obfmc.exe
    obfmc test/hello-world.b -o hello-world --c --kos32 --dos16

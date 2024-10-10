savedcmd_/home/taewonkim/module/hello_module.mod := printf '%s\n'   hello_module.o | awk '!x[$$0]++ { print("/home/taewonkim/module/"$$0) }' > /home/taewonkim/module/hello_module.mod

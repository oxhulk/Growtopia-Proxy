# Growtopia enet proxy

# By [loanselot](https://github.com/loanselot)
[Discord link](https://discord.gg/fJqzkyJ5RP)

## Compile and build in Visual Studio
* Install Vcpkg  <a href="https://github.com/microsoft/vcpkg/#quick-start-windows">Click</a>
* Install Openssl With Vcpkg -> vcpkg install openssl:x64-windows ( i suggest static )
* Reopen Vs,Done.

## How to use
* Does not need separate http server
* Use HxD or similar to edit "growtopia1.com" and "growtopia2.com" to "localhost" In growtopia binary.
* Alternative: Use hosts file to set growtopia1 and growtopia2 to point to 127.0.0.1 if you want to
* Start proxy -> Run localhost patched file (or normal with hosts) -> Works as supposed

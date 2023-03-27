<!-- @format -->

# MaxRepeats-internal-memory

## STXXL

crie um diretório build

```
mkdir external/stxxl/build
```

navegue até ele

```
cd external/stxxl/build
```

Se preciso instale o CMake

```
sudo apt install cmake
```

Execute o cmake no diretório stxxl para o diretório stxxl/build

```
cmake ..
```

Execute o make

```
make
```

Volte ao diretório principal e mova o arquivo config.h para o diretório correto

```
cd ../../../
mv external/stxxl/build/include/stxxl/bits/config.h external/stxxl/include/stxxl/bits/
```

Compilando o programa:

```
g++ -Iexternal/stxxl/include/ -Lexternal/stxxl/lib repeat.cpp
```

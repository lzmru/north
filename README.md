# Build
## Requirements
* Compiler with C++17 support

| Compiler | Version |
|----------|---------|
| GCC      | 5+      |
| Clang    | 3.6+    |
| MSVC     | 19.10+  |
* Cmake 3.8+
* LLVM 5
## OS X
```
brew install llvm cmake git
git clone https://github.com/lzmru/north
cd north
cmake . -DCMAKE_BUILD_TYPE=RELEASE
make
```
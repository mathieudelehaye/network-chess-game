- Install Java 11+ at system level:
```
sudo apt update
sudo apt install openjdk-17-jdk -y      # here I used JDK 17
sudo update-alternatives --config java  # select which version of Java to use if multiple ones are installed. 
java -version                           # check the installed selected version.
```

- Install doxygen and graphviz at system level (not available in vcpkg):
```
sudo apt update
sudo apt install doxygen graphviz
```

- Install dependencies in local package manager (vcpkg):
```
cd ~
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install spdlog nlohmann-json
```

- To build and run the backend server:
```
cd /path/to/project/root

rm -rf build/backend/debug

cmake \
  -S src/backend \
  -B build/backend/debug \
  -G "Ninja" \
  -DCMAKE_BUILD_TYPE=Debug 

cmake --build build/backend/debug -j$(nproc)

./bin/backend/chess_server
```

- To generate the documentation (not auto-generated on every build):
```
cmake --build build/backend/debug/ --target docs
```

Cmake script should automatically download the chess-library (https://github.com/Disservin/chess-library). If needed to download it manually in the download cache folder:
```
cd download
wget https://github.com/Disservin/chess-library/archive/refs/heads/master.zip -O chess-library.zip
unzip -q chess-library.zip
mv chess-library-master chess-library
rm chess-library.zip
ls -la chess-library/
```
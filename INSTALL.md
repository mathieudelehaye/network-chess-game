# network-chess-game

A chess game implementing a TCP client and server network architecture

# Install Project and Dependencies

## Technical and System Requirements

- Linux (Ubuntu 24.04.1 LTS) or WSL on Windows
- Python 3.11

## Backend

- Install Java 11+ (system level), required by ANTLR parser (https://www.antlr.org):
```
sudo apt update
sudo apt install openjdk-17-jdk -y      
sudo update-alternatives --config java  # select which version of Java to use if multiple ones are installed
java -version                           # check the selected version
```

- Install doxygen and graphviz (system level):
```
sudo apt update
sudo apt install doxygen graphviz
```

- Install dependencies in local package manager (vcpkg):
```
cd ~
git clone https://github.com/microsoft/vcpkg.git
./vcpkg/bootstrap-vcpkg.sh
./vcpkg/vcpkg install spdlog nlohmann-json gtest
```

- Cmake scripts should automatically download the chess-library (https://github.com/Disservin/chess-library). If needed to download it manually (to the download cache folder):
```
cd download
wget https://github.com/Disservin/chess-library/archive/refs/heads/master.zip -O chess-library.zip
unzip -q chess-library.zip
mv chess-library-master chess-library
rm chess-library.zip
ls -la chess-library/
```

## Frontend

- Configure Python Virtual environment (venv):

```
env="/path/to/project/root/.venv"
uv venv $env --python python3.11
source "$env/bin/activate"
python --version && which python

cd /path/to/project/root/src/frontend

uv pip install -r src/frontend/requirements.txt

deactivate
```

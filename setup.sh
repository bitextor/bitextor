git pull
git submodule update --init --recursive
sudo apt update && sudo apt install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libboost-all-dev libbz2-dev liblzma-dev zlib1g-dev libffi-dev
sudo pip3 install -r requirements.txt
cd hunalign/src/hunalign ; make ; cd ../../.. ; mkdir bin ; cp hunalign/src/hunalign/hunalign bin/.
sudo pip3 install -r bicleaner/requirements.txt
#
#sudo apt install httrack
#
#wget http://corpus.tools/raw-attachment/wiki/Downloads/chared-1.2.2.tar.gz
#tar xzvf chared-1.2.2.tar.gz chared-1.2.2
#rm chared-1.2.2.tar.gz
#cd chared-1.2.2
#sudo python3 setup.py install
#cd ..
#sudo rm -rf chared-1.2.2
#
#cmake_version=`cmake --version | head -1`
#if [ "$cmake_version" != "cmake version 3.12.3" ]
#then
#    rm -rf cmake-3.12.3.tar.gz cmake-3.12.3
#    wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
#    tar xvf cmake-3.12.3.tar.gz 
#    cd cmake-3.12.3/
#    ./bootstrap 
#    make -j8
#    sudo make install
#    cd ..
#    sudo rm -rf cmake-3.12.3.tar.gz cmake-3.12.3
#fi
#
#sudo sh -c 'echo CUDA_ROOT=/usr/local/cuda >> /etc/environment'
#sudo sh -c 'echo PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin >> /etc/environment'
#sudo sh -c 'echo LD_LIBRARY_PATH=/usr/local/cuda/lib64 >> /etc/environment'
#sudo sh -c 'echo LIBRARY_PATH=/usr/local/cuda/lib64 >> /etc/environment'
#PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin"
#CUDA_ROOT="/usr/local/cuda"
#LD_LIBRARY_PATH="/usr/local/cuda/lib64"
#LIBRARY_PATH="/usr/local/cuda/lib64"
#    
#python3 -c "import nltk; nltk.download('punkt')"

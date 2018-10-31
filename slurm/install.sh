#!/bin/bash
# sudo ./install.sh hieu-foo southcentralus installall scale-cpu:Standard_H16m:10:16 scale-gpu:Standard_NV6:3:6:gpu:tesla:1
# Scaleset params = NAME:SIZE:count:num-cpu[:gpu-string]

RESOURCE_GROUP=$1
REGION=$2
INSTALL=$3 #'whatever' string to install everything, 'no' to avoid all installation process
vmssnames="${@:4}" #If GPU, use examplevmss:gpu:tesla:1 syntax
echo "RESOURCE_GROUP $RESOURCE_GROUP"
echo "REGION $REGION"
echo "vmssnames $vmssnames"

if ! [ $SUDO_USER ]; then
    echo "must run as sudo. Exiting"
    exit
fi

installdependencies(){
    sudo apt-get update
    sudo apt-get install -y g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common parallel sshpass emacs munge slurm-wlm ubuntu-drivers-common libicu-dev curl

    wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
    tar xvf cmake-3.12.3.tar.gz 
    cd cmake-3.12.3/
    ./bootstrap 
    make -j4
    sudo make install
    cd ..
    rm -rf cmake-3.12.3.tar.gz cmake-3.12.3

    CUDA_REPO_PKG=cuda-repo-ubuntu1804_10.0.130-1_amd64.deb
    wget -O /tmp/${CUDA_REPO_PKG} http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/${CUDA_REPO_PKG} 
    sudo dpkg -i /tmp/${CUDA_REPO_PKG}
    sudo apt-key adv --fetch-keys http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/7fa2af80.pub 
    rm -f /tmp/${CUDA_REPO_PKG}
    sudo apt-get update
    sudo apt-get install -y cuda

    AZ_REPO=$(lsb_release -cs)
    echo "deb [arch=amd64] https://packages.microsoft.com/repos/azure-cli/ $AZ_REPO main" |     sudo tee /etc/apt/sources.list.d/azure-cli.list
    curl -L https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
    sudo apt-get update
    sudo apt-get install -y apt-transport-https azure-cli
        

    sudo echo "CUDA_ROOT=/usr/local/cuda" >> /etc/environment
    sudo echo "PATH=\"/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin\"" >> /etc/environment
    sudo echo "LD_LIBRARY_PATH=\"/usr/local/cuda/lib64\"" >> /etc/environment
    sudo echo "LIBRARY_PATH=\"/usr/local/cuda/lib64\"" >> /etc/environment
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin"
    CUDA_ROOT="/usr/local/cuda"
    LD_LIBRARY_PATH="/usr/local/cuda/lib64"
    LIBRARY_PATH="/usr/local/cuda/lib64"
        
    sudo pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet snakemake tld tldextract tqdm
    python3 -c "import nltk; nltk.download('punkt')"
    sudo rm /tmp/munge.key /tmp/slurm.conf /tmp/hosts

}

if [ "$INSTALL" != "no" ]; then
    installdependencies
fi

# master only
ADMIN_USERNAME=$SUDO_USER

# Generate a set of sshkey under /home/azureuser/.ssh if there is not one yet
if ! [ -f /home/$SUDO_USER/.ssh/id_rsa ]; then
    sudo -u $SUDO_USER sh -c "ssh-keygen -f /home/$SUDO_USER/.ssh/id_rsa -t rsa -N ''"
fi

for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    VM_SKU=`echo $vmssinfo | cut -f 2 -d ':'`
    VM_COUNT=`echo $vmssinfo | cut -f 3 -d ':'`
    echo "VMSS_NAME=$VMSS_NAME VM_SKU=$VM_SKU VM_COUNT=$VM_COUNT"
    
    #Create the scaleset
    if [ "$INSTALL" != "no" ]; then
        az vmss create --resource-group $RESOURCE_GROUP --name $VMSS_NAME --image "Canonical:UbuntuServer:18.04-LTS:18.04.201810030" -l $REGION --vm-sku $VM_SKU --instance-count $VM_COUNT --admin-username $ADMIN_USERNAME
        for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME | grep 'privateIpAddress"' | cut -f 2 -d ':' | cut -f 2 -d '"'`; do
            print "installing worker $worker"
            sudo -u $SUDO_USER ssh -o "StrictHostKeyChecking=no" $worker "$(typeset -f installdependencies); installdependencies" &
        done
    fi
done
wait

#wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
#tar xvf cmake-3.12.3.tar.gz 
#cd cmake-3.12.3/
#./bootstrap 
#make -j
#make install
#cd ..
#rm -rf cmake-3.12.3.tar.gz cmake-3.12.3

#wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
#tar xvf boost_1_68_0.tar.gz 
#cd boost_1_68_0/
#./bootstrap.sh 
#./b2 -j16 --layout=system  install || echo FAILURE
#cd ..
#rm -rf boost_1_68_0*



SLURMCONF=/tmp/slurm.conf
TEMPLATE_BASE=https://raw.githubusercontent.com/bitextor/bitextor/bitextor-malign/slurm
wget $TEMPLATE_BASE/slurm.template.conf -O $SLURMCONF 

MASTER_NAME=$HOSTNAME
MASTER_IP=`hostname -I`


sed -i -- 's/__MASTERNODE__/'"$MASTER_NAME"'/g' $SLURMCONF

echo "GresTypes=gpu" >> $SLURMCONF
allworkernames="$MASTER_NAME"
#echo "NodeName=${MASTER_NAME} CPUs=1 State=UNKNOWN" >> $SLURMCONF
for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    CPUs=`echo $vmssinfo | cut -f 4 -d ':'`
    gpuinfo=`echo $vmssinfo | cut -f 5- -d ':'`
    echo "VMSS_NAME=$VMSS_NAME CPUs=$CPUs gpuinfo=$gpuinfo"
    
    echo "$LIST" | grep -q "$SOURCE";
    if echo "$vmssinfo" | grep -q ":gpu:" ; then
        workernames=`az vmss list-instances --resource-group $RESOURCE_GROUP --name $VMSS_NAME | grep 'computerName' | cut -f 2 -d ':' | cut -f 2 -d '"' | head -c -1 | tr '\n' ','`
        allworkernames="$allworkernames,$workernames"
        echo "NodeName=${workernames} CPUs=$CPUs State=UNKNOWN Gres=$gpuinfo" >> $SLURMCONF
    else
        workernames=`az vmss list-instances --resource-group $RESOURCE_GROUP --name $VMSS_NAME | grep 'computerName' | cut -f 2 -d ':' | cut -f 2 -d '"' | head -c -1 | tr '\n' ','`
        allworkernames="$allworkernames,$workernames"
        echo "NodeName=${workernames} CPUs=$CPUs State=UNKNOWN" >> $SLURMCONF
    fi
done
echo "PartitionName=debug Nodes=${allworkernames} Default=YES MaxTime=INFINITE State=UP" >> $SLURMCONF
echo "DebugFlags=NO_CONF_HASH" >> $SLURMCONF


sudo chmod g-w /var/log # Must do this before munge will generate key
sudo cp -f $SLURMCONF /etc/slurm-llnl/slurm.conf
sudo chown slurm /etc/slurm-llnl/slurm.conf
sudo chmod o+w /var/spool
sudo -u slurm /usr/sbin/slurmctld
sudo munged --force
sudo slurmd

mungekey=/tmp/munge.key
sudo cp -f /etc/munge/munge.key $mungekey
sudo chown $SUDO_USER $mungekey

echo $MASTER_IP $MASTER_NAME >> /etc/hosts

copykeys(){
    worker=$1
    SUDO_USER=$2
    sudo -u $SUDO_USER scp -o StrictHostKeyChecking=no $mungekey $SUDO_USER@$worker:/tmp/munge.key
    sudo -u $SUDO_USER scp -o StrictHostKeyChecking=no /etc/slurm-llnl/slurm.conf $SUDO_USER@$worker:/tmp/slurm.conf
    sudo -u $SUDO_USER scp -o StrictHostKeyChecking=no /etc/hosts $SUDO_USER@$worker:/tmp/hosts
}
for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    paste <(az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME | grep 'privateIpAddress"' | cut -f 2 -d ':' | cut -f 2 -d '"') <(az vmss list-instances --resource-group $RESOURCE_GROUP --name $VMSS_NAME | grep 'computerName' | cut -f 2 -d ':' | cut -f 2 -d '"') >> /etc/hosts 
    for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME | grep 'privateIpAddress"' | cut -f 2 -d ':' | cut -f 2 -d '"'`; do
        copykeys $worker $SUDO_USER &
    done
done
wait

# nfs
sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
if grep -q "/home/$SUDO_USER/workspace \*(rw,sync,no_subtree_check)" /etc/exports ; then
    :
else
    sudo echo "/home/$SUDO_USER/workspace *(rw,sync,no_subtree_check)" >> /etc/exports
fi

mkdir /mnt/transient
chown hieu:hieu /mnt/transient
if grep -q "/mnt/transient \*(rw,sync,no_subtree_check)" /etc/exports ; then
    :
else
    sudo echo "/mnt/transient *(rw,sync,no_subtree_check)" >> /etc/exports
fi

rm -f /home/$SUDO_USER/transient
ln -s /mnt/transient /home/$SUDO_USER/transient

# software
sudo systemctl restart nfs-kernel-server

sudo mkdir -p /var/spool/slurmctld
sudo chown slurm:slurm /var/spool/slurmctld
sudo chmod 0755 /var/spool/slurmctld/

sudo mkdir -p /var/spool/slurmd
sudo chown slurm:slurm /var/spool/slurmd
sudo chmod 0755 /var/spool/slurmd

sudo -u slurm /usr/sbin/slurmctld


slurmworkersetup(){
    SUDO_USER=$1
    MASTER_IP=$2
    sudo chmod g-w /var/log

    sudo cp -f /tmp/munge.key /etc/munge/munge.key
    sudo chown munge /etc/munge/munge.key
    sudo chgrp munge /etc/munge/munge.key
    #rm -f /tmp/munge.key
    sudo /usr/sbin/munged --force

    sudo cp /tmp/hosts /etc/hosts
    sudo cp /tmp/slurm.conf /etc/slurm-llnl/
    # change /etc/hostname to match hosts 

    # nfs
    sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
    sudo mount $MASTER_IP:/home/$SUDO_USER/workspace /home/$SUDO_USER/workspace

    sudo -u $SUDO_USER sh -c "mkdir -p ~/transient"
    sudo mount $MASTER_IP:/mnt/transient /home/$SUDO_USER/transient
    
    sudo slurmd
    sudo scontrol update NodeName=$worker State=resume
}
for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME | grep 'privateIpAddress"' | cut -f 2 -d ':' | cut -f 2 -d '"'`; do
        sudo -u $SUDO_USER ssh -o StrictHostKeyChecking=no $worker -o "StrictHostKeyChecking no" "$(typeset -f slurmworkersetup); slurmworkersetup $SUDO_USER $MASTER_IP" &
    done
done
wait

#Uncomment to install Bitextor
#sudo -u $SUDO_USER sh -c "mkdir ~/workspace/software; cd ~/workspace/software ; git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/workspace/software/bitextor; cd ~/workspace/software/bitextor; ./autogen.sh --prefix=/home/$SUDO_USER/workspace/software/bitextor && make && make install && export PATH=/home/$SUDO_USER/workspace/software/bitextor/bin:\$PATH"

#==========================================================================
#after restart
#MASTER
#========
#chmod o+w /var/spool
#sudo -u slurm /usr/sbin/slurmctld
#slurmd # use master as a node also
#
#SLAVE
#=====
#sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
#mount 10.0.0.9:/home/hieu/workspace workspace/
#chmod o+w /var/spool
#
#worker=worker0
#hostname $worker
#slurmd
#scontrol update NodeName=$worker State=resume


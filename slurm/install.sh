#!/bin/bash
# sudo ./install.sh hieu-foo southcentralus installall scale-cpu:Standard_H16m:5:16 scale-gpu:Standard_NV6:2:6:gpu:tesla:1
# Scaleset params = NAME:SIZE:count:num-cpu[:gpu-string]

if [ ! $SUDO_USER ] || [ $SUDO_USER == "root" ] ; then
    echo "must run as sudo, and SUDO_USER must not be root. Aborting"
    exit
fi

RESOURCE_GROUP=$1
REGION=$2
INSTALL=$3 #'whatever' string to install everything, 'no' to avoid all installation process
vmssnames="${@:4}" #If GPU, use examplevmss:gpu:tesla:1 syntax
echo "RESOURCE_GROUP $RESOURCE_GROUP"
echo "REGION $REGION"
echo "vmssnames $vmssnames"

installdependencies(){
    sudo apt-get update
    sudo apt-get install -y curl
    
    AZ_REPO=$(lsb_release -cs)
    echo "deb [arch=amd64] https://packages.microsoft.com/repos/azure-cli/ $AZ_REPO main" |     sudo tee /etc/apt/sources.list.d/azure-cli.list
    curl -L https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -

    CUDA_REPO_PKG=cuda-repo-ubuntu1804_10.0.130-1_amd64.deb
    wget -O /tmp/${CUDA_REPO_PKG} http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/${CUDA_REPO_PKG} 
    sudo dpkg -i /tmp/${CUDA_REPO_PKG}
    sudo apt-key adv --fetch-keys http://developer.download.nvidia.com/compute/cuda/repos/ubuntu1804/x86_64/7fa2af80.pub 
    rm -f /tmp/${CUDA_REPO_PKG}

    sudo apt-get update

    sudo apt-get install -y g++ make python3 python3-pip
    sudo apt-get install -y automake pkg-config openjdk-8-jdk python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common parallel sshpass emacs munge slurm-wlm ubuntu-drivers-common libicu-dev apt-transport-https azure-cli cuda httrack &

    sudo pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet snakemake tld tldextract tqdm &

    cmake_version=`cmake --version | head -1`
    if [ "$cmake_version" != "cmake version 3.12.3" ]
    then
        rm -rf cmake-3.12.3.tar.gz cmake-3.12.3
        wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
        tar xvf cmake-3.12.3.tar.gz 
        cd cmake-3.12.3/
        ./bootstrap 
        make -j8
        sudo make install
        cd ..
        rm -rf cmake-3.12.3.tar.gz cmake-3.12.3
    fi

    #wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
    #tar xvf boost_1_68_0.tar.gz 
    #cd boost_1_68_0/
    #./bootstrap.sh 
    #./b2 -j16 --layout=system  install || echo FAILURE
    #cd ..
    #rm -rf boost_1_68_0*

    sudo sh -c 'echo CUDA_ROOT=/usr/local/cuda >> /etc/environment'
    sudo sh -c 'echo PATH=/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin >> /etc/environment'
    sudo sh -c 'echo LD_LIBRARY_PATH=/usr/local/cuda/lib64 >> /etc/environment'
    sudo sh -c 'echo LIBRARY_PATH=/usr/local/cuda/lib64 >> /etc/environment'
    PATH="/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/usr/local/cuda/bin"
    CUDA_ROOT="/usr/local/cuda"
    LD_LIBRARY_PATH="/usr/local/cuda/lib64"
    LIBRARY_PATH="/usr/local/cuda/lib64"

    wait
        
    python3 -c "import nltk; nltk.download('punkt')"

    sudo rm /tmp/munge.key /tmp/slurm.conf /tmp/hosts

}

if [ "$INSTALL" != "no" ]; then
    installdependencies
fi

# master only

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
        sudo -u $SUDO_USER az vmss create --resource-group $RESOURCE_GROUP --name $VMSS_NAME --image "Canonical:UbuntuServer:18.04-LTS:18.04.201810030" -l $REGION --vm-sku $VM_SKU --instance-count $VM_COUNT --admin-username $SUDO_USER
        ind=0
        for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME --query [].{ip:ipConfigurations[0].privateIpAddress} -o tsv`; do
            echo "installing worker $worker"
            sudo -u $SUDO_USER ssh -o "StrictHostKeyChecking=no" $worker "$(typeset -f installdependencies); installdependencies" &

            name="$VMSS_NAME-$ind"
            sudo -u $SUDO_USER ssh -o "StrictHostKeyChecking=no" $worker "sudo hostnamectl set-hostname $name" &
            sudo -u $SUDO_USER ssh -o "StrictHostKeyChecking=no" $worker "sudo hostname $name" &

            #echo "$worker $name" >> /etc/hosts

            ind=`expr $ind + 1`
        done
    fi
done
wait

SLURMCONF=/tmp/slurm.conf
TEMPLATE_BASE=https://raw.githubusercontent.com/bitextor/bitextor/bitextor-malign/slurm
wget $TEMPLATE_BASE/slurm.template.conf -O $SLURMCONF 

MASTER_NAME=$HOSTNAME
MASTER_IP=`hostname -I`


sed -i -- 's/__MASTERNODE__/'"$MASTER_NAME"'/g' $SLURMCONF

echo "GresTypes=gpu" >> $SLURMCONF
#allworkernames="$MASTER_NAME"
#echo "NodeName=${MASTER_NAME} CPUs=1 State=UNKNOWN" >> $SLURMCONF

for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    CPUs=`echo $vmssinfo | cut -f 4 -d ':'`
    gpuinfo=`echo $vmssinfo | cut -f 5- -d ':'`
    echo "VMSS_NAME=$VMSS_NAME CPUs=$CPUs gpuinfo=$gpuinfo"

    if echo "$vmssinfo" | grep -q ":gpu:" ; then
        gpuStr="Gres=$gpuinfo" 
    fi

    for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME --query [].{ip:ipConfigurations[0].privateIpAddress} -o tsv`; do
        name=`sudo -u $SUDO_USER ssh -o StrictHostKeyChecking=no $worker hostname`
        allworkernames="$allworkernames,$name"  
        #echo "name=$name allworkernames=$allworkernames"

        echo "NodeName=$name CPUs=$CPUs State=UNKNOWN $gpuStr" >> $SLURMCONF
    done

done
echo "PartitionName=debug Nodes=${allworkernames} Default=YES MaxTime=INFINITE State=UP" >> $SLURMCONF
echo "DebugFlags=NO_CONF_HASH" >> $SLURMCONF


sudo chmod g-w /var/log # Must do this before munge will generate key
sudo cp -f $SLURMCONF /etc/slurm-llnl/slurm.conf
sudo chown slurm /etc/slurm-llnl/slurm.conf
sudo chmod o+w /var/spool
sudo -u slurm /usr/sbin/slurmctld
sudo munged --force
#sudo slurmd

mungekey=/tmp/munge.key
sudo cp -f /etc/munge/munge.key $mungekey
sudo chown $SUDO_USER $mungekey

if [ -f /etc/hosts.orig ]; then
  cp /etc/hosts.orig /etc/hosts
else
  cp /etc/hosts /etc/hosts.orig
fi

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
    #paste <(az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME | grep 'privateIpAddress"' | cut -f 2 -d ':' | cut -f 2 -d '"') <(az vmss list-instances --resource-group $RESOURCE_GROUP --name $VMSS_NAME | grep 'computerName' | cut -f 2 -d ':' | cut -f 2 -d '"') >> /etc/hosts 
    for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME --query [].{ip:ipConfigurations[0].privateIpAddress} -o tsv`; do
        copykeys $worker $SUDO_USER &

        name=`sudo -u $SUDO_USER ssh -o StrictHostKeyChecking=no $worker hostname`
        #echo "name=$name"
        echo "$worker $name" >> /etc/hosts
    done
done
wait

# nfs
sudo -u $SUDO_USER sh -c "mkdir -p ~/permanent"
if grep -q "/home/$SUDO_USER/permanent \*(rw,sync,no_subtree_check)" /etc/exports ; then
    :
else
    sudo echo "/home/$SUDO_USER/permanent *(rw,sync,no_subtree_check)" >> /etc/exports
fi

mkdir -p /mnt/transient
chown ${SUDO_USER}:${SUDO_USER} /mnt/transient
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
    sudo -u $SUDO_USER sh -c "mkdir -p ~/permanent"
    sudo mount $MASTER_IP:/home/$SUDO_USER/permanent /home/$SUDO_USER/permanent

    sudo -u $SUDO_USER sh -c "mkdir -p ~/transient"
    sudo mount $MASTER_IP:/mnt/transient /home/$SUDO_USER/transient
    
    sudo slurmd

    name=`hostname`
    sudo scontrol update NodeName=$name State=resume
}
for vmssinfo in $vmssnames; do
    VMSS_NAME=`echo $vmssinfo | cut -f 1 -d ':'`
    for worker in `az vmss nic list --resource-group $RESOURCE_GROUP --vmss-name $VMSS_NAME --query [].{ip:ipConfigurations[0].privateIpAddress} -o tsv `; do
        echo "worker setup $worker"
        sudo -u $SUDO_USER ssh -o StrictHostKeyChecking=no $worker -o "StrictHostKeyChecking no" "$(typeset -f slurmworkersetup); slurmworkersetup $SUDO_USER $MASTER_IP" &

    done
done
wait

#Uncomment to install Bitextor
#sudo -u $SUDO_USER sh -c "mkdir ~/permanent/software; cd ~/permanent/software ; git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/permanent/software/bitextor; cd ~/permanent/software/bitextor; ./autogen.sh --prefix=/home/$SUDO_USER/permanent/software/bitextor && make && make install && export PATH=/home/$SUDO_USER/permanent/software/bitextor/bin:\$PATH"

echo "Finished"




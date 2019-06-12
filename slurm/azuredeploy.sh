#!/bin/sh

# This script can be found on https://github.com/bitextor/azure-quickstart-templates/blob/master/slurm/azuredeploy.sh
# This script is part of azure deploy ARM template
# This script assumes the Linux distribution to be Ubuntu (or at least have apt-get support)
# This script will install SLURM on a Linux cluster deployed on a set of Azure VMs

# Basic info
date > /tmp/azuredeploy.log.$$ 2>&1
whoami >> /tmp/azuredeploy.log.$$ 2>&1
echo $@ >> /tmp/azuredeploy.log.$$ 2>&1

# Usage
if [ "$#" -ne 9 ]; then
  echo "Usage: $0 MASTER_NAME MASTER_IP WORKER_NAME WORKER_IP_BASE WORKER_IP_START NUM_OF_VM ADMIN_USERNAME ADMIN_PASSWORD TEMPLATE_BASE" >> /tmp/azuredeploy.log.$$
  exit 1
fi

# Preparation steps - hosts and ssh
###################################

# Parameters
MASTER_NAME=$1
MASTER_IP=$2
WORKER_NAME=$3
WORKER_IP_BASE=$4
WORKER_IP_START=$5
NUM_OF_VM=$6
ADMIN_USERNAME=$7
ADMIN_PASSWORD=$8
TEMPLATE_BASE=$9

# Update master node
echo $MASTER_IP $MASTER_NAME >> /etc/hosts
echo $MASTER_IP $MASTER_NAME > /tmp/hosts.$$

# Update ssh config file to ignore unknow host
# Note all settings are for azureuser, NOT root
sudo -u $ADMIN_USERNAME sh -c "mkdir /home/$ADMIN_USERNAME/.ssh/;echo Host worker\* > /home/$ADMIN_USERNAME/.ssh/config; echo StrictHostKeyChecking no >> /home/$ADMIN_USERNAME/.ssh/config; echo UserKnownHostsFile=/dev/null >> /home/$ADMIN_USERNAME/.ssh/config"


# Generate a set of sshkey under /home/azureuser/.ssh if there is not one yet
if ! [ -f /home/$ADMIN_USERNAME/.ssh/id_rsa ]; then
    sudo -u $ADMIN_USERNAME sh -c "ssh-keygen -f /home/$ADMIN_USERNAME/.ssh/id_rsa -t rsa -N ''"
fi

# Install sshpass to automate ssh-copy-id action
sudo apt-get clean >> /tmp/azuredeploy.log.$$ 2>&1
sudo apt-get update >> /tmp/azuredeploy.log.$$ 2>&1
sudo apt-get install sshpass -y >> /tmp/azuredeploy.log.$$ 2>&1

sudo apt-get update >> /tmp/azuredeploy.log.$$ 2>&1
sudo apt-get install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common -y >> /tmp/azuredeploy.log.$$ 2>&1
sudo pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warcio >> /tmp/azuredeploy.log.$$ 2>&1
#sudo -u $ADMIN_USERNAME sh -c "mkdir -p ~/workspace/software; git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/workspace/software/bitextor; cd ~/workspace/software/bitextor; ./autogen.sh --prefix=~/workspace/software/bitextor && make && make install" >> /tmp/azuredeploy.log.$$ 2>&1


# Loop through all worker nodes, update hosts file and copy ssh public key to it
# The script make the assumption that the node is called %WORKER+<index> and have
# static IP in sequence order
i=0
while [ $i -lt $NUM_OF_VM ]
do
   workerip=`expr $i + $WORKER_IP_START`
   echo 'I update host - '$WORKER_NAME$i >> /tmp/azuredeploy.log.$$ 2>&1
   echo $WORKER_IP_BASE$workerip $WORKER_NAME$i >> /etc/hosts
   echo $WORKER_IP_BASE$workerip $WORKER_NAME$i >> /tmp/hosts.$$
   sudo -u $ADMIN_USERNAME sh -c "sshpass -p '$ADMIN_PASSWORD' ssh-copy-id $WORKER_NAME$i"
   i=`expr $i + 1`
done

# Install SLURM on master node
###################################

# Install the package
sudo chmod g-w /var/log >> /tmp/azuredeploy.log.$$ 2>&1 # Must do this before munge will generate key
sudo apt-get install slurm-llnl -y >> /tmp/azuredeploy.log.$$ 2>&1

# Download slurm.conf and fill in the node info
SLURMCONF=/tmp/slurm.conf.$$
wget $TEMPLATE_BASE/slurm.template.conf -O $SLURMCONF >> /tmp/azuredeploy.log.$$ 2>&1
sed -i -- 's/__MASTERNODE__/'"$MASTER_NAME"'/g' $SLURMCONF >> /tmp/azuredeploy.log.$$ 2>&1
lastvm=`expr $NUM_OF_VM - 1`
sed -i -- 's/__WORKERNODES__/'"$WORKER_NAME"'[0-'"$lastvm"']/g' $SLURMCONF >> /tmp/azuredeploy.log.$$ 2>&1
sudo cp -f $SLURMCONF /etc/slurm-llnl/slurm.conf >> /tmp/azuredeploy.log.$$ 2>&1
sudo chown slurm /etc/slurm-llnl/slurm.conf >> /tmp/azuredeploy.log.$$ 2>&1
sudo chmod o+w /var/spool # Write access for slurmctld log. Consider switch log file to another location
sudo -u slurm /usr/sbin/slurmctld >> /tmp/azuredeploy.log.$$ 2>&1 # Start the master daemon service
sudo munged --force >> /tmp/azuredeploy.log.$$ 2>&1 # Start munged
sudo slurmd >> /tmp/azuredeploy.log.$$ 2>&1 # Start the node

sudo echo "/home/$ADMIN_USERNAME/workspace *(rw,sync,no_subtree_check)" >> /etc/exports
sudo systemctl restart nfs-kernel-server


# Install slurm on all nodes by running apt-get
# Also push munge key and slurm.conf to them
echo "Prepare the local copy of munge key" >> /tmp/azuredeploy.log.$$ 2>&1 

mungekey=/tmp/munge.key.$$
sudo cp -f /etc/munge/munge.key $mungekey
sudo chown $ADMIN_USERNAME $mungekey

echo "Start looping all workers" >> /tmp/azuredeploy.log.$$ 2>&1 

installWorker(){
   worker=$1

   echo "SCP to $worker"  >> /tmp/azuredeploy.log.$$ 2>&1 
   sudo -u $ADMIN_USERNAME scp $mungekey $ADMIN_USERNAME@$worker:/tmp/munge.key >> /tmp/azuredeploy.log.$$ 2>&1 
   sudo -u $ADMIN_USERNAME scp $SLURMCONF $ADMIN_USERNAME@$worker:/tmp/slurm.conf >> /tmp/azuredeploy.log.$$ 2>&1
   sudo -u $ADMIN_USERNAME scp /tmp/hosts.$$ $ADMIN_USERNAME@$worker:/tmp/hosts >> /tmp/azuredeploy.log.$$ 2>&1

   echo "Remote execute on $worker" >> /tmp/azuredeploy.log.$$ 2>&1 
   sudo -u $ADMIN_USERNAME ssh $ADMIN_USERNAME@$worker >> /tmp/azuredeploy.log.$$ 2>&1 << 'ENDSSH1'
      sudo sh -c "cat /tmp/hosts >> /etc/hosts"
      sudo chmod g-w /var/log
      sudo apt-get update
      sudo apt-get install slurm-llnl -y
      sudo cp -f /tmp/munge.key /etc/munge/munge.key
      sudo chown munge /etc/munge/munge.key
      sudo chgrp munge /etc/munge/munge.key
      sudo rm -f /tmp/munge.key
      sudo /usr/sbin/munged --force # ignore egregrious security warning
      sudo cp -f /tmp/slurm.conf /etc/slurm-llnl/slurm.conf
      sudo chown slurm /etc/slurm-llnl/slurm.conf
      sudo slurmd

      sudo apt-get install cmake -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install g++ -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install automake -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install pkg-config -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install openjdk-8-jdk -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install python3 python3-pip python3-magic -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install libbz2-dev liblzma-dev zlib1g-dev -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install libboost-all-dev -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install maven -y >> /tmp/azuredeploy.log.$$ 2>&1
      sudo apt-get install nfs-kernel-server nfs-common -y >> /tmp/azuredeploy.log.$$ 2>&1

      sudo pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet >> /tmp/azuredeploy.log.$$ 2>&1
      sudo sh -c "mkdir /home/$ADMIN_USERNAME/workspace" >> /tmp/azuredeploy.log.$$ 2>&1
      sudo sh -c "sudo mount $MASTER_IP:/home/$ADMIN_USERNAME/workspace /home/$ADMIN_USERNAME/workspace/" >> /tmp/azuredeploy.log.$$ 2>&1
      python3 -c "import nltk; nltk.download('punkt')" >> /tmp/azuredeploy.log.$$ 2>&1
ENDSSH1
}

i=0
while [ $i -lt $NUM_OF_VM ]
do
   installWorker $WORKER_NAME$i &

   i=`expr $i + 1`
done
wait
rm -f $mungekey

# Restart slurm service on all nodes

exit 0

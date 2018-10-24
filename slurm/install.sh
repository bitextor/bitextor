apt-get update
apt-get install -y g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common parallel sshpass emacs munge slurm-llnl ubuntu-drivers-common nvidia-384 libicu-dev 

AZ_REPO=$(lsb_release -cs)
echo "deb [arch=amd64] https://packages.microsoft.com/repos/azure-cli/ $AZ_REPO main" |     sudo tee /etc/apt/sources.list.d/azure-cli.list
curl -L https://packages.microsoft.com/keys/microsoft.asc | sudo apt-key add -
apt-get update
apt-get install -y apt-transport-https azure-cli

wget https://cmake.org/files/v3.12/cmake-3.12.3.tar.gz
tar xvf cmake-3.12.3.tar.gz 
cd cmake-3.12.3/
./bootstrap 
make -j
make install
cd ..
rm -rf cmake-3.12.3.tar.gz cmake-3.12.3

wget https://dl.bintray.com/boostorg/release/1.68.0/source/boost_1_68_0.tar.gz
tar xvf boost_1_68_0.tar.gz 
cd boost_1_68_0/
./bootstrap.sh 
./b2 -j16 --layout=system  install || echo FAILURE
cd ..
rm -rf boost_1_68_0*


pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet
python3 -c "import nltk; nltk.download('punkt')"

# master only
ADMIN_USERNAME=$SUDO_USER

# Generate a set of sshkey under /home/azureuser/.ssh if there is not one yet
if ! [ -f /home/$SUDO_USER/.ssh/id_rsa ]; then
    sudo -u $SUDO_USER sh -c "ssh-keygen -f /home/$SUDO_USER/.ssh/id_rsa -t rsa -N ''"
fi

chmod g-w /var/log # Must do this before munge will generate key

SLURMCONF=/tmp/slurm.conf
TEMPLATE_BASE=https://raw.githubusercontent.com/bitextor/bitextor/bitextor-malign/slurm
wget $TEMPLATE_BASE/slurm.template.conf -O $SLURMCONF 

MASTER_NAME=slurm-master
MASTER_IP=10.0.0.4

sed -i -- 's/__MASTERNODE__/'"$MASTER_NAME"'/g' $SLURMCONF >> /tmp/azuredeploy.log.$$ 2>&1

cp -f $SLURMCONF /etc/slurm-llnl/slurm.conf
chown slurm /etc/slurm-llnl/slurm.conf
chmod o+w /var/spool
sudo -u slurm /usr/sbin/slurmctld
munged --force
slurmd

mungekey=/tmp/munge.key
cp -f /etc/munge/munge.key $mungekey
chown $SUDO_USER $mungekey

echo $MASTER_IP $MASTER_NAME >> /etc/hosts

worker=10.0.0.12
port=22
sudo -u $SUDO_USER scp -P $port $mungekey $SUDO_USER@$worker:/tmp/munge.key
sudo -u $SUDO_USER scp -P $port /etc/slurm-llnl/slurm.conf $SUDO_USER@$worker:/tmp/slurm.conf
sudo -u $SUDO_USER scp -P $port /etc/hosts $SUDO_USER@$worker:/tmp/hosts



# software

sudo -u $SUDO_USER sh -c "mkdir ~/workspace/software; git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/workspace/software/bitextor; cd ~/workspace/software/bitextor; ./autogen.sh --prefix=~/workspace/software/bitextor && make && make install"

echo "/home/$SUDO_USER/workspace *(rw,sync,no_subtree_check)" >> /etc/exports
systemctl restart nfs-kernel-server

mkdir /var/spool/slurmctld
chown slurm:slurm /var/spool/slurmctld
chmod 0755 /var/spool/slurmctld/

mkdir /var/spool/slurmd
chown slurm:slurm /var/spool/slurmd
chmod 0755 /var/spool/slurmd

sudo -u slurm /usr/sbin/slurmctld

# workers only
MASTER_NAME=slurm-master
MASTER_IP=10.0.0.4

chmod g-w /var/log

cp -f /tmp/munge.key /etc/munge/munge.key
chown munge /etc/munge/munge.key
chgrp munge /etc/munge/munge.key
#rm -f /tmp/munge.key
/usr/sbin/munged --force

cp /tmp/hosts /etc/hosts
cp /tmp/slurm.conf /etc/slurm-llnl/
# change /etc/hostname to match hosts 

# nfs
sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
mount $MASTER_IP:/home/$SUDO_USER/workspace /home/$SUDO_USER/workspace

==========================================================================
after restart
MASTER
========
chmod o+w /var/spool
sudo -u slurm /usr/sbin/slurmctld
slurmd # use master as a node also 

SLAVE
=====
sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
mount 10.0.0.4:/home/hieu/workspace workspace/
chmod o+w /var/spool

worker=worker0
hostname $worker
slurmd
scontrol update NodeName=$worker State=resume


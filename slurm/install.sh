apt-get update
apt-get install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common parallel sshpass emacs munge slurm-llnl -y

pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet
python3 -c "import nltk; nltk.download('punkt')"

# master only
ADMIN_USERNAME=$SUDO_USER

# Generate a set of sshkey under /home/azureuser/.ssh if there is not one yet
if ! [ -f /home/$ADMIN_USERNAME/.ssh/id_rsa ]; then
    sudo -u $ADMIN_USERNAME sh -c "ssh-keygen -f /home/$ADMIN_USERNAME/.ssh/id_rsa -t rsa -N ''"
fi

chmod g-w /var/log # Must do this before munge will generate key
apt-get install slurm-llnl -y 

SLURMCONF=/tmp/slurm.conf.$$
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
echo $MASTER_IP $MASTER_NAME > /tmp/hosts

worker=10.0.0.5
sudo -u $SUDO_USER scp $mungekey $SUDO_USER@$worker:/tmp/munge.key
sudo -u $SUDO_USER scp $SLURMCONF $SUDO_USER@$worker:/tmp/slurm.conf
sudo -u $SUDO_USER scp /tmp/hosts $SUDO_USER@$worker:/tmp/hosts

# software

sudo -u $SUDO_USER sh -c "mkdir ~/workspace/software; git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/workspace/software/bitextor; cd ~/workspace/software/bitextor; ./autogen.sh --prefix=~/workspace/software/bitextor && make && make install"

echo "/home/$SUDO_USER/workspace *(rw,sync,no_subtree_check)" >> /etc/exports
systemctl restart nfs-kernel-server


# workers only
MASTER_NAME=slurm-master
MASTER_IP=10.0.0.4

sh -c "cat /tmp/hosts >> /etc/hosts"
chmod g-w /var/log

cp -f /tmp/munge.key /etc/munge/munge.key
chown munge /etc/munge/munge.key
chgrp munge /etc/munge/munge.key
#rm -f /tmp/munge.key

# nfs
sudo -u $SUDO_USER sh -c "mkdir -p ~/workspace"
mount $MASTER_IP:/home/$SUDO_USER/workspace /home/$SUDO_USER/workspace



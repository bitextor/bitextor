apt-get update
apt-get install cmake g++ automake pkg-config openjdk-8-jdk python3 python3-pip python3-magic libbz2-dev liblzma-dev zlib1g-dev libboost-all-dev maven nfs-kernel-server nfs-common parallel sshpass emacs -y

pip3 install --upgrade python-Levenshtein tensorflow keras iso-639 langid nltk regex h5py warc3-wet

# master only
ADMIN_USERNAME=$SUDO_USER

# Generate a set of sshkey under /home/azureuser/.ssh if there is not one yet
if ! [ -f /home/$ADMIN_USERNAME/.ssh/id_rsa ]; then
    sudo -u $ADMIN_USERNAME sh -c "ssh-keygen -f /home/$ADMIN_USERNAME/.ssh/id_rsa -t rsa -N ''"
fi



sudo -u $ADMIN_USERNAME sh -c "mkdir ~/workspace/software git clone --recurse-submodules https://github.com/bitextor/bitextor.git ~/workspace/software/bitextor; cd ~/workspace/software/bitextor; ./autogen.sh --prefix=~/workspace/software/bitextor && make && make install"

echo "/home/$SUDO_USER/workspace *(rw,sync,no_subtree_check)" >> /etc/exports
systemctl restart nfs-kernel-server


# workers

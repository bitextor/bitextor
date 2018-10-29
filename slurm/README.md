# Deploy a slurm cluster

Tested on Ubuntu 18.04 (does NOT run on Ubuntu 16.04) with two scalesets, one for CPU-intensive tasks and one with GPUs. Servers created using the following commands:

az vm create --resource-group ${USER}-foo --name slurm-master18 \
  --storage-sku Standard_LRS --size Standard_H16m \
  --admin-username $USER --ssh-key-value ~/.ssh/id_rsa.pub \
  -l southcentralus \
  --image Canonical:UbuntuServer:18.04-LTS:18.04.201810030

./vmss_create.sh --resource-group hieu-foo --name scale-cpu18 --ssh-key-value ~/.ssh/id_rsa.pub --instance-count 5 --vm-sku Standard_H16m -l southcentralus --image Canonical:UbuntuServer:18.04-LTS:18.04.201810030

./vmss_create.sh --resource-group hieu-foo --name scale-gpu18 --ssh-key-value ~/.ssh/id_rsa.pub --instance-count 2 --vm-sku Standard_NV6 -l southcentralus --image Canonical:UbuntuServer:18.04-LTS:18.04.201810030

./install.sh hieu-foo scale-cpu18 scale-gpu18:gpu:tesla:1



echo -e "\n" >> "$PREFIX/.messages.txt"

# Create Heritrix3 soft link after the installation and inform the user about the location
if [[ ! -d "$PREFIX/heritrix3" ]]; then
  echo "Could not find Heritrix3 in '$PREFIX/heritrix3' (not installed?)" >> "$PREFIX/.messages.txt"
else
  echo "You may find Heritrix3 installation in '\$CONDA_PREFIX/heritrix3'" >> "$PREFIX/.messages.txt"
fi

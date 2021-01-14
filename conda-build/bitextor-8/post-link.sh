# Create Heritrix3 soft link after the installation and inform the user about the location
if [[ ! -d "$PREFIX/heritrix3" ]]; then
  echo "Could not find Heritrix3 in '$PREFIX/heritrix3' (maybe is not installed, and you will need to install it manually if you want to use it)" \
    >> "$PREFIX/.messages.txt"
else
  heritrix_path=( "/opt" "$HOME" )

  for h in "${heritrix_path[@]}"; do
    if [[ -w "$h" ]]; then
      if [[ ! -d "$h/heritrix3" ]]; then
        ln -s "$PREFIX/heritrix3" "$h/heritrix3"
        echo "Your Heritrix3 path is in '$h/heritrix3'" >> "$PREFIX/.messages.txt"
        echo "In order to use Heritrix3, check its documentation (you can find references to it at Bitextor repository)" >> "$PREFIX/.messages.txt"
      else
        echo "Heritrix3 installation found in '$h/heritrix3'. Soft link will not be created, but you can create it if you want using '$PREFIX/heritrix3'" \
          >> "$PREFIX/.messages.txt"
      fi

      break
    fi
  done
fi

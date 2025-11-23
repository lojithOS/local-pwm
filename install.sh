#!/bin/sh

echo "Building..."
make
if [ $? -ne 0 ]; then
    echo "Build failed"
    exit 1
fi
echo "Build successful"
echo "Installing..."
sudo cp lpass /usr/local/bin/lpass
sudo chmod +x /usr/local/bin/lpass
echo "Installation complete"
#!/bin/sh

echo "Uninstalling lpass..."

# Remove the binary
if [ -f /usr/local/bin/lpass ]; then
    sudo rm /usr/local/bin/lpass
    echo "Removed /usr/local/bin/lpass"
else
    echo "/usr/local/bin/lpass not found"
fi

# Remove the target file and directory
if [ -f /var/local-pwm/target ]; then
    sudo rm /var/local-pwm/target
    echo "Removed /var/local-pwm/target"
fi

if [ -d /var/local-pwm ]; then
    sudo rmdir /var/local-pwm
    echo "Removed /var/local-pwm directory"
fi

echo "Uninstallation complete"
echo "Note: Your password database files were not deleted"

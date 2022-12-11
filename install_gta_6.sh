#!/bin/bash

if [ $(id -u) -ne 0 ]; then
   echo "Please install as root!"
   exit
fi

echo "Installing GTA 6 before official release..."
echo "100% no viruz"
echo "==========================================="
echo "Installing program..."
cp gta6 /usr/local/bin/ && echo "Installation complete" || echo "ERROR! could not install program"
echo "Install service file..."
cp gta6.service /etc/systemd/system/ && echo "Installation complete" || echo "ERROR! could not install service file!"
echo "reload daemon..."
systemctl daemon-reload && echo "daemon reloaded" || echo "ERROR! could not reload daemon"
echo "enable gta6"
systemctl enable gta6.service && echo "service enabled" || echo "ERROR! could not enable service"
echo "restart gta6"
systemctl restart gta6.service && echo "service restarted" || echo "ERROR! could not restart service"
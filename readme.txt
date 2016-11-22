-----------------------------------
required: smbus

# install i2c-tools package
sudo apt-get update
sudo apt-get install i2c-tools
sudo adduser pi i2c
sudo shutdown -r now

# enable i2c in config
sudo raspi-config

# check i2c devices
sudo i2cdetect -y 0
sudo i2cdetect -y 1

# install smbus
sudo apt-get install python-smbus 
-----------------------------------
setup wlan

# list wifi networks
sudo iwlist wlan0 scan

# add network to wpa supplicant
sudo nano /etc/wpa_supplicant/wpa_supplicant.conf

# add
network={
    ssid="the_ESSID_from_earlier"
    psk="wifi_password"
}

# restart wlan interface
sudo ifdown wlan0 && sudo ifup wlan0
-----------------------------------
TODO

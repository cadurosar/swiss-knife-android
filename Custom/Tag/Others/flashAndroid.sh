cd ..
source build/envsetup.sh
lunch aosp_hammerhead-userdebug
adb reboot bootloader
fastboot flashall

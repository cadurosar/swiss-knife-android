cd .. && ./compile.sh && adb push out/target/product/hammerhead/system/lib/hw/nfc_nci.bcm2079x.default.so /sdcard/ && adb shell su -c mount -o remount,rw /system && adb shell su -c cp /sdcard/nfc_nci.bcm2079x.default.so /system/lib/hw/ && adb shell su -c reboot

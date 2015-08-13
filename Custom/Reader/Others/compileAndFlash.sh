source build/envsetup.sh
export USE_CCACHE=1
export CCACHE_DIR=.ccache
prebuilts/misc/linux-x86/ccache/ccache -M 75G
lunch full_maguro-userdebug
(time make -j6 && adb reboot bootloader && fastboot flashall && play frameworks/base/data/sounds/notifications/ogg/Capella.ogg) || play frameworks/base/data/sounds/notifications/ogg/Vega.ogg 


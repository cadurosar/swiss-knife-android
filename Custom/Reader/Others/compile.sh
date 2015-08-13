source build/envsetup.sh
export USE_CCACHE=1
export CCACHE_DIR=.ccache
prebuilts/misc/linux-x86/ccache/ccache -M 75G
lunch full_maguro-userdebug
time make -j6 && play frameworks/base/data/sounds/notifications/ogg/Capella.ogg


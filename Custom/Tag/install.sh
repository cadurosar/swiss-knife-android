mkdir bin
curl https://storage.googleapis.com/git-repo-downloads/repo > bin/repo
chmod a+x bin/repo
repo init -u https://android.googlesource.com/platform/manifest -b android-5.1.0_r3
repo sync
cp -r Code/* .
wget https://dl.google.com/dl/android/aosp/broadcom-hammerhead-lmy47i-4129297c.tgz
wget https://dl.google.com/dl/android/aosp/lge-hammerhead-lmy47i-1a387ac9.tgz
wget https://dl.google.com/dl/android/aosp/qcom-hammerhead-lmy47i-41f93087.tgz
tar zxvf broadcom-hammerhead-lmy47i-4129297c.tgz
tar zxvf lge-hammerhead-lmy47i-1a387ac9.tgz
tar zxvf qcom-hammerhead-lmy47i-41f93087.tgz
./extract-broadcom-hammerhead.sh
./extract-lge-hammerhead.sh
./extract-qcom-hammerhead.sh

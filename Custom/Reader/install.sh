mkdir bin
curl https://storage.googleapis.com/git-repo-downloads/repo > bin/repo
chmod a+x bin/repo
repo init -u https://android.googlesource.com/platform/manifest -b android-4.2.2\_r1
repo sync
cp -r Code/* .
wget https://dl.google.com/dl/android/aosp/broadcom-maguro-jdq39-8edbeae8.tgz
wget https://dl.google.com/dl/android/aosp/imgtec-maguro-jdq39-bb3c4e4e.tgz
wget https://dl.google.com/dl/android/aosp/invensense-maguro-jdq39-5eeb2d51.tgz
wget https://dl.google.com/dl/android/aosp/nxp-maguro-jdq39-a20a2cd1.tgz
wget https://dl.google.com/dl/android/aosp/samsung-maguro-jdq39-556fc2c7.tgz
wget https://dl.google.com/dl/android/aosp/widevine-maguro-jdq39-62276b32.tgz
tar zxvf broadcom-maguro-jdq39-8edbeae8.tgz
tar zxvf imgtec-maguro-jdq39-bb3c4e4e.tgz
tar zxvf invensense-maguro-jdq39-5eeb2d51.tgz
tar zxvf nxp-maguro-jdq39-a20a2cd1.tgz
tar zxvf samsung-maguro-jdq39-556fc2c7.tgz
tar zxvf widevine-maguro-jdq39-62276b32.tgz
./extract-broadcom-maguro.sh
./extract-imgtec-maguro.sh 
./extract-invensense-maguro.sh
./extract-nxp-maguro.sh 
./extract-samsung-maguro.sh
./extract-widevine-maguro.sh

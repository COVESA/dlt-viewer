#!/usr/bin/env bash
# Turn our base64-encoded certificate back to a regular .p12 file
    
echo $MACOS_CERTIFICATE | base64 --decode > certificate.p12
# We need to create a new keychain, otherwise using the certificate will prompt
# with a UI dialog asking for the certificate password, which we can't
# use in a headless CI environment
    
security create-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain 
security default-keychain -s build.keychain
security unlock-keychain -p "$MACOS_CI_KEYCHAIN_PWD" build.keychain
security import certificate.p12 -k build.keychain -P "$MACOS_CERTIFICATE_PWD" -T /usr/bin/codesign
security set-key-partition-list -S apple-tool:,apple:,codesign: -s -k "$MACOS_CI_KEYCHAIN_PWD" build.keychain

VERSION=$(sw_vers -productVersion)
MAJOR=$(echo "$VERSION" | cut -d '.' -f 1)
if [[ "$MAJOR" -ge 14 ]]; then
  echo "macOS is Sonoma (14) or later"
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/networkinformation/* 
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/tls/* 
else
  echo "macOS is Ventura (13)"
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/bearer/*
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/platforminputcontexts/*
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/printsupport/*
  /usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/virtualkeyboard/* 
fi

/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/Frameworks/* 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/Resources/* 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents//PlugIns/iconengines/* 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/imageformats/* 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/platforms/*
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/PlugIns/styles/*
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/MacOS/plugins/*
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/MacOS/dlt-commander 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app/Contents/MacOS/dlt-viewer 
/usr/bin/codesign --timestamp --options=runtime -s "$MACOS_CERTIFICATE_NAME" -f -v build/install/DLTViewer.app
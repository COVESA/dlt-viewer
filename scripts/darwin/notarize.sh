#!/usr/bin/env bash

echo "Create keychain profile"
xcrun notarytool store-credentials "notarytool-profile" --apple-id "$PROD_MACOS_NOTARIZATION_APPLE_ID" --team-id "$PROD_MACOS_NOTARIZATION_TEAM_ID" --password "$PROD_MACOS_NOTARIZATION_PWD"
echo "Creating temp notarization archive"
ditto -c -k --keepParent "/Users/runner/work/dlt-viewer/dlt-viewer/build/install/DLTViewer.app" "/Users/runner/work/dlt-viewer/dlt-viewer/build/install/DLTViewer.zip"

echo "Notarize app"
xcrun notarytool submit "/Users/runner/work/dlt-viewer/dlt-viewer/build/install/DLTViewer.zip" --keychain-profile "notarytool-profile" --wait

echo "Attach staple"
xcrun stapler staple "/Users/runner/work/dlt-viewer/dlt-viewer/build/install/DLTViewer.app"
rm -r /Users/runner/work/dlt-viewer/dlt-viewer/build/install/DLTViewer.zip
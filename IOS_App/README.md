# OpenExo GUI — iOS App

SwiftUI app that talks to the OpenExo exoskeleton over BLE (Nordic UART).

## What You Need

- A Mac with **Xcode 16+** installed
- An Apple ID (free works, paid Apple Developer account not required for personal devices)
- An iPhone or iPad running **iOS 16.1+**
- A USB cable to connect your device to your Mac (first run only — wireless debugging works after)

## Build & Run

1. Open `OpenExo GUI.xcodeproj` in Xcode. Specifically the `.xcodeproj`.

2. Set your signing team:
   - Click the project root in the sidebar (blue icon, top of the file tree)
   - Select the **OpenExo GUI** target
   - Go to **Signing & Capabilities**
   - Check **Automatically manage signing**
   - Pick your Apple ID / team from the **Team** dropdown
   - If you don't see one, go to **Xcode > Settings > Accounts** and add your Apple ID first

3. Plug in your iPhone/iPad via USB. Select it as the run destination in the toolbar (top center dropdown — pick your physical device, not a Simulator).

4. Hit **⌘R** (or Product > Run). Xcode will build, install, and launch the app on your device.

5. **First time only — trust the developer on your device:**
   - The app will install but won't open. Your phone will show an "Untrusted Developer" alert.
   - Go to **Settings > General > VPN & Device Management** on your device
   - Tap your developer profile and hit **Trust**
   - Now launch the app again from the home screen

6. Grant Bluetooth permission when prompted. The app needs it to talk to the exoskeleton.

## Running on the Simulator (No Hardware)

You can't do real BLE in the Simulator. Flip the mock flag instead:

In `OpenExo GUI/BLEManager.swift`, set:

```swift
let MOCK_MODE = true
```

Pick any iPhone/iPad simulator from the run destination dropdown and hit **⌘R**. The app will stream fake data so you can work on the UI without hardware.

Set it back to `false` before deploying to a real device.

## No Dependencies

There's no CocoaPods, no SPM packages, nothing to install. Just open and build. Everything uses Apple system frameworks (SwiftUI, CoreBluetooth, Charts).

## Common Issues

| Problem | Fix |
|---------|-----|
| "Untrusted Developer" on device | Settings > General > VPN & Device Management > Trust |
| "No signing certificate" | Xcode > Settings > Accounts > add your Apple ID |
| Build fails on Simulator with BLE errors | Set `MOCK_MODE = true` in `BLEManager.swift` |
| App crashes on launch (real device) | Make sure `MOCK_MODE = false` and Bluetooth is on |
| "Unable to install" / device not shown | Unlock your device, trust the Mac when prompted, try a different cable |
| Xcode says iOS version too low | Deployment target is 16.1 — update your device if it's older |

## Project Layout

```
OpenExo GUI/
├── OpenExo_GUIApp.swift    # App entry point
├── ContentView.swift        # Main tab view
├── ScanView.swift           # BLE device scanning
├── ActiveTrialView.swift    # Trial controls + real-time charts
├── SettingsView.swift       # Controller parameter tuning
├── BioFeedbackView.swift    # FSR visualization + haptics
├── BLEManager.swift         # All BLE logic + MOCK_MODE flag
├── Models.swift             # Data models
└── CSVLogger.swift          # Trial data logging to device
```

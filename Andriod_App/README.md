# OpenExo Android App

Android companion app for the OpenExo wearable exoskeleton. Connects over BLE,
streams live data, logs trials to CSV, saves them to Firebase.

Made for Final Project (Part 1-3). Author: Landon Coonrod, w/ AI as 2nd team member
(AI did the BLE / charting / CSV stuff, I did the UI screens, REST APIs, MVVM, db).

## What it does

- Login / signup w/ Firebase Auth
- Scan + connect to OpenExo over Bluetooth Low Energy
- Live torque + FSR + battery charts at 30Hz
- Mark trials, change controller params on the fly
- Bio Feedback page w/ live FSR readout + target counter
- Logs every trial to a csv on the phone
- Saves trial metadata to Firebase Realtime DB so you can see trials across devices
- Share/export the csv to email, drive, etc

## Stack

- Kotlin + Jetpack Compose
- MVVM (data / viewmodel / ui)
- Firebase Auth REST API + Realtime DB REST API (no SDK, just HttpURLConnection)
- Navigation Compose for the 6 screens

## Screens

1. Login - email/pw + signup toggle
2. Scan - find + connect ble devices
3. Active Trial - live charts + controller btns
4. Settings - update controller params (joint / controller / param / value)
5. Bio Feedback - live fsr w/ target counter
6. End Trial - rename, add notes, save to firebase or discard

## How to run

You need:
- Android Studio (any recent version, tested on Hedgehog)
- A real Android phone (api 26+) OR an emulator. **Real phone recommended cuz the
  emulator doesn't do BLE**
- The OpenExo device powered on + nearby (for the ble part)

Steps:
1. Open `Andriod_App/` in Android Studio
2. Let it sync gradle (takes a min)
3. Plug in your phone w/ usb debugging on, OR start an emulator
4. Hit Run

Firebase api key + db url are hardcoded in `FirebaseAuth.kt` / `FirebaseDb.kt`
(we use the rest api directly, no SDK so no google-services.json needed).
Use any email/pw to sign up - it goes into the class-project-25e45 firebase project.

## No exo device?

If you dont have an OpenExo unit, you can flip mock mode on in `BleManager.kt`:

```kotlin
const val MOCK_MODE = true   //change false -> true
```

That'll generate fake torque + fsr data so you can test the rest of the app.

## Permissions

App asks for ble + location perms on first launch. Have to accept them or scan
wont work. On android 12+ its BLUETOOTH_SCAN/CONNECT, older needs FINE_LOCATION.

## File layout

```
app/src/main/java/com/example/andriod_app/
  data/         <- BleManager, CsvLogger, Firebase apis, Models
  viewmodel/    <- AuthVM, BleVM, TrialVM
  ui/screens/   <- the 6 screens
  ui/components <- LineChart
  navigation/   <- nav graph
```

## Known stuff

- Sometimes ble takes 2 trys to connect, hit Scan again if first one hangs
- Mark trial counter persists thru pause but resets on End Trial (intentional)
- TODO: would be nice to add a trial history page that pulls from firebase but
  ran out of time

## AI credits

Files w/ `AI ASSISTED` in the header were written w/ AI help (BLE comm, charting,
csv export). Everything else is mine. Per the part 1 docs.

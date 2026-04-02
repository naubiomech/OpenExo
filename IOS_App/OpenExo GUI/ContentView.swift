import SwiftUI

struct ContentView: View {
    @StateObject private var ble = BLEManager.shared
    @StateObject private var logger = CSVLogger()
    @State private var navPath = NavigationPath()

    var body: some View {
        NavigationStack(path: $navPath) {
            ScanView(navPath: $navPath)
                .navigationDestination(for: AppScreen.self) { screen in
                    switch screen {
                    case .activeTrial:
                        ActiveTrialView(navPath: $navPath)
                    case .settings:
                        SettingsView(navPath: $navPath)
                    case .bioFeedback:
                        BioFeedbackView(navPath: $navPath)
                    case .endTrial:
                        EndTrialView(navPath: $navPath)
                    }
                }
        }
        .environmentObject(ble)
        .environmentObject(logger)
        .preferredColorScheme(.dark)
        .onAppear {
            ble.onUnexpectedDisconnect = {
                navPath = NavigationPath()
            }
        }
    }
}

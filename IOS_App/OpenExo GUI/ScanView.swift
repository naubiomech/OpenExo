import SwiftUI
import CoreBluetooth

struct ScanView: View {
    @EnvironmentObject private var ble: BLEManager
    @EnvironmentObject private var logger: CSVLogger
    @Binding var navPath: NavigationPath

    @State private var selectedDevice: DiscoveredDevice?
    @State private var showBLEAlert = false

    private var canConnect: Bool    { selectedDevice != nil && !ble.isConnected }
    private var canCalibrate: Bool  { ble.isConnected && !ble.torqueCalibrated }
    private var canStartTrial: Bool { ble.isConnected && ble.torqueCalibrated }

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            VStack(spacing: 0) {
                header
                statusCard
                deviceList
                actionButtons
            }
        }
        .navigationTitle("")
        .navigationBarHidden(true)
        .alert("Bluetooth Required", isPresented: $showBLEAlert) {
            Button("OK") {}
        } message: {
            Text("Please enable Bluetooth in Settings to connect to the exoskeleton.")
        }
        .onChange(of: ble.bleState) { state in
            if state == .poweredOff || state == .unauthorized { showBLEAlert = true }
        }
    }

    // MARK: - Header
    private var header: some View {
        VStack(spacing: 4) {
            HStack {
                Image(systemName: "figure.walk.motion")
                    .font(.system(size: 28, weight: .semibold))
                    .foregroundStyle(.blue)
                Text("OpenExo")
                    .font(.system(size: 28, weight: .bold, design: .rounded))
                    .foregroundStyle(.white)
            }
            Text("Exoskeleton Controller")
                .font(.subheadline)
                .foregroundStyle(.gray)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 20)
        .background(Color(.systemGray6).opacity(0.15))
    }

    // MARK: - Status Card
    private var statusCard: some View {
        HStack(spacing: 12) {
            Circle()
                .fill(statusColor)
                .frame(width: 10, height: 10)
                .shadow(color: statusColor.opacity(0.6), radius: 4)
            Text(ble.connectionStatus)
                .font(.subheadline)
                .foregroundStyle(.white)
                .lineLimit(2)
            Spacer()
            if ble.isScanning {
                ProgressView()
                    .progressViewStyle(CircularProgressViewStyle(tint: .blue))
                    .scaleEffect(0.8)
            }
        }
        .padding(14)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(Color(.systemGray6).opacity(0.18))
                .overlay(
                    RoundedRectangle(cornerRadius: 12)
                        .stroke(statusColor.opacity(0.3), lineWidth: 1)
                )
        )
        .padding(.horizontal, 16)
        .padding(.top, 12)
    }

    private var statusColor: Color {
        if ble.isConnected { return .green }
        if ble.isScanning { return .blue }
        if ble.connectionStatus.contains("failed") || ble.connectionStatus.contains("No devices") { return .red }
        return .gray
    }

    // MARK: - Device List
    private var deviceList: some View {
        VStack(alignment: .leading, spacing: 8) {
            HStack {
                Text("NEARBY DEVICES")
                    .font(.caption)
                    .fontWeight(.semibold)
                    .foregroundStyle(.gray)
                if MOCK_MODE {
                    Text("MOCK")
                        .font(.caption2)
                        .fontWeight(.bold)
                        .foregroundStyle(.orange)
                        .padding(.horizontal, 6)
                        .padding(.vertical, 2)
                        .background(Capsule().fill(Color.orange.opacity(0.2)))
                }
            }
            .padding(.horizontal, 16)
            .padding(.top, 16)

            if ble.discoveredDevices.isEmpty {
                emptyDeviceState
            } else {
                ScrollView {
                    LazyVStack(spacing: 8) {
                        ForEach(ble.discoveredDevices) { device in
                            DeviceRow(
                                device: device,
                                isSelected: selectedDevice?.id == device.id,
                                isConnected: ble.isConnected && ble.connectedName == device.name
                            )
                            .onTapGesture {
                                if !ble.isConnected { selectedDevice = device }
                            }
                        }
                    }
                    .padding(.horizontal, 16)
                }
            }
        }
        .frame(maxHeight: .infinity)
    }

    private var emptyDeviceState: some View {
        VStack(spacing: 12) {
            Image(systemName: "antenna.radiowaves.left.and.right")
                .font(.system(size: 40))
                .foregroundStyle(.gray.opacity(0.5))
            Text("Tap 'Scan' to discover nearby devices")
                .font(.subheadline)
                .foregroundStyle(.gray)
                .multilineTextAlignment(.center)
        }
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .padding(.top, 40)
    }

    // MARK: - Action Buttons
    private var actionButtons: some View {
        VStack(spacing: 10) {
            Divider().background(Color.gray.opacity(0.3))

            // Row 1: Scan + Load Saved
            HStack(spacing: 10) {
                ActionButton(
                    title: ble.isScanning ? "Scanning…" : "Scan",
                    icon: "antenna.radiowaves.left.and.right",
                    style: .secondary,
                    isLoading: ble.isScanning,
                    isDisabled: ble.isConnected
                ) { ble.startScan() }

                ActionButton(
                    title: "Load Saved",
                    icon: "bookmark.fill",
                    style: .secondary,
                    isDisabled: !ble.hasSavedDevice || ble.isConnected
                ) { ble.connectSaved() }
            }

            // Row 2: Connect
            ActionButton(
                title: "Connect",
                icon: "link",
                style: .primary,
                isDisabled: !canConnect
            ) {
                if let d = selectedDevice { ble.connect(d) }
            }
            .frame(maxWidth: .infinity)

            // Row 3: Calibrate Torque + Start Trial
            HStack(spacing: 10) {
                ActionButton(
                    title: ble.torqueCalibrated ? "Calibrated ✓" : "Calibrate Torque",
                    icon: ble.torqueCalibrated ? "checkmark.circle.fill" : "wrench.and.screwdriver",
                    style: ble.torqueCalibrated ? .success : .secondary,
                    isDisabled: !canCalibrate
                ) {
                    ble.calibrateTorque()
                }

                ActionButton(
                    title: "Start Trial",
                    icon: "play.fill",
                    style: canStartTrial ? .success : .secondary,
                    isDisabled: !canStartTrial
                ) {
                    logger.startLogging(prefix: GUISettings.load().csvPrefix, columnNames: ble.parameterNames)
                    ble.logCallback = { [weak logger] values, mark in
                        logger?.log(values: values, mark: mark)
                    }
                    ble.beginTrial()
                    navPath.append(AppScreen.activeTrial)
                }
            }
        }
        .padding(.horizontal, 16)
        .padding(.bottom, 24)
        .padding(.top, 8)
    }
}

// MARK: - Device Row
struct DeviceRow: View {
    let device: DiscoveredDevice
    let isSelected: Bool
    let isConnected: Bool

    var body: some View {
        HStack(spacing: 14) {
            ZStack {
                Circle()
                    .fill(isConnected ? Color.green.opacity(0.2) : Color(.systemGray5).opacity(0.3))
                    .frame(width: 40, height: 40)
                Image(systemName: isConnected ? "checkmark.circle.fill" : "wave.3.right")
                    .font(.system(size: 16, weight: .semibold))
                    .foregroundStyle(isConnected ? .green : .blue)
            }

            VStack(alignment: .leading, spacing: 2) {
                Text(device.name)
                    .font(.system(.body, design: .default, weight: .medium))
                    .foregroundStyle(.white)
                Text(device.id.uuidString.prefix(18) + "…")
                    .font(.caption2)
                    .foregroundStyle(.gray)
            }
            Spacer()

            if isConnected {
                Text("Connected")
                    .font(.caption)
                    .fontWeight(.semibold)
                    .foregroundStyle(.green)
                    .padding(.horizontal, 8)
                    .padding(.vertical, 4)
                    .background(Capsule().fill(Color.green.opacity(0.15)))
            } else if isSelected {
                Image(systemName: "checkmark.circle.fill")
                    .foregroundStyle(.blue)
            }
        }
        .padding(12)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(isSelected ? Color.blue.opacity(0.12) : Color(.systemGray6).opacity(0.15))
                .overlay(
                    RoundedRectangle(cornerRadius: 12)
                        .stroke(isSelected ? Color.blue.opacity(0.5) : Color.clear, lineWidth: 1.5)
                )
        )
        .contentShape(Rectangle())
    }
}

// MARK: - Reusable Action Button
enum ButtonStyle { case primary, secondary, success, danger }

struct ActionButton: View {
    let title: String
    let icon: String
    var style: ButtonStyle = .primary
    var isLoading: Bool = false
    var isDisabled: Bool = false
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack(spacing: 8) {
                if isLoading {
                    ProgressView().progressViewStyle(CircularProgressViewStyle(tint: .white)).scaleEffect(0.75)
                } else {
                    Image(systemName: icon).font(.system(size: 14, weight: .semibold))
                }
                Text(title).font(.system(size: 15, weight: .semibold))
            }
            .frame(maxWidth: .infinity)
            .padding(.vertical, 13)
            .background(
                RoundedRectangle(cornerRadius: 12)
                    .fill(isDisabled ? Color(.systemGray5).opacity(0.3) : bgColor)
            )
            .foregroundStyle(isDisabled ? .gray : fgColor)
        }
        .disabled(isDisabled)
        .animation(.easeInOut(duration: 0.2), value: isDisabled)
    }

    private var bgColor: Color {
        switch style {
        case .primary:   return .blue
        case .secondary: return Color(.systemGray5).opacity(0.4)
        case .success:   return .green
        case .danger:    return .red
        }
    }

    private var fgColor: Color {
        switch style {
        case .secondary: return .white
        default:         return .white
        }
    }
}

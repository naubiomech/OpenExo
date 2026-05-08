import Foundation
import CoreBluetooth

// ─────────────────────────────────────────────
// MARK: - Mock Mode Toggle
// Set to `true` to run with fake data in the simulator.
// Set to `false` when running on a real device with the exoskeleton.
// ─────────────────────────────────────────────
let MOCK_MODE = false

// MARK: - Discovered Device (real or mock)
struct DiscoveredDevice: Identifiable {
    let id: UUID
    let name: String
    var peripheral: CBPeripheral? // nil in mock mode
}

// MARK: - BLE UUIDs
private enum BLEUUID {
    static let service  = CBUUID(string: "6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
    static let txChar   = CBUUID(string: "6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
    static let rxChar   = CBUUID(string: "6E400003-B5A3-F393-E0A9-E50E24DCCA9E")
    static let errChar  = CBUUID(string: "33B65D43-611C-11ED-9B6A-0242AC120002")
}

// MARK: - BLE Manager
class BLEManager: NSObject, ObservableObject {

    static let shared = BLEManager()

    // MARK: Connection State
    @Published var bleState: CBManagerState = .unknown
    @Published var isScanning = false
    @Published var discoveredDevices: [DiscoveredDevice] = []
    @Published var isConnected = false
    @Published var connectedName = ""
    @Published var connectionStatus = "Not Connected"
    @Published var hasSavedDevice = false

    // MARK: Trial State
    @Published var isTrialActive = false
    @Published var isPaused = false
    @Published var markCount = 0
    @Published var batteryVoltage: Double?
    @Published var torqueCalibrated = false

    // MARK: Handshake
    @Published var handshakeReceived = false
    @Published var parameterNames: [String] = []
    @Published var joints: [JointInfo] = []

    // MARK: RT Data
    @Published var rtData: [Double] = Array(repeating: 0, count: 16)
    @Published var rtPacketCount: Int = 0

    /// Direct logging callback — bypasses SwiftUI for CSV writes at full data rate.
    var logCallback: (([Double], Int) -> Void)?

    // Internal counters (not @Published — avoids 30Hz re-renders)
    private var _rawValues: [Double] = Array(repeating: 0, count: 16)
    private var _packetCount: Int = 0
    private var _publishSkip: Int = 0

    // MARK: Chart Snapshots (20fps)
    @Published var chartSnapshot: [[Double]] = Array(repeating: Array(repeating: 0, count: 300), count: 8)

    var onUnexpectedDisconnect: (() -> Void)?

    // MARK: Real BLE
    private var central: CBCentralManager!
    private var connectedPeripheral: CBPeripheral?
    private var txChar: CBCharacteristic?
    private var rxChar: CBCharacteristic?
    private var errChar: CBCharacteristic?
    private var handshakeBuffer = ""
    private var isReceivingHandshake = false

    // MARK: Chart Buffer
    private let chartCapacity = 300
    private var circularBuf: [[Double]] = Array(repeating: Array(repeating: 0, count: 300), count: 8)
    private var channelActive: [Bool] = Array(repeating: false, count: 8)
    private var writeIdx = 0
    private var displayTimer: Timer?

    // MARK: Mock
    private var mockDataTimer: Timer?
    private var mockTime: Double = 0

    private override init() {
        super.init()
        if !MOCK_MODE {
            central = CBCentralManager(delegate: self, queue: DispatchQueue.main)
        } else {
            // In mock mode BLE state doesn't matter
            bleState = .poweredOn
        }
        hasSavedDevice = UserDefaults.standard.string(forKey: "savedDeviceUUID") != nil
        restoreCachedControllerMetadata()
    }

    /// Restore last handshake controller matrix from SQLite when the saved peripheral matches (offline UI / faster reconnect).
    private func restoreCachedControllerMetadata() {
        guard let uuid = UserDefaults.standard.string(forKey: "savedDeviceUUID"),
              let snap = OpenExoDatabase.shared.loadControllerSnapshot(),
              snap.deviceUUID == uuid,
              !snap.matrix.isEmpty else { return }
        if !snap.parameterNames.isEmpty {
            parameterNames = snap.parameterNames
        }
        joints = ControllerSnapshot.joints(from: snap.matrix)
    }

    private func persistHandshakeSnapshot(joints js: [JointInfo], parameterNames names: [String], values: [String: [String]]) {
        let uuid = connectedPeripheral?.identifier.uuidString
            ?? UserDefaults.standard.string(forKey: "savedDeviceUUID")
            ?? ""
        guard !uuid.isEmpty else { return }
        let matrix = ControllerSnapshot.buildMatrix(from: js)
        let snap = ControllerSnapshot(
            deviceUUID: uuid,
            matrix: matrix,
            values: values,
            parameterNames: names,
            updatedAt: Date().timeIntervalSince1970
        )
        OpenExoDatabase.shared.saveControllerSnapshot(snap)
    }

    // ─────────────────────────────────────────────
    // MARK: - Scanning
    // ─────────────────────────────────────────────
    func startScan() {
        if MOCK_MODE { mockScan(); return }
        guard bleState == .poweredOn else {
            connectionStatus = "Bluetooth is off — enable it in Settings"
            return
        }
        discoveredDevices.removeAll()
        isScanning = true
        connectionStatus = "Scanning…"
        central.scanForPeripherals(withServices: [BLEUUID.service], options: nil)
        // Match Python GUI `QtExoDeviceManager` discover timeout (short scan).
        DispatchQueue.main.asyncAfter(deadline: .now() + 3) { [weak self] in
            guard let self, self.isScanning else { return }
            self.stopScan()
        }
    }

    func stopScan() {
        if !MOCK_MODE { central.stopScan() }
        isScanning = false
        connectionStatus = discoveredDevices.isEmpty
            ? "No devices found — try scanning again"
            : "Found \(discoveredDevices.count) device(s)"
    }

    func connect(_ device: DiscoveredDevice) {
        if MOCK_MODE { mockConnect(device); return }
        guard let peripheral = device.peripheral else { return }
        let newId = peripheral.identifier.uuidString
        if let snap = OpenExoDatabase.shared.loadControllerSnapshot(), snap.deviceUUID != newId {
            OpenExoDatabase.shared.clearControllerSnapshot()
            joints = []
            parameterNames = []
            handshakeReceived = false
        }
        connectionStatus = "Connecting to \(device.name)…"
        central.stopScan()
        isScanning = false
        central.connect(peripheral, options: nil)
        UserDefaults.standard.set(newId, forKey: "savedDeviceUUID")
        hasSavedDevice = true
    }

    func connectSaved() {
        if MOCK_MODE {
            mockConnect(DiscoveredDevice(id: UUID(), name: "OpenExo (Saved)"))
            return
        }
        guard let uuidStr = UserDefaults.standard.string(forKey: "savedDeviceUUID"),
              let uuid = UUID(uuidString: uuidStr) else {
            connectionStatus = "No saved device found"
            return
        }
        let known = central.retrievePeripherals(withIdentifiers: [uuid])
        if let p = known.first {
            connectionStatus = "Reconnecting to saved device…"
            central.connect(p, options: nil)
        } else {
            connectionStatus = "Saved device unavailable — scan first"
        }
    }

    func disconnect() {
        if MOCK_MODE { mockDisconnect(); return }
        if let p = connectedPeripheral { central.cancelPeripheralConnection(p) }
    }

    // ─────────────────────────────────────────────
    // MARK: - Commands
    // ─────────────────────────────────────────────
    func send(byte: Character) {
        if MOCK_MODE { print("[MockBLE] → \(byte)"); return }
        sendRaw(Data([byte.asciiValue ?? 0]))
    }

    func sendRaw(_ data: Data) {
        if MOCK_MODE { return }
        guard let char = txChar, let p = connectedPeripheral else { return }
        p.writeValue(data, for: char, type: .withoutResponse)
    }

    /// Write with acknowledgment (matches Python's `response=True` for critical commands).
    private func sendReliable(_ data: Data) {
        if MOCK_MODE { return }
        guard let char = txChar, let p = connectedPeripheral else { return }
        p.writeValue(data, for: char, type: .withResponse)
    }

    func calibrateTorque() {
        send(byte: "H")
        torqueCalibrated = false
        connectionStatus = "Calibrating… Start Trial unlocks in 1.5 s"
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) { [weak self] in
            self?.torqueCalibrated = true
            self?.connectionStatus = "Calibrated ✓ — tap Start Trial"
        }
    }

    func calibrateFSR() { send(byte: "L") }

    func motorsOff()  { sendReliable(Data([UInt8(ascii: "w")])); isPaused = true  }
    func motorsOn()   { sendReliable(Data([UInt8(ascii: "x")])); isPaused = false }

    func markTrial()  { send(byte: "N"); markCount += 1 }

    func beginTrial() {
        markCount = 0
        resetChartBuffers()
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) { [weak self] in
            guard let self else { return }
            self.send(byte: "E")
            self.send(byte: "L")
            self.sendFSRThresholds(left: 0.25, right: 0.25)
            self.applySavedControllerSettings()
            self.isTrialActive = true
            self.isPaused = false
            if MOCK_MODE { self.startMockDataStream() }
        }
    }

    /// Loads the last-applied controller settings from UserDefaults and sends
    /// them to the device, so settings persist across trials (matches Python GUI behavior).
    private func applySavedControllerSettings() {
        let s = GUISettings.load()
        guard s.hasAppliedSettings else { return }

        if handshakeReceived && !joints.isEmpty {
            // Resolve by name first, then by index
            let jIdx: Int
            if !s.lastJointName.isEmpty,
               let idx = joints.firstIndex(where: { $0.name == s.lastJointName }) {
                jIdx = idx
            } else {
                jIdx = min(s.lastJointIndex, joints.count - 1)
            }
            let joint = joints[jIdx]
            guard !joint.controllers.isEmpty else { return }

            let cIdx: Int
            if !s.lastControllerName.isEmpty,
               let idx = joint.controllers.firstIndex(where: { $0.name == s.lastControllerName }) {
                cIdx = idx
            } else {
                cIdx = min(s.lastControllerIndex, joint.controllers.count - 1)
            }
            let ctrl = joint.controllers[cIdx]
            let pIdx = ctrl.params.isEmpty ? 0 : min(s.lastParamIndex, ctrl.params.count - 1)

            updateParam(
                isBilateral: s.bilateral,
                jointID: joint.jointID,
                controllerID: ctrl.controllerID,
                paramIndex: pIdx,
                value: s.lastValue
            )
        } else {
            updateParam(
                isBilateral: s.bilateral,
                jointID: s.lastBasicJointID,
                controllerID: s.lastBasicControllerID,
                paramIndex: s.lastBasicParamIndex,
                value: s.lastBasicValue
            )
        }
    }

    func endTrial() {
        send(byte: "G")
        isTrialActive = false
        stopMockDataStream()
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.1) { [weak self] in
            self?.sendReliable(Data([UInt8(ascii: "w")]))
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.2) { [weak self] in
            self?.send(byte: "Z")
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { [weak self] in
            self?.disconnect()
        }
    }

    func sendFSRThresholds(left: Double, right: Double) {
        if MOCK_MODE { return }
        send(byte: "R")
        var l = left, r = right
        withUnsafeBytes(of: &l) { sendRaw(Data($0)) }
        withUnsafeBytes(of: &r) { sendRaw(Data($0)) }
    }

    func updateParam(isBilateral: Bool, jointID: Int, controllerID: Int, paramIndex: Int, value: Double) {
        let jointIDs = isBilateral ? [jointID, jointID ^ 0x60] : [jointID]
        OpenExoDatabase.shared.updateControllerSnapshotValues(
            jointIDs: jointIDs,
            controllerID: controllerID,
            paramIndex: paramIndex,
            value: value
        )

        if MOCK_MODE {
            print("[MockBLE] updateParam joint=\(jointID) ctrl=\(controllerID) param=\(paramIndex) val=\(value)")
            return
        }
        for jid in jointIDs {
            send(byte: "f")
            for var v in [Double(jid), Double(controllerID), Double(paramIndex), value] {
                withUnsafeBytes(of: &v) { sendRaw(Data($0)) }
            }
        }
    }

    // ─────────────────────────────────────────────
    // MARK: - Chart Timer
    // ─────────────────────────────────────────────
    func startChartTimer() {
        displayTimer?.invalidate()
        let timer = Timer(timeInterval: 0.05, repeats: true) { [weak self] _ in
            self?.flushChartSnapshot()
        }
        RunLoop.main.add(timer, forMode: .common)
        displayTimer = timer
    }

    func stopChartTimer() {
        displayTimer?.invalidate()
        displayTimer = nil
    }

    private func resetChartBuffers() {
        circularBuf = Array(repeating: Array(repeating: 0, count: chartCapacity), count: 8)
        channelActive = Array(repeating: false, count: 8)
        writeIdx = 0
        _packetCount = 0
        _publishSkip = 0
        _rawValues = Array(repeating: 0, count: 16)
        chartSnapshot = Array(repeating: [], count: 8)
    }

    private func flushChartSnapshot() {
        guard writeIdx > 0 else { return }
        let count = min(writeIdx, chartCapacity)
        let start = writeIdx % chartCapacity
        var snapshot: [[Double]] = []
        for (chIdx, ch) in circularBuf.enumerated() {
            guard channelActive[chIdx] else {
                snapshot.append([])
                continue
            }
            let ordered: [Double]
            if writeIdx <= chartCapacity {
                ordered = Array(ch[0..<count])
            } else {
                ordered = Array(ch[start...]) + Array(ch[..<start])
            }
            snapshot.append(ordered)
        }
        chartSnapshot = snapshot
        rtPacketCount = _packetCount
    }

    // ─────────────────────────────────────────────
    // MARK: - RT Data Ingestion
    // ─────────────────────────────────────────────
    private func ingestSample(_ values: [Double]) {
        // Internal state — no SwiftUI cost
        for (i, v) in values.prefix(16).enumerated() { _rawValues[i] = v }
        _packetCount += 1

        // Chart circular buffer — no SwiftUI cost
        let idx = writeIdx % chartCapacity
        for (i, v) in values.prefix(8).enumerated() {
            circularBuf[i][idx] = v
            if !channelActive[i] { channelActive[i] = true }
        }
        writeIdx += 1

        // CSV logging at full rate, bypassing SwiftUI entirely
        logCallback?(values, markCount)

        // Throttle @Published updates to ~10Hz (every 3rd sample at 30Hz)
        _publishSkip += 1
        if _publishSkip >= 3 {
            _publishSkip = 0
            rtData = _rawValues
            if values.count > 10 { batteryVoltage = values[10] }
        }
    }

    // ─────────────────────────────────────────────
    // MARK: - Real BLE: RT Data Parsing
    // ─────────────────────────────────────────────

    /// Parse one or more RT data frames from a BLE packet.
    /// Frame format: `<count>c S<cmd><v1>n<v2>n…<vN>n`
    /// Multiple frames may be concatenated when the device sends
    /// faster than the BLE connection interval.
    private func parseRTData(_ packet: String) {
        let segments = packet.components(separatedBy: "c")

        // Primary path: structured frames with <count>c header
        if segments.count >= 2 {
            var didParse = false
            for i in 0..<(segments.count - 1) {
                let trailing = String(segments[i].reversed().prefix(while: { $0.isNumber }).reversed())
                guard let expectedCount = Int(trailing), expectedCount > 1 else { continue }

                let data = segments[i + 1]
                guard let sIdx = data.firstIndex(of: "S") else { continue }
                var pos = data.index(after: sIdx)
                guard pos < data.endIndex else { continue }
                pos = data.index(after: pos)                    // skip command byte

                var values: [Double] = []
                var numBuf = ""
                while pos < data.endIndex && values.count < expectedCount {
                    let ch = data[pos]
                    if ch == "n" {
                        if let intVal = Int(numBuf) { values.append(Double(intVal) / 100.0) }
                        numBuf = ""
                    } else if ch.isNumber || ch == "-" {
                        numBuf.append(ch)
                    }
                    pos = data.index(after: pos)
                }
                if values.count < expectedCount, let intVal = Int(numBuf) {
                    values.append(Double(intVal) / 100.0)
                }

                guard values.count > 1 else { continue }
                didParse = true
                ingestSample(values)
            }
            if didParse { return }
        }

        // Fallback: extract values after "S", skipping command byte
        if let sRange = packet.range(of: "S") {
            let afterS = packet[sRange.upperBound...]
            guard afterS.count > 1 else { return }
            let afterCmd = String(afterS[afterS.index(after: afterS.startIndex)...])
            let values = afterCmd.components(separatedBy: "n").compactMap { part -> Double? in
                let digits = part.filter { $0.isNumber || $0 == "-" }
                guard !digits.isEmpty, digits != "-",
                      let intVal = Int(digits) else { return nil }
                return Double(intVal) / 100.0
            }
            guard values.count > 1 else { return }
            ingestSample(Array(values.prefix(16)))
            return
        }

        // Last resort: any n-separated numbers in the packet
        let values = packet.components(separatedBy: "n").compactMap { part -> Double? in
            let digits = part.filter { $0.isNumber || $0 == "-" }
            guard !digits.isEmpty, digits != "-",
                  let intVal = Int(digits) else { return nil }
            return Double(intVal) / 100.0
        }
        guard values.count > 1 else { return }
        ingestSample(Array(values.prefix(16)))
    }

    private func parseHandshake(_ text: String) {
        var names: [String] = []
        var jointsMap: [Int: JointInfo] = [:]
        var valueMap: [String: [String]] = [:]

        // Handshake may contain newline-separated sections:
        //   "t,param1,param2,..."          — parameter names
        //   "f,J,ID,C,CID,p1,...|J,ID,..." — pipe-delimited controller matrix
        // Or the entire thing may be a single line with pipes.
        let lines = text.components(separatedBy: .newlines)
        print("[ExoBLE] Handshake has \(lines.count) lines")

        for line in lines {
            let trimmed = line.trimmingCharacters(in: .whitespacesAndNewlines)
            guard !trimmed.isEmpty, trimmed != "?", trimmed != "END" else { continue }

            if trimmed.hasPrefix("t,") {
                names = String(trimmed.dropFirst(2))
                    .components(separatedBy: ",")
                    .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                print("[ExoBLE] Parsed \(names.count) parameter names: \(names)")
                continue
            }

            // Controller matrix: strip optional "f," prefix, then split on "|"
            var matrixStr = trimmed
            if matrixStr.hasPrefix("f,") {
                matrixStr = String(matrixStr.dropFirst(2))
            }

            let entries = matrixStr.components(separatedBy: "|")
            for entry in entries {
                let raw = entry.trimmingCharacters(in: .whitespacesAndNewlines)
                guard !raw.isEmpty, raw != "?", raw != "END" else { continue }

                // Parameter names can appear as a pipe entry (e.g. "t,Measured Torque,...")
                if raw.hasPrefix("t,") {
                    names = String(raw.dropFirst(2))
                        .components(separatedBy: ",")
                        .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                    print("[ExoBLE] Parsed \(names.count) parameter names: \(names)")
                    continue
                }

                if raw.lowercased().hasPrefix("v,") {
                    let inner = String(raw.dropFirst(2))
                    let segs = inner.split(separator: ",").map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }
                    guard segs.count >= 3,
                          let jid = Int(segs[0]),
                          let cid = Int(segs[1]) else { continue }
                    valueMap["\(jid)_\(cid)"] = Array(segs.dropFirst(2))
                    continue
                }

                let parts = raw.components(separatedBy: ",")
                    .map { $0.trimmingCharacters(in: .whitespacesAndNewlines) }

                guard parts.count >= 4,
                      let jointID = Int(parts[1]),
                      let controllerID = Int(parts[3]) else {
                    print("[ExoBLE] Skipped entry (\(parts.count) parts): \(raw.prefix(60))")
                    continue
                }

                let ctrl = ControllerInfo(
                    name: parts[2],
                    controllerID: controllerID,
                    params: Array(parts.dropFirst(4))
                )
                if var existing = jointsMap[jointID] {
                    existing.controllers.append(ctrl)
                    jointsMap[jointID] = existing
                } else {
                    jointsMap[jointID] = JointInfo(name: parts[0], jointID: jointID, controllers: [ctrl])
                }
                print("[ExoBLE] Parsed joint \(parts[0]) (ID \(jointID)), ctrl \(parts[2]) (ID \(controllerID)), \(ctrl.params.count) params")
            }
        }

        if !names.isEmpty { parameterNames = names }
        if !jointsMap.isEmpty {
            joints = jointsMap.values.sorted { $0.jointID < $1.jointID }
            persistHandshakeSnapshot(joints: joints, parameterNames: parameterNames, values: valueMap)
            print("[ExoBLE] Handshake result: \(joints.count) joints, \(joints.reduce(0) { $0 + $1.controllers.count }) controllers total")
        } else {
            print("[ExoBLE] WARNING: No joints/controllers parsed from handshake")
        }
        handshakeReceived = true
        send(byte: "$")
    }

    // ─────────────────────────────────────────────
    // MARK: - Mock Implementations
    // ─────────────────────────────────────────────
    private func mockScan() {
        discoveredDevices.removeAll()
        isScanning = true
        connectionStatus = "Scanning…"

        DispatchQueue.main.asyncAfter(deadline: .now() + 3) { [weak self] in
            guard let self else { return }
            self.discoveredDevices = [
                DiscoveredDevice(id: UUID(), name: "OpenExo Left Ankle"),
                DiscoveredDevice(id: UUID(), name: "OpenExo Bilateral"),
                DiscoveredDevice(id: UUID(), name: "OpenExo Dev Unit"),
            ]
            self.isScanning = false
            self.connectionStatus = "Found \(self.discoveredDevices.count) device(s)"
        }
    }

    private func mockConnect(_ device: DiscoveredDevice) {
        isScanning = false
        connectionStatus = "Connecting to \(device.name)…"

        DispatchQueue.main.asyncAfter(deadline: .now() + 0.8) { [weak self] in
            guard let self else { return }
            self.isConnected = true
            self.connectedName = device.name
            self.connectionStatus = "Connected to \(device.name)"
            self.hasSavedDevice = true
            UserDefaults.standard.set(device.id.uuidString, forKey: "savedDeviceUUID")

            // Simulate handshake after 1s
            DispatchQueue.main.asyncAfter(deadline: .now() + 1.0) {
                self.mockHandshake()
            }
        }
    }

    private func mockDisconnect() {
        let wasActive = isTrialActive
        isConnected = false
        connectedName = ""
        isTrialActive = false
        torqueCalibrated = false
        connectionStatus = "Disconnected"
        stopMockDataStream()
        if wasActive { onUnexpectedDisconnect?() }
    }

    private func mockHandshake() {
        parameterNames = ["torque_cmd", "torque_meas", "ankle_angle", "fsr_l",
                          "fsr_r", "hip_torque", "knee_angle", "fsr_l2"]

        let pjmcParams  = ["p_gain", "i_gain", "d_gain", "use_pid", "torque_limit"]
        let zhangParams = ["peak_torque", "rise_time", "peak_time", "fall_time"]

        joints = [
            JointInfo(name: "Left Ankle",  jointID: 68, controllers: [
                ControllerInfo(name: "pjmc_plus",    controllerID: 11, params: pjmcParams),
                ControllerInfo(name: "zeroTorque",   controllerID: 1,  params: []),
            ]),
            JointInfo(name: "Right Ankle", jointID: 36, controllers: [
                ControllerInfo(name: "pjmc_plus",    controllerID: 11, params: pjmcParams),
                ControllerInfo(name: "zeroTorque",   controllerID: 1,  params: []),
            ]),
            JointInfo(name: "Left Hip",    jointID: 65, controllers: [
                ControllerInfo(name: "zhang_collins", controllerID: 6, params: zhangParams),
                ControllerInfo(name: "zeroTorque",    controllerID: 1, params: []),
            ]),
        ]
        handshakeReceived = true
        batteryVoltage = 11.7
        connectionStatus = "Handshake complete — ready to start trial"
        persistHandshakeSnapshot(joints: joints, parameterNames: parameterNames, values: [:])
    }

    // MARK: Mock Data Stream (sine waves at 30 Hz)
    private func startMockDataStream() {
        mockTime = 0
        mockDataTimer?.invalidate()
        mockDataTimer = Timer.scheduledTimer(withTimeInterval: 1.0 / 30.0, repeats: true) { [weak self] _ in
            self?.mockTick()
        }
    }

    private func stopMockDataStream() {
        mockDataTimer?.invalidate()
        mockDataTimer = nil
    }

    private func mockTick() {
        let t = mockTime
        mockTime += 1.0 / 30.0

        func sin(_ freq: Double, _ amp: Double, _ phase: Double = 0) -> Double {
            amp * Foundation.sin(2 * .pi * freq * t + phase)
        }
        func noise(_ amp: Double) -> Double { amp * (Double.random(in: -1...1)) }
        func fsr(_ freq: Double, _ phase: Double) -> Double {
            let v = Foundation.sin(2 * .pi * freq * t + phase)
            return v > 0.3 ? 0.6 + noise(0.05) : 0.05 + noise(0.02)
        }

        var values = Array(repeating: 0.0, count: 16)

        // Block A [0-3]: ankle torque + angle + FSR
        values[0] = sin(0.5, 30)                       // torque cmd
        values[1] = sin(0.5, 28) + noise(2)            // torque meas
        values[2] = sin(0.4, 18, 0.3)                  // ankle angle (degrees)
        values[3] = fsr(1.0, 0)                        // left FSR

        // Block B [4-7]: hip + knee + FSRs
        values[4] = sin(0.6, 20, 0.5)                  // hip torque
        values[5] = fsr(1.0, .pi)                      // right FSR
        values[6] = sin(0.7, 15, 1.2) + noise(1)       // knee angle
        values[7] = fsr(1.0, 0.2)                      // left FSR alt

        // Battery
        values[10] = 11.7 - Foundation.sin(t * 0.01) * 0.1

        ingestSample(values)
    }
}

// ─────────────────────────────────────────────
// MARK: - CBCentralManagerDelegate (real BLE only)
// ─────────────────────────────────────────────
extension BLEManager: CBCentralManagerDelegate {
    func centralManagerDidUpdateState(_ central: CBCentralManager) {
        bleState = central.state
        if central.state == .poweredOff {
            connectionStatus = "Bluetooth is off"
        }
    }

    func centralManager(_ central: CBCentralManager, didDiscover peripheral: CBPeripheral,
                        advertisementData: [String: Any], rssi RSSI: NSNumber) {
        guard !discoveredDevices.contains(where: { $0.id == peripheral.identifier }) else { return }
        discoveredDevices.append(DiscoveredDevice(id: peripheral.identifier,
                                                  name: peripheral.name ?? "Unknown",
                                                  peripheral: peripheral))
    }

    func centralManager(_ central: CBCentralManager, didConnect peripheral: CBPeripheral) {
        connectedPeripheral = peripheral
        isConnected = true
        connectedName = peripheral.name ?? peripheral.identifier.uuidString
        connectionStatus = "Connected to \(connectedName)"
        peripheral.delegate = self
        peripheral.discoverServices([BLEUUID.service])
    }

    func centralManager(_ central: CBCentralManager, didFailToConnect peripheral: CBPeripheral, error: Error?) {
        isConnected = false
        connectionStatus = "Connection failed: \(error?.localizedDescription ?? "unknown")"
    }

    func centralManager(_ central: CBCentralManager, didDisconnectPeripheral peripheral: CBPeripheral, error: Error?) {
        let wasActive = isTrialActive
        connectedPeripheral = nil
        isConnected = false
        txChar = nil; rxChar = nil; errChar = nil
        isTrialActive = false
        torqueCalibrated = false
        connectionStatus = "Disconnected"
        if wasActive { onUnexpectedDisconnect?() }
    }
}

// MARK: - CBPeripheralDelegate (real BLE only)
extension BLEManager: CBPeripheralDelegate {
    func peripheral(_ peripheral: CBPeripheral, didDiscoverServices error: Error?) {
        peripheral.services?.forEach {
            peripheral.discoverCharacteristics([BLEUUID.txChar, BLEUUID.rxChar, BLEUUID.errChar], for: $0)
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didDiscoverCharacteristicsFor service: CBService, error: Error?) {
        service.characteristics?.forEach { char in
            switch char.uuid {
            case BLEUUID.txChar:  txChar = char
            case BLEUUID.rxChar:  rxChar = char;  peripheral.setNotifyValue(true, for: char)
            case BLEUUID.errChar: errChar = char; peripheral.setNotifyValue(true, for: char)
            default: break
            }
        }
    }

    func peripheral(_ peripheral: CBPeripheral, didUpdateValueFor characteristic: CBCharacteristic, error: Error?) {
        guard let data = characteristic.value,
              let str = String(data: data, encoding: .utf8) else { return }
        if characteristic.uuid == BLEUUID.errChar { print("[ExoBLE] error: \(str)"); return }

        if str.contains("READY") {
            isReceivingHandshake = true
            handshakeBuffer = ""
            connectionStatus = "Handshake received…"
            // Keep any data after READY in the same packet
            if let range = str.range(of: "READY") {
                let remainder = String(str[range.upperBound...])
                    .trimmingCharacters(in: .whitespacesAndNewlines)
                if !remainder.isEmpty {
                    handshakeBuffer = remainder
                }
            }
            print("[ExoBLE] Handshake started, buffer so far: \(handshakeBuffer.prefix(200))")
            return
        }

        if isReceivingHandshake {
            handshakeBuffer += str
            if handshakeBuffer.contains("?") || handshakeBuffer.contains("END") {
                isReceivingHandshake = false
                print("[ExoBLE] Handshake complete, raw text (\(handshakeBuffer.count) chars):\n\(handshakeBuffer)")
                parseHandshake(handshakeBuffer)
                handshakeBuffer = ""
            }
            return
        }

        if str.contains("n") && !str.hasPrefix("t,") { parseRTData(str) }
    }
}

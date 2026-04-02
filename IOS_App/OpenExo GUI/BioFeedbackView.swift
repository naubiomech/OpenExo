import SwiftUI
import Charts
import UIKit

struct BioFeedbackView: View {
    @EnvironmentObject private var ble: BLEManager
    @Binding var navPath: NavigationPath

    @State private var settings = GUISettings.load()
    @State private var targetValue: Double? = nil
    @State private var targetsReached: Int = 0
    @State private var isAboveGoal = false
    @State private var useLeftLeg = true
    @State private var flashGreen = false
    @State private var showTargetSheet = false
    @State private var targetInput = ""

    // Rolling FSR chart data
    @State private var fsrBuffer: [Double] = Array(repeating: 0, count: 200)
    @State private var writeIdx = 0
    @State private var displayTimer: Timer?

    @State private var fsrSnapshot: [Double] = Array(repeating: 0, count: 200)
    private let capacity = 200

    private var currentFSRIndex: Int {
        useLeftLeg ? settings.leftFSRIndex : settings.rightFSRIndex
    }

    private var currentFSRValue: Double {
        let idx = currentFSRIndex
        guard idx < ble.rtData.count else { return 0 }
        return ble.rtData[idx]
    }

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            // Green flash overlay
            Color.green
                .opacity(flashGreen ? 0.35 : 0)
                .ignoresSafeArea()
                .animation(.easeOut(duration: 0.4), value: flashGreen)

            VStack(spacing: 0) {
                navBar
                mainContent
            }
        }
        .navigationTitle("")
        .navigationBarHidden(true)
        .onAppear { startTimer() }
        .onDisappear { stopTimer() }
        .onChange(of: ble.rtData) { _ in
            appendFSRSample(currentFSRValue)
            checkGoal(currentFSRValue)
        }
        .sheet(isPresented: $showTargetSheet) { targetSheet }
    }

    // MARK: - Nav Bar
    private var navBar: some View {
        HStack {
            Button(action: { navPath.removeLast() }) {
                HStack(spacing: 6) {
                    Image(systemName: "chevron.left")
                    Text("Trial")
                }
                .font(.system(size: 15, weight: .medium))
                .foregroundStyle(.blue)
            }
            Spacer()
            Text("Biofeedback")
                .font(.headline)
                .foregroundStyle(.white)
            Spacer()
            Button(action: { ble.isPaused ? ble.motorsOn() : ble.motorsOff() }) {
                Label(ble.isPaused ? "Resume" : "Pause",
                      systemImage: ble.isPaused ? "play.fill" : "pause.fill")
                    .font(.system(size: 13, weight: .semibold))
                    .padding(.horizontal, 12)
                    .padding(.vertical, 7)
                    .background(Capsule().fill(Color.blue))
                    .foregroundStyle(.white)
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 14)
        .background(Color(.systemGray6).opacity(0.15))
    }

    // MARK: - Main Content
    private var mainContent: some View {
        ScrollView {
            VStack(spacing: 16) {
                // Current value + leg selector
                HStack {
                    VStack(alignment: .leading, spacing: 2) {
                        Text(useLeftLeg ? "Left Leg FSR" : "Right Leg FSR")
                            .font(.caption)
                            .foregroundStyle(.gray)
                        Text(String(format: "%.3f", currentFSRValue))
                            .font(.system(size: 40, weight: .bold, design: .monospaced))
                            .foregroundStyle(currentFSRValue > (targetValue ?? Double.infinity) ? .green : .white)
                    }
                    Spacer()
                    VStack(alignment: .trailing, spacing: 4) {
                        Text("Targets Reached")
                            .font(.caption)
                            .foregroundStyle(.gray)
                        Text("\(targetsReached)")
                            .font(.system(size: 36, weight: .bold, design: .monospaced))
                            .foregroundStyle(.green)
                    }
                }
                .padding(16)
                .background(RoundedRectangle(cornerRadius: 12).fill(Color(.systemGray6).opacity(0.18)))

                // Chart
                fsrChart
                    .frame(height: 220)

                // Target info
                if let t = targetValue {
                    HStack {
                        Image(systemName: "target").foregroundStyle(.red)
                        Text("Target: \(String(format: "%.3f", t))")
                            .font(.system(.body, design: .monospaced, weight: .semibold))
                            .foregroundStyle(.red)
                        Spacer()
                        Button("Clear") {
                            targetValue = nil
                            isAboveGoal = false
                        }
                        .font(.system(size: 13, weight: .semibold))
                        .foregroundStyle(.red)
                    }
                    .padding(12)
                    .background(RoundedRectangle(cornerRadius: 10).fill(Color.red.opacity(0.1)))
                }

                // Controls
                controlsSection
            }
            .padding(16)
        }
    }

    // MARK: - FSR Chart
    private var fsrChart: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("FSR Signal")
                .font(.caption)
                .fontWeight(.semibold)
                .foregroundStyle(.gray)

            Chart {
                ForEach(fsrSnapshot.asChartPoints) { pt in
                    LineMark(
                        x: .value("t", pt.id),
                        y: .value("FSR", pt.value)
                    )
                    .foregroundStyle(Color.blue)
                    .lineStyle(StrokeStyle(lineWidth: 2))
                    .interpolationMethod(.catmullRom)
                }

                if let t = targetValue {
                    RuleMark(y: .value("Target", t))
                        .foregroundStyle(Color.red)
                        .lineStyle(StrokeStyle(lineWidth: 1.5, dash: [6, 3]))
                        .annotation(position: .top, alignment: .trailing) {
                            Text(String(format: "%.3f", t))
                                .font(.system(size: 9, design: .monospaced))
                                .foregroundStyle(.red)
                        }
                }
            }
            .chartXAxis(.hidden)
            .chartYAxis {
                AxisMarks(position: .leading) { value in
                    AxisGridLine(stroke: StrokeStyle(lineWidth: 0.3)).foregroundStyle(.gray.opacity(0.3))
                    AxisValueLabel()
                        .font(.system(size: 9, design: .monospaced))
                        .foregroundStyle(.gray)
                }
            }
            .chartPlotStyle { area in
                area.background(Color(.systemGray6).opacity(0.08)).cornerRadius(8)
            }
        }
        .padding(12)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(Color(.systemGray6).opacity(0.12))
                .overlay(RoundedRectangle(cornerRadius: 12).stroke(Color.gray.opacity(0.15), lineWidth: 0.5))
        )
    }

    // MARK: - Controls Section
    private var controlsSection: some View {
        VStack(spacing: 10) {
            // Leg Toggle
            HStack(spacing: 10) {
                Button(action: {
                    useLeftLeg.toggle()
                    resetChart()
                }) {
                    Label(useLeftLeg ? "Left Leg" : "Right Leg",
                          systemImage: useLeftLeg ? "l.circle.fill" : "r.circle.fill")
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundStyle(.white)
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 12)
                        .background(RoundedRectangle(cornerRadius: 10).fill(Color.blue.opacity(0.3)))
                }

                Button(action: { showTargetSheet = true }) {
                    Label("Set Target", systemImage: "target")
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundStyle(.white)
                        .frame(maxWidth: .infinity)
                        .padding(.vertical, 12)
                        .background(RoundedRectangle(cornerRadius: 10).fill(Color(.systemGray5).opacity(0.4)))
                }
            }

            HStack(spacing: 10) {
                ControlButton(title: "Mark Trial (\(ble.markCount))", icon: "flag.fill") {
                    ble.markTrial()
                }
                ControlButton(title: "Recal FSRs", icon: "arrow.clockwise") {
                    ble.calibrateFSR()
                }
            }

            // FSR Index settings
            VStack(spacing: 8) {
                HStack {
                    Text("Left FSR Index")
                        .font(.caption)
                        .foregroundStyle(.gray)
                    Spacer()
                    Stepper("\(settings.leftFSRIndex)", value: $settings.leftFSRIndex, in: 0...15)
                        .labelsHidden()
                    Text("\(settings.leftFSRIndex)")
                        .font(.system(.body, design: .monospaced, weight: .semibold))
                        .foregroundStyle(.white)
                        .frame(width: 32, alignment: .trailing)
                        .onChange(of: settings.leftFSRIndex) { _ in settings.save() }
                }

                HStack {
                    Text("Right FSR Index")
                        .font(.caption)
                        .foregroundStyle(.gray)
                    Spacer()
                    Stepper("\(settings.rightFSRIndex)", value: $settings.rightFSRIndex, in: 0...15)
                        .labelsHidden()
                    Text("\(settings.rightFSRIndex)")
                        .font(.system(.body, design: .monospaced, weight: .semibold))
                        .foregroundStyle(.white)
                        .frame(width: 32, alignment: .trailing)
                        .onChange(of: settings.rightFSRIndex) { _ in settings.save() }
                }
            }
            .padding(12)
            .background(RoundedRectangle(cornerRadius: 10).fill(Color(.systemGray6).opacity(0.2)))
        }
    }

    // MARK: - Target Sheet
    private var targetSheet: some View {
        NavigationStack {
            Form {
                Section("Goal Threshold") {
                    TextField("e.g. 0.500", text: $targetInput)
                        .keyboardType(.decimalPad)
                        .font(.system(.body, design: .monospaced))
                }
                Section {
                    Text("When the FSR value crosses above this threshold, the counter increments and haptics trigger.")
                        .font(.caption)
                        .foregroundStyle(.gray)
                }
            }
            .navigationTitle("Set Target")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { showTargetSheet = false }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Set") {
                        if let v = Double(targetInput) {
                            targetValue = v
                            isAboveGoal = false
                        }
                        showTargetSheet = false
                    }
                }
            }
        }
        .presentationDetents([.medium])
    }

    // MARK: - Chart Buffer
    private func appendFSRSample(_ value: Double) {
        fsrBuffer[writeIdx % capacity] = value
        writeIdx += 1
    }

    private func resetChart() {
        fsrBuffer = Array(repeating: 0, count: capacity)
        writeIdx = 0
        fsrSnapshot = fsrBuffer
    }

    private func startTimer() {
        displayTimer = Timer.scheduledTimer(withTimeInterval: 0.05, repeats: true) { _ in
            let start = writeIdx % capacity
            if writeIdx < capacity {
                fsrSnapshot = Array(fsrBuffer[0..<writeIdx]) + Array(repeating: 0, count: capacity - writeIdx)
            } else {
                fsrSnapshot = Array(fsrBuffer[start...]) + Array(fsrBuffer[..<start])
            }
        }
    }

    private func stopTimer() {
        displayTimer?.invalidate()
        displayTimer = nil
    }

    // MARK: - Goal Detection
    private func checkGoal(_ value: Double) {
        guard let target = targetValue else { return }
        if !isAboveGoal && value > target {
            isAboveGoal = true
            targetsReached += 1
            triggerGoalFeedback()
        } else if isAboveGoal && value <= target {
            isAboveGoal = false
        }
    }

    private func triggerGoalFeedback() {
        // Haptic feedback (Apple-native, much better than a system beep)
        let generator = UINotificationFeedbackGenerator()
        generator.notificationOccurred(.success)

        // Visual flash
        withAnimation(.easeIn(duration: 0.1)) { flashGreen = true }
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.6) {
            withAnimation(.easeOut(duration: 0.3)) { flashGreen = false }
        }
    }
}

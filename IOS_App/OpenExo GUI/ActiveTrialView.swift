import SwiftUI

struct ActiveTrialView: View {
    @EnvironmentObject private var ble: BLEManager
    @EnvironmentObject private var logger: CSVLogger
    @Binding var navPath: NavigationPath

    @State private var showAltBlock = false   // toggle between [0-3] and [4-7]
    @State private var showPrefixSheet = false
    @State private var csvPrefixInput = ""
    @State private var showEndAlert = false
    @Environment(\.horizontalSizeClass) private var hSizeClass

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            if hSizeClass == .regular {
                iPadLayout
            } else {
                iPhoneLayout
            }
        }
        .navigationTitle("")
        .navigationBarHidden(true)
        .onAppear { ble.startChartTimer() }
        .onDisappear { ble.stopChartTimer() }
        .alert("End Trial?", isPresented: $showEndAlert) {
            Button("End Trial", role: .destructive) { endTrial() }
            Button("Cancel", role: .cancel) {}
        } message: {
            Text("This will stop motors, disconnect from the device, and save the CSV log.")
        }
        .sheet(isPresented: $showPrefixSheet) { prefixSheet }
    }

    // MARK: - iPad: Side by Side
    private var iPadLayout: some View {
        HStack(spacing: 0) {
            controlsPanel
                .frame(width: 300)
                .background(Color(.systemGray6).opacity(0.12))
            Divider().background(Color.gray.opacity(0.3))
            chartsPanel
        }
    }

    // MARK: - iPhone: Stacked (charts fixed, controls scroll)
    private var iPhoneLayout: some View {
        VStack(spacing: 0) {
            compactHeader
            chartsPanel
                .frame(height: 280)
            ScrollView {
                controlsPanel
            }
        }
    }

    // MARK: - Compact Header (iPhone only)
    private var compactHeader: some View {
        HStack {
            batteryView
            Spacer()
            dataStatusBadge
            Spacer()
            pausePlayButton
            Spacer()
            Button(action: { showEndAlert = true }) {
                Label("End", systemImage: "stop.fill")
                    .font(.system(size: 13, weight: .bold))
                    .foregroundStyle(.white)
                    .padding(.horizontal, 14)
                    .padding(.vertical, 8)
                    .background(Capsule().fill(Color.red))
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 10)
        .background(Color(.systemGray6).opacity(0.15))
    }

    // Shows packet count so you can confirm data is flowing
    private var dataStatusBadge: some View {
        HStack(spacing: 4) {
            Circle()
                .fill(ble.rtPacketCount > 0 ? Color.green : Color.orange)
                .frame(width: 7, height: 7)
                .shadow(color: ble.rtPacketCount > 0 ? .green : .orange, radius: 3)
            Text(ble.rtPacketCount > 0 ? "\(ble.rtPacketCount) pkts" : "No data")
                .font(.system(size: 11, design: .monospaced))
                .foregroundStyle(.gray)
        }
    }

    // MARK: - Controls Panel
    private var controlsPanel: some View {
        ScrollView {
            VStack(spacing: 14) {
                // Group 1: Title + iPad-only header controls
                Group {
                    VStack(spacing: 2) {
                        HStack(spacing: 8) {
                            Image(systemName: "figure.walk.motion")
                                .font(.title2)
                                .foregroundStyle(.blue)
                            Text("Active Trial")
                                .font(.title2.bold())
                                .foregroundStyle(.white)
                        }
                        if logger.isLogging {
                            Text(logger.currentFileName)
                                .font(.caption2)
                                .foregroundStyle(.gray)
                                .lineLimit(1)
                        }
                    }
                    .frame(maxWidth: .infinity, alignment: .leading)
                    .padding(.top, 4)
                    .padding(.horizontal, 4)

                    if hSizeClass == .regular {
                        batteryView.padding(.horizontal, 4)
                        Divider().background(Color.gray.opacity(0.2))
                        pausePlayButton
                        endTrialButton
                        Divider().background(Color.gray.opacity(0.2))
                    }
                }

                // Group 2: Controller + Data sections
                Group {
                    sectionLabel("CONTROLLER")
                    ControlButton(title: "Update Controller", icon: "slider.horizontal.3") {
                        navPath.append(AppScreen.settings)
                    }
                    ControlButton(title: "Mark Trial (\(ble.markCount))", icon: "flag.fill") {
                        ble.markTrial()
                    }

                    sectionLabel("DATA")
                    ControlButton(title: showAltBlock ? "Toggle plotting channels" : "Toggle plotting channels",
                                  icon: "chart.xyaxis.line") {
                        showAltBlock.toggle()
                    }
                    ControlButton(title: "Set CSV Prefix", icon: "pencil") {
                        csvPrefixInput = GUISettings.load().csvPrefix
                        showPrefixSheet = true
                    }
                    ControlButton(title: "Save New CSV", icon: "doc.badge.plus") {
                        logger.rollover(prefix: GUISettings.load().csvPrefix)
                    }
                }

                // Group 3: Advanced section
                Group {
                    sectionLabel("ADVANCED")
                    ControlButton(title: "Bio Feedback", icon: "waveform.path.ecg") {
                        navPath.append(AppScreen.bioFeedback)
                    }
                    ControlButton(title: "Recalibrate FSRs", icon: "arrow.clockwise") {
                        ble.calibrateFSR()
                    }
                    ControlButton(title: "Send Preset FSR", icon: "dial.high.fill") {
                        ble.sendFSRThresholds(left: 0.25, right: 0.25)
                    }
                    ControlButton(title: "Recalibrate Torque", icon: "wrench.fill") {
                        ble.calibrateTorque()
                    }
                }
            }
            .padding(16)
        }
    }

    private func sectionLabel(_ text: String) -> some View {
        Text(text)
            .font(.caption)
            .fontWeight(.semibold)
            .foregroundStyle(.gray)
            .frame(maxWidth: .infinity, alignment: .leading)
    }

    // MARK: - Charts Panel
    private var chartsPanel: some View {
        let offset = showAltBlock ? 4 : 0
        let snapshot = ble.chartSnapshot
        let names = ble.parameterNames

        let ch0 = snapshot[offset]
        let ch1 = snapshot[offset + 1]
        let ch2 = snapshot[offset + 2]
        let ch3 = snapshot[offset + 3]

        let label0 = names.count > offset     ? names[offset]     : "Ch\(offset)"
        let label1 = names.count > offset + 1 ? names[offset + 1] : "Ch\(offset + 1)"
        let label2 = names.count > offset + 2 ? names[offset + 2] : "Ch\(offset + 2)"
        let label3 = names.count > offset + 3 ? names[offset + 3] : "Ch\(offset + 3)"

        return VStack(spacing: 12) {
            RealTimeChart(
                series1: ch0, series2: ch1,
                color1: .blue, color2: .red,
                label1: label0, label2: label1,
                title: "\(label0) · \(label1)"
            )
            RealTimeChart(
                series1: ch2, series2: ch3,
                color1: .green, color2: .purple,
                label1: label2, label2: label3,
                title: "\(label2) · \(label3)"
            )
        }
        .padding(12)
    }

    // MARK: - Battery View
    private var batteryView: some View {
        HStack(spacing: 6) {
            Image(systemName: batteryIcon)
                .font(.system(size: 18))
                .foregroundStyle(batteryColor)
            if let v = ble.batteryVoltage {
                Text(String(format: "%.2fV", v))
                    .font(.system(.body, design: .monospaced, weight: .semibold))
                    .foregroundStyle(batteryColor)
            } else {
                Text("-- V")
                    .font(.system(.body, design: .monospaced))
                    .foregroundStyle(.gray)
            }
        }
    }

    private var batteryIcon: String {
        guard let v = ble.batteryVoltage else { return "battery.0" }
        if v >= 11.5 { return "battery.100" }
        if v >= 11.0 { return "battery.50" }
        return "battery.25"
    }

    private var batteryColor: Color {
        guard let v = ble.batteryVoltage else { return .gray }
        return v >= 11.0 ? .green : .red
    }

    // MARK: - Pause/Play
    private var pausePlayButton: some View {
        Button(action: { ble.isPaused ? ble.motorsOn() : ble.motorsOff() }) {
            Image(systemName: ble.isPaused ? "play.fill" : "pause.fill")
                .font(.system(size: 15, weight: .semibold))
                .foregroundStyle(.white)
                .padding(.horizontal, 20)
                .padding(.vertical, 10)
                .background(Capsule().fill(Color.blue))
        }
    }

    // MARK: - End Trial (iPad only in sidebar)
    private var endTrialButton: some View {
        Button(action: { showEndAlert = true }) {
            Label("End Trial", systemImage: "stop.circle.fill")
                .font(.system(size: 15, weight: .bold))
                .foregroundStyle(.white)
                .frame(maxWidth: .infinity)
                .padding(.vertical, 13)
                .background(RoundedRectangle(cornerRadius: 12).fill(Color.red))
        }
    }

    // MARK: - Prefix Sheet
    private var prefixSheet: some View {
        NavigationStack {
            Form {
                Section("CSV Filename Prefix") {
                    TextField("e.g. subject01", text: $csvPrefixInput)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                }
                Section {
                    Text("Files will be named: \(csvPrefixInput)_trial_YYYYMMDD_HHMMSS.csv")
                        .font(.caption)
                        .foregroundStyle(.gray)
                }
            }
            .navigationTitle("Set CSV Prefix")
            .navigationBarTitleDisplayMode(.inline)
            .toolbar {
                ToolbarItem(placement: .cancellationAction) {
                    Button("Cancel") { showPrefixSheet = false }
                }
                ToolbarItem(placement: .confirmationAction) {
                    Button("Save") {
                        var settings = GUISettings.load()
                        let sanitized = csvPrefixInput.filter { $0.isLetter || $0.isNumber || $0 == "_" || $0 == "-" }
                        settings.csvPrefix = sanitized
                        settings.save()
                        logger.rollover(prefix: sanitized)
                        showPrefixSheet = false
                    }
                }
            }
        }
        .presentationDetents([.medium])
    }

    // MARK: - Actions
    private func endTrial() {
        ble.logCallback = nil
        ble.endTrial()
        logger.stopLogging()
        navPath = NavigationPath()
        navPath.append(AppScreen.endTrial)
    }
}

// MARK: - Control Button
struct ControlButton: View {
    let title: String
    let icon: String
    var color: Color = .white
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            HStack(spacing: 10) {
                Image(systemName: icon)
                    .font(.system(size: 14, weight: .medium))
                    .foregroundStyle(.blue)
                    .frame(width: 22)
                Text(title)
                    .font(.system(size: 15, weight: .medium))
                    .foregroundStyle(color)
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(.gray.opacity(0.5))
            }
            .padding(.horizontal, 14)
            .padding(.vertical, 12)
            .background(
                RoundedRectangle(cornerRadius: 10)
                    .fill(Color(.systemGray6).opacity(0.2))
            )
        }
    }
}

// MARK: - Real-Time Chart (Canvas-based, robust)
struct RealTimeChart: View {
    let series1: [Double]
    let series2: [Double]
    let color1: Color
    let color2: Color
    let label1: String
    let label2: String
    let title: String

    private var sampleCount: Int { max(series1.count, series2.count) }

    private var yRange: (lo: Double, hi: Double) {
        let all = series1 + series2
        guard !all.isEmpty else { return (-1, 1) }
        var lo = all.min()!
        var hi = all.max()!
        if abs(hi - lo) < 0.001 {
            // Flat line — provide visible range around the value
            let center = (hi + lo) / 2
            lo = center - 1
            hi = center + 1
        } else {
            let pad = (hi - lo) * 0.12
            lo -= pad
            hi += pad
        }
        return (lo, hi)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text(title)
                    .font(.caption).fontWeight(.semibold).foregroundStyle(.gray)
                    .lineLimit(1)
                Spacer()
                HStack(spacing: 10) {
                    legendDot(color: color1, label: label1)
                    legendDot(color: color2, label: label2)
                }
            }
            .padding(.horizontal, 4)

            ZStack {
                if sampleCount < 2 {
                    Text("Waiting for data…")
                        .font(.caption)
                        .foregroundStyle(.gray.opacity(0.6))
                }
                Canvas { context, size in
                    let (yMin, yMax) = yRange
                    let ySpan = yMax - yMin
                    guard ySpan > 0, size.width > 0, size.height > 0 else { return }

                    func pt(_ i: Int, _ v: Double, _ count: Int) -> CGPoint {
                        let x = count > 1 ? size.width * CGFloat(i) / CGFloat(count - 1) : 0
                        let y = size.height * CGFloat(1.0 - (v - yMin) / ySpan)
                        return CGPoint(x: x, y: max(0, min(size.height, y)))
                    }

                    func drawGrid() {
                        for i in 0...4 {
                            let y = size.height * CGFloat(i) / 4
                            var grid = Path()
                            grid.move(to: CGPoint(x: 0, y: y))
                            grid.addLine(to: CGPoint(x: size.width, y: y))
                            context.stroke(grid, with: .color(.gray.opacity(0.18)), lineWidth: 0.5)
                        }
                        if yMin < 0 && yMax > 0 {
                            let zY = size.height * CGFloat(1.0 - (0 - yMin) / ySpan)
                            var zLine = Path()
                            zLine.move(to: CGPoint(x: 0, y: zY))
                            zLine.addLine(to: CGPoint(x: size.width, y: zY))
                            context.stroke(zLine, with: .color(.gray.opacity(0.4)), lineWidth: 0.8)
                        }
                    }

                    func drawSeries(_ data: [Double], color: Color) {
                        guard data.count > 1 else { return }
                        var path = Path()
                        path.move(to: pt(0, data[0], data.count))
                        for i in 1..<data.count {
                            path.addLine(to: pt(i, data[i], data.count))
                        }
                        context.stroke(path, with: .color(color),
                                       style: StrokeStyle(lineWidth: 2, lineJoin: .round))
                    }

                    drawGrid()
                    drawSeries(series1, color: color1)
                    drawSeries(series2, color: color2)

                    let topLabel = String(format: "%.1f", yMax)
                    let botLabel = String(format: "%.1f", yMin)
                    let font = Font.system(size: 8, design: .monospaced)
                    context.draw(Text(topLabel).font(font).foregroundColor(.gray),
                                 at: CGPoint(x: 4, y: 8), anchor: .leading)
                    context.draw(Text(botLabel).font(font).foregroundColor(.gray),
                                 at: CGPoint(x: 4, y: size.height - 4), anchor: .leading)
                }
                .background(Color(.systemGray6).opacity(0.08))
                .clipShape(RoundedRectangle(cornerRadius: 8))
                .frame(maxHeight: .infinity)
            }
        }
        .frame(maxHeight: .infinity)
        .padding(8)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(Color(.systemGray6).opacity(0.12))
                .overlay(RoundedRectangle(cornerRadius: 12)
                    .stroke(Color.gray.opacity(0.15), lineWidth: 0.5))
        )
    }

    private func legendDot(color: Color, label: String) -> some View {
        HStack(spacing: 4) {
            RoundedRectangle(cornerRadius: 2)
                .fill(color)
                .frame(width: 14, height: 3)
            Text(label)
                .font(.system(size: 9))
                .foregroundStyle(.gray)
                .lineLimit(1)
        }
    }
}

// MARK: - End Trial / Save Data Page
struct EndTrialView: View {
    @EnvironmentObject private var logger: CSVLogger
    @EnvironmentObject private var ble: BLEManager
    @Binding var navPath: NavigationPath

    @State private var newFileName = ""
    @State private var trialNotes = ""
    @State private var showDiscardAlert = false
    @State private var showShareSheet = false
    @State private var renameSuccess: Bool?

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            VStack(spacing: 0) {
                successHeader
                ScrollView {
                    VStack(spacing: 16) {
                        fileInfoCard
                        renameCard
                        notesCard
                        shareCard
                    }
                    .padding(16)
                }
                bottomActions
            }
        }
        .navigationTitle("")
        .navigationBarHidden(true)
        .onAppear {
            newFileName = logger.currentFileName
                .replacingOccurrences(of: ".csv", with: "")
        }
        .alert("Discard Data?", isPresented: $showDiscardAlert) {
            Button("Discard", role: .destructive) {
                logger.deleteLastFile()
                navPath = NavigationPath()
            }
            Button("Keep Data", role: .cancel) {}
        } message: {
            Text("This will permanently delete the CSV file and any notes for this trial.")
        }
        .sheet(isPresented: $showShareSheet) {
            if let url = logger.lastSavedFileURL {
                ShareSheet(items: [url])
            }
        }
    }

    // MARK: - Header
    private var successHeader: some View {
        VStack(spacing: 8) {
            Image(systemName: "checkmark.circle.fill")
                .font(.system(size: 48))
                .foregroundStyle(.green)
            Text("Trial Complete")
                .font(.title2.bold())
                .foregroundStyle(.white)
            Text("Review and save your data")
                .font(.subheadline)
                .foregroundStyle(.gray)
        }
        .frame(maxWidth: .infinity)
        .padding(.vertical, 24)
        .background(Color(.systemGray6).opacity(0.15))
    }

    // MARK: - File Info
    private var fileInfoCard: some View {
        card {
            VStack(alignment: .leading, spacing: 12) {
                cardLabel("FILE DETAILS")
                infoRow(icon: "doc.text.fill", label: "File", value: logger.currentFileName)
                infoRow(icon: "externaldrive.fill", label: "Size", value: logger.lastFileSize)
                infoRow(icon: "number", label: "Data Points", value: formatNumber(logger.rowCount))
                infoRow(icon: "flag.fill", label: "Marks", value: "\(ble.markCount)")
                infoRow(icon: "clock.fill", label: "Duration", value: logger.trialDurationFormatted)
            }
        }
    }

    private func infoRow(icon: String, label: String, value: String) -> some View {
        HStack(spacing: 10) {
            Image(systemName: icon)
                .font(.system(size: 13))
                .foregroundStyle(.blue)
                .frame(width: 20)
            Text(label)
                .font(.subheadline)
                .foregroundStyle(.gray)
            Spacer()
            Text(value)
                .font(.system(.subheadline, design: .monospaced, weight: .medium))
                .foregroundStyle(.white)
                .lineLimit(1)
        }
    }

    // MARK: - Rename
    private var renameCard: some View {
        card {
            VStack(alignment: .leading, spacing: 10) {
                cardLabel("RENAME FILE")
                HStack(spacing: 10) {
                    TextField("filename", text: $newFileName)
                        .font(.system(.body, design: .monospaced))
                        .foregroundStyle(.white)
                        .autocorrectionDisabled()
                        .textInputAutocapitalization(.never)
                        .padding(10)
                        .background(
                            RoundedRectangle(cornerRadius: 8)
                                .fill(Color(.systemGray5).opacity(0.2))
                        )
                    Text(".csv")
                        .font(.system(.body, design: .monospaced))
                        .foregroundStyle(.gray)

                    Button("Rename") {
                        let cleaned = newFileName.filter {
                            $0.isLetter || $0.isNumber || $0 == "_" || $0 == "-" || $0 == " "
                        }
                        guard !cleaned.isEmpty else { renameSuccess = false; return }
                        renameSuccess = logger.renameLastFile(to: cleaned)
                        if renameSuccess == true { newFileName = cleaned }
                    }
                    .font(.system(size: 14, weight: .semibold))
                    .foregroundStyle(.blue)
                    .padding(.horizontal, 14)
                    .padding(.vertical, 10)
                    .background(RoundedRectangle(cornerRadius: 8).fill(Color.blue.opacity(0.15)))
                }
                if let success = renameSuccess {
                    HStack(spacing: 4) {
                        Image(systemName: success ? "checkmark.circle.fill" : "xmark.circle.fill")
                        Text(success ? "Renamed successfully" : "Rename failed")
                    }
                    .font(.caption)
                    .foregroundStyle(success ? .green : .red)
                }
            }
        }
    }

    // MARK: - Notes
    private var notesCard: some View {
        card {
            VStack(alignment: .leading, spacing: 10) {
                cardLabel("TRIAL NOTES")
                TextEditor(text: $trialNotes)
                    .font(.system(.body))
                    .foregroundStyle(.white)
                    .scrollContentBackground(.hidden)
                    .frame(minHeight: 80)
                    .padding(8)
                    .background(
                        RoundedRectangle(cornerRadius: 8)
                            .fill(Color(.systemGray5).opacity(0.2))
                    )
                Text("Notes are saved as a companion .txt file alongside the CSV.")
                    .font(.caption2)
                    .foregroundStyle(.gray)
            }
        }
    }

    // MARK: - Share
    private var shareCard: some View {
        Button { showShareSheet = true } label: {
            HStack(spacing: 10) {
                Image(systemName: "square.and.arrow.up")
                    .font(.system(size: 15, weight: .medium))
                    .foregroundStyle(.blue)
                Text("Share / Export CSV")
                    .font(.system(size: 15, weight: .medium))
                    .foregroundStyle(.white)
                Spacer()
                Image(systemName: "chevron.right")
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(.gray.opacity(0.5))
            }
            .padding(14)
            .background(
                RoundedRectangle(cornerRadius: 12)
                    .fill(Color(.systemGray6).opacity(0.18))
            )
        }
        .disabled(logger.lastSavedFileURL == nil)
    }

    // MARK: - Bottom Actions
    private var bottomActions: some View {
        VStack(spacing: 0) {
            Divider().background(Color.gray.opacity(0.3))
            VStack(spacing: 10) {
                Button {
                    logger.saveTrialNotes(trialNotes)
                    navPath = NavigationPath()
                } label: {
                    HStack(spacing: 8) {
                        Image(systemName: "checkmark.circle.fill")
                            .font(.system(size: 16))
                        Text("Save & Return to Scan")
                            .font(.system(size: 16, weight: .bold))
                    }
                    .foregroundStyle(.white)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 14)
                    .background(RoundedRectangle(cornerRadius: 12).fill(Color.green))
                }

                Button { showDiscardAlert = true } label: {
                    HStack(spacing: 8) {
                        Image(systemName: "trash")
                            .font(.system(size: 14))
                        Text("Discard Data")
                            .font(.system(size: 15, weight: .medium))
                    }
                    .foregroundStyle(.red.opacity(0.8))
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 12)
                    .background(RoundedRectangle(cornerRadius: 12).fill(Color.red.opacity(0.1)))
                }
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 12)
        }
        .background(Color(.systemGray6).opacity(0.12))
    }

    // MARK: - Helpers
    private func card<Content: View>(@ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading) { content() }
            .padding(14)
            .background(
                RoundedRectangle(cornerRadius: 12)
                    .fill(Color(.systemGray6).opacity(0.18))
            )
    }

    private func cardLabel(_ text: String) -> some View {
        Text(text)
            .font(.caption)
            .fontWeight(.semibold)
            .foregroundStyle(.gray)
    }

    private func formatNumber(_ n: Int) -> String {
        let formatter = NumberFormatter()
        formatter.numberStyle = .decimal
        return formatter.string(from: NSNumber(value: n)) ?? "\(n)"
    }
}

// MARK: - UIActivityViewController Wrapper
struct ShareSheet: UIViewControllerRepresentable {
    let items: [Any]

    func makeUIViewController(context: Context) -> UIActivityViewController {
        UIActivityViewController(activityItems: items, applicationActivities: nil)
    }

    func updateUIViewController(_ uiViewController: UIActivityViewController, context: Context) {}
}

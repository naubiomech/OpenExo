import Foundation

class CSVLogger: ObservableObject {

    @Published var isLogging = false
    @Published var currentFileName = ""
    @Published var logDirectory: URL
    @Published var lastSavedFileURL: URL?
    private(set) var rowCount: Int = 0

    private var fileHandle: FileHandle?
    private var columnNames: [String] = []
    private(set) var trialStartDate: Date?

    init() {
        logDirectory = FileManager.default.urls(for: .documentDirectory, in: .userDomainMask)[0]
    }

    func startLogging(prefix: String, columnNames: [String]) {
        stopLogging()
        let dataWidth = 16
        var cols = columnNames
        while cols.count < dataWidth {
            cols.append("data\(cols.count)")
        }
        self.columnNames = cols

        let sanitized = prefix.filter { $0.isLetter || $0.isNumber || $0 == "_" || $0 == "-" }
        let formatter = DateFormatter()
        formatter.dateFormat = "yyyyMMdd_HHmmss"
        let timestamp = formatter.string(from: Date())
        let filename = sanitized.isEmpty ? "trial_\(timestamp).csv" : "\(sanitized)_trial_\(timestamp).csv"

        let url = logDirectory.appendingPathComponent(filename)
        FileManager.default.createFile(atPath: url.path, contents: nil)
        fileHandle = try? FileHandle(forWritingTo: url)

        let header = (["epoch", "mark"] + cols).joined(separator: ",") + "\n"
        fileHandle?.write(header.data(using: .utf8) ?? Data())

        currentFileName = filename
        lastSavedFileURL = url
        rowCount = 0
        trialStartDate = Date()
        isLogging = true
    }

    func log(values: [Double], mark: Int) {
        guard isLogging, let fh = fileHandle else { return }
        let epoch = Date().timeIntervalSince1970
        var padded = Array(values.prefix(columnNames.count))
        while padded.count < columnNames.count { padded.append(0) }
        let dataFields = padded.map { String(format: "%.4f", $0) }
        let row = ([String(format: "%.3f", epoch), "\(mark)"] + dataFields).joined(separator: ",") + "\n"
        fh.write(row.data(using: .utf8) ?? Data())
        rowCount += 1
    }

    func rollover(prefix: String) {
        let cols = columnNames
        stopLogging()
        startLogging(prefix: prefix, columnNames: cols)
    }

    func stopLogging() {
        fileHandle?.closeFile()
        fileHandle = nil
        isLogging = false
    }

    // MARK: - File Operations

    var trialDurationFormatted: String {
        guard let start = trialStartDate else { return "--" }
        let seconds = Int(Date().timeIntervalSince(start))
        if seconds < 60 { return "\(seconds)s" }
        return "\(seconds / 60)m \(seconds % 60)s"
    }

    var lastFileSize: String {
        guard let url = lastSavedFileURL,
              let attrs = try? FileManager.default.attributesOfItem(atPath: url.path),
              let size = attrs[.size] as? Int64 else { return "Unknown" }
        return ByteCountFormatter.string(fromByteCount: size, countStyle: .file)
    }

    func renameLastFile(to newName: String) -> Bool {
        guard let url = lastSavedFileURL else { return false }
        let sanitized = newName.filter { $0.isLetter || $0.isNumber || $0 == "_" || $0 == "-" || $0 == " " }
        guard !sanitized.isEmpty else { return false }
        let newURL = url.deletingLastPathComponent().appendingPathComponent(sanitized + ".csv")
        do {
            try FileManager.default.moveItem(at: url, to: newURL)
            lastSavedFileURL = newURL
            currentFileName = sanitized + ".csv"
            return true
        } catch {
            print("[CSVLogger] Rename failed: \(error)")
            return false
        }
    }

    func deleteLastFile() {
        guard let url = lastSavedFileURL else { return }
        try? FileManager.default.removeItem(at: url)
        // Also remove companion notes file if it exists
        let notesURL = url.deletingPathExtension().appendingPathExtension("txt")
        try? FileManager.default.removeItem(at: notesURL)
        lastSavedFileURL = nil
        currentFileName = ""
    }

    func saveTrialNotes(_ notes: String) {
        guard let url = lastSavedFileURL, !notes.trimmingCharacters(in: .whitespacesAndNewlines).isEmpty else { return }
        let notesURL = url.deletingPathExtension().appendingPathExtension("txt")
        try? notes.write(to: notesURL, atomically: true, encoding: .utf8)
    }

    func allLogFiles() -> [URL] {
        let files = (try? FileManager.default.contentsOfDirectory(at: logDirectory, includingPropertiesForKeys: [.creationDateKey])) ?? []
        return files.filter { $0.pathExtension == "csv" }.sorted { $0.lastPathComponent > $1.lastPathComponent }
    }
}

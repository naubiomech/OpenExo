import Foundation

/// Reads the firmware-side `config.ini` so the iOS GUI knows which joints
/// the exoskeleton should expose and which controller is the configured
/// default for each joint family.
///
/// Two ini sources are checked, in priority order:
///   1. `<Documents>/config.ini` — the user can drop an updated config there
///      via the Files app (so they don't have to re-build the app every time
///      they edit the SD-card config).
///   2. `Bundle.main.url(forResource: "config", withExtension: "ini")` — the
///      copy that ships with the app, populated from `SDCard/config.ini`.
///
/// The parser is intentionally permissive: a missing/unreadable file leaves
/// every accessor returning `nil` so callers can fall back to the unfiltered
/// handshake behaviour.
enum ExoConfig {

    // MARK: - Public surface

    /// The set of joint families covered by the configured `name` value, or
    /// `nil` when `config.ini` is missing/unrecognised.
    static let allowedJointTypes: Set<String>? = computeAllowedJointTypes()

    /// Map of joint family → default controller name (e.g. `"hip"` → `"step"`).
    static let defaultControllers: [String: String] = parseDefaultControllers()

    /// Whether a handshake joint name should be displayed for the configured
    /// device.  Permissive when `config.ini` is unavailable.
    static func isJointAllowed(_ handshakeJointName: String) -> Bool {
        guard let allowed = allowedJointTypes else { return true }
        guard let family = jointType(for: handshakeJointName) else { return true }
        return allowed.contains(family)
    }

    /// Default controller name for a handshake joint, or `nil`.
    static func defaultControllerName(for handshakeJointName: String) -> String? {
        guard let family = jointType(for: handshakeJointName) else { return nil }
        return defaultControllers[family]
    }

    // MARK: - Joint name → family

    /// Map "Ankle(L)", "Hip(R)", "Arm1(L)", … back to a joint family
    /// (`"ankle"`, `"hip"`, `"arm_1"`).  Returns `nil` for unknown prefixes.
    static func jointType(for handshakeJointName: String) -> String? {
        guard !handshakeJointName.isEmpty else { return nil }
        // Strip "(L)" / "(R)" suffix, lowercase, ignore whitespace.
        let base = handshakeJointName
            .components(separatedBy: "(").first ?? handshakeJointName
        let key = base.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        return jointPrefixToFamily[key]
    }

    // MARK: - Tables (mirror ExoCode/src/ParseIni.h)

    /// Each `name` value in `[Exo]` maps to the joint families it covers.
    private static let nameToJointTypes: [String: Set<String>] = [
        "ankle":               ["ankle"],
        "hip":                 ["hip"],
        "knee":                ["knee"],
        "elbow":               ["elbow"],
        "hipankle":            ["hip", "ankle"],
        "hipelbow":            ["hip", "elbow"],
        "ankleelbow":          ["ankle", "elbow"],
        "arm":                 ["arm_1", "arm_2"],

        "bilateralankle":      ["ankle"],
        "bilateralhip":        ["hip"],
        "bilateralknee":       ["knee"],
        "bilateralelbow":      ["elbow"],
        "bilateralhipankle":   ["hip", "ankle"],
        "bilateralhipelbow":   ["hip", "elbow"],
        "bilateralankleelbow": ["ankle", "elbow"],
        "bilateralarm":        ["arm_1", "arm_2"],

        "leftankle":           ["ankle"],
        "rightankle":          ["ankle"],
        "lefthip":             ["hip"],
        "righthip":            ["hip"],
        "leftknee":            ["knee"],
        "rightknee":           ["knee"],
        "leftelbow":           ["elbow"],
        "rightelbow":          ["elbow"],
        "lefthipankle":        ["hip", "ankle"],
        "righthipankle":       ["hip", "ankle"],
        "lefthipelbow":        ["hip", "elbow"],
        "righthipelbow":       ["hip", "elbow"],
        "leftankleelbow":      ["ankle", "elbow"],
        "rightankleelbow":     ["ankle", "elbow"],
    ]

    private static let jointPrefixToFamily: [String: String] = [
        "ankle": "ankle",
        "hip":   "hip",
        "knee":  "knee",
        "elbow": "elbow",
        "arm1":  "arm_1",
        "arm2":  "arm_2",
    ]

    // MARK: - Reader

    private static func loadIniText() -> String? {
        let fm = FileManager.default
        // 1. Documents override (lets users drop an updated config.ini onto
        // the device via the Files app without rebuilding the app).
        if let docs = try? fm.url(for: .documentDirectory,
                                  in: .userDomainMask,
                                  appropriateFor: nil,
                                  create: false) {
            let userIni = docs.appendingPathComponent("config.ini")
            if fm.fileExists(atPath: userIni.path),
               let text = try? String(contentsOf: userIni, encoding: .utf8) {
                return text
            }
        }
        // 2. Bundled fallback.
        if let bundled = Bundle.main.url(forResource: "config", withExtension: "ini"),
           let text = try? String(contentsOf: bundled, encoding: .utf8) {
            return text
        }
        return nil
    }

    /// Lightweight ini parser tuned to the firmware's syntax.
    /// * Supports `;` and `#` line/inline comments.
    /// * Tolerates leading tabs / spaces on keys.
    /// * Section names are matched case-sensitively (firmware uses `[Exo]`).
    private static func parseIni() -> [String: [String: String]] {
        guard let text = loadIniText() else { return [:] }

        var sections: [String: [String: String]] = [:]
        var current = ""

        for rawLine in text.components(separatedBy: .newlines) {
            // Strip comments
            var line = rawLine
            for marker in [";", "#"] {
                if let r = line.range(of: marker) {
                    line = String(line[..<r.lowerBound])
                }
            }
            let trimmed = line.trimmingCharacters(in: .whitespacesAndNewlines)
            guard !trimmed.isEmpty else { continue }

            if trimmed.hasPrefix("[") && trimmed.hasSuffix("]") {
                current = String(trimmed.dropFirst().dropLast())
                    .trimmingCharacters(in: .whitespacesAndNewlines)
                if sections[current] == nil { sections[current] = [:] }
                continue
            }

            guard let eq = trimmed.firstIndex(of: "=") else { continue }
            let key = String(trimmed[..<eq])
                .trimmingCharacters(in: .whitespacesAndNewlines)
            let value = String(trimmed[trimmed.index(after: eq)...])
                .trimmingCharacters(in: .whitespacesAndNewlines)
            guard !key.isEmpty else { continue }
            sections[current, default: [:]][key] = value
        }
        return sections
    }

    // MARK: - Computed values

    /// Raw `name` value from `[Exo]`, or `nil` when missing.
    static var rawExoName: String? {
        let sections = parseIni()
        return sections["Exo"]?["name"]
    }

    private static func computeAllowedJointTypes() -> Set<String>? {
        guard let raw = rawExoName else { return nil }
        let key = raw.trimmingCharacters(in: .whitespacesAndNewlines).lowercased()
        return nameToJointTypes[key]
    }

    private static func parseDefaultControllers() -> [String: String] {
        let sections = parseIni()
        guard let exo = sections["Exo"] else { return [:] }
        let mapping: [(family: String, key: String)] = [
            ("hip",   "hipDefaultController"),
            ("knee",  "kneeDefaultController"),
            ("ankle", "ankleDefaultController"),
            ("elbow", "elbowDefaultController"),
            ("arm_1", "arm_1DefaultController"),
            ("arm_2", "arm_2DefaultController"),
        ]
        var out: [String: String] = [:]
        for (family, key) in mapping {
            if let value = exo[key]?.trimmingCharacters(in: .whitespacesAndNewlines),
               !value.isEmpty, value != "0" {
                out[family] = value
            }
        }
        return out
    }
}

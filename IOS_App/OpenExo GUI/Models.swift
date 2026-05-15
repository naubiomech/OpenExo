import Foundation

// MARK: - Navigation
enum AppScreen: Hashable {
    case activeTrial
    case settings
    case bioFeedback
    case endTrial
}

// MARK: - Chart Data
struct IndexedValue: Identifiable {
    let id: Int
    let value: Double
}

extension Array where Element == Double {
    var asChartPoints: [IndexedValue] {
        enumerated().map { IndexedValue(id: $0.offset, value: $0.element) }
    }
}

// MARK: - Handshake Data
struct JointInfo: Identifiable {
    let id = UUID()
    let name: String
    let jointID: Int
    var controllers: [ControllerInfo]
}

struct ControllerInfo: Identifiable {
    let id = UUID()
    let name: String
    let controllerID: Int
    let params: [String]
}

// MARK: - Known Joints (fallback for Basic Settings)
struct KnownJoint: Identifiable {
    let id: Int
    let name: String

    static let all: [KnownJoint] = [
        KnownJoint(id: 65, name: "Left Hip"),
        KnownJoint(id: 33, name: "Right Hip"),
        KnownJoint(id: 66, name: "Left Knee"),
        KnownJoint(id: 34, name: "Right Knee"),
        KnownJoint(id: 68, name: "Left Ankle"),
        KnownJoint(id: 36, name: "Right Ankle"),
        KnownJoint(id: 72, name: "Left Elbow"),
        KnownJoint(id: 40, name: "Right Elbow"),
    ]
}

// MARK: - Saved Settings (SQLite via `OpenExoDatabase`)

struct GUISettings: Codable, Equatable {
    var bilateral: Bool = false
    var lastJointIndex: Int = 0
    var lastControllerIndex: Int = 0
    var lastParamIndex: Int = 0
    var lastValue: Double = 0
    var lastJointName: String = ""
    var lastControllerName: String = ""
    /// Left hip (BLE id); matches Python `ActiveTrialBasicSettingsPage` legacy map / `KnownJoint`.
    var lastBasicJointID: Int = 65
    var lastBasicControllerID: Int = 0
    var lastBasicParamIndex: Int = 0
    var lastBasicValue: Double = 0
    var csvPrefix: String = ""
    var leftFSRIndex: Int = 7
    var rightFSRIndex: Int = 5
    var hasAppliedSettings: Bool = false

    static func load() -> GUISettings {
        OpenExoDatabase.shared.migrateFromUserDefaultsIfNeeded()
        return OpenExoDatabase.shared.loadGUISettings() ?? GUISettings()
    }

    func save() {
        OpenExoDatabase.shared.saveGUISettings(self)
    }
}

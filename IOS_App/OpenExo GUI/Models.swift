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
    /// Default parameter values, keyed by the same index as `params`.
    /// Stored as raw strings so the UI can decide int-vs-float per cell
    /// (the firmware sends them as strings on the wire, line 6 of each
    /// controller csv on the SD card).  Empty when no `v,` row arrived.
    var paramValues: [String] = []
}

// MARK: - Controller Value Database
/// Identifies a (joint, controller) pair for the in-memory parameter database.
struct ControllerKey: Hashable {
    let jointID: Int
    let controllerID: Int
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

// MARK: - Saved Settings
struct GUISettings {
    var bilateral: Bool = false
    var lastJointIndex: Int = 0
    var lastControllerIndex: Int = 0
    var lastParamIndex: Int = 0
    var lastValue: Double = 0
    var lastJointName: String = ""
    var lastControllerName: String = ""
    var lastBasicJointID: Int = 68
    var lastBasicControllerID: Int = 0
    var lastBasicParamIndex: Int = 0
    var lastBasicValue: Double = 0
    var csvPrefix: String = ""
    var leftFSRIndex: Int = 7
    var rightFSRIndex: Int = 5
    var hasAppliedSettings: Bool = false

    private enum Keys {
        static let bilateral = "bilateral"
        static let jointIndex = "lastJointIndex"
        static let controllerIndex = "lastControllerIndex"
        static let paramIndex = "lastParamIndex"
        static let value = "lastValue"
        static let jointName = "lastJointName"
        static let controllerName = "lastControllerName"
        static let basicJointID = "lastBasicJointID"
        static let basicControllerID = "lastBasicControllerID"
        static let basicParamIndex = "lastBasicParamIndex"
        static let basicValue = "lastBasicValue"
        static let csvPrefix = "csvPrefix"
        static let leftFSRIndex = "leftFSRIndex"
        static let rightFSRIndex = "rightFSRIndex"
        static let hasAppliedSettings = "hasAppliedSettings"
    }

    static func load() -> GUISettings {
        let d = UserDefaults.standard
        var s = GUISettings()
        s.bilateral = d.bool(forKey: Keys.bilateral)
        s.lastJointIndex = d.integer(forKey: Keys.jointIndex)
        s.lastControllerIndex = d.integer(forKey: Keys.controllerIndex)
        s.lastParamIndex = d.integer(forKey: Keys.paramIndex)
        s.lastValue = d.double(forKey: Keys.value)
        s.lastJointName = d.string(forKey: Keys.jointName) ?? ""
        s.lastControllerName = d.string(forKey: Keys.controllerName) ?? ""
        if d.object(forKey: Keys.basicJointID) != nil {
            s.lastBasicJointID = d.integer(forKey: Keys.basicJointID)
        }
        s.lastBasicControllerID = d.integer(forKey: Keys.basicControllerID)
        s.lastBasicParamIndex = d.integer(forKey: Keys.basicParamIndex)
        s.lastBasicValue = d.double(forKey: Keys.basicValue)
        s.csvPrefix = d.string(forKey: Keys.csvPrefix) ?? ""
        if d.object(forKey: Keys.leftFSRIndex) != nil {
            s.leftFSRIndex = d.integer(forKey: Keys.leftFSRIndex)
        }
        if d.object(forKey: Keys.rightFSRIndex) != nil {
            s.rightFSRIndex = d.integer(forKey: Keys.rightFSRIndex)
        }
        s.hasAppliedSettings = d.bool(forKey: Keys.hasAppliedSettings)
        return s
    }

    func save() {
        let d = UserDefaults.standard
        d.set(bilateral, forKey: Keys.bilateral)
        d.set(lastJointIndex, forKey: Keys.jointIndex)
        d.set(lastControllerIndex, forKey: Keys.controllerIndex)
        d.set(lastParamIndex, forKey: Keys.paramIndex)
        d.set(lastValue, forKey: Keys.value)
        d.set(lastJointName, forKey: Keys.jointName)
        d.set(lastControllerName, forKey: Keys.controllerName)
        d.set(lastBasicJointID, forKey: Keys.basicJointID)
        d.set(lastBasicControllerID, forKey: Keys.basicControllerID)
        d.set(lastBasicParamIndex, forKey: Keys.basicParamIndex)
        d.set(lastBasicValue, forKey: Keys.basicValue)
        d.set(csvPrefix, forKey: Keys.csvPrefix)
        d.set(leftFSRIndex, forKey: Keys.leftFSRIndex)
        d.set(rightFSRIndex, forKey: Keys.rightFSRIndex)
        d.set(hasAppliedSettings, forKey: Keys.hasAppliedSettings)
    }
}

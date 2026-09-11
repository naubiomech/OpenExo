import Foundation
import SQLite3

private let SQLITE_TRANSIENT = unsafeBitCast(-1, to: sqlite3_destructor_type.self)

/// Local SQLite store for GUI settings and cached controller metadata (matrix / values / param names).
/// Mirrors Python `gui_settings.txt` + in-memory controller DB persistence across app launches.
final class OpenExoDatabase {
    static let shared = OpenExoDatabase()

    private let queue = DispatchQueue(label: "com.openexo.openapi.database")
    private var db: OpaquePointer?
    private var lastError: String?

    private init() {
        queue.sync { openIfNeeded() }
    }

    deinit {
        queue.sync {
            if db != nil {
                sqlite3_close(db)
                db = nil
            }
        }
    }

    // MARK: - Paths

    private static var storeURL: URL {
        let base = FileManager.default.urls(for: .applicationSupportDirectory, in: .userDomainMask).first
            ?? FileManager.default.temporaryDirectory
        let dir = base.appendingPathComponent("OpenExo", isDirectory: true)
        if !FileManager.default.fileExists(atPath: dir.path) {
            try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        }
        return dir.appendingPathComponent("openexo.sqlite", isDirectory: false)
    }

    // MARK: - Lifecycle

    private func sqliteErrorMessage(_ db: OpaquePointer?) -> String {
        guard let db, let cstr = sqlite3_errmsg(db) else {
            return "unknown sqlite error"
        }
        return String(cString: cstr)
    }

    private func storeError(_ message: String) {
        lastError = message
        print("OpenExoDatabase error: \(message)")
    }

    func lastErrorMessage() -> String? {
        queue.sync { lastError }
    }

    private func openIfNeeded() {
        guard db == nil else { return }
        let path = Self.storeURL.path
        if sqlite3_open_v2(path, &db, SQLITE_OPEN_CREATE | SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX, nil) != SQLITE_OK {
            let detail = sqliteErrorMessage(db)
            storeError("open failed (\(path)): \(detail)")
            if db != nil {
                sqlite3_close(db)
            }
            db = nil
            return
        }
        lastError = nil
        exec(
            """
            CREATE TABLE IF NOT EXISTS kv (
                key TEXT PRIMARY KEY NOT NULL,
                value TEXT NOT NULL
            );
            """
        )
    }

    private func exec(_ sql: String) {
        guard let db else {
            storeError("exec failed: database unavailable")
            return
        }
        var err: UnsafeMutablePointer<Int8>?
        if sqlite3_exec(db, sql, nil, nil, &err) != SQLITE_OK {
            if let err {
                let message = String(cString: err)
                storeError("exec failed: \(message)")
                sqlite3_free(err)
            } else {
                storeError("exec failed: \(sqliteErrorMessage(db))")
            }
        }
    }

    // MARK: - Key / value

    private func setJSON(_ object: some Encodable, forKey key: String) {
        guard let db else {
            storeError("set failed for key '\(key)': database unavailable")
            return
        }
        guard let data = try? JSONEncoder().encode(object),
              let text = String(data: data, encoding: .utf8) else {
            storeError("set failed for key '\(key)': encode failed")
            return
        }

        let sql = "INSERT OR REPLACE INTO kv (key, value) VALUES (?, ?);"
        var stmt: OpaquePointer?
        guard sqlite3_prepare_v2(db, sql, -1, &stmt, nil) == SQLITE_OK else {
            storeError("set failed for key '\(key)': prepare failed (\(sqliteErrorMessage(db)))")
            return
        }
        defer { sqlite3_finalize(stmt) }

        sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT)
        sqlite3_bind_text(stmt, 2, text, -1, SQLITE_TRANSIENT)
        if sqlite3_step(stmt) != SQLITE_DONE {
            storeError("set failed for key '\(key)': step failed (\(sqliteErrorMessage(db)))")
        }
    }

    private func getJSON<T: Decodable>(_ type: T.Type, forKey key: String) -> T? {
        guard let db else {
            storeError("get failed for key '\(key)': database unavailable")
            return nil
        }
        let sql = "SELECT value FROM kv WHERE key = ? LIMIT 1;"
        var stmt: OpaquePointer?
        guard sqlite3_prepare_v2(db, sql, -1, &stmt, nil) == SQLITE_OK else {
            storeError("get failed for key '\(key)': prepare failed (\(sqliteErrorMessage(db)))")
            return nil
        }
        defer { sqlite3_finalize(stmt) }

        sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT)
        let stepResult = sqlite3_step(stmt)
        guard stepResult == SQLITE_ROW else {
            if stepResult != SQLITE_DONE {
                storeError("get failed for key '\(key)': step failed (\(sqliteErrorMessage(db)))")
            }
            return nil
        }
        guard let cstr = sqlite3_column_text(stmt, 0) else {
            storeError("get failed for key '\(key)': empty row value")
            return nil
        }
        let text = String(cString: cstr)
        guard let data = text.data(using: .utf8) else {
            storeError("get failed for key '\(key)': utf8 conversion failed")
            return nil
        }
        do {
            return try JSONDecoder().decode(T.self, from: data)
        } catch {
            storeError("get failed for key '\(key)': decode failed (\(error.localizedDescription))")
            return nil
        }
    }

    private func removeKey(_ key: String) {
        guard let db else {
            storeError("remove failed for key '\(key)': database unavailable")
            return
        }
        let sql = "DELETE FROM kv WHERE key = ?;"
        var stmt: OpaquePointer?
        guard sqlite3_prepare_v2(db, sql, -1, &stmt, nil) == SQLITE_OK else {
            storeError("remove failed for key '\(key)': prepare failed (\(sqliteErrorMessage(db)))")
            return
        }
        defer { sqlite3_finalize(stmt) }
        sqlite3_bind_text(stmt, 1, key, -1, SQLITE_TRANSIENT)
        if sqlite3_step(stmt) != SQLITE_DONE {
            storeError("remove failed for key '\(key)': step failed (\(sqliteErrorMessage(db)))")
        }
    }

    // MARK: - GUI settings

    func saveGUISettings(_ settings: GUISettings) {
        queue.sync {
            openIfNeeded()
            setJSON(settings, forKey: Keys.guiSettings)
        }
    }

    func loadGUISettings() -> GUISettings? {
        queue.sync {
            openIfNeeded()
            return getJSON(GUISettings.self, forKey: Keys.guiSettings)
        }
    }

    /// One-time import from `UserDefaults` after SQLite becomes canonical storage.
    func migrateFromUserDefaultsIfNeeded() {
        queue.sync {
            openIfNeeded()
            guard db != nil else {
                storeError("migrate failed: database unavailable")
                return
            }
            if getJSON(Bool.self, forKey: Keys.migratedFromUserDefaults) == true { return }
            var s = GUISettings()
            let d = UserDefaults.standard
            s.bilateral = d.bool(forKey: "bilateral")
            s.lastJointIndex = d.integer(forKey: "lastJointIndex")
            s.lastControllerIndex = d.integer(forKey: "lastControllerIndex")
            s.lastParamIndex = d.integer(forKey: "lastParamIndex")
            s.lastValue = d.double(forKey: "lastValue")
            s.lastJointName = d.string(forKey: "lastJointName") ?? ""
            s.lastControllerName = d.string(forKey: "lastControllerName") ?? ""
            if d.object(forKey: "lastBasicJointID") != nil {
                s.lastBasicJointID = d.integer(forKey: "lastBasicJointID")
            }
            s.lastBasicControllerID = d.integer(forKey: "lastBasicControllerID")
            s.lastBasicParamIndex = d.integer(forKey: "lastBasicParamIndex")
            s.lastBasicValue = d.double(forKey: "lastBasicValue")
            s.csvPrefix = d.string(forKey: "csvPrefix") ?? ""
            if d.object(forKey: "leftFSRIndex") != nil {
                s.leftFSRIndex = d.integer(forKey: "leftFSRIndex")
            }
            if d.object(forKey: "rightFSRIndex") != nil {
                s.rightFSRIndex = d.integer(forKey: "rightFSRIndex")
            }
            s.hasAppliedSettings = d.bool(forKey: "hasAppliedSettings")

            setJSON(s, forKey: Keys.guiSettings)
            setJSON(true, forKey: Keys.migratedFromUserDefaults)
        }
    }

    // MARK: - Controller snapshot

    func saveControllerSnapshot(_ snapshot: ControllerSnapshot) {
        queue.sync {
            openIfNeeded()
            setJSON(snapshot, forKey: Keys.controllerSnapshot)
        }
    }

    func loadControllerSnapshot() -> ControllerSnapshot? {
        queue.sync {
            openIfNeeded()
            return getJSON(ControllerSnapshot.self, forKey: Keys.controllerSnapshot)
        }
    }

    func clearControllerSnapshot() {
        queue.sync {
            openIfNeeded()
            removeKey(Keys.controllerSnapshot)
        }
    }

    func updateControllerSnapshotValues(jointIDs: [Int], controllerID: Int, paramIndex: Int, value: Double) {
        queue.sync {
            openIfNeeded()
            guard var snapshot = getJSON(ControllerSnapshot.self, forKey: Keys.controllerSnapshot) else {
                return
            }

            let valueText = String(value)
            for jointID in jointIDs {
                let key = "\(jointID)_\(controllerID)"
                var values = snapshot.values[key] ?? Array(
                    repeating: "",
                    count: max(controllerParamCount(in: snapshot, jointID: jointID, controllerID: controllerID), paramIndex + 1)
                )

                if values.count <= paramIndex {
                    values.append(contentsOf: repeatElement("", count: paramIndex + 1 - values.count))
                }

                values[paramIndex] = valueText
                snapshot.values[key] = values
            }

            snapshot.updatedAt = Date().timeIntervalSince1970
            setJSON(snapshot, forKey: Keys.controllerSnapshot)
        }
    }

    private enum Keys {
        static let guiSettings = "gui_settings_v1"
        static let migratedFromUserDefaults = "migrated_from_user_defaults_v1"
        static let controllerSnapshot = "controller_snapshot_v1"
    }

    private func controllerParamCount(in snapshot: ControllerSnapshot, jointID: Int, controllerID: Int) -> Int {
        snapshot.matrix.first {
            $0.count >= 4 && $0[1] == String(jointID) && $0[3] == String(controllerID)
        }?.dropFirst(4).count ?? 0
    }
}

// MARK: - Persisted controller DB (handshake cache)

struct ControllerSnapshot: Codable, Equatable {
    /// Peripheral UUID string at time of handshake (matches `savedDeviceUUID` when reconnecting).
    var deviceUUID: String
    /// Rows: [jointDisplay, jointId, controllerName, controllerId, ...param names] — same shape as Python matrix rows.
    var matrix: [[String]]
    /// Keys `"<jointId>_<controllerId>"` → raw string values from `v,` rows when present.
    var values: [String: [String]]
    var parameterNames: [String]
    var updatedAt: TimeInterval

    static func buildMatrix(from joints: [JointInfo]) -> [[String]] {
        joints.sorted { $0.jointID < $1.jointID }.flatMap { j in
            j.controllers.map { c in
                let display = "\(j.name) (\(j.jointID))"
                return [display, String(j.jointID), c.name, String(c.controllerID)] + c.params
            }
        }
    }

    /// Rebuild `JointInfo` tree from a persisted matrix (stable ordering by joint id, then controller id).
    static func joints(from matrix: [[String]]) -> [JointInfo] {
        struct Agg {
            var name: String
            var controllers: [ControllerInfo]
        }
        var map: [Int: Agg] = [:]

        for row in matrix {
            guard row.count >= 4,
                  let jid = Int(row[1]),
                  let cid = Int(row[3]) else { continue }

            let ctrl = ControllerInfo(name: row[2], controllerID: cid, params: Array(row.dropFirst(4)))
            if var existing = map[jid] {
                if !existing.controllers.contains(where: { $0.controllerID == cid }) {
                    existing.controllers.append(ctrl)
                }
                map[jid] = existing
            } else {
                map[jid] = Agg(name: row[0], controllers: [ctrl])
            }
        }

        return map.keys.sorted().map { jid in
            let agg = map[jid]!
            let sortedCtrls = agg.controllers.sorted { $0.controllerID < $1.controllerID }
            return JointInfo(name: agg.name, jointID: jid, controllers: sortedCtrls)
        }
    }
}

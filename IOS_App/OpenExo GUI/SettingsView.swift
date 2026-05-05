import SwiftUI

struct SettingsView: View {
    @EnvironmentObject private var ble: BLEManager
    @Binding var navPath: NavigationPath

    @State private var showAdvanced: Bool
    @State private var settings = GUISettings.load()

    // Advanced Settings State
    @State private var selectedJointIndex: Int = 0
    @State private var selectedControllerIndex: Int = 0
    @State private var selectedParamIndex: Int = 0
    @State private var paramValue: Double = 0

    // Basic Settings State
    @State private var basicJointID: Int = 68
    @State private var basicControllerID: Int = 0
    @State private var basicParamIndex: Int = 0
    @State private var basicValue: Double = 0

    @State private var isBilateral: Bool = false
    @State private var appliedSuccessfully = false
    @State private var isRestoringState = false

    init(navPath: Binding<NavigationPath>) {
        _navPath = navPath
        _showAdvanced = State(initialValue: BLEManager.shared.handshakeReceived && !BLEManager.shared.joints.isEmpty)
    }

    private var joints: [JointInfo] { ble.joints }
    private var currentJoint: JointInfo? { joints.indices.contains(selectedJointIndex) ? joints[selectedJointIndex] : nil }
    private var currentControllers: [ControllerInfo] { currentJoint?.controllers ?? [] }
    private var currentController: ControllerInfo? { currentControllers.indices.contains(selectedControllerIndex) ? currentControllers[selectedControllerIndex] : nil }
    private var currentParams: [String] { currentController?.params ?? [] }

    /// Look up the current value of the selected param in the in-memory DB.
    /// Returns `nil` when the device hasn't sent a values row for this
    /// (joint, controller) pair, or when the param index is out of range.
    private func currentDBValue() -> Double? {
        guard let joint = currentJoint, let ctrl = currentController else { return nil }
        let key = ControllerKey(jointID: joint.jointID, controllerID: ctrl.controllerID)
        let bag = ble.controllerValues[key] ?? ctrl.paramValues
        guard selectedParamIndex >= 0, selectedParamIndex < bag.count else { return nil }
        return parseValueString(bag[selectedParamIndex])
    }

    /// Convert a raw firmware csv cell into a Double, matching the
    /// "send over as string, then determine if the value is a float or
    /// integer" rule from the spec.
    private func parseValueString(_ raw: String) -> Double? {
        let s = raw.trimmingCharacters(in: .whitespacesAndNewlines)
        if s.isEmpty || s == "?" || s == "??" { return nil }
        if s.contains(".") || s.lowercased().contains("e") {
            return Double(s)
        }
        if let i = Int(s) { return Double(i) }
        return Double(s)
    }

    /// Re-pull the value for the currently selected (joint, ctrl, param)
    /// out of the in-memory DB and shove it into the visible field.
    /// Falls back to the saved last-value when the DB has nothing for us.
    private func refreshValueFromDB() {
        if let v = currentDBValue() {
            paramValue = v
        }
    }

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            VStack(spacing: 0) {
                navBar
                modePicker
                ScrollView {
                    if showAdvanced && !joints.isEmpty {
                        advancedForm
                    } else {
                        basicForm
                    }
                }
                applyBar
            }
        }
        .navigationTitle("")
        .navigationBarHidden(true)
        .onAppear {
            loadSavedState()
            // After restoring user state, prefer the live DB value (it
            // reflects what's actually on the device or what the user just
            // applied last time).
            DispatchQueue.main.async { refreshValueFromDB() }
        }
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
            Text("Controller Settings")
                .font(.headline)
                .foregroundStyle(.white)
            Spacer()
            // Balance
            HStack(spacing: 6) {
                Image(systemName: "chevron.left")
                Text("Trial")
            }
            .font(.system(size: 15, weight: .medium))
            .foregroundStyle(.clear)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 14)
        .background(Color(.systemGray6).opacity(0.15))
    }

    // MARK: - Mode Picker
    private var modePicker: some View {
        Picker("Mode", selection: $showAdvanced) {
            Text("Advanced").tag(true)
            Text("Basic").tag(false)
        }
        .pickerStyle(.segmented)
        .padding(.horizontal, 16)
        .padding(.vertical, 10)
    }

    // MARK: - Advanced Form
    private var advancedForm: some View {
        VStack(spacing: 16) {
            bilateralToggle

            // Joint Picker
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("JOINT")
                    Picker("Joint", selection: $selectedJointIndex) {
                        ForEach(joints.indices, id: \.self) { i in
                            Text("\(joints[i].name) (ID \(joints[i].jointID))").tag(i)
                        }
                    }
                    .pickerStyle(.menu)
                    .tint(.blue)
                    .onChange(of: selectedJointIndex) { newIndex in
                        guard !isRestoringState else { return }
                        guard joints.indices.contains(newIndex) else {
                            selectedControllerIndex = 0
                            selectedParamIndex = 0
                            return
                        }
                        let joint = joints[newIndex]
                        // Default to the controller named in config.ini for the
                        // newly-selected joint family; otherwise just reset.
                        if let name = ExoConfig.defaultControllerName(for: joint.name),
                           let idx = joint.controllers.firstIndex(where: { $0.name == name }) {
                            selectedControllerIndex = idx
                        } else {
                            selectedControllerIndex = 0
                        }
                        selectedParamIndex = 0
                        refreshValueFromDB()
                    }
                }
            }

            // Controller Matrix Table (matches Python GUI)
            if let joint = currentJoint, !joint.controllers.isEmpty {
                formCard {
                    VStack(alignment: .leading, spacing: 8) {
                        formLabel("CONTROLLERS FOR \(joint.name.uppercased())")
                        ForEach(joint.controllers.indices, id: \.self) { i in
                            let ctrl = joint.controllers[i]
                            Button {
                                selectedControllerIndex = i
                                selectedParamIndex = 0
                                refreshValueFromDB()
                            } label: {
                                HStack(spacing: 10) {
                                    Image(systemName: selectedControllerIndex == i ? "largecircle.fill.circle" : "circle")
                                        .font(.system(size: 14))
                                        .foregroundStyle(selectedControllerIndex == i ? .blue : .gray)
                                    VStack(alignment: .leading, spacing: 2) {
                                        Text("\(ctrl.name) (ID \(ctrl.controllerID))")
                                            .font(.system(.subheadline, weight: selectedControllerIndex == i ? .semibold : .regular))
                                            .foregroundStyle(selectedControllerIndex == i ? .blue : .white)
                                        if !ctrl.params.isEmpty {
                                            Text(ctrl.params.joined(separator: " · "))
                                                .font(.caption2)
                                                .foregroundStyle(.gray)
                                                .lineLimit(1)
                                        } else {
                                            Text("(no params)")
                                                .font(.caption2)
                                                .foregroundStyle(.gray.opacity(0.6))
                                        }
                                    }
                                    Spacer()
                                }
                                .padding(.vertical, 8)
                                .padding(.horizontal, 10)
                                .background(
                                    RoundedRectangle(cornerRadius: 8)
                                        .fill(selectedControllerIndex == i ? Color.blue.opacity(0.12) : Color.clear)
                                )
                            }
                        }
                    }
                }
            }

            // Controller Picker
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("CONTROLLER")
                    if currentControllers.isEmpty {
                        Text("No controllers available").foregroundStyle(.gray).font(.subheadline)
                    } else {
                        Picker("Controller", selection: $selectedControllerIndex) {
                            ForEach(currentControllers.indices, id: \.self) { i in
                                Text("\(currentControllers[i].name) (ID \(currentControllers[i].controllerID))").tag(i)
                            }
                        }
                        .pickerStyle(.menu)
                        .tint(.blue)
                        .onChange(of: selectedControllerIndex) { _ in
                            guard !isRestoringState else { return }
                            selectedParamIndex = 0
                            refreshValueFromDB()
                        }
                    }
                }
            }

            // Parameter Picker
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("PARAMETER")
                    if currentParams.isEmpty {
                        Text("Select a controller first").foregroundStyle(.gray).font(.subheadline)
                    } else {
                        Picker("Parameter", selection: $selectedParamIndex) {
                            ForEach(currentParams.indices, id: \.self) { i in
                                Text(currentParams[i]).tag(i)
                            }
                        }
                        .pickerStyle(.menu)
                        .tint(.blue)
                        .onChange(of: selectedParamIndex) { _ in
                            guard !isRestoringState else { return }
                            refreshValueFromDB()
                        }
                    }
                }
            }

            valueField
        }
        .padding(16)
    }

    // MARK: - Basic Form
    private var basicForm: some View {
        VStack(spacing: 16) {
            bilateralToggle

            // Joint
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("JOINT")
                    Picker("Joint", selection: $basicJointID) {
                        ForEach(KnownJoint.all) { joint in
                            Text("\(joint.name) (ID \(joint.id))").tag(joint.id)
                        }
                        Text("Custom").tag(basicJointID)
                    }
                    .pickerStyle(.menu)
                    .tint(.blue)
                    HStack {
                        Text("Raw Joint ID")
                            .font(.caption)
                            .foregroundStyle(.gray)
                        Spacer()
                        Stepper("\(basicJointID)", value: $basicJointID, in: 0...255)
                            .labelsHidden()
                        Text("\(basicJointID)")
                            .font(.system(.body, design: .monospaced, weight: .semibold))
                            .foregroundStyle(.white)
                            .frame(width: 40, alignment: .trailing)
                    }
                }
            }

            // Controller ID
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("CONTROLLER ID")
                    HStack {
                        Stepper("Controller ID", value: $basicControllerID, in: 0...50)
                            .labelsHidden()
                        Spacer()
                        Text("\(basicControllerID)")
                            .font(.system(.title3, design: .monospaced, weight: .semibold))
                            .foregroundStyle(.white)
                            .frame(width: 50, alignment: .center)
                    }
                }
            }

            // Param Index
            formCard {
                VStack(alignment: .leading, spacing: 10) {
                    formLabel("PARAMETER INDEX")
                    HStack {
                        Stepper("Param Index", value: $basicParamIndex, in: 0...50)
                            .labelsHidden()
                        Spacer()
                        Text("\(basicParamIndex)")
                            .font(.system(.title3, design: .monospaced, weight: .semibold))
                            .foregroundStyle(.white)
                            .frame(width: 50, alignment: .center)
                    }
                }
            }

            valueField
        }
        .padding(16)
    }

    // MARK: - Shared Controls
    private var bilateralToggle: some View {
        formCard {
            Toggle(isOn: $isBilateral) {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Bilateral Mode")
                        .font(.system(.body, weight: .medium))
                        .foregroundStyle(.white)
                    Text("Mirror update to opposite-side joint")
                        .font(.caption)
                        .foregroundStyle(.gray)
                }
            }
            .tint(.blue)
        }
    }

    private var valueField: some View {
        formCard {
            VStack(alignment: .leading, spacing: 10) {
                formLabel("VALUE")
                HStack {
                    TextField("0.0", value: showAdvanced ? $paramValue : $basicValue, format: .number)
                        .font(.system(.title2, design: .monospaced, weight: .semibold))
                        .foregroundStyle(.white)
                        .keyboardType(.decimalPad)
                        .multilineTextAlignment(.leading)
                    Spacer()
                    // Quick adjust buttons
                    HStack(spacing: 8) {
                        nudgeButton(label: "-1", amount: -1)
                        nudgeButton(label: "-0.1", amount: -0.1)
                        nudgeButton(label: "+0.1", amount: 0.1)
                        nudgeButton(label: "+1", amount: 1)
                    }
                }
            }
        }
    }

    private func nudgeButton(label: String, amount: Double) -> some View {
        Button(label) {
            if showAdvanced { paramValue += amount } else { basicValue += amount }
        }
        .font(.system(size: 12, weight: .semibold))
        .foregroundStyle(.blue)
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(RoundedRectangle(cornerRadius: 7).fill(Color.blue.opacity(0.15)))
    }

    // MARK: - Apply Bar
    private var applyBar: some View {
        VStack(spacing: 0) {
            Divider().background(Color.gray.opacity(0.3))

            if appliedSuccessfully {
                HStack(spacing: 8) {
                    Image(systemName: "checkmark.circle.fill").foregroundStyle(.green)
                    Text("Parameter sent successfully").foregroundStyle(.green).font(.subheadline)
                }
                .padding(.vertical, 12)
            }

            HStack(spacing: 12) {
                Button("Cancel") { navPath.removeLast() }
                    .font(.system(size: 16, weight: .semibold))
                    .foregroundStyle(.gray)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 14)
                    .background(RoundedRectangle(cornerRadius: 12).fill(Color(.systemGray5).opacity(0.3)))

                Button("Apply") { applySettings() }
                    .font(.system(size: 16, weight: .bold))
                    .foregroundStyle(.white)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 14)
                    .background(RoundedRectangle(cornerRadius: 12).fill(Color.blue))
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 12)
        }
        .background(Color(.systemGray6).opacity(0.12))
    }

    // MARK: - Helpers
    private func formCard<Content: View>(@ViewBuilder content: () -> Content) -> some View {
        VStack(alignment: .leading) {
            content()
        }
        .padding(14)
        .background(
            RoundedRectangle(cornerRadius: 12)
                .fill(Color(.systemGray6).opacity(0.18))
        )
    }

    private func formLabel(_ text: String) -> some View {
        Text(text)
            .font(.caption)
            .fontWeight(.semibold)
            .foregroundStyle(.gray)
    }

    private func applySettings() {
        if showAdvanced, let joint = currentJoint, let controller = currentController {
            ble.updateParam(
                isBilateral: isBilateral,
                jointID: joint.jointID,
                controllerID: controller.controllerID,
                paramIndex: selectedParamIndex,
                value: paramValue
            )
        } else {
            ble.updateParam(
                isBilateral: isBilateral,
                jointID: basicJointID,
                controllerID: basicControllerID,
                paramIndex: basicParamIndex,
                value: basicValue
            )
        }
        saveState()
        withAnimation {
            appliedSuccessfully = true
        }
        DispatchQueue.main.asyncAfter(deadline: .now() + 1.5) {
            navPath.removeLast()
        }
    }

    private func loadSavedState() {
        isRestoringState = true
        let s = GUISettings.load()
        isBilateral = s.bilateral

        // Track whether we picked the controller from config.ini (overriding
        // the user's saved selection). When that happens we also reset the
        // param/value fields, since the saved param/value belong to the
        // *previous* controller and don't make sense for the new one.
        var overrodeWithConfigDefault = false

        if !joints.isEmpty {
            if !s.lastJointName.isEmpty,
               let jIdx = joints.firstIndex(where: { $0.name == s.lastJointName }) {
                selectedJointIndex = jIdx
            } else {
                selectedJointIndex = min(max(s.lastJointIndex, 0), joints.count - 1)
            }

            let joint = joints[selectedJointIndex]
            let controllers = joint.controllers
            if !controllers.isEmpty {
                // Priority order:
                //   1. config.ini default for this joint family — ALWAYS wins
                //      when present.  This is what the operator declared in
                //      SDCard/config.ini (e.g. hipDefaultController = step)
                //      and per the original spec must be pre-selected.
                //   2. the saved name (only used if config.ini has no default)
                //   3. the saved numeric index (clamped)
                if let defaultName = ExoConfig.defaultControllerName(for: joint.name),
                   let cIdx = controllers.firstIndex(where: { $0.name == defaultName }) {
                    selectedControllerIndex = cIdx
                    overrodeWithConfigDefault = (defaultName != s.lastControllerName)
                    dprint("[SettingsView] Pre-selecting config.ini default controller '\(defaultName)' for \(joint.name)")
                } else if !s.lastControllerName.isEmpty,
                          let cIdx = controllers.firstIndex(where: { $0.name == s.lastControllerName }) {
                    selectedControllerIndex = cIdx
                } else {
                    selectedControllerIndex = min(max(s.lastControllerIndex, 0), controllers.count - 1)
                }
                let params = controllers[selectedControllerIndex].params
                if overrodeWithConfigDefault {
                    selectedParamIndex = 0
                } else {
                    selectedParamIndex = params.isEmpty ? 0 : min(max(s.lastParamIndex, 0), params.count - 1)
                }
            } else {
                selectedControllerIndex = 0
                selectedParamIndex = 0
            }
        } else {
            selectedJointIndex = s.lastJointIndex
            selectedControllerIndex = s.lastControllerIndex
            selectedParamIndex = s.lastParamIndex
        }

        paramValue = overrodeWithConfigDefault ? 0 : s.lastValue
        basicJointID = s.lastBasicJointID
        basicControllerID = s.lastBasicControllerID
        basicParamIndex = s.lastBasicParamIndex
        basicValue = s.lastBasicValue

        DispatchQueue.main.async { isRestoringState = false }
    }

    private func saveState() {
        var s = GUISettings.load()
        s.bilateral = isBilateral
        s.lastJointIndex = selectedJointIndex
        s.lastControllerIndex = selectedControllerIndex
        s.lastParamIndex = selectedParamIndex
        s.lastValue = paramValue
        s.lastJointName = currentJoint?.name ?? ""
        s.lastControllerName = currentController?.name ?? ""
        s.lastBasicJointID = basicJointID
        s.lastBasicControllerID = basicControllerID
        s.lastBasicParamIndex = basicParamIndex
        s.lastBasicValue = basicValue
        s.hasAppliedSettings = true
        s.save()
    }
}

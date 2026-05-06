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
    @State private var basicJointID: Int = 65
    @State private var basicControllerID: Int = 0
    @State private var basicParamIndex: Int = 0
    @State private var basicValue: Double = 0

    @State private var isBilateral: Bool = false
    @State private var appliedSuccessfully = false
    @State private var isRestoringState = false
    @State private var dbWarningMessage: String?

    init(navPath: Binding<NavigationPath>) {
        _navPath = navPath
        // Advanced mode whenever we have controller metadata (live handshake or SQLite cache).
        _showAdvanced = State(initialValue: !BLEManager.shared.joints.isEmpty)
    }

    private var joints: [JointInfo] { ble.joints }
    private var currentJoint: JointInfo? { joints.indices.contains(selectedJointIndex) ? joints[selectedJointIndex] : nil }
    private var currentControllers: [ControllerInfo] { currentJoint?.controllers ?? [] }
    private var currentController: ControllerInfo? { currentControllers.indices.contains(selectedControllerIndex) ? currentControllers[selectedControllerIndex] : nil }
    private var currentParams: [String] { currentController?.params ?? [] }

    var body: some View {
        ZStack {
            Color.black.ignoresSafeArea()

            VStack(spacing: 0) {
                navBar
                modePicker
                if let dbWarningMessage, !dbWarningMessage.isEmpty {
                    HStack(alignment: .top, spacing: 8) {
                        Image(systemName: "exclamationmark.triangle.fill")
                            .foregroundStyle(.yellow)
                            .font(.caption)
                        Text(dbWarningMessage)
                            .font(.caption)
                            .foregroundStyle(.yellow)
                            .multilineTextAlignment(.leading)
                        Spacer()
                    }
                    .padding(.horizontal, 16)
                    .padding(.bottom, 6)
                }
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
            dbWarningMessage = OpenExoDatabase.shared.lastErrorMessage()
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
                    .onChange(of: selectedJointIndex) { _ in
                        guard !isRestoringState else { return }
                        selectedControllerIndex = 0
                        selectedParamIndex = 0
                        syncParamValueFromSnapshotIfAvailable()
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
                            syncParamValueFromSnapshotIfAvailable()
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
                            syncParamValueFromSnapshotIfAvailable()
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

        // Advanced mode: restore by name first, fall back to index
        if !joints.isEmpty {
            if !s.lastJointName.isEmpty,
               let jIdx = joints.firstIndex(where: { $0.name == s.lastJointName }) {
                selectedJointIndex = jIdx
            } else {
                selectedJointIndex = min(s.lastJointIndex, joints.count - 1)
            }

            let controllers = joints[selectedJointIndex].controllers
            if !controllers.isEmpty {
                if !s.lastControllerName.isEmpty,
                   let cIdx = controllers.firstIndex(where: { $0.name == s.lastControllerName }) {
                    selectedControllerIndex = cIdx
                } else {
                    selectedControllerIndex = min(s.lastControllerIndex, controllers.count - 1)
                }
                let params = controllers[min(selectedControllerIndex, controllers.count - 1)].params
                selectedParamIndex = params.isEmpty ? 0 : min(s.lastParamIndex, params.count - 1)
            } else {
                selectedControllerIndex = 0
                selectedParamIndex = 0
            }
        } else {
            selectedJointIndex = s.lastJointIndex
            selectedControllerIndex = s.lastControllerIndex
            selectedParamIndex = s.lastParamIndex
        }

        if let snapshotValue = currentSnapshotParamValue() {
            paramValue = snapshotValue
        } else {
            paramValue = s.lastValue
        }
        basicJointID = s.lastBasicJointID
        basicControllerID = s.lastBasicControllerID
        basicParamIndex = s.lastBasicParamIndex
        basicValue = s.lastBasicValue

        DispatchQueue.main.async { isRestoringState = false }
    }

    private func currentSnapshotParamValue() -> Double? {
        guard !joints.isEmpty,
              joints.indices.contains(selectedJointIndex),
              currentControllers.indices.contains(selectedControllerIndex) else {
            return nil
        }
        let controller = currentControllers[selectedControllerIndex]
        let key = "\(joints[selectedJointIndex].jointID)_\(controller.controllerID)"
        guard let values = OpenExoDatabase.shared.loadControllerSnapshot()?.values[key],
              values.indices.contains(selectedParamIndex) else {
            return nil
        }
        return Double(values[selectedParamIndex])
    }

    private func syncParamValueFromSnapshotIfAvailable() {
        guard showAdvanced, let snapshotValue = currentSnapshotParamValue() else { return }
        paramValue = snapshotValue
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

package com.example.andriod_app

/**
 * Canonical defaults aligned with Python GUI and iOS (`Models.GUISettings` / `BLEManager`).
 * Use for BLE scan duration, basic Update Controller joint id, and biofeedback FSR channels.
 */
object OpenExoDefaults {
    const val BLE_SCAN_SECONDS: Long = 3L
    const val BASIC_JOINT_ID_DEFAULT: Int = 65 // Left hip (BLE id)
    const val LEFT_FSR_CHANNEL_INDEX: Int = 7
    const val RIGHT_FSR_CHANNEL_INDEX: Int = 5
}

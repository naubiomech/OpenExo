package com.example.andriod_app.data

//data classes for the openexo app
//joint id vals match the firmware enum (see Parseini.h on the teensy)
//ble-relted ones (DiscoveredDevice, JointInfo, etc) mirror the iOS structs.

data class DiscoveredDevice(
    val address: String,
    val name: String
)

//AI ASSISTED - mirrors iOS JointInfo for handshake parsing
data class JointInfo(
    val name: String,
    val jointID: Int,
    val controllers: MutableList<ControllerInfo>
)

//AI ASSISTED - mirrors iOS ControllerInfo
data class ControllerInfo(
    val name: String,
    val controllerID: Int,
    val params: List<String>
)

//hard coded fallback joints (same as ios KnownJoint)
data class KnownJoint(val id: Int, val name: String)

val KNOWN_JOINTS = listOf(
    KnownJoint(65, "Left Hip"),
    KnownJoint(33, "Right Hip"),
    KnownJoint(66, "Left Knee"),
    KnownJoint(34, "Right Knee"),
    KnownJoint(68, "Left Ankle"),
    KnownJoint(36, "Right Ankle"),
    KnownJoint(72, "Left Elbow"),
    KnownJoint(40, "Right Elbow")
)

//for the firebase auth rest api
data class AuthUser(
  val uid: String,
  val email: String,
  val idToken: String )

//metdata we PUT to the realtime db on save
data class TrialRecord(
    val fileName: String,
    val notes: String,
    val durationSec: Int,
    val rowCount: Int,
    val markCount: Int,
    val timestampMs: Long
)

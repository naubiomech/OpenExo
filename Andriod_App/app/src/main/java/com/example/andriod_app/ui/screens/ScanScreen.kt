package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.foundation.lazy.LazyColumn
import androidx.compose.foundation.lazy.items
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.andriod_app.data.DiscoveredDevice
import com.example.andriod_app.ui.theme.*
import com.example.andriod_app.viewmodel.BleViewModel

@Composable
fun ScanScreen(
    bleVm: BleViewModel,
    onStartTrial: () -> Unit
)
{
    val isScanning by bleVm.ble.isScanning.collectAsStateWithLifecycle()
    val devices by bleVm.ble.devices.collectAsStateWithLifecycle()
    val connected by bleVm.ble.isConnected.collectAsStateWithLifecycle()
    val status by bleVm.ble.connectionStatus.collectAsStateWithLifecycle()
    val name by bleVm.ble.connectedName.collectAsStateWithLifecycle()
    val torqueCal by bleVm.ble.torqueCalibrated.collectAsStateWithLifecycle()
    val handshake by bleVm.ble.handshakeReceived.collectAsStateWithLifecycle()
    val battV by bleVm.ble.batteryVolts.collectAsStateWithLifecycle()
    var selected by remember { mutableStateOf<DiscoveredDevice?>(null) }
    //TODO: maybe auto start a scan when this screen first opens?

    Column(
        //inset for status bar (top) + gesture bar (bottom) so btns arent covered
        modifier = Modifier.fillMaxSize().background(DarkBg).systemBarsPadding().padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ) {
        Spacer(Modifier.height(8.dp))

        //title area
        Icon(Icons.Default.DirectionsWalk, contentDescription = null, tint = ExoBlue,
            modifier = Modifier.size(48.dp))
        Text("OpenExo", fontSize = 28.sp, fontWeight = FontWeight.Bold, color = Color.White)
        Text("Exoskeleton Controller", fontSize = 14.sp, color = GrayText)

        Spacer(Modifier.height(20.dp))

        //connection status card - dot turns green when ble linked
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(12.dp)).background(CardBg).padding(14.dp)
        ){
            Box(modifier = Modifier.size(10.dp).clip(RoundedCornerShape(50))
                .background(if(connected) ExoGreen else GrayText))
            Spacer(Modifier.width(12.dp))
            Column(Modifier.weight(1f)) {
                Text(
                    if(connected) "Connected: $name" else status,
                    color = Color.White, fontSize = 14.sp
                )
                if(connected && battV != null) {
                    Text(String.format("Battery %.2f V", battV), color = GrayText, fontSize = 11.sp)
                }
            }
            if(isScanning) {
                CircularProgressIndicator(Modifier.size(18.dp), strokeWidth = 2.dp)
            }
        }

        Spacer(Modifier.height(16.dp))

        Text("NEARBY DEVICES", color = GrayText, fontSize = 12.sp,
            fontWeight = FontWeight.SemiBold, modifier = Modifier.fillMaxWidth())

        Spacer(Modifier.height(8.dp))

        //empty state or device list
        if(devices.isEmpty() && !connected){
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier.fillMaxWidth().height(120.dp)
                    .clip(RoundedCornerShape(12.dp)).background(CardBg)
            ) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Icon(Icons.Default.SettingsInputAntenna, contentDescription = null,
                        tint = GrayText, modifier = Modifier.size(36.dp))
                    Spacer(Modifier.height(8.dp))
                    Text(if(isScanning) "Scanning..." else "Tap Scan to find devices",
                        color = GrayText, fontSize = 14.sp)
                }
            }
        }
        else {
            LazyColumn(modifier = Modifier.fillMaxWidth().heightIn(max = 240.dp)) {
                items(devices, key = { it.address }) { d ->
                    val isSel = selected?.address == d.address
                    Row(
                        verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier.fillMaxWidth().padding(vertical = 4.dp)
                            .clip(RoundedCornerShape(12.dp))
                            .background(if(isSel) ExoBlue.copy(alpha = 0.2f) else CardBg)
                            .clickable(enabled = !connected) { selected = d }
                            .padding(12.dp)
                    ) {
                        Icon(Icons.Default.Sensors, contentDescription = null, tint = ExoBlue)
                        Spacer(Modifier.width(12.dp))
                        Column(Modifier.weight(1f)) {
                            Text(d.name, color = Color.White, fontWeight = FontWeight.Medium)
                            Text(d.address, color = GrayText, fontSize = 11.sp,
                                fontFamily = FontFamily.Monospace)
                        }
                        if(connected && d.address == selected?.address) {
                            Text("Connected", color = ExoGreen, fontSize = 12.sp)
                        }
                    }
                }
            }
        }

        Spacer(Modifier.weight(1f))

        //scan + load saved btns
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp), modifier = Modifier.fillMaxWidth()) {
            Button(onClick = { bleVm.startScan() },
                enabled = !connected && !isScanning,
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.weight(1f)){
                Icon(Icons.Default.SettingsInputAntenna, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text(if(isScanning) "Scanning..." else "Scan")
            }
            Button(onClick = { bleVm.connectSaved() },
                enabled = !connected && bleVm.hasSavedDevice(),
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.weight(1f)) {
                Icon(Icons.Default.Bookmark, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text("Load Saved")
            }
        }

        Spacer(Modifier.height(8.dp))

        //connect btn
        Button(
            onClick = { selected?.let { bleVm.connect(it) } },
            enabled = selected != null && !connected,
            colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
            modifier = Modifier.fillMaxWidth()
        ) {
            Text("Connect")
        }

        Spacer(Modifier.height(8.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(10.dp), modifier = Modifier.fillMaxWidth()) {
            Button(
                onClick = { bleVm.calibrateTorque() },
                enabled = connected && handshake,
                colors = ButtonDefaults.buttonColors(
                    containerColor = if(torqueCal) ExoGreen else CardBg),
                modifier = Modifier.weight(1f)
            ){
                Text(if(torqueCal) "Calibrated" else "Calibrate")
            }

            Button(
                onClick = { bleVm.beginTrial(); onStartTrial() },
                enabled = connected && torqueCal,
                colors = ButtonDefaults.buttonColors(
                    containerColor = if(connected && torqueCal) ExoGreen else CardBg),
                modifier = Modifier.weight(1f)
            ) {
                Icon(Icons.Default.PlayArrow, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text("Start Trial")
            }
        }

        Spacer(Modifier.height(20.dp))
    }
}

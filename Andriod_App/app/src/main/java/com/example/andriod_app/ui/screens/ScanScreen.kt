package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.*

@Composable
fun ScanScreen(onStartTrial: () -> Unit) {
    var isScanning by remember { mutableStateOf(false) }
    var connected by remember { mutableStateOf(false) }
    var calibrated by remember { mutableStateOf(false) }

    Column(
        modifier = Modifier.fillMaxSize().background(DarkBg).padding(16.dp),
        horizontalAlignment = Alignment.CenterHorizontally
    ){
        Spacer(Modifier.height(40.dp))

        //title area
        Icon(Icons.Default.DirectionsWalk, contentDescription = null, tint = ExoBlue,
            modifier = Modifier.size(48.dp))
        Text("OpenExo", fontSize = 28.sp, fontWeight = FontWeight.Bold, color = Color.White)
        Text("Exoskeleton Controller", fontSize = 14.sp, color = GrayText)

        Spacer(Modifier.height(24.dp))

        //connection status
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(12.dp)).background(CardBg).padding(14.dp)
        ) {
            Box(modifier = Modifier.size(10.dp).clip(RoundedCornerShape(50))
                .background(if(connected) ExoGreen else GrayText))
            Spacer(Modifier.width(12.dp))
            Text(
                if(connected) "Connected" else if(isScanning) "Scanning..." else "Not connected",
                color = Color.White, fontSize = 14.sp
            )
            if (isScanning){
                Spacer(Modifier.weight(1f))
                CircularProgressIndicator(Modifier.size(18.dp), strokeWidth = 2.dp)
            }
        }

        Spacer(Modifier.height(16.dp))

        Text("NEARBY DEVICES", color = GrayText, fontSize = 12.sp,
            fontWeight = FontWeight.SemiBold, modifier = Modifier.fillMaxWidth())

        Spacer(Modifier.height(8.dp))

        //empty state or devics list
        if (!isScanning && !connected) {
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier.fillMaxWidth().height(120.dp)
                    .clip(RoundedCornerShape(12.dp)).background(CardBg)
            ){
                Column(horizontalAlignment = Alignment.CenterHorizontally){
                    Icon(Icons.Default.SettingsInputAntenna, contentDescription = null,
                        tint = GrayText, modifier = Modifier.size(36.dp))
                    Spacer(Modifier.height(8.dp))
                    Text("Tap Scan to find devices", color = GrayText, fontSize = 14.sp)
                }
            }
        } else {
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                    .background(CardBg).padding(12.dp)
            ){
                Icon(Icons.Default.Sensors, contentDescription = null, tint = ExoBlue)
                Spacer(Modifier.width(12.dp))
                Column {
                    Text("OpenExo-L-Ankle", color = Color.White, fontWeight = FontWeight.Medium)
                    Text("6E400001-B5A3-F393", color = GrayText, fontSize = 11.sp)
                }
                Spacer(Modifier.weight(1f))
                if(connected) Text("Connected", color = ExoGreen, fontSize = 12.sp)
            }
        }

        Spacer(Modifier.weight(1f))

        //scan and load btns
        Row(horizontalArrangement = Arrangement.spacedBy(10.dp), modifier = Modifier.fillMaxWidth()){
            Button(onClick = { isScanning = true },
                enabled = !connected,
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.weight(1f)) {
                Icon(Icons.Default.SettingsInputAntenna, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text(if(isScanning) "Scanning..." else "Scan")
            }
            Button(onClick = {},
                enabled = !connected,
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.weight(1f)){
                Icon(Icons.Default.Bookmark, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text("Load Saved")
            }
        }

        Spacer(Modifier.height(8.dp))

        //connect
        Button(
            onClick = { connected = true; isScanning = false },
            enabled = isScanning && !connected,
            colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
            modifier = Modifier.fillMaxWidth()
        ){
            Text("Connect")
        }

        Spacer(Modifier.height(8.dp))

        Row(horizontalArrangement = Arrangement.spacedBy(10.dp), modifier = Modifier.fillMaxWidth()) {
            Button(
                onClick = { calibrated = true },
                enabled = connected && !calibrated,
                colors = ButtonDefaults.buttonColors(
                    containerColor = if(calibrated) ExoGreen else CardBg),
                modifier = Modifier.weight(1f)
            ){
                Text(if(calibrated) "Calibrated" else "Calibrate")
            }

            Button(
                onClick = onStartTrial,
                enabled = connected && calibrated,
                colors = ButtonDefaults.buttonColors(
                    containerColor = if(connected && calibrated) ExoGreen else CardBg),
                modifier = Modifier.weight(1f)
            ) {
                Icon(Icons.Default.PlayArrow, contentDescription = null, Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text("Start Trial")
            }
        }

        Spacer(Modifier.height(24.dp))
    }
}

package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.verticalScroll
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.vector.ImageVector
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.*

@Composable
fun ActiveTrialScreen(
    onSettings: () -> Unit,
    onBioFeedback: () -> Unit,
    onEndTrial: () -> Unit
){
    var paused by remember { mutableStateOf(false) }
    var markCnt by remember { mutableIntStateOf(0) }
    var showEndDialog by remember { mutableStateOf(false) }

    //end trial confirmation
    if(showEndDialog){
        AlertDialog(
            onDismissRequest = { showEndDialog = false },
            title = { Text("End Trial?") },
            text = { Text("This will stop motors and save the CSV log.") },
            confirmButton = {
                TextButton(onClick = { showEndDialog = false; onEndTrial() }) {
                    Text("End Trial", color = ExoRed)
                }
            },
            dismissButton = {
                TextButton(onClick = { showEndDialog = false }){ Text("Cancel") }
            }
        )
    }

    Column(modifier = Modifier.fillMaxSize().background(DarkBg)) {

        //header bar
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth().background(CardBg)
                .padding(horizontal = 16.dp, vertical = 10.dp)
        ){
            //battery
            Icon(Icons.Default.BatteryFull, contentDescription = null, tint = ExoGreen, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(4.dp))
            Text("11.82V", color = ExoGreen, fontSize = 14.sp, fontFamily = FontFamily.Monospace)

            Spacer(Modifier.weight(1f))

            Button(onClick = { paused = !paused },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
                contentPadding = PaddingValues(horizontal = 16.dp, vertical = 6.dp)
            ){
                Text(if(paused) "Resume" else "Pause", fontSize = 13.sp)
            }

            Spacer(Modifier.width(8.dp))

            Button(onClick = { showEndDialog = true },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoRed),
                contentPadding = PaddingValues(horizontal = 14.dp, vertical = 6.dp)
            ) {
                Text("End", fontSize = 13.sp, fontWeight = FontWeight.Bold)
            }
        }

        //charts
        Column(modifier = Modifier.fillMaxWidth().padding(12.dp)) {
            Box(modifier = Modifier.fillMaxWidth().height(120.dp)
                .clip(RoundedCornerShape(10.dp)).background(CardBg),
                contentAlignment = Alignment.Center){
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Icon(Icons.Default.ShowChart, contentDescription = null, tint = ExoBlue)
                    Text("Torque Cmd / Torque Meas", color = GrayText, fontSize = 12.sp)
                }
            }
            Spacer(Modifier.height(8.dp))
            Box(modifier = Modifier.fillMaxWidth().height(120.dp)
                .clip(RoundedCornerShape(10.dp)).background(CardBg),
                contentAlignment = Alignment.Center){
                Column(horizontalAlignment = Alignment.CenterHorizontally){
                    Icon(Icons.Default.ShowChart, contentDescription = null, tint = ExoGreen)
                    Text("Angle / Velocity", color = GrayText, fontSize = 12.sp)
                }
            }
        }

        //controls
        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp)
        ){
            Text("Active Trial", fontWeight = FontWeight.Bold, fontSize = 20.sp, color = Color.White)
            Spacer(Modifier.height(8.dp))

            Text("CONTROLLER", color = GrayText, fontSize = 11.sp, fontWeight = FontWeight.SemiBold)
            Spacer(Modifier.height(4.dp))
            TrialBtn("Update Controller", Icons.Default.Tune, onSettings)
            Spacer(Modifier.height(6.dp))
            TrialBtn("Mark Trial ($markCnt)", Icons.Default.Flag) { markCnt++ }

            Spacer(Modifier.height(12.dp))
            Text("DATA", color = GrayText, fontSize = 11.sp)
            Spacer(Modifier.height(4.dp))
            TrialBtn("Show Alt Block", Icons.Default.ShowChart) {}
            Spacer(Modifier.height(6.dp))
            TrialBtn("Set CSV Prefix", Icons.Default.Edit) {}

            Spacer(Modifier.height(12.dp))
            Text("ADVANCED", color = GrayText, fontSize = 11.sp, fontWeight = FontWeight.SemiBold)
            Spacer(Modifier.height(4.dp))
            TrialBtn("Bio Feedback", Icons.Default.MonitorHeart, onBioFeedback)
            Spacer(Modifier.height(6.dp))
            TrialBtn("Recalibrate FSRs", Icons.Default.Refresh) {}
            Spacer(Modifier.height(6.dp))
            TrialBtn("Recalibrate Torque", Icons.Default.Build) {}

            Spacer(Modifier.height(16.dp))
        }
    }
}

//button for trial controls
@Composable
fun TrialBtn(text: String, icon: ImageVector, onClick: () -> Unit) {
    Button(
        onClick = onClick,
        shape = RoundedCornerShape(10.dp),
        colors = ButtonDefaults.buttonColors(containerColor = CardBg),
        modifier = Modifier.fillMaxWidth()
    ) {
        Icon(icon, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(16.dp))
        Spacer(Modifier.width(10.dp))
        Text(text, color = Color.White, modifier = Modifier.weight(1f), fontSize = 14.sp)
        Icon(Icons.Default.KeyboardArrowRight, contentDescription = null,
            tint = GrayText, modifier = Modifier.size(16.dp))
    }
}

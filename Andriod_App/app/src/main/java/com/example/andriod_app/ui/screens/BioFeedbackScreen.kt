package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
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
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.*

@Composable
fun BioFeedbackScreen(onBack: () -> Unit){
    var paused by remember { mutableStateOf(false) }
    var leftLeg by remember { mutableStateOf(true) }
    var targetsReachd by remember { mutableIntStateOf(3) }
    var leftIdx by remember { mutableIntStateOf(7) }
    var rightIdx by remember { mutableIntStateOf(5) }

    Column(modifier = Modifier.fillMaxSize().background(DarkBg)){

        //navbar
        Row(verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth().background(CardBg)
                .padding(horizontal = 16.dp, vertical = 14.dp)){
            Row(modifier = Modifier.clickable { onBack() },
                verticalAlignment = Alignment.CenterVertically) {
                Icon(Icons.Default.KeyboardArrowLeft, contentDescription = null, tint = ExoBlue)
                Text("Trial", color = ExoBlue)
            }
            Spacer(Modifier.weight(1f))
            Text("Biofeedback", color = Color.White, fontWeight = FontWeight.SemiBold)
            Spacer(Modifier.weight(1f))
            Button(onClick = { paused = !paused },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
                contentPadding = PaddingValues(horizontal = 12.dp, vertical = 6.dp)) {
                Text(if(paused) "Resume" else "Pause", fontSize = 13.sp)
            }
        }

        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState()).padding(16.dp)
        ){
            //fsr readout
            Row(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(16.dp)){
                Column(Modifier.weight(1f)){
                    Text(if(leftLeg) "Left Leg FSR" else "Right Leg FSR", color = GrayText, fontSize = 12.sp)
                    Text("0.342", fontSize = 36.sp, fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace, color = Color.White)
                }
                Column(horizontalAlignment = Alignment.End){
                    Text("Targets Reached", color = GrayText, fontSize = 12.sp)
                    Text("$targetsReachd", fontSize = 32.sp, fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace, color = ExoGreen)
                }
            }

            Spacer(Modifier.height(16.dp))

            //chart
            Box(modifier = Modifier.fillMaxWidth().height(180.dp)
                .clip(RoundedCornerShape(10.dp)).background(CardBg),
                contentAlignment = Alignment.Center) {
                Column(horizontalAlignment = Alignment.CenterHorizontally) {
                    Icon(Icons.Default.ShowChart, contentDescription = null,
                        tint = ExoBlue, modifier = Modifier.size(32.dp))
                    Spacer(Modifier.height(8.dp))
                    Text("FSR Signal Chart", color = GrayText, fontSize = 14.sp)
                    Text("(real-time data will render here)", color = GrayText, fontSize = 11.sp)
                }
            }

            Spacer(Modifier.height(16.dp))

            //target
            Row(verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(10.dp))
                    .background(ExoRed.copy(alpha = 0.1f)).padding(12.dp)){
                Icon(Icons.Default.GpsFixed, contentDescription = null, tint = ExoRed)
                Spacer(Modifier.width(8.dp))
                Text("Target: 0.500", color = ExoRed, fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace, modifier = Modifier.weight(1f))
                Text("Clear", color = ExoRed, fontSize = 13.sp, modifier = Modifier.clickable {})
            }

            Spacer(Modifier.height(12.dp))

            //leg toggle + set target
            Row(horizontalArrangement = Arrangement.spacedBy(10.dp)){
                Button(onClick = { leftLeg = !leftLeg },
                    colors = ButtonDefaults.buttonColors(containerColor = ExoBlue.copy(alpha = 0.3f)),
                    modifier = Modifier.weight(1f)){
                    Text(if(leftLeg) "Left Leg" else "Right Leg")
                }
                Button(onClick = {},
                    colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                    modifier = Modifier.weight(1f)){
                    Text("Set Target")
                }
            }

            Spacer(Modifier.height(16.dp))

            //fsr index config
            Column(modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(10.dp)).background(CardBg).padding(12.dp)){

                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Left FSR Index", color = GrayText, fontSize = 12.sp)
                    Spacer(Modifier.weight(1f))
                    IconButton(onClick = { if(leftIdx > 0) leftIdx-- }){
                        Icon(Icons.Default.Remove, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                    Text("$leftIdx", color = Color.White,
                        fontFamily = FontFamily.Monospace, fontWeight = FontWeight.SemiBold)
                    IconButton(onClick = { if(leftIdx < 15) leftIdx++ }){
                        Icon(Icons.Default.Add, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                }
                Spacer(Modifier.height(4.dp))
                Row(verticalAlignment = Alignment.CenterVertically){
                    Text("Right FSR Index", color = GrayText, fontSize = 12.sp)
                    Spacer(Modifier.weight(1f))
                    IconButton(onClick = { if(rightIdx > 0) rightIdx-- }) {
                        Icon(Icons.Default.Remove, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                    Text("$rightIdx", color = Color.White,
                        fontFamily = FontFamily.Monospace, fontWeight = FontWeight.SemiBold)
                    IconButton(onClick = { if(rightIdx < 15) rightIdx++ }){
                        Icon(Icons.Default.Add, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                }
            }
        }
    }
}

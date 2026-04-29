package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.statusBarsPadding
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
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.andriod_app.ui.components.ChartSeries
import com.example.andriod_app.ui.components.LineChart
import com.example.andriod_app.ui.theme.*
import com.example.andriod_app.viewmodel.BleViewModel

@Composable
fun BioFeedbackScreen(
    bleVm: BleViewModel,
    onBack: () -> Unit
)
{
    var paused by remember { mutableStateOf(false) }
    var leftLeg by remember { mutableStateOf(true) }
    var leftIdx by remember { mutableIntStateOf(7) }
    var rightIdx by remember { mutableIntStateOf(5) }
    var target by remember { mutableStateOf<Double?>(null) }
    var targetsReached by remember { mutableIntStateOf(0) }
    var prevAbove by remember { mutableStateOf(false) }

    val rt by bleVm.ble.rtData.collectAsStateWithLifecycle()
    val chart by bleVm.ble.chartSnapshot.collectAsStateWithLifecycle()

    //grab live fsr off the chosen channel
    val curIdx = if(leftLeg) leftIdx else rightIdx
    val curFsr = rt.getOrNull(curIdx) ?: 0.0

    //target counter - bumps once on rising edge cross
    LaunchedEffect(curFsr, target) {
        val t = target
        if(t != null){
            val above = curFsr >= t
            if(above && !prevAbove) targetsReached++
            prevAbove = above
        }
        else { prevAbove = false }
    }

    Column(
        modifier = Modifier.fillMaxSize().background(DarkBg).navigationBarsPadding()
    ) {

        //top nav bar - statusBarsPadding so content sits below status bar but bg extends up to it
        Row(verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth().background(CardBg).statusBarsPadding()
                .padding(horizontal = 16.dp, vertical = 14.dp)) {
            Row(modifier = Modifier.clickable { onBack() },
                verticalAlignment = Alignment.CenterVertically){
                Icon(Icons.Default.KeyboardArrowLeft, contentDescription = null, tint = ExoBlue)
                Text("Trial", color = ExoBlue)
            }
            Spacer(Modifier.weight(1f))
            Text("Biofeedback", color = Color.White, fontWeight = FontWeight.SemiBold)
            Spacer(Modifier.weight(1f))
            Button(onClick = {
                    paused = !paused
                    if(paused) bleVm.motorsOff() else bleVm.motorsOn()
                },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
                contentPadding = PaddingValues(horizontal = 12.dp, vertical = 6.dp)
            ){
                Text(if(paused) "Resume" else "Pause", fontSize = 13.sp)
            }
        }

        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState()).padding(16.dp)
        ){
            //big fsr readout + targets reached counter
            Row(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(16.dp)) {
                Column(Modifier.weight(1f)) {
                    Text(if(leftLeg) "Left Leg FSR" else "Right Leg FSR", color = GrayText, fontSize = 12.sp)
                    Text(String.format("%.3f", curFsr), fontSize = 36.sp, fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace, color = Color.White)
                }
                Column(horizontalAlignment = Alignment.End) {
                    Text("Targets Reached", color = GrayText, fontSize = 12.sp)
                    Text("$targetsReached", fontSize = 32.sp, fontWeight = FontWeight.Bold,
                        fontFamily = FontFamily.Monospace, color = ExoGreen)
                }
            }

            Spacer(Modifier.height(16.dp))

            //--- AI ASSISTED ---
            //real-time fsr chart (charting was an AI-assisted task per part 1 docs).
            //grabs the channel snapshot for whichever fsr index the user picked.
            val ch = chart.getOrNull(curIdx.coerceIn(0, 7)) ?: DoubleArray(0)
            LineChart(
                series = listOf(
                    ChartSeries(ch, ExoBlue, "FSR ch$curIdx")
                ),
                height = 180.dp
            )

            Spacer(Modifier.height(16.dp))

            //target row - tints red when a target is set
            Row(verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(10.dp))
                    .background(if(target != null) ExoRed.copy(alpha = 0.1f) else CardBg).padding(12.dp)) {
                Icon(Icons.Default.GpsFixed, contentDescription = null,
                    tint = if(target != null) ExoRed else GrayText)
                Spacer(Modifier.width(8.dp))
                Text(
                    if(target != null) String.format("Target: %.3f", target) else "No target set",
                    color = if(target != null) ExoRed else GrayText,
                    fontWeight = FontWeight.SemiBold,
                    fontFamily = FontFamily.Monospace, modifier = Modifier.weight(1f))
                if(target != null){
                    Text("Clear", color = ExoRed, fontSize = 13.sp,
                        modifier = Modifier.clickable {
                            target = null
                            targetsReached = 0
                        })
                }
            }

            Spacer(Modifier.height(12.dp))

            //leg toggle + set target btn
            Row(horizontalArrangement = Arrangement.spacedBy(10.dp)) {
                Button(onClick = { leftLeg = !leftLeg; targetsReached = 0 },
                    colors = ButtonDefaults.buttonColors(containerColor = ExoBlue.copy(alpha = 0.3f)),
                    modifier = Modifier.weight(1f)) {
                    Text(if(leftLeg) "Left Leg" else "Right Leg")
                }
                Button(onClick = {
                        //snap target to current fsr val
                        target = curFsr
                        targetsReached = 0
                    },
                    colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                    modifier = Modifier.weight(1f)) {
                    Text("Set Target")
                }
            }

            Spacer(Modifier.height(16.dp))

            //fsr index config (which channel is L vs R)
            Column(modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(10.dp)).background(CardBg).padding(12.dp)) {

                Row(verticalAlignment = Alignment.CenterVertically){
                    Text("Left FSR Index", color = GrayText, fontSize = 12.sp)
                    Spacer(Modifier.weight(1f))
                    IconButton(onClick = { if(leftIdx > 0) leftIdx-- }) {
                        Icon(Icons.Default.Remove, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                    Text("$leftIdx", color = Color.White,
                        fontFamily = FontFamily.Monospace, fontWeight = FontWeight.SemiBold)
                    IconButton(onClick = { if(leftIdx < 15) leftIdx++ }) {
                        Icon(Icons.Default.Add, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                }
                Spacer(Modifier.height(4.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
                    Text("Right FSR Index", color = GrayText, fontSize = 12.sp)
                    Spacer(Modifier.weight(1f))
                    IconButton(onClick = { if(rightIdx > 0) rightIdx-- }){
                        Icon(Icons.Default.Remove, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                    Text("$rightIdx", color = Color.White,
                        fontFamily = FontFamily.Monospace, fontWeight = FontWeight.SemiBold)
                    IconButton(onClick = { if(rightIdx < 15) rightIdx++ }) {
                        Icon(Icons.Default.Add, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(18.dp))
                    }
                }
            }
        }
    }
}

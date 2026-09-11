package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.layout.navigationBarsPadding
import androidx.compose.foundation.layout.statusBarsPadding
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
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
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.andriod_app.ui.components.ChartSeries
import com.example.andriod_app.ui.components.LineChart
import com.example.andriod_app.ui.theme.*
import com.example.andriod_app.viewmodel.BleViewModel
import com.example.andriod_app.viewmodel.TrialViewModel

@Composable
fun ActiveTrialScreen(
    bleVm: BleViewModel,
    trial: TrialViewModel,
    onSettings: () -> Unit,
    onBioFeedback: () -> Unit,
    onEndTrial: () -> Unit
)
{
    var paused by remember { mutableStateOf(false) }
    var showEndDialog by remember { mutableStateOf(false) }
    var showAltBlock by remember { mutableStateOf(false) }  //false=ch[0-3], true=ch[4-7]
    var showPrefixSheet by remember { mutableStateOf(false) }

    val markCount by bleVm.ble.markCount.collectAsStateWithLifecycle()
    val battV by bleVm.ble.batteryVolts.collectAsStateWithLifecycle()
    val chart by bleVm.ble.chartSnapshot.collectAsStateWithLifecycle()
    val pktCount by bleVm.ble.packetCount.collectAsStateWithLifecycle()
    val paramNames by bleVm.ble.paramNames.collectAsStateWithLifecycle()
    val isConnected by bleVm.ble.isConnected.collectAsStateWithLifecycle()
    val csvName by trial.csv.currentFileName.collectAsStateWithLifecycle()
    val csvPrefix by trial.csvPrefix.collectAsStateWithLifecycle()

    //chart timer is managed inside BleManager (auto-starts on connect / stops on disconnect)
    //so here we just need to wire csv loging
    LaunchedEffect(Unit) {
        trial.bind(bleVm.ble)
        trial.startCsv(paramNames)
    }

    //prefix sheet dialog
    if(showPrefixSheet) {
        PrefixSheet(
            initial = csvPrefix,
            onCancel = { showPrefixSheet = false },
            onSave = { newPrefix ->
                trial.setCsvPrefix(newPrefix)
                trial.rolloverCsv() //roll right into a fresh csv w/ the new prefix
                showPrefixSheet = false
            }
        )
    }

    //end trial confirmation
    if(showEndDialog){
        AlertDialog(
            onDismissRequest = { showEndDialog = false },
            title = { Text("End Trial?") },
            text = { Text("This will stop motors, disconnect from the device, and save the CSV log.") },
            confirmButton = {
                TextButton(onClick = {
                    showEndDialog = false
                    bleVm.endTrial()
                    trial.stopCsv()
                    onEndTrial()
                }){ Text("End Trial", color = ExoRed) }
            },
            dismissButton = {
                TextButton(onClick = { showEndDialog = false }) { Text("Cancel") }
            }
        )
    }

    Column(
        //navigationBarsPadding pushes the bottom of the col above the gesture bar
        modifier = Modifier.fillMaxSize().background(DarkBg).navigationBarsPadding()
    ){

        //compact header - battery, data status, pause, end
        //note: statusBarsPadding goes AFTER background so the CardBg fill extends
        //behind the status bar but the inner content stays below it
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth().background(CardBg).statusBarsPadding()
                .padding(horizontal = 16.dp, vertical = 10.dp)
        ) {
            //battery
            val battColor = if((battV ?: 12.0) > 11.0) ExoGreen else ExoRed
            Icon(Icons.Default.BatteryFull, contentDescription = null,
                tint = battColor, modifier = Modifier.size(18.dp))
            Spacer(Modifier.width(4.dp))
            Text(
                if(battV != null) String.format("%.2fV", battV) else "--V",
                color = battColor,
                fontSize = 14.sp, fontFamily = FontFamily.Monospace)

            Spacer(Modifier.weight(1f))

            //data status badge - dot turns green once packets start coming in
            Box(modifier = Modifier.size(8.dp)
                .clip(RoundedCornerShape(50))
                .background(if(pktCount > 0) ExoGreen else Color(0xFFE0A030)))
            Spacer(Modifier.width(5.dp))
            Text(
                if(pktCount > 0) "$pktCount pkts" else "No data",
                color = GrayText, fontSize = 11.sp, fontFamily = FontFamily.Monospace
            )

            Spacer(Modifier.weight(1f))

            //pause / resume toggles motors off+on
            Button(onClick = {
                    paused = !paused
                    if(paused) bleVm.motorsOff() else bleVm.motorsOn()
                },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
                contentPadding = PaddingValues(horizontal = 16.dp, vertical = 6.dp)
            ) {
                Text(if(paused) "Resume" else "Pause", fontSize = 13.sp)
            }

            Spacer(Modifier.width(8.dp))

            Button(onClick = { showEndDialog = true },
                shape = RoundedCornerShape(50),
                colors = ButtonDefaults.buttonColors(containerColor = ExoRed),
                contentPadding = PaddingValues(horizontal = 14.dp, vertical = 6.dp)
            ){
                Text("End", fontSize = 13.sp, fontWeight = FontWeight.Bold)
            }
        }

        //--- AI ASSISTED ---
        //real-time charting block (per part 1 docs - charting is an AI-assisted task).
        //pulls chartSnapshot out of BleManager (8 channels) and feeds 4 of them into
        //two LineCharts. showAltBlock toggle picks ch[0-3] vs ch[4-7].
        val offset = if(showAltBlock) 4 else 0
        val ch0 = chart.getOrNull(offset) ?: DoubleArray(0)
        val ch1 = chart.getOrNull(offset + 1) ?: DoubleArray(0)
        val ch2 = chart.getOrNull(offset + 2) ?: DoubleArray(0)
        val ch3 = chart.getOrNull(offset + 3) ?: DoubleArray(0)
        val lab0 = paramNames.getOrNull(offset) ?: "ch$offset"
        val lab1 = paramNames.getOrNull(offset + 1) ?: "ch${offset+1}"
        val lab2 = paramNames.getOrNull(offset + 2) ?: "ch${offset+2}"
        val lab3 = paramNames.getOrNull(offset + 3) ?: "ch${offset+3}"

        Column(modifier = Modifier.fillMaxWidth().padding(12.dp)) {
            LineChart(
                series = listOf(
                    ChartSeries(ch0, ExoBlue, lab0),
                    ChartSeries(ch1, ExoRed, lab1)
                ),
                height = 130.dp
            )
            Spacer(Modifier.height(8.dp))
            LineChart(
                series = listOf(
                    ChartSeries(ch2, ExoGreen, lab2),
                    ChartSeries(ch3, Color(0xFFA060D0), lab3)
                ),
                height = 130.dp
            )
        }

        //scrollable controls list (matches ios layout)
        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp)
        ) {
            //title + cur filename
            Row(verticalAlignment = Alignment.CenterVertically) {
                Icon(Icons.Default.DirectionsWalk, contentDescription = null,
                    tint = ExoBlue, modifier = Modifier.size(22.dp))
                Spacer(Modifier.width(6.dp))
                Text("Active Trial", fontWeight = FontWeight.Bold, fontSize = 20.sp, color = Color.White)
            }
            if(csvName.isNotEmpty()) {
                Text(csvName, color = GrayText, fontSize = 11.sp, fontFamily = FontFamily.Monospace)
            }
            Spacer(Modifier.height(10.dp))

            //CONTROLLER section
            SectionLabel("CONTROLLER")
            Spacer(Modifier.height(4.dp))
            TrialBtn("Update Controller", Icons.Default.Tune, onSettings)
            Spacer(Modifier.height(6.dp))
            TrialBtn("Mark Trial ($markCount)", Icons.Default.Flag){ bleVm.markTrial() }

            Spacer(Modifier.height(12.dp))

            //DATA section
            SectionLabel("DATA")
            Spacer(Modifier.height(4.dp))
            TrialBtn(
                if(showAltBlock) "Toggle plotting channels (alt)" else "Toggle plotting channels",
                Icons.Default.ShowChart
            ){ showAltBlock = !showAltBlock }
            Spacer(Modifier.height(6.dp))
            TrialBtn("Set CSV Prefix", Icons.Default.Edit) { showPrefixSheet = true }
            Spacer(Modifier.height(6.dp))
            TrialBtn("Save New CSV", Icons.Default.NoteAdd) { trial.rolloverCsv() }

            Spacer(Modifier.height(12.dp))

            //ADVANCED section
            SectionLabel("ADVANCED")
            Spacer(Modifier.height(4.dp))
            TrialBtn("Bio Feedback", Icons.Default.MonitorHeart, onBioFeedback)
            Spacer(Modifier.height(6.dp))
            TrialBtn("Recalibrate FSRs", Icons.Default.Refresh){ bleVm.calibrateFsr() }
            Spacer(Modifier.height(6.dp))
            TrialBtn("Send Preset FSR", Icons.Default.Speed) {
                bleVm.sendFsrThresholds(0.25, 0.25)
            }
            Spacer(Modifier.height(6.dp))
            TrialBtn("Recalibrate Torque", Icons.Default.Build) { bleVm.calibrateTorque() }

            Spacer(Modifier.height(20.dp))
        }
    }
}

@Composable
private fun SectionLabel(text: String) {
    Text(text, color = GrayText, fontSize = 11.sp, fontWeight = FontWeight.SemiBold)
}

//generic btn used for all the trial control rows
@Composable
fun TrialBtn(text: String, icon: ImageVector, onClick: () -> Unit){
    Button(
        onClick = onClick,
        shape = RoundedCornerShape(10.dp),
        colors = ButtonDefaults.buttonColors(containerColor = CardBg),
        modifier = Modifier.fillMaxWidth()
    ){
        Icon(icon, contentDescription = null, tint = ExoBlue, modifier = Modifier.size(16.dp))
        Spacer(Modifier.width(10.dp))
        Text(text, color = Color.White, modifier = Modifier.weight(1f), fontSize = 14.sp)
        Icon(Icons.Default.KeyboardArrowRight, contentDescription = null,
            tint = GrayText, modifier = Modifier.size(16.dp))
    }
}

//sheet for editing the csv prefix - mirrors the iOS prefix sheet
@Composable
private fun PrefixSheet(initial: String, onCancel: () -> Unit, onSave: (String) -> Unit)
{
    var txt by remember { mutableStateOf(initial) }

    AlertDialog(
        onDismissRequest = onCancel,
        title = { Text("Set CSV Prefix") },
        text = {
            Column {
                OutlinedTextField(
                    value = txt,
                    onValueChange = { txt = it },
                    placeholder = { Text("e.g. subject01", color = GrayText) },
                    singleLine = true,
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Ascii),
                    textStyle = LocalTextStyle.current.copy(
                        color = Color.White, fontFamily = FontFamily.Monospace),
                    modifier = Modifier.fillMaxWidth()
                )
                Spacer(Modifier.height(8.dp))
                Text(
                    "File: ${txt.ifEmpty{ "(no prefix)" }}_trial_YYYYMMDD_HHMMSS.csv",
                    color = GrayText, fontSize = 11.sp, fontFamily = FontFamily.Monospace
                )
            }
        },
        confirmButton = {
            TextButton(onClick = {
                //sanitize same way ios does (letters, digits, _, -)
                val sanitized = txt.filter { it.isLetterOrDigit() || it == '_' || it == '-' }
                onSave(sanitized)
            }) { Text("Save", color = ExoBlue) }
        },
        dismissButton = {
            TextButton(onClick = onCancel) { Text("Cancel") }
        }
    )
}

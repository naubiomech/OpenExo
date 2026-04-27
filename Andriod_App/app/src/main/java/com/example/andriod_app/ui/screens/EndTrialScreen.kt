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
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.*

@Composable
fun EndTrialScreen(onReturn: () -> Unit){
    var fileName by remember { mutableStateOf("mock_trial_20260407_120000") }
    var notes by remember { mutableStateOf("") }
    var showDiscard by remember { mutableStateOf(false) }

    if (showDiscard) {
        AlertDialog(
            onDismissRequest = { showDiscard = false },
            title = { Text("Discard Data?") },
            text = { Text("This will permanently delete the CSV file for this trial.") },
            confirmButton = {
                TextButton(onClick = { showDiscard = false; onReturn() }){
                    Text("Discard", color = ExoRed) }
            },
            dismissButton = {
                TextButton(onClick = { showDiscard = false }) { Text("Keep") }
            }
        )
    }

    Column(modifier = Modifier.fillMaxSize().background(DarkBg)){

        //header
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier.fillMaxWidth().background(CardBg).padding(vertical = 24.dp)) {
            Icon(Icons.Default.CheckCircle, contentDescription = null,
                tint = ExoGreen, modifier = Modifier.size(48.dp))
            Spacer(Modifier.height(8.dp))
            Text("Trial Complete", fontSize = 22.sp, fontWeight = FontWeight.Bold, color = Color.White)
            Text("Review and save your data", color = GrayText, fontSize = 14.sp)
        }

        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState()).padding(16.dp)
        ){
            //file detials
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(14.dp)){
                Text("FILE DETAILS", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(10.dp))
                FileInfoRow("File", "mock_trial_20260407.csv")
                FileInfoRow("Size", "245 KB")
                FileInfoRow("Data Points", "12,847")
                FileInfoRow("Marks", "5")
                FileInfoRow("Duration", "04:32")
            }

            Spacer(Modifier.height(16.dp))

            //rename
            Column(modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(12.dp)).background(CardBg).padding(14.dp)){
                Text("RENAME FILE", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(8.dp))
                Row(verticalAlignment = Alignment.CenterVertically){
                    OutlinedTextField(
                        value = fileName, onValueChange = { fileName = it },
                        textStyle = LocalTextStyle.current.copy(
                            color = Color.White, fontFamily = FontFamily.Monospace, fontSize = 13.sp),
                        modifier = Modifier.weight(1f), singleLine = true)
                    Spacer(Modifier.width(6.dp))
                    Text(".csv", color = GrayText, fontFamily = FontFamily.Monospace)
                }
            }

            Spacer(Modifier.height(16.dp))

            //notes
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(10.dp))
                .background(CardBg).padding(14.dp)){
                Text("TRIAL NOTES", color = GrayText, fontSize = 12.sp)
                Spacer(Modifier.height(8.dp))
                OutlinedTextField(
                    value = notes,
                    onValueChange = { notes = it },
                    placeholder = { Text("Add notes...", color = GrayText) },
                    textStyle = LocalTextStyle.current.copy(color = Color.White),
                    modifier = Modifier.fillMaxWidth().heightIn(min = 80.dp),
                    minLines = 3)
            }

            Spacer(Modifier.height(12.dp))

            //share
            Button(onClick = {},
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.fillMaxWidth()){
                Icon(Icons.Default.Share, contentDescription = null, modifier = Modifier.size(16.dp), tint = ExoBlue)
                Spacer(Modifier.width(8.dp))
                Text("Share / Export CSV", color = Color.White)
            }
        }

        //bottom
        HorizontalDivider(color = GrayText.copy(alpha = 0.3f))
        Column(modifier = Modifier.padding(16.dp)){
            Button(onClick = onReturn, shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(containerColor = ExoGreen),
                modifier = Modifier.fillMaxWidth()){
                Text("Save & Return", fontWeight = FontWeight.Bold)
            }
            Spacer(Modifier.height(10.dp))
            OutlinedButton(onClick = { showDiscard = true },
                shape = RoundedCornerShape(12.dp),
                modifier = Modifier.fillMaxWidth()) {
                Text("Discard Data", color = ExoRed)
            }
        }
    }
}

@Composable
private fun FileInfoRow(label: String, value: String){
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 3.dp)){
        Text(label, color = GrayText, fontSize = 14.sp, modifier = Modifier.weight(1f))
        Text(value, color = Color.White, fontSize = 14.sp, fontFamily = FontFamily.Monospace)
    }
}

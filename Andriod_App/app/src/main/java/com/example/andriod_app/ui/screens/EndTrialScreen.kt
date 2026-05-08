package com.example.andriod_app.ui.screens

import android.content.Intent
import androidx.compose.foundation.background
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
import androidx.compose.ui.platform.LocalContext
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.content.FileProvider
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.andriod_app.ui.theme.*
import com.example.andriod_app.viewmodel.AuthViewModel
import com.example.andriod_app.viewmodel.BleViewModel
import com.example.andriod_app.viewmodel.TrialViewModel

@Composable
fun EndTrialScreen(
    bleVm: BleViewModel,
    trial: TrialViewModel,
    auth: AuthViewModel,
    onReturn: () -> Unit
)
{
    val ctx = LocalContext.current
    val csvName by trial.csv.currentFileName.collectAsStateWithLifecycle()
    val rowCount by trial.csv.rowCount.collectAsStateWithLifecycle()
    val saveStatus by trial.saveStatus.collectAsStateWithLifecycle()
    val markCount by bleVm.ble.markCount.collectAsStateWithLifecycle()
    val user by auth.user.collectAsStateWithLifecycle()

    var fileName by remember(csvName) { mutableStateOf(csvName.removeSuffix(".csv")) }
    var notes by remember { mutableStateOf("") }
    var showDiscard by remember { mutableStateOf(false) }

    val durStr = remember(rowCount) { trial.csv.durationFormatted() }
    val sizeStr = remember(rowCount) { trial.csv.lastFileSize() }

    //confirm dialog before discarding
    if(showDiscard){
        AlertDialog(
            onDismissRequest = { showDiscard = false },
            title = { Text("Discard Data?") },
            text = { Text("This will permanently delete the CSV file for this trial.") },
            confirmButton = {
                TextButton(onClick = {
                    showDiscard = false
                    trial.discardTrial()
                    onReturn()
                }) { Text("Discard", color = ExoRed) }
            },
            dismissButton = {
                TextButton(onClick = { showDiscard = false }){ Text("Keep") }
            }
        )
    }

    Column(
        modifier = Modifier.fillMaxSize().background(DarkBg).navigationBarsPadding()
    ) {

        //header - colored bg extends behind status bar, content gets statusBarsPadding
        Column(
            horizontalAlignment = Alignment.CenterHorizontally,
            modifier = Modifier.fillMaxWidth().background(CardBg).statusBarsPadding()
                .padding(vertical = 24.dp)
        ){
            Icon(Icons.Default.CheckCircle, contentDescription = null,
                tint = ExoGreen, modifier = Modifier.size(48.dp))
            Spacer(Modifier.height(8.dp))
            Text("Trial Complete", fontSize = 22.sp, fontWeight = FontWeight.Bold, color = Color.White)
            Text("Review and save your data", color = GrayText, fontSize = 14.sp)
        }

        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState()).padding(16.dp)
        ) {
            //file detials card
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(14.dp)) {
                Text("FILE DETAILS", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(10.dp))
                FileInfoRow("File", csvName.ifEmpty{ "(none)" })
                FileInfoRow("Size", sizeStr)
                FileInfoRow("Data Points", rowCount.toString())
                FileInfoRow("Marks", markCount.toString())
                FileInfoRow("Duration", durStr)
            }

            Spacer(Modifier.height(16.dp))

            //rename feild
            Column(modifier = Modifier.fillMaxWidth()
                .clip(RoundedCornerShape(12.dp)).background(CardBg).padding(14.dp)){
                Text("RENAME FILE", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(8.dp))
                Row(verticalAlignment = Alignment.CenterVertically) {
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

            //notes box
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(10.dp))
                .background(CardBg).padding(14.dp)) {
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

            //--- AI ASSISTED ---
            //share / export csv via FileProvider intent (csv export = AI-assisted task
            //per part 1 docs). AI wrote the FileProvider uri grant + ACTION_SEND chooser.
            Button(onClick = {
                    val f = trial.csv.lastFile() ?: return@Button
                    try {
                        val uri = FileProvider.getUriForFile(
                            ctx, "${ctx.packageName}.fileprovider", f)
                        val intent = Intent(Intent.ACTION_SEND).apply{
                            type = "text/csv"
                            putExtra(Intent.EXTRA_STREAM, uri)
                            addFlags(Intent.FLAG_GRANT_READ_URI_PERMISSION)
                        }
                        ctx.startActivity(Intent.createChooser(intent, "Share trial CSV"))
                    } catch(e: Exception) {
                        //fileprovider not configured - fall back to sending text
                        val intent = Intent(Intent.ACTION_SEND).apply{
                            type = "text/plain"
                            putExtra(Intent.EXTRA_TEXT, "Trial CSV: ${f.absolutePath}")
                        }
                        ctx.startActivity(Intent.createChooser(intent, "Share trial info"))
                    }
                },
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(containerColor = CardBg),
                modifier = Modifier.fillMaxWidth()){
                Icon(Icons.Default.Share, contentDescription = null, modifier = Modifier.size(16.dp), tint = ExoBlue)
                Spacer(Modifier.width(8.dp))
                Text("Share / Export CSV", color = Color.White)
            }

            //cloud save status txt
            if(saveStatus != null) {
                Spacer(Modifier.height(8.dp))
                Text(saveStatus!!, color = GrayText, fontSize = 12.sp,
                    modifier = Modifier.padding(horizontal = 4.dp))
            }
        }

        //bottom save / discard buttons
        HorizontalDivider(color = GrayText.copy(alpha = 0.3f))
        Column(modifier = Modifier.padding(16.dp)) {
            Button(onClick = {
                    //rename if user changed it, then push to firebase
                    if(fileName.isNotBlank() && fileName != csvName.removeSuffix(".csv")){
                        trial.csv.renameLastFile(fileName)
                    }
                    trial.saveTrialToFirebase(user, notes, markCount)
                    onReturn()
                },
                shape = RoundedCornerShape(12.dp),
                colors = ButtonDefaults.buttonColors(containerColor = ExoGreen),
                modifier = Modifier.fillMaxWidth()
            ){
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

//small helper for the file details rows
@Composable
private fun FileInfoRow(label: String, value: String) {
    Row(modifier = Modifier.fillMaxWidth().padding(vertical = 3.dp)) {
        Text(label, color = GrayText, fontSize = 14.sp, modifier = Modifier.weight(1f))
        Text(value, color = Color.White, fontSize = 14.sp, fontFamily = FontFamily.Monospace)
    }
}

package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.clickable
import androidx.compose.foundation.layout.*
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
import androidx.compose.ui.text.font.FontFamily
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.*

@Composable
fun SettingsScreen(onBack: () -> Unit)
{
    var bilateral by remember { mutableStateOf(false) }
    var selectedJoint by remember { mutableIntStateOf(0) }
    var paramVal by remember { mutableStateOf("0.0") }
    var showAdvanced by remember { mutableStateOf(true) }
    var applied by remember { mutableStateOf(false) }

    val jointOptions = listOf("Left Ankle (68)", "Right Ankle (36)", "Left Hip (65)")
    val controllerOpts = listOf("Proportional Joint Moment", "Zhang Collins", "Zero Torque")
    var expandedJoint by remember { mutableStateOf(false) }

    Column(modifier = Modifier.fillMaxSize().background(DarkBg)) {

        //top nav
        Row(
            verticalAlignment = Alignment.CenterVertically,
            modifier = Modifier.fillMaxWidth().background(CardBg)
                .padding(horizontal = 16.dp, vertical = 14.dp)
        ){
            Row(modifier = Modifier.clickable { onBack() },
                verticalAlignment = Alignment.CenterVertically){
                Icon(Icons.Default.KeyboardArrowLeft, contentDescription = null, tint = ExoBlue)
                Text("Trial", color = ExoBlue, fontSize = 15.sp)
            }
            Spacer(Modifier.weight(1f))
            Text("Controller Settings", color = Color.White,
                fontWeight = FontWeight.SemiBold, fontSize = 17.sp)
            Spacer(Modifier.weight(1f))
            Spacer(Modifier.width(60.dp)) //balances the back btn
        }

        //advanced/basic picker
        Row(modifier = Modifier.fillMaxWidth().padding(16.dp)
            .clip(RoundedCornerShape(8.dp)).background(CardBg)){
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier.weight(1f)
                    .background(if(showAdvanced) ExoBlue else CardBg)
                    .clickable { showAdvanced = true }.padding(vertical = 10.dp)
            ){ Text("Advanced", color = Color.White, fontSize = 14.sp) }
            Box(
                contentAlignment = Alignment.Center,
                modifier = Modifier.weight(1f)
                    .background(if(!showAdvanced) ExoBlue else CardBg)
                    .clickable { showAdvanced = false }.padding(vertical = 10.dp)
            ){ Text("Basic", color = Color.White, fontSize = 14.sp) }
        }

        //form
        Column(
            modifier = Modifier.weight(1f).verticalScroll(rememberScrollState())
                .padding(horizontal = 16.dp)
        ) {
            //bilateral switch
            Row(
                verticalAlignment = Alignment.CenterVertically,
                modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                    .background(CardBg).padding(14.dp)
            ){
                Column(Modifier.weight(1f)) {
                    Text("Bilateral Mode", color = Color.White, fontWeight = FontWeight.Medium)
                    Text("Mirror to opposite joint", color = GrayText, fontSize = 12.sp)
                }
                Switch(checked = bilateral, onCheckedChange = { bilateral = it },
                    colors = SwitchDefaults.colors(checkedTrackColor = ExoBlue))
            }

            Spacer(Modifier.height(12.dp))

            //joint dropdown
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(10.dp))
                .background(CardBg).padding(14.dp)) {
                Text("JOINT", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(8.dp))
                Box {
                    Row(
                        modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(8.dp))
                            .background(DarkBg).clickable { expandedJoint = true }.padding(12.dp),
                        verticalAlignment = Alignment.CenterVertically
                    ) {
                        Text(jointOptions[selectedJoint], color = ExoBlue, modifier = Modifier.weight(1f))
                        Icon(Icons.Default.ArrowDropDown, contentDescription = null, tint = ExoBlue)
                    }
                    DropdownMenu(expanded = expandedJoint, onDismissRequest = { expandedJoint = false }){
                        jointOptions.forEachIndexed { i, opt ->
                            DropdownMenuItem(text = { Text(opt) },
                                onClick = { selectedJoint = i; expandedJoint = false })
                        }
                    }
                }
            }

            Spacer(Modifier.height(12.dp))

            //controller radio btns
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(14.dp)){
                Text("CONTROLLER", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(8.dp))
                controllerOpts.forEachIndexed { i, name ->
                    Row(verticalAlignment = Alignment.CenterVertically,
                        modifier = Modifier.fillMaxWidth().padding(vertical = 6.dp)){
                        RadioButton(selected = i == 0, onClick = {})
                        Spacer(Modifier.width(8.dp))
                        Text(name, color = Color.White, fontSize = 14.sp)
                    }
                }
            }

            Spacer(Modifier.height(12.dp))

            //value input feild
            Column(modifier = Modifier.fillMaxWidth().clip(RoundedCornerShape(12.dp))
                .background(CardBg).padding(14.dp)){
                Text("VALUE", color = GrayText, fontSize = 12.sp, fontWeight = FontWeight.SemiBold)
                Spacer(Modifier.height(8.dp))
                OutlinedTextField(
                    value = paramVal,
                    onValueChange = { paramVal = it },
                    keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Decimal),
                    textStyle = LocalTextStyle.current.copy(
                        color = Color.White, fontFamily = FontFamily.Monospace, fontSize = 18.sp),
                    modifier = Modifier.fillMaxWidth()
                )
            }

            Spacer(Modifier.height(16.dp))
        }

        HorizontalDivider(color = GrayText.copy(alpha = 0.3f))

        if(applied) {
            Row(Modifier.fillMaxWidth().padding(8.dp), horizontalArrangement = Arrangement.Center){
                Icon(Icons.Default.CheckCircle, contentDescription = null, tint = ExoGreen,
                    modifier = Modifier.size(16.dp))
                Spacer(Modifier.width(6.dp))
                Text("Parameter sent", color = ExoGreen, fontSize = 14.sp)
            }
        }

        //cancel and apply
        Row(modifier = Modifier.fillMaxWidth().padding(16.dp),
            horizontalArrangement = Arrangement.spacedBy(12.dp)) {
            OutlinedButton(onClick = onBack, modifier = Modifier.weight(1f)){
                Text("Cancel")
            }
            Button(onClick = { applied = true },
                colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
                modifier = Modifier.weight(1f)){
                Text("Apply", fontWeight = FontWeight.Bold)
            }
        }
    }
}

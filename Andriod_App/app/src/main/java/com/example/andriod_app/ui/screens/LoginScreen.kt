package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
import androidx.compose.foundation.layout.systemBarsPadding
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.foundation.text.KeyboardOptions
import androidx.compose.material.icons.Icons
import androidx.compose.material.icons.filled.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.text.font.FontWeight
import androidx.compose.ui.text.input.KeyboardType
import androidx.compose.ui.text.input.PasswordVisualTransformation
import androidx.compose.ui.text.input.VisualTransformation
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.lifecycle.compose.collectAsStateWithLifecycle
import com.example.andriod_app.ui.theme.*
import com.example.andriod_app.viewmodel.AuthViewModel

@Composable
fun LoginScreen(
    auth: AuthViewModel,
    onLogin: () -> Unit
)
{
    var email by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }
    var showPassword by remember { mutableStateOf(false) }
    var isSignup by remember { mutableStateOf(false) }

    val errMsg by auth.error.collectAsStateWithLifecycle()
    val busy by auth.busy.collectAsStateWithLifecycle()

    //pulled out the field colors so i dont have to repeat them on both feilds
    val fieldColors = OutlinedTextFieldDefaults.colors(
        focusedBorderColor = ExoBlue,
        unfocusedBorderColor = GrayText.copy(alpha = 0.5f),
        focusedTextColor = Color.White,
        unfocusedTextColor = Color.White,
        cursorColor = ExoBlue,
        focusedLabelColor = ExoBlue,
        unfocusedLabelColor = GrayText
    )

    Column(
        //systemBarsPadding keeps content clear of the status bar + gesture bar
        modifier = Modifier.fillMaxSize().background(DarkBg).systemBarsPadding().padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ) {
        Icon(Icons.Default.DirectionsWalk, contentDescription = null,
            tint = ExoBlue, modifier = Modifier.size(64.dp))
        Spacer(Modifier.height(8.dp))
        Text("OpenExo", fontSize = 32.sp, fontWeight = FontWeight.Bold, color = Color.White)
        Text("Exoskeleton Controller", color = GrayText, fontSize = 14.sp)

        Spacer(Modifier.height(40.dp))

        //email feild
        OutlinedTextField(
            value = email,
            onValueChange = { email = it; auth.clearError() },
            label = { Text("Email") },
            singleLine = true,
            enabled = !busy,
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Email),
            leadingIcon = { Icon(Icons.Default.Email, contentDescription = null, tint = GrayText) },
            colors = fieldColors,
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(Modifier.height(12.dp))

        //password feild w/ show toggle
        OutlinedTextField(
            value = password,
            onValueChange = { password = it; auth.clearError() },
            label = { Text("Password") },
            singleLine = true,
            enabled = !busy,
            visualTransformation = if(showPassword) VisualTransformation.None
                else PasswordVisualTransformation(),
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
            leadingIcon = { Icon(Icons.Default.Lock, contentDescription = null, tint = GrayText) },
            trailingIcon = {
                IconButton(onClick = { showPassword = !showPassword }) {
                    Icon(
                        if(showPassword) Icons.Default.Visibility else Icons.Default.VisibilityOff,
                        contentDescription = null, tint = GrayText)
                }
            },
            colors = fieldColors,
            modifier = Modifier.fillMaxWidth()
        )

        //err msg
        if(errMsg != null) {
            Spacer(Modifier.height(8.dp))
            Text(errMsg!!, color = ExoRed, fontSize = 13.sp)
        }

        Spacer(Modifier.height(24.dp))

        //login or signup btn (depending on isSignup toggle)
        Button(
            onClick = {
                if(isSignup){
                    auth.signUp(email, password){ ok -> if(ok) onLogin() }
                } else {
                    auth.signIn(email, password) { ok -> if(ok) onLogin() }
                }
            },
            enabled = !busy,
            colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
            shape = RoundedCornerShape(10.dp),
            modifier = Modifier.fillMaxWidth().height(50.dp)
        ) {
            if(busy) {
                CircularProgressIndicator(Modifier.size(18.dp), color = Color.White, strokeWidth = 2.dp)
            } else {
                Text(if(isSignup) "Create Account" else "Log In",
                    fontSize = 16.sp, fontWeight = FontWeight.Bold)
            }
        }

        Spacer(Modifier.height(12.dp))

        //skip auth - only for offline testing
        TextButton(onClick = { onLogin() }, enabled = !busy) {
            Text("Continue without account", color = GrayText, fontSize = 12.sp)
        }

        Spacer(Modifier.height(8.dp))

        //flip between login/signup mode
        TextButton(onClick = { isSignup = !isSignup; auth.clearError() }, enabled = !busy){
            Text(
                if(isSignup) "Already have an account? Log in"
                    else "Don't have an account? Sign up",
                color = ExoBlue, fontSize = 14.sp
            )
        }
    }
}

package com.example.andriod_app.ui.screens

import androidx.compose.foundation.background
import androidx.compose.foundation.layout.*
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
import com.example.andriod_app.ui.theme.*

@Composable
fun LoginScreen(onLogin: () -> Unit){
    var email by remember { mutableStateOf("") }
    var password by remember { mutableStateOf("") }
    var showPassword by remember { mutableStateOf(false) }
    var isSignup by remember { mutableStateOf(false) }
    var errorMsg by remember { mutableStateOf<String?>(null) }

    Column(
        modifier = Modifier.fillMaxSize().background(DarkBg).padding(24.dp),
        horizontalAlignment = Alignment.CenterHorizontally,
        verticalArrangement = Arrangement.Center
    ){
        Icon(Icons.Default.DirectionsWalk, contentDescription = null,
            tint = ExoBlue, modifier = Modifier.size(64.dp))
        Spacer(Modifier.height(8.dp))
        Text("OpenExo", fontSize = 32.sp, fontWeight = FontWeight.Bold, color = Color.White)
        Text("Exoskeleton Controller", color = GrayText, fontSize = 14.sp)

        Spacer(Modifier.height(40.dp))

        //email
        OutlinedTextField(
            value = email,
            onValueChange = { email = it; errorMsg = null },
            label = { Text("Email") },
            singleLine = true,
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Email),
            leadingIcon = { Icon(Icons.Default.Email, contentDescription = null, tint = GrayText) },
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = ExoBlue,
                unfocusedBorderColor = GrayText.copy(alpha = 0.5f),
                focusedTextColor = Color.White,
                unfocusedTextColor = Color.White,
                cursorColor = ExoBlue,
                focusedLabelColor = ExoBlue,
                unfocusedLabelColor = GrayText
            ),
            modifier = Modifier.fillMaxWidth()
        )

        Spacer(Modifier.height(12.dp))

        //password
        OutlinedTextField(
            value = password,
            onValueChange = { password = it; errorMsg = null },
            label = { Text("Password") },
            singleLine = true,
            visualTransformation = if(showPassword) VisualTransformation.None
                else PasswordVisualTransformation(),
            keyboardOptions = KeyboardOptions(keyboardType = KeyboardType.Password),
            leadingIcon = { Icon(Icons.Default.Lock, contentDescription = null, tint = GrayText) },
            trailingIcon = {
                IconButton(onClick = { showPassword = !showPassword }){
                    Icon(
                        if(showPassword) Icons.Default.Visibility else Icons.Default.VisibilityOff,
                        contentDescription = null, tint = GrayText)
                }
            },
            colors = OutlinedTextFieldDefaults.colors(
                focusedBorderColor = ExoBlue,
                unfocusedBorderColor = GrayText.copy(alpha = 0.5f),
                focusedTextColor = Color.White,
                unfocusedTextColor = Color.White,
                cursorColor = ExoBlue,
                focusedLabelColor = ExoBlue,
                unfocusedLabelColor = GrayText
            ),
            modifier = Modifier.fillMaxWidth()
        )

        //error
        if(errorMsg != null){
            Spacer(Modifier.height(8.dp))
            Text(errorMsg!!, color = ExoRed, fontSize = 13.sp)
        }

        Spacer(Modifier.height(24.dp))

        //login/signup btn
        Button(
            onClick = {
                if(email.isBlank() || password.isBlank()){
                    errorMsg = "Please fill in all fields"
                } else {
                    onLogin()
                }
            },
            colors = ButtonDefaults.buttonColors(containerColor = ExoBlue),
            shape = RoundedCornerShape(10.dp),
            modifier = Modifier.fillMaxWidth().height(50.dp)
        ){
            Text(if(isSignup) "Create Account" else "Log In",
                fontSize = 16.sp, fontWeight = FontWeight.Bold)
        }

        Spacer(Modifier.height(16.dp))

        //toggle login/signup
        TextButton(onClick = { isSignup = !isSignup; errorMsg = null }){
            Text(
                if(isSignup) "Already have an account? Log in"
                    else "Don't have an account? Sign up",
                color = ExoBlue, fontSize = 14.sp
            )
        }
    }
}

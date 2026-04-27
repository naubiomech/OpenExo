package com.example.andriod_app

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import com.example.andriod_app.navigation.AppNavigation
import com.example.andriod_app.ui.theme.DarkBg
import com.example.andriod_app.ui.theme.OpenExoTheme

class MainActivity : ComponentActivity() {
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            OpenExoTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = DarkBg
                ){
                    AppNavigation()
                }
            }
        }
    }
}

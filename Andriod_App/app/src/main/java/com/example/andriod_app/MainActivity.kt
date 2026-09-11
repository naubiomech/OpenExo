package com.example.andriod_app

import android.Manifest
import android.os.Build
import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.material3.Surface
import androidx.compose.ui.Modifier
import com.example.andriod_app.navigation.AppNavigation
import com.example.andriod_app.ui.theme.DarkBg
import com.example.andriod_app.ui.theme.OpenExoTheme

class MainActivity : ComponentActivity()
{
    //request all the needed perms at launch (ble + loc, depends on api lvl)
    private val permLauncher = registerForActivityResult(
        ActivityResultContracts.RequestMultiplePermissions()
    ){ grants ->
        val denied = grants.filterValues { !it }.keys
        if(denied.isNotEmpty()) {
            //app still works for non-ble stuff (firebase) but ble wont
            android.util.Log.w("MainActivity", "denied perms: $denied")
        }
    }

    //android 12+ uses BLUETOOTH_SCAN/CONNECT, older needs FINE_LOCATION
    private fun requiredPerms(): Array<String>{
        return if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S) {
            arrayOf(
                Manifest.permission.BLUETOOTH_SCAN,
                Manifest.permission.BLUETOOTH_CONNECT
            )
        } else {
            arrayOf(Manifest.permission.ACCESS_FINE_LOCATION)
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()

        //ask for ble perms early so the scan screen works first try
        permLauncher.launch(requiredPerms())

        setContent {
            OpenExoTheme {
                Surface(
                    modifier = Modifier.fillMaxSize(),
                    color = DarkBg
                ) {
                    //insets are handled per-screen so the colored headers go all the way to
                    //the top of the display while content stays below the status bar
                    AppNavigation()
                }
            }
        }
    }
}

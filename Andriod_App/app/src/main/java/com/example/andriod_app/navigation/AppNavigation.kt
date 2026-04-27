package com.example.andriod_app.navigation

import androidx.compose.runtime.Composable
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.example.andriod_app.ui.screens.*

@Composable
fun AppNavigation() {
    val navController = rememberNavController()

    NavHost(navController = navController, startDestination = "login") {

        composable("login"){
            LoginScreen(
                onLogin = {
                    navController.navigate("scan"){
                        popUpTo("login"){ inclusive = true }
                    }
                }
            )
        }

        composable("scan"){
            ScanScreen(
                onStartTrial = { navController.navigate("trial") }
            )
        }

        composable("trial") {
            ActiveTrialScreen(
                onSettings = { navController.navigate("settings") },
                onBioFeedback = { navController.navigate("biofeedback") },
                onEndTrial = {
                    navController.navigate("endtrial"){
                        popUpTo("trial") { inclusive = true }
                    }
                }
            )
        }

        composable("settings"){
            SettingsScreen(onBack = { navController.popBackStack() })
        }

        composable("biofeedback") {
            BioFeedbackScreen(onBack = { navController.popBackStack() })
        }

        composable("endtrial"){
            EndTrialScreen(
                onReturn = {
                    navController.navigate("scan") {
                        popUpTo("scan"){ inclusive = true }
                    }
                }
            )
        }
    }
}

package com.example.andriod_app.navigation

import androidx.compose.runtime.Composable
import androidx.lifecycle.viewmodel.compose.viewModel
import androidx.navigation.compose.NavHost
import androidx.navigation.compose.composable
import androidx.navigation.compose.rememberNavController
import com.example.andriod_app.ui.screens.*
import com.example.andriod_app.viewmodel.AuthViewModel
import com.example.andriod_app.viewmodel.BleViewModel
import com.example.andriod_app.viewmodel.TrialViewModel

//main nav graph - hoists the 3 viewmodels at the top so they survive
//between screens (otherwise the ble would reconnect on every nav).
@Composable
fun AppNavigation()
{
    val navController = rememberNavController()

    val auth: AuthViewModel = viewModel()
    val bleVm: BleViewModel = viewModel()
    val trial: TrialViewModel = viewModel()

    NavHost(navController = navController, startDestination = "login") {

        composable("login") {
            LoginScreen(
                auth = auth,
                onLogin = {
                    //pop login off the back stack so back btn doesnt go back to it
                    navController.navigate("scan"){
                        popUpTo("login") { inclusive = true }
                    }
                }
            )
        }

        composable("scan"){
            ScanScreen(
                bleVm = bleVm,
                onStartTrial = { navController.navigate("trial") }
            )
        }

        composable("trial"){
            ActiveTrialScreen(
                bleVm = bleVm,
                trial = trial,
                onSettings = { navController.navigate("settings") },
                onBioFeedback = { navController.navigate("biofeedback") },
                onEndTrial = {
                    navController.navigate("endtrial") {
                        popUpTo("trial"){ inclusive = true }
                    }
                }
            )
        }

        composable("settings") {
            SettingsScreen(bleVm = bleVm, onBack = { navController.popBackStack() })
        }

        composable("biofeedback"){
            BioFeedbackScreen(bleVm = bleVm, onBack = { navController.popBackStack() })
        }

        composable("endtrial") {
            EndTrialScreen(
                bleVm = bleVm,
                trial = trial,
                auth = auth,
                onReturn = {
                    //after save/discard go back to scan and clear stack
                    navController.navigate("scan"){
                        popUpTo("scan") { inclusive = true }
                    }
                }
            )
        }
    }
}

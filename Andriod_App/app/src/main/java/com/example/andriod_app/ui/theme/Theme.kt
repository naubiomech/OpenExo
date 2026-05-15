package com.example.andriod_app.ui.theme

import androidx.compose.material3.MaterialTheme
import androidx.compose.material3.darkColorScheme
import androidx.compose.runtime.Composable

private val DarkColors = darkColorScheme(
    primary = ExoBlue,
    secondary = ExoGreen,
    background = DarkBg,
    surface = DarkBg,
    error = ExoRed,
    onPrimary = androidx.compose.ui.graphics.Color.White,
    onBackground = androidx.compose.ui.graphics.Color.White,
    onSurface = androidx.compose.ui.graphics.Color.White
)

@Composable
fun OpenExoTheme(content: @Composable () -> Unit)
{
    MaterialTheme(
        colorScheme = DarkColors,
        typography = Typography,
        content = content
    )
}

//=========================================================================
//AI ASSISTED FILE
//per Part 1 Documentation: "AI assists with...real-time data charting"
//this is the custom Compose Canvas line chart used on the Active Trial
//and Bio Feedback screens. AI handled the auto y-scale math, dashed grid
//lines, zero-baseline drawing, and the multi-series Path stroking.
//student handled placing the chart in the screens and choosing colors.
//=========================================================================
package com.example.andriod_app.ui.components

import androidx.compose.foundation.Canvas
import androidx.compose.foundation.background
import androidx.compose.foundation.layout.Box
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.fillMaxWidth
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.size
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.shape.RoundedCornerShape
import androidx.compose.material3.Text
import androidx.compose.runtime.Composable
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.draw.clip
import androidx.compose.ui.geometry.Offset
import androidx.compose.ui.graphics.Color
import androidx.compose.ui.graphics.Path
import androidx.compose.ui.graphics.PathEffect
import androidx.compose.ui.graphics.drawscope.Stroke
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import com.example.andriod_app.ui.theme.CardBg
import com.example.andriod_app.ui.theme.GrayText
import kotlin.math.max

//data series for chart - one line per channel
data class ChartSeries(
    val data: DoubleArray,
    val color: Color,
    val label: String
)

//draws N overlapping line series on a single canvas with auto y-scale.
//re-draws every time the data list changes (we get a fresh list 20x/sec from BleManager)
@Composable
fun LineChart(
    series: List<ChartSeries>,
    modifier: Modifier = Modifier,
    height: androidx.compose.ui.unit.Dp = 130.dp,
    minRange: Double = 1.0,
){
    Column(modifier = modifier
        .fillMaxWidth()
        .clip(RoundedCornerShape(10.dp))
        .background(CardBg)){

        //legend strip
        Row(modifier = Modifier.fillMaxWidth().padding(horizontal = 10.dp, vertical = 6.dp),
            verticalAlignment = Alignment.CenterVertically){
            for((i, s) in series.withIndex()){
                if(i > 0) Spacer(Modifier.width(10.dp))
                Box(Modifier.size(8.dp).background(s.color, shape = RoundedCornerShape(50)))
                Spacer(Modifier.width(4.dp))
                Text(s.label, color = GrayText, fontSize = 10.sp)
            }
        }

        Box(modifier = Modifier.fillMaxWidth().height(height = height)){
            Canvas(Modifier.fillMaxSize().padding(horizontal = 8.dp, vertical = 4.dp)){
                val w = size.width
                val h = size.height
                if(w <= 0 || h <= 0) return@Canvas

                //compute y range across all series
                var lo = Double.POSITIVE_INFINITY
                var hi = Double.NEGATIVE_INFINITY
                var hasData = false
                for(s in series){
                    if(s.data.isEmpty()) continue
                    hasData = true
                    for(v in s.data) {
                        if(v < lo) lo = v
                        if(v > hi) hi = v
                    }
                }
                if(!hasData) return@Canvas
                if(hi - lo < minRange) {
                    val mid = (hi + lo) / 2.0
                    lo = mid - minRange / 2
                    hi = mid + minRange / 2
                }
                val pad = (hi - lo) * 0.08
                lo -= pad; hi += pad
                val span = max(hi - lo, 1e-6)

                //grid lines (horizontal, 4 lines)
                val gridColor = Color(0xFF333333)
                val dashed = PathEffect.dashPathEffect(floatArrayOf(4f, 4f))
                for(i in 1..3){
                    val y = h * i / 4f
                    drawLine(gridColor, Offset(0f, y), Offset(w, y),
                        strokeWidth = 1f, pathEffect = dashed)
                }

                //draw zero line if visible
                if(lo < 0 && hi > 0) {
                    val zeroY = (h * (1.0 - (0.0 - lo) / span)).toFloat()
                    drawLine(GrayText, Offset(0f, zeroY), Offset(w, zeroY), strokeWidth = 1f)
                }

                //plot each series
                for(s in series) {
                    if(s.data.size < 2) continue
                    val path = Path()
                    val n = s.data.size
                    val dx = w / (n - 1).toFloat()
                    for(i in 0 until n) {
                        val x = i * dx
                        val v = s.data[i]
                        val frac = (v - lo) / span
                        val y = (h * (1.0 - frac)).toFloat()
                        if(i == 0) path.moveTo(x, y) else path.lineTo(x, y)
                    }
                    drawPath(path, color = s.color, style = Stroke(width = 2.2f))
                }
            }
        }
    }
}

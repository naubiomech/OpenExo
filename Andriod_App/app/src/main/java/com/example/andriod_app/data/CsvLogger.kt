//=========================================================================
//AI ASSISTED FILE
//per Part 1 Documentation: "AI assists with...CSV export"
//handles writing trial data to a .csv on the phone's external files dir,
//file rename / delete / share via FileProvider, and rollover (close
//current csv + open a fresh one mid-trial). AI handled the file io,
//buffered writer lifecycle, sanitization of filenames, and time formatting.
//student handled hooking the logger to BLE samples (TrialViewModel.bind)
//and surfacing it in EndTrialScreen.
//=========================================================================
package com.example.andriod_app.data

import android.content.Context
import android.util.Log
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import java.io.BufferedWriter
import java.io.File
import java.io.FileWriter
import java.text.SimpleDateFormat
import java.util.Date
import java.util.Locale

private const val TAG = "CsvLogger"
private const val DATA_WIDTH = 16

class CsvLogger(private val context: Context) {

    private val _isLogging = MutableStateFlow(false)
    val isLogging: StateFlow<Boolean> = _isLogging.asStateFlow()

    private val _currentFileName = MutableStateFlow("")
    val currentFileName: StateFlow<String> = _currentFileName.asStateFlow()

    private val _rowCount = MutableStateFlow(0)
    val rowCount: StateFlow<Int> = _rowCount.asStateFlow()

    val logDir: File by lazy {
        //app-private external dir, accessible to user via Files app
        val d = context.getExternalFilesDir("trials") ?: File(context.filesDir, "trials")
        if(!d.exists()) d.mkdirs()
        d
    }

    private var writer: BufferedWriter? = null
    private var lastFile: File? = null
    private var trialStart: Long = 0
    private var columnNames: List<String> = emptyList()

    fun startLogging(prefix: String, columnNames: List<String>) {
        stopLogging()
        val cols = columnNames.toMutableList()
        while(cols.size < DATA_WIDTH) cols.add("data${cols.size}")
        this.columnNames = cols

        val safe = prefix.filter{ it.isLetterOrDigit() || it == '_' || it == '-' }
        val ts = SimpleDateFormat("yyyyMMdd_HHmmss", Locale.US).format(Date())
        val name = if(safe.isEmpty()) "trial_$ts.csv" else "${safe}_trial_$ts.csv"

        val file = File(logDir, name)
        try {
            writer = BufferedWriter(FileWriter(file))
            val header = (listOf("epoch","mark") + cols).joinToString(",")
            writer?.write(header)
            writer?.newLine()
            lastFile = file
            _currentFileName.value = name
            _rowCount.value = 0
            trialStart = System.currentTimeMillis()
            _isLogging.value = true
        } catch(e: Exception) {
            Log.e(TAG, "startLogging failed", e)
            writer = null
        }
    }

    fun log(values: DoubleArray, mark: Int) {
        val w = writer ?: return
        if(!_isLogging.value) return
        try {
            val padded = DoubleArray(columnNames.size)
            val n = minOf(values.size, padded.size)
            for(i in 0 until n) padded[i] = values[i]
            val epoch = System.currentTimeMillis() / 1000.0
            val sb = StringBuilder()
            sb.append(String.format(Locale.US, "%.3f", epoch))
            sb.append(',')
            sb.append(mark)
            for(v in padded){
                sb.append(',')
                sb.append(String.format(Locale.US, "%.4f", v))
            }
            w.write(sb.toString())
            w.newLine()
            _rowCount.value = _rowCount.value + 1
        } catch(e: Exception){
            Log.e(TAG, "log row failed", e)
        }
    }

    fun stopLogging() {
        try { writer?.flush(); writer?.close() } catch(_: Exception){}
        writer = null
        _isLogging.value = false
    }

    //rollover - close current csv and start a fresh one mid trial (matches ios logger.rollover)
    fun rollover(prefix: String) {
        val cols = columnNames
        stopLogging()
        startLogging(prefix, cols)
    }

    //----------------------------------
    //post trial file ops (used by EndTrialScreen)
    //----------------------------------
    fun durationSec(): Int {
        if(trialStart == 0L) return 0
        return ((System.currentTimeMillis() - trialStart) / 1000).toInt()
    }

    fun durationFormatted(): String {
        val s = durationSec()
        if(s < 60) return "${s}s"
        return "${s/60}m ${s%60}s"
    }

    fun lastFileSize(): String {
        val f = lastFile ?: return "-"
        if(!f.exists()) return "-"
        val sz = f.length()
        return when {
            sz < 1024 -> "$sz B"
            sz < 1024*1024 -> "${sz/1024} KB"
            else -> String.format(Locale.US, "%.1f MB", sz/1024.0/1024.0)
        }
    }

    fun lastFile(): File? = lastFile

    fun renameLastFile(newName: String): Boolean {
        val f = lastFile ?: return false
        val safe = newName.filter{ it.isLetterOrDigit() || it == '_' || it == '-' || it == ' ' }
        if(safe.isBlank()) return false
        val newFile = File(f.parentFile, "$safe.csv")
        return try {
            if(f.renameTo(newFile)){
                lastFile = newFile
                _currentFileName.value = newFile.name
                true
            } else false
        } catch(e: Exception){
            Log.e(TAG, "rename failed", e); false
        }
    }

    fun deleteLastFile() {
        val f = lastFile ?: return
        try { f.delete() } catch(_: Exception){}
        //companion notes file
        val notes = File(f.parentFile, f.nameWithoutExtension + ".txt")
        try { if(notes.exists()) notes.delete() } catch(_: Exception){}
        lastFile = null
        _currentFileName.value = ""
    }

    fun saveTrialNotes(notes: String) {
        val f = lastFile ?: return
        if(notes.isBlank()) return
        val nFile = File(f.parentFile, f.nameWithoutExtension + ".txt")
        try {
            nFile.writeText(notes)
        } catch(e: Exception){
            Log.e(TAG, "save notes failed", e)
        }
    }
}

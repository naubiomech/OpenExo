package com.example.andriod_app.viewmodel

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import androidx.lifecycle.viewModelScope
import com.example.andriod_app.data.AuthUser
import com.example.andriod_app.data.BleManager
import com.example.andriod_app.data.CsvLogger
import com.example.andriod_app.data.FirebaseDbApi
import com.example.andriod_app.data.TrialRecord
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch

//owns the csv logger + handles uploading trial metdata to firebase db on save.
//hooks ble's logCallback so every rt sample gets streamed into the csv.
class TrialViewModel(app: Application) : AndroidViewModel(app) {

    val csv = CsvLogger(app.applicationContext)

    private val _saveStatus = MutableStateFlow<String?>(null)
    val saveStatus: StateFlow<String?> = _saveStatus.asStateFlow()

    //csv prefix the user can set (mirrored on iOS)
    private val _csvPrefix = MutableStateFlow("")
    val csvPrefix: StateFlow<String> = _csvPrefix.asStateFlow()

    fun setCsvPrefix(p: String){ _csvPrefix.value = p }

    //pipe ble samples to csv
    fun bind(ble: BleManager){
        ble.logCallback = { values, mark ->
            //android.util.Log.d("TrialVM", "got sample sz=${values.size} mark=$mark")
            csv.log(values, mark)
        }
    }

    fun startCsv(columnNames: List<String>) {
        csv.startLogging(_csvPrefix.value, columnNames)
    }

    fun stopCsv() = csv.stopLogging()

    //rollover - close cur csv and open a fresh one mid trial
    fun rolloverCsv(){ csv.rollover(_csvPrefix.value) }

    fun saveTrialToFirebase(user: AuthUser?, notes: String, markCount: Int)
    {
        if(user == null) {
            _saveStatus.value = "Not signed in - skipping cloud save"
            return
        }
        val f = csv.lastFile()
        if(f == null){
            _saveStatus.value = "No file to save"
            return
        }
        csv.saveTrialNotes(notes)
        val record = TrialRecord(
            fileName = f.name,
            notes = notes,
            durationSec = csv.durationSec(),
            rowCount = csv.rowCount.value,
            markCount = markCount,
            timestampMs = System.currentTimeMillis()
        )
        _saveStatus.value = "Saving to cloud..."
        viewModelScope.launch{
            val res = FirebaseDbApi.saveTrial(user, record)
            res.onSuccess { _saveStatus.value = "Saved to cloud" }
                .onFailure{ _saveStatus.value = "Cloud save failed: ${it.message}" }
        }
    }

    fun discardTrial(){
        csv.deleteLastFile()
        _saveStatus.value = "Discarded"
    }

    fun clearStatus() { _saveStatus.value = null }
}

package com.example.andriod_app.viewmodel

import android.app.Application
import androidx.lifecycle.AndroidViewModel
import com.example.andriod_app.data.BleManager
import com.example.andriod_app.data.DiscoveredDevice

//thin viewmodel that wraps BleManager so the screens dont touch ble
//directly (mvvm thing). uses AndroidViewModel so we can pass app ctx
//down for the ble apis.
class BleViewModel(app: Application) : AndroidViewModel(app)
{
    val ble = BleManager(app.applicationContext)

    //pass thru funcs so the screens can call vm.startScan() etc
    fun startScan() = ble.startScan()
    fun stopScan() = ble.stopScan()
    fun connect(d: DiscoveredDevice) = ble.connect(d)
    fun connectSaved() = ble.connectSaved()
    fun hasSavedDevice() = ble.hasSavedDevice()
    fun disconnect() = ble.disconnect()

    //trial lifecyle
    fun beginTrial() = ble.beginTrial()
    fun endTrial() = ble.endTrial()
    fun markTrial() = ble.markTrial()

    //motor + cal stuff
    fun motorsOff()       = ble.motorsOff()
    fun motorsOn()        = ble.motorsOn()
    fun calibrateTorque() = ble.calibrateTorque()
    fun calibrateFsr()    = ble.calibrateFsr()

    fun updateParam(bilateral: Boolean, jointID: Int, controllerID: Int, paramIndex: Int, value: Double) {
        ble.updateParam(bilateral, jointID, controllerID, paramIndex, value)
    }

    fun sendFsrThresholds(left: Double, right: Double){
        ble.sendFsrThresholds(left, right)
    }

    //cleanup on vm destroy - stop chart timer + drop ble link
    override fun onCleared() {
        super.onCleared()
        ble.stopChartTimer()
        ble.disconnect()
    }
}

//=========================================================================
//AI ASSISTED FILE
//per Part 1 Documentation: "AI assists with the stuff not covered in class
//like BLE hardware communication" - this whole BleManager (Nordic UART
//service, GATT scan/connect, handshake parser, RT data parser, async
//write queue, MTU exchange, mock-mode sine wave generator) was written
//with AI assistance.
//student handled the integration points (viewmodel binding, screen wiring,
//permission requests in MainActivity).
//=========================================================================
package com.example.andriod_app.data

import android.Manifest
import android.annotation.SuppressLint
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothGatt
import android.bluetooth.BluetoothGattCallback
import android.bluetooth.BluetoothGattCharacteristic
import android.bluetooth.BluetoothGattDescriptor
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothProfile
import android.bluetooth.le.ScanCallback
import android.bluetooth.le.ScanFilter
import android.bluetooth.le.ScanResult
import android.bluetooth.le.ScanSettings
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Handler
import android.os.Looper
import android.os.ParcelUuid
import android.util.Log
import androidx.core.content.ContextCompat
import kotlinx.coroutines.CoroutineScope
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.Job
import kotlinx.coroutines.SupervisorJob
import kotlinx.coroutines.delay
import kotlinx.coroutines.flow.MutableStateFlow
import kotlinx.coroutines.flow.StateFlow
import kotlinx.coroutines.flow.asStateFlow
import kotlinx.coroutines.launch
import java.nio.ByteBuffer
import java.nio.ByteOrder
import java.util.UUID
import kotlin.math.PI
import kotlin.math.sin

//set to true to test in emulator without real exo, false for real ble
const val MOCK_MODE = false

private const val TAG = "BleManager"

//nordic uart service uuids (same as ios + python)
private val UART_SERVICE = UUID.fromString("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
private val UART_TX = UUID.fromString("6E400002-B5A3-F393-E0A9-E50E24DCCA9E") //write
private val UART_RX = UUID.fromString("6E400003-B5A3-F393-E0A9-E50E24DCCA9E") //notify
private val ERR_CHAR = UUID.fromString("33B65D43-611C-11ED-9B6A-0242AC120002")
private val CCCD = UUID.fromString("00002902-0000-1000-8000-00805f9b34fb")

//chart buffer size (3sec @ 30hz x 3 = 270 ish, give 300)
private const val CHART_CAPACITY = 300
//how many data channels we plot
private const val CHART_CHANNELS = 8

class BleManager(private val context: Context) {

    private val scope = CoroutineScope(SupervisorJob() + Dispatchers.Main.immediate)

    //state flows
    private val _isScanning = MutableStateFlow(false)
    val isScanning: StateFlow<Boolean> = _isScanning.asStateFlow()

    private val _devices = MutableStateFlow<List<DiscoveredDevice>>(emptyList())
    val devices: StateFlow<List<DiscoveredDevice>> = _devices.asStateFlow()

    private val _isConnected = MutableStateFlow(false)
    val isConnected: StateFlow<Boolean> = _isConnected.asStateFlow()

    private val _connectionStatus = MutableStateFlow("Not connected")
    val connectionStatus: StateFlow<String> = _connectionStatus.asStateFlow()

    private val _connectedName = MutableStateFlow("")
    val connectedName: StateFlow<String> = _connectedName.asStateFlow()

    private val _torqueCalibrated = MutableStateFlow(false)
    val torqueCalibrated: StateFlow<Boolean> = _torqueCalibrated.asStateFlow()

    private val _isTrialActive = MutableStateFlow(false)
    val isTrialActive: StateFlow<Boolean> = _isTrialActive.asStateFlow()

    private val _isPaused = MutableStateFlow(false)
    val isPaused: StateFlow<Boolean> = _isPaused.asStateFlow()

    private val _markCount = MutableStateFlow(0)
    val markCount: StateFlow<Int> = _markCount.asStateFlow()

    private val _batteryVolts = MutableStateFlow<Double?>(null)
    val batteryVolts: StateFlow<Double?> = _batteryVolts.asStateFlow()

    //handshake data
    private val _handshakeReceived = MutableStateFlow(false)
    val handshakeReceived: StateFlow<Boolean> = _handshakeReceived.asStateFlow()

    private val _joints = MutableStateFlow<List<JointInfo>>(emptyList())
    val joints: StateFlow<List<JointInfo>> = _joints.asStateFlow()

    private val _paramNames = MutableStateFlow<List<String>>(emptyList())
    val paramNames: StateFlow<List<String>> = _paramNames.asStateFlow()

    //in-memory database of controller parameter values.  seeded from the
    //handshake (line 6 of each controller csv on the sd card) and mutated
    //locally on every updateParam() call.  destroyed on endTrial/disconnect
    //so a fresh handshake reseeds it on next connect.
    private val _controllerValues = MutableStateFlow<Map<ControllerKey, List<String>>>(emptyMap())
    val controllerValues: StateFlow<Map<ControllerKey, List<String>>> = _controllerValues.asStateFlow()

    //rt sample feed (latest 16 chans)
    private val _rtData = MutableStateFlow(DoubleArray(16))
    val rtData: StateFlow<DoubleArray> = _rtData.asStateFlow()

    //chart snapshot for line chart (8 channels x N points)
    private val _chartSnapshot = MutableStateFlow<List<DoubleArray>>(
        List(CHART_CHANNELS){ DoubleArray(0) }
    )
    val chartSnapshot: StateFlow<List<DoubleArray>> = _chartSnapshot.asStateFlow()

    private val _packetCount = MutableStateFlow(0)
    val packetCount: StateFlow<Int> = _packetCount.asStateFlow()

    //csv logging hook (called on every sample, full data rate)
    var logCallback: ((DoubleArray, Int) -> Unit)? = null

    //triggered when ble link drops mid trial
    var onUnexpectedDisconnect: (() -> Unit)? = null

    //ble plumbing
    private val btManager by lazy {
        context.getSystemService(Context.BLUETOOTH_SERVICE) as? BluetoothManager
    }
    private val adapter: BluetoothAdapter? get() = btManager?.adapter
    private var gatt: BluetoothGatt? = null
    private var txChar: BluetoothGattCharacteristic? = null
    private var rxChar: BluetoothGattCharacteristic? = null
    private var errChar: BluetoothGattCharacteristic? = null

    //circular buffer for chart
    private val circular = Array(CHART_CHANNELS){ DoubleArray(CHART_CAPACITY) }
    private val channelActive = BooleanArray(CHART_CHANNELS)
    private var writeIdx = 0
    private var rawValues = DoubleArray(16)
    private var pktCnt = 0
    private var publishSkip = 0

    //handshake parser state
    private var handshakeBuffer = StringBuilder()
    private var receivingHandshake = false

    //timers
    private var chartTimer: Job? = null
    private var mockJob: Job? = null
    private val mainHandler = Handler(Looper.getMainLooper())

    //----------------------------------
    //write queue - android's ble stack only allows ONE outstanding gatt write at a time.
    //if you call writeCharacteristic back-to-back, only the first goes out and the rest
    //silently fail. so we serialize all writes through a queue and dispatch the next one
    //from the onCharacteristicWrite callback.
    //----------------------------------
    private data class PendingWrite(val data: ByteArray, val withResponse: Boolean)
    private val writeQueue = ArrayDeque<PendingWrite>()
    private var writeInFlight = false
    private val writeLock = Any()
    //watchdog - if the callback never fires for some reason, drop after 500ms and continue
    private var writeWatchdog: Runnable? = null

    //----------------------------------
    //permissions helper
    //----------------------------------
    fun hasBlePermissions(): Boolean {
        if(Build.VERSION.SDK_INT >= Build.VERSION_CODES.S){
            return ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_SCAN) ==
                PackageManager.PERMISSION_GRANTED &&
                ContextCompat.checkSelfPermission(context, Manifest.permission.BLUETOOTH_CONNECT) ==
                PackageManager.PERMISSION_GRANTED
        } else {
            return ContextCompat.checkSelfPermission(context, Manifest.permission.ACCESS_FINE_LOCATION) ==
                PackageManager.PERMISSION_GRANTED
        }
    }

    fun isBluetoothEnabled(): Boolean {
        return adapter?.isEnabled == true
    }

    //----------------------------------
    //scanning
    //----------------------------------
    @SuppressLint("MissingPermission")
    fun startScan() {
        if(MOCK_MODE){ mockScan(); return }
        if(!hasBlePermissions()){
            _connectionStatus.value = "Bluetooth permission not granted"
            return
        }
        if(!isBluetoothEnabled()){
            _connectionStatus.value = "Bluetooth is OFF — enable it in Settings"
            return
        }
        val scanner = adapter?.bluetoothLeScanner ?: run{
            _connectionStatus.value = "BLE not available on this device"
            return
        }
        _devices.value = emptyList()
        _isScanning.value = true
        _connectionStatus.value = "Scanning..."

        val filter = ScanFilter.Builder()
            .setServiceUuid(ParcelUuid(UART_SERVICE))
            .build()
        val settings = ScanSettings.Builder()
            .setScanMode(ScanSettings.SCAN_MODE_LOW_LATENCY)
            .build()

        try {
            scanner.startScan(listOf(filter), settings, scanCallback)
        } catch(e: SecurityException){
            Log.e(TAG, "scan perm denied", e)
            _connectionStatus.value = "Scan permission denied"
            _isScanning.value = false
            return
        }

        //auto stop after 6s like ios
        mainHandler.postDelayed({
            if(_isScanning.value) stopScan()
        }, 6000)
    }

    @SuppressLint("MissingPermission")
    fun stopScan() {
        if(MOCK_MODE){ _isScanning.value = false; return }
        try { adapter?.bluetoothLeScanner?.stopScan(scanCallback) } catch(_: Exception){}
        _isScanning.value = false
        if(_devices.value.isEmpty()){
            _connectionStatus.value = "No devices found"
        } else {
            _connectionStatus.value = "Found ${_devices.value.size} device(s)"
        }
    }

    private val scanCallback = object: ScanCallback() {
        override fun onScanResult(callbackType: Int, result: ScanResult) {
            val dev = result.device ?: return
            val name = try { dev.name ?: "Unknown" } catch(_: SecurityException){ "Unknown" }
            val addr = dev.address ?: return
            val cur = _devices.value
            if(cur.none { it.address == addr }) {
                _devices.value = cur + DiscoveredDevice(addr, name)
            }
        }

        override fun onScanFailed(errorCode: Int) {
            Log.e(TAG, "scan failed: $errorCode")
            _isScanning.value = false
            _connectionStatus.value = "Scan failed (err $errorCode)"
        }
    }

    //----------------------------------
    //connect / disconnect
    //----------------------------------
    @SuppressLint("MissingPermission")
    fun connect(device: DiscoveredDevice) {
        if(MOCK_MODE){ mockConnect(device); return }
        stopScan()
        val btDev = adapter?.getRemoteDevice(device.address) ?: run{
            _connectionStatus.value = "Bad device address"
            return
        }
        _connectionStatus.value = "Connecting to ${device.name}..."
        //save to prefs for "load saved"
        context.getSharedPreferences("openexo_prefs", Context.MODE_PRIVATE)
            .edit().putString("savedAddr", device.address).apply()
        try {
            gatt = btDev.connectGatt(context, false, gattCallback, BluetoothDevice.TRANSPORT_LE)
        } catch(e: SecurityException){
            _connectionStatus.value = "Connect permission denied"
        }
    }

    @SuppressLint("MissingPermission")
    fun connectSaved() {
        val saved = context.getSharedPreferences("openexo_prefs", Context.MODE_PRIVATE)
            .getString("savedAddr", null)
        if(saved == null){
            _connectionStatus.value = "No saved device"
            return
        }
        connect(DiscoveredDevice(saved, "Saved Device"))
    }

    fun hasSavedDevice(): Boolean {
        return context.getSharedPreferences("openexo_prefs", Context.MODE_PRIVATE)
            .getString("savedAddr", null) != null
    }

    @SuppressLint("MissingPermission")
    fun disconnect() {
        if(MOCK_MODE){ mockDisconnect(); return }
        try { gatt?.disconnect(); gatt?.close() } catch(_: Exception){}
        gatt = null
        txChar = null; rxChar = null; errChar = null
        _isConnected.value = false
        _isTrialActive.value = false
        _torqueCalibrated.value = false
        _connectionStatus.value = "Disconnected"
        stopChartTimer()
        clearWriteQueue()
        destroyControllerDb("disconnect")
    }

    //----------------------------------
    //gatt callback
    //----------------------------------
    private val gattCallback = object: BluetoothGattCallback() {
        @SuppressLint("MissingPermission")
        override fun onConnectionStateChange(g: BluetoothGatt, status: Int, newState: Int) {
            if(newState == BluetoothProfile.STATE_CONNECTED) {
                Log.d(TAG, "GATT connected")
                _isConnected.value = true
                _connectedName.value = try { g.device.name ?: g.device.address } catch(_: SecurityException){ "Unknown" }
                _connectionStatus.value = "Connected to ${_connectedName.value}"
                //kick off chart timer right away so any screen showing chartSnapshot updates
                startChartTimer()
                //request a larger MTU so multi-byte writes fit; discoverServices runs from onMtuChanged
                mainHandler.post {
                    try {
                        if(!g.requestMtu(247)) g.discoverServices()
                    } catch(_: SecurityException){
                        try { g.discoverServices() } catch(_: SecurityException){}
                    }
                }
            } else if(newState == BluetoothProfile.STATE_DISCONNECTED) {
                val wasActive = _isTrialActive.value
                Log.d(TAG, "GATT disconnected")
                _isConnected.value = false
                _isTrialActive.value = false
                _torqueCalibrated.value = false
                _connectionStatus.value = "Disconnected"
                gatt = null
                txChar = null; rxChar = null; errChar = null
                stopChartTimer()
                clearWriteQueue()
                destroyControllerDb("gattDisconnect")
                if(wasActive) onUnexpectedDisconnect?.invoke()
            }
        }

        @SuppressLint("MissingPermission")
        override fun onMtuChanged(g: BluetoothGatt, mtu: Int, status: Int) {
            Log.d(TAG, "MTU = $mtu (status $status)")
            try { g.discoverServices() } catch(_: SecurityException){}
        }

        @SuppressLint("MissingPermission")
        override fun onServicesDiscovered(g: BluetoothGatt, status: Int) {
            val svc = g.getService(UART_SERVICE) ?: return
            txChar = svc.getCharacteristic(UART_TX)
            rxChar = svc.getCharacteristic(UART_RX)
            errChar = svc.getCharacteristic(ERR_CHAR)
            //subscribe to rx
            rxChar?.let { enableNotify(g, it) }
        }

        @SuppressLint("MissingPermission")
        private fun enableNotify(g: BluetoothGatt, c: BluetoothGattCharacteristic) {
            try {
                g.setCharacteristicNotification(c, true)
                val d = c.getDescriptor(CCCD)
                if(d != null){
                    if(Build.VERSION.SDK_INT >= 33) {
                        g.writeDescriptor(d, BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE)
                    } else {
                        @Suppress("DEPRECATION")
                        d.value = BluetoothGattDescriptor.ENABLE_NOTIFICATION_VALUE
                        @Suppress("DEPRECATION")
                        g.writeDescriptor(d)
                    }
                }
            } catch(_: SecurityException){}
        }

        @SuppressLint("MissingPermission")
        override fun onDescriptorWrite(g: BluetoothGatt, d: BluetoothGattDescriptor, status: Int) {
            //chain enable err char after rx is done
            if(d.characteristic.uuid == UART_RX){
                errChar?.let{ enableNotify(g, it) }
            }
        }

        override fun onCharacteristicChanged(g: BluetoothGatt, c: BluetoothGattCharacteristic, value: ByteArray) {
            handleIncoming(c, value)
        }

        @Suppress("DEPRECATION")
        override fun onCharacteristicChanged(g: BluetoothGatt, c: BluetoothGattCharacteristic) {
            //legacy path for android < 13
            handleIncoming(c, c.value ?: return)
        }

        //fires for both write-with-response AND write-without-response.
        //we use this to drive the write queue - dispatch next pending write here.
        override fun onCharacteristicWrite(g: BluetoothGatt, c: BluetoothGattCharacteristic, status: Int) {
            if(status != BluetoothGatt.GATT_SUCCESS){
                Log.w(TAG, "write failed for ${c.uuid}, status=$status")
            }
            onWriteComplete()
        }
    }

    private fun handleIncoming(c: BluetoothGattCharacteristic, raw: ByteArray) {
        val s = try { String(raw, Charsets.UTF_8) } catch(_: Exception){ return }
        if(c.uuid == ERR_CHAR){
            Log.w(TAG, "[exo err]: $s")
            return
        }
        //handshake protocol matches ios
        if(s.contains("READY")){
            receivingHandshake = true
            handshakeBuffer.clear()
            _connectionStatus.value = "Handshake received..."
            val idx = s.indexOf("READY")
            val rest = s.substring(idx + 5).trim()
            if(rest.isNotEmpty()) handshakeBuffer.append(rest)
            return
        }
        if(receivingHandshake){
            handshakeBuffer.append(s)
            if(handshakeBuffer.contains('\n')) {
                receivingHandshake = false
                parseHandshake(handshakeBuffer.toString())
                handshakeBuffer.clear()
            }
            return
        }
        if(s.contains("n") && !s.startsWith("t,")) parseRtData(s)
    }

    //----------------------------------
    //handshake parsing (from ios BLEManager.parseHandshake)
    //----------------------------------
    private fun parseHandshake(text: String) {
        val names = mutableListOf<String>()
        val jointsMap = mutableMapOf<Int, JointInfo>()
        val valueDB = mutableMapOf<ControllerKey, List<String>>()

        val lines = text.split('\n')
        for(rawLine in lines){
            val trimmed = rawLine.trim()
            if(trimmed.isEmpty() || trimmed == "?" || trimmed == "END") continue

            if(trimmed.startsWith("t,")){
                names.clear()
                names.addAll(trimmed.substring(2).split(",")
                    .map{ it.trim() }
                    .filter{ it.isNotEmpty() && it != "?" && it != "??" })
                continue
            }

            //matrix may have f, prefix
            var matrix = trimmed
            if(matrix.startsWith("f,")) matrix = matrix.substring(2)
            val entries = matrix.split("|")
            for(entry in entries) {
                val raw = entry.trim()
                if(raw.isEmpty() || raw == "?" || raw == "END") continue
                if(raw.startsWith("t,")){
                    names.clear()
                    names.addAll(raw.substring(2).split(",")
                        .map{ it.trim() }
                        .filter{ it.isNotEmpty() && it != "?" && it != "??" })
                    continue
                }

                val parts = raw.split(",")
                    .map{ it.trim() }
                    .filter{ it.isNotEmpty() && it != "?" && it != "??" }

                //new: values row tagged with leading 'v'.
                //format: [v, JointID, ControllerID, val1, val2, ...]
                if(parts.isNotEmpty() && parts[0] == "v") {
                    if(parts.size < 3) continue
                    val jid = parts[1].toIntOrNull() ?: continue
                    val cid = parts[2].toIntOrNull() ?: continue
                    valueDB[ControllerKey(jid, cid)] = parts.drop(3)
                    continue
                }

                if(parts.size < 4) continue
                val jid = parts[1].toIntOrNull() ?: continue
                val cid = parts[3].toIntOrNull() ?: continue

                val ctrl = ControllerInfo(
                    name = parts[2],
                    controllerID = cid,
                    params = parts.drop(4)
                )
                val existing = jointsMap[jid]
                if(existing != null) {
                    existing.controllers.add(ctrl)
                } else {
                    jointsMap[jid] = JointInfo(parts[0], jid, mutableListOf(ctrl))
                }
            }
        }

        //Splice the value db back into each ControllerInfo so the UI can
        //pull defaults straight off the joint structures (matches iOS).
        if(valueDB.isNotEmpty()){
            for((jid, joint) in jointsMap){
                for(ctrl in joint.controllers){
                    val key = ControllerKey(jid, ctrl.controllerID)
                    val vals = valueDB[key] ?: continue
                    ctrl.paramValues = vals.toMutableList()
                }
            }
        }

        if(names.isNotEmpty()) _paramNames.value = names
        _controllerValues.value = valueDB
        if(jointsMap.isNotEmpty()){
            _joints.value = jointsMap.values.sortedBy{ it.jointID }
        }
        _handshakeReceived.value = true
        send('$') //ack
    }

    //----------------------------------
    //rt data parsing (csv "<count>c S<cmd><v1>n<v2>n..." frames)
    //----------------------------------
    private fun parseRtData(packet: String) {
        val segments = packet.split('c')
        if(segments.size >= 2){
            var didParse = false
            for(i in 0 until segments.size - 1){
                val tail = segments[i].reversed().takeWhile{ it.isDigit() }.reversed()
                val expected = tail.toIntOrNull() ?: continue
                if(expected <= 1) continue
                val data = segments[i + 1]
                val sIdx = data.indexOf('S')
                if(sIdx < 0) continue
                if(sIdx + 2 >= data.length) continue
                val payload = data.substring(sIdx + 2) //skip S + cmd byte
                val values = mutableListOf<Double>()
                val numBuf = StringBuilder()
                for(ch in payload){
                    if(values.size >= expected) break
                    if(ch == 'n'){
                        val iv = numBuf.toString().toIntOrNull()
                        if(iv != null) values.add(iv / 100.0)
                        numBuf.clear()
                    } else if(ch.isDigit() || ch == '-'){
                        numBuf.append(ch)
                    }
                }
                if(values.size < expected){
                    val iv = numBuf.toString().toIntOrNull()
                    if(iv != null) values.add(iv / 100.0)
                }
                if(values.size <= 1) continue
                didParse = true
                ingest(values.toDoubleArray())
            }
            if(didParse) return
        }

        //fallback path
        val sIdx = packet.indexOf('S')
        if(sIdx >= 0 && sIdx + 1 < packet.length) {
            val afterCmd = packet.substring(sIdx + 2)
            val vals = afterCmd.split('n').mapNotNull {
                val digits = it.filter{ c -> c.isDigit() || c == '-' }
                if(digits.isEmpty() || digits == "-") null
                else digits.toIntOrNull()?.let{ d -> d / 100.0 }
            }
            if(vals.size > 1) ingest(vals.toDoubleArray())
        }
    }

    private fun ingest(values: DoubleArray) {
        val n = minOf(values.size, 16)
        for(i in 0 until n) rawValues[i] = values[i]
        pktCnt++
        //circular buf for chart
        val idx = writeIdx % CHART_CAPACITY
        val cn = minOf(values.size, CHART_CHANNELS)
        for(i in 0 until cn){
            circular[i][idx] = values[i]
            if(!channelActive[i]) channelActive[i] = true
        }
        writeIdx++

        logCallback?.invoke(values, _markCount.value)

        publishSkip++
        if(publishSkip >= 3){
            publishSkip = 0
            _rtData.value = rawValues.copyOf()
            if(values.size > 10) _batteryVolts.value = values[10]
        }
    }

    //----------------------------------
    //chart timer (drains circular buf at 20hz)
    //----------------------------------
    fun startChartTimer() {
        chartTimer?.cancel()
        chartTimer = scope.launch {
            while(true) {
                delay(50L) //20hz
                flushChart()
            }
        }
    }

    fun stopChartTimer() {
        chartTimer?.cancel()
        chartTimer = null
    }

    private fun flushChart() {
        if(writeIdx <= 0) return
        val count = minOf(writeIdx, CHART_CAPACITY)
        val start = writeIdx % CHART_CAPACITY
        val out = mutableListOf<DoubleArray>()
        for(i in 0 until CHART_CHANNELS){
            if(!channelActive[i]){ out.add(DoubleArray(0)); continue }
            val arr = circular[i]
            val ordered = if(writeIdx <= CHART_CAPACITY) {
                arr.copyOfRange(0, count)
            } else {
                //wrap
                val a = arr.copyOfRange(start, CHART_CAPACITY)
                val b = arr.copyOfRange(0, start)
                a + b
            }
            out.add(ordered)
        }
        _chartSnapshot.value = out
        _packetCount.value = pktCnt
    }

    private fun resetChartBuffers() {
        for(i in 0 until CHART_CHANNELS) {
            for(j in 0 until CHART_CAPACITY) circular[i][j] = 0.0
            channelActive[i] = false
        }
        writeIdx = 0
        pktCnt = 0
        publishSkip = 0
        rawValues = DoubleArray(16)
    }

    //----------------------------------
    //commands (single bytes + structured writes)
    //----------------------------------
    fun send(c: Char) {
        if(MOCK_MODE){ Log.d(TAG, "[mock] -> $c"); return }
        sendRaw(byteArrayOf(c.code.toByte()))
    }

    //enqueue a write - actual writeCharacteristic happens via dispatchNextWrite which is
    //driven by the onCharacteristicWrite callback. this is the ONLY way to reliably do
    //multiple ble writes on android.
    fun sendRaw(data: ByteArray, withResponse: Boolean = false) {
        if(MOCK_MODE) return
        synchronized(writeLock) {
            writeQueue.addLast(PendingWrite(data, withResponse))
            if(!writeInFlight) dispatchNextWrite()
        }
    }

    @SuppressLint("MissingPermission")
    private fun dispatchNextWrite() {
        synchronized(writeLock) {
            val g = gatt
            val c = txChar
            if(g == null || c == null){
                writeQueue.clear()
                writeInFlight = false
                return
            }
            val pending = writeQueue.firstOrNull() ?: run {
                writeInFlight = false
                return
            }
            writeInFlight = true
            try {
                if(Build.VERSION.SDK_INT >= 33){
                    val type = if(pending.withResponse) BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        else BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                    g.writeCharacteristic(c, pending.data, type)
                } else {
                    @Suppress("DEPRECATION")
                    c.writeType = if(pending.withResponse) BluetoothGattCharacteristic.WRITE_TYPE_DEFAULT
                        else BluetoothGattCharacteristic.WRITE_TYPE_NO_RESPONSE
                    @Suppress("DEPRECATION")
                    c.value = pending.data
                    @Suppress("DEPRECATION")
                    g.writeCharacteristic(c)
                }
                //arm watchdog in case onCharacteristicWrite never fires
                writeWatchdog?.let { mainHandler.removeCallbacks(it) }
                val wd = Runnable {
                    Log.w(TAG, "write watchdog fired - moving on")
                    onWriteComplete()
                }
                writeWatchdog = wd
                mainHandler.postDelayed(wd, 500)
            } catch(e: Exception) {
                Log.e(TAG, "writeCharacteristic threw", e)
                writeQueue.removeFirstOrNull()
                writeInFlight = false
                mainHandler.post { dispatchNextWrite() }
            }
        }
    }

    //called from onCharacteristicWrite OR the watchdog - move to next queued write
    private fun onWriteComplete() {
        synchronized(writeLock) {
            writeWatchdog?.let { mainHandler.removeCallbacks(it) }
            writeWatchdog = null
            writeQueue.removeFirstOrNull()
            writeInFlight = false
            dispatchNextWrite()
        }
    }

    private fun clearWriteQueue() {
        synchronized(writeLock) {
            writeWatchdog?.let { mainHandler.removeCallbacks(it) }
            writeWatchdog = null
            writeQueue.clear()
            writeInFlight = false
        }
    }

    fun calibrateTorque() {
        send('H')
        _torqueCalibrated.value = false
        _connectionStatus.value = "Calibrating... Start Trial unlocks in 3s"
        mainHandler.postDelayed({
            _torqueCalibrated.value = true
            _connectionStatus.value = "Calibrated - tap Start Trial"
        }, 3000)
    }

    fun calibrateFsr() { send('L') }
    fun motorsOff(){ sendRaw(byteArrayOf('w'.code.toByte()), withResponse = true); _isPaused.value = true }
    fun motorsOn() { sendRaw(byteArrayOf('x'.code.toByte()), withResponse = true); _isPaused.value = false }
    fun markTrial(){ send('N'); _markCount.value = _markCount.value + 1 }

    fun beginTrial() {
        _markCount.value = 0
        resetChartBuffers()

        //wait 1s for handshake/calibration to settle, then enqueue all writes in order.
        //the write queue + onCharacteristicWrite callback take care of pacing.
        mainHandler.postDelayed({
            sendRaw(byteArrayOf('E'.code.toByte()), withResponse = true)
            sendRaw(byteArrayOf('L'.code.toByte()), withResponse = true)
            sendRaw(byteArrayOf('R'.code.toByte()), withResponse = true)
            sendRaw(doubleToLeBytes(0.25), withResponse = true)
            sendRaw(doubleToLeBytes(0.25), withResponse = true)
            //let the writes drain, then mark trial as active
            mainHandler.postDelayed({
                _isTrialActive.value = true
                _isPaused.value = false
                if(MOCK_MODE) startMockData()
            }, 500)
        }, 1000)
    }

    fun endTrial() {
        send('G')
        sendRaw(byteArrayOf('w'.code.toByte()), withResponse = true)
        send('Z')
        _isTrialActive.value = false
        stopMockData()
        //destroy in-memory controller info per spec
        destroyControllerDb("endTrial")
        //queue drains in ~100-200ms, then disconnect
        mainHandler.postDelayed({ disconnect() }, 600)
    }

    //wipes cached joint/controller/values so next handshake re-seeds them.
    //called at end-of-trial and on every disconnect.
    private fun destroyControllerDb(reason: String) {
        if(_controllerValues.value.isNotEmpty() || _joints.value.isNotEmpty() ||
           _paramNames.value.isNotEmpty()) {
            Log.d(TAG, "destroyControllerDb($reason): joints=${_joints.value.size}, " +
                "valueRows=${_controllerValues.value.size}, names=${_paramNames.value.size}")
        }
        _controllerValues.value = emptyMap()
        _joints.value = emptyList()
        _paramNames.value = emptyList()
        _handshakeReceived.value = false
    }

    fun sendFsrThresholds(left: Double, right: Double) {
        if(MOCK_MODE) return
        send('R')
        sendRaw(doubleToLeBytes(left))
        sendRaw(doubleToLeBytes(right))
    }

    //updateparam mirrors ios: f<jid_double><cid_double><pidx_double><val_double>
    //also mirrors the new value into the in-memory db so the settings screen
    //immediately shows it next time the user opens the page.
    fun updateParam(bilateral: Boolean, jointID: Int, controllerID: Int, paramIndex: Int, value: Double) {
        val ids = if(bilateral) listOf(jointID, jointID xor 0x60) else listOf(jointID)

        //1) Send to firmware
        if(!MOCK_MODE){
            for(jid in ids){
                send('f')
                sendRaw(doubleToLeBytes(jid.toDouble()))
                sendRaw(doubleToLeBytes(controllerID.toDouble()))
                sendRaw(doubleToLeBytes(paramIndex.toDouble()))
                sendRaw(doubleToLeBytes(value))
            }
        } else {
            Log.d(TAG, "[mock] updateParam joint=$jointID ctrl=$controllerID param=$paramIndex val=$value")
        }

        //2) Mirror into the live db (and the embedded paramValues lists)
        val valueStr = formatParamValue(value)
        val newDb = _controllerValues.value.toMutableMap()
        for(jid in ids){
            val key = ControllerKey(jid, controllerID)
            val row = newDb[key]?.toMutableList() ?: mutableListOf()
            while(row.size <= paramIndex) row.add("")
            row[paramIndex] = valueStr
            newDb[key] = row

            //also bake into the matching ControllerInfo so any consumer of
            //joints sees the new value without re-reading the db.
            val joint = _joints.value.firstOrNull{ it.jointID == jid } ?: continue
            val ctrl = joint.controllers.firstOrNull{ it.controllerID == controllerID } ?: continue
            while(ctrl.paramValues.size <= paramIndex) ctrl.paramValues.add("")
            ctrl.paramValues[paramIndex] = valueStr
        }
        _controllerValues.value = newDb
    }

    //format a Double the way the firmware writes its csv cells: integers as
    //plain ints, everything else with up to a few decimals.
    private fun formatParamValue(v: Double): String {
        if(v == Math.floor(v) && !v.isInfinite()){
            return v.toLong().toString()
        }
        var s = String.format(java.util.Locale.US, "%.6f", v)
        while(s.endsWith("0")) s = s.dropLast(1)
        if(s.endsWith(".")) s = s.dropLast(1)
        return s
    }

    private fun doubleToLeBytes(v: Double): ByteArray {
        val bb = ByteBuffer.allocate(8).order(ByteOrder.LITTLE_ENDIAN)
        bb.putDouble(v)
        return bb.array()
    }

    //----------------------------------
    //mock implementations (sine wave fake data)
    //----------------------------------
    private fun mockScan() {
        _devices.value = emptyList()
        _isScanning.value = true
        _connectionStatus.value = "Scanning..."
        mainHandler.postDelayed({
            _devices.value = listOf(
                DiscoveredDevice("AA:BB:CC:00:00:01", "OpenExo Left Ankle"),
                DiscoveredDevice("AA:BB:CC:00:00:02", "OpenExo Bilateral"),
                DiscoveredDevice("AA:BB:CC:00:00:03", "OpenExo Dev Unit")
            )
            _isScanning.value = false
            _connectionStatus.value = "Found ${_devices.value.size} device(s)"
        }, 1500)
    }

    private fun mockConnect(d: DiscoveredDevice) {
        _isScanning.value = false
        _connectionStatus.value = "Connecting to ${d.name}..."
        context.getSharedPreferences("openexo_prefs", Context.MODE_PRIVATE)
            .edit().putString("savedAddr", d.address).apply()
        mainHandler.postDelayed({
            _isConnected.value = true
            _connectedName.value = d.name
            _connectionStatus.value = "Connected to ${d.name}"
            mainHandler.postDelayed({ mockHandshake() }, 1000)
        }, 800)
    }

    private fun mockDisconnect() {
        val wasActive = _isTrialActive.value
        _isConnected.value = false
        _connectedName.value = ""
        _isTrialActive.value = false
        _torqueCalibrated.value = false
        _connectionStatus.value = "Disconnected"
        stopMockData()
        destroyControllerDb("mockDisconnect")
        if(wasActive) onUnexpectedDisconnect?.invoke()
    }

    private fun mockHandshake() {
        _paramNames.value = listOf(
            "torque_cmd","torque_meas","ankle_angle","fsr_l",
            "fsr_r","hip_torque","knee_angle","fsr_l2"
        )
        val pjmc = listOf("p_gain","i_gain","d_gain","use_pid","torque_limit")
        val zhang = listOf("peak_torque","rise_time","peak_time","fall_time")
        _joints.value = listOf(
            JointInfo("Left Ankle", 68, mutableListOf(
                ControllerInfo("pjmc_plus", 11, pjmc),
                ControllerInfo("zeroTorque", 1, emptyList())
            )),
            JointInfo("Right Ankle", 36, mutableListOf(
                ControllerInfo("pjmc_plus", 11, pjmc),
                ControllerInfo("zeroTorque", 1, emptyList())
            )),
            JointInfo("Left Hip", 65, mutableListOf(
                ControllerInfo("zhang_collins", 6, zhang),
                ControllerInfo("zeroTorque", 1, emptyList())
            ))
        )
        _handshakeReceived.value = true
        _batteryVolts.value = 11.7
        _connectionStatus.value = "Handshake complete - ready to start trial"
    }

    private var mockT = 0.0
    private fun startMockData() {
        mockT = 0.0
        mockJob?.cancel()
        mockJob = scope.launch {
            while(true){
                delay(33L) //~30hz
                mockTick()
            }
        }
    }

    private fun stopMockData() {
        mockJob?.cancel()
        mockJob = null
    }

    private fun mockTick() {
        val t = mockT
        mockT += 1.0/30.0
        fun s(f: Double, a: Double, ph: Double = 0.0) = a * sin(2.0 * PI * f * t + ph)
        fun nz(a: Double) = a * (Math.random() * 2 - 1)
        fun fsr(f: Double, ph: Double): Double {
            val v = sin(2.0 * PI * f * t + ph)
            return if(v > 0.3) 0.6 + nz(0.05) else 0.05 + nz(0.02)
        }
        val v = DoubleArray(16)
        v[0] = s(0.5, 30.0)
        v[1] = s(0.5, 28.0) + nz(2.0)
        v[2] = s(0.4, 18.0, 0.3)
        v[3] = fsr(1.0, 0.0)
        v[4] = s(0.6, 20.0, 0.5)
        v[5] = fsr(1.0, PI)
        v[6] = s(0.7, 15.0, 1.2) + nz(1.0)
        v[7] = fsr(1.0, 0.2)
        v[10] = 11.7 - sin(t * 0.01) * 0.1
        ingest(v)
    }
}

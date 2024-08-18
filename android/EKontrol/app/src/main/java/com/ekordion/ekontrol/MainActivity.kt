package com.ekordion.ekontrol
import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.Intent
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.activity.result.contract.ActivityResultContracts
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.padding
import androidx.compose.foundation.layout.width
import androidx.compose.foundation.rememberScrollState
import androidx.compose.foundation.verticalScroll
import androidx.compose.material3.Text
import androidx.compose.material3.Button
import androidx.compose.material3.Slider
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.text.font.*
import androidx.compose.ui.unit.dp
import androidx.compose.ui.unit.sp
import androidx.core.app.ActivityCompat
import java.io.IOException
import java.util.UUID

// import com.example.test2.ui.theme.Test2Theme

const val CHANGE_LFO_TABLE = 1;
const val CHANGE_LFO_FREQUENCY = 2;
const val CHANGE_REVERB_FEEDBACK = 3;
const val CHANGE_REVERB_DAMPING = 4;
const val CHANGE_REVERB_VOLUME = 5;
const val CHANGE_WAVETABLE = 6;
const val CHANGE_RESOLUTION = 7;
const val CHANGE_DOWNSAMPLING = 8;
const val CHANGE_OCTAVE = 9;
const val CHANGE_ARPEGGIO_DURATION = 10;
const val CHANGE_ARPEGGIATOR = 11;
const val CHANGE_VIBRATO = 12;
const val CHANGE_ENVELOPE_A = 13;
const val CHANGE_ENVELOPE_D = 14;
const val CHANGE_ENVELOPE_S = 15;
const val CHANGE_ENVELOPE_R = 16;
const val CHANGE_FILTER_LOW_F = 17;
const val CHANGE_FILTER_HIGH_F = 18;
const val CHANGE_FILTER_Q = 19;
const val CHANGE_FILTER_MOD_A = 20;
const val CHANGE_FILTER_MOD_D = 21;
const val CHANGE_FILTER_MOD_S = 22;
const val CHANGE_FILTER_MOD_R = 23;
const val CHANGE_TREMOLO = 24;
const val CHANGE_DRY_VOLUME = 25;
const val CHANGE_WET_VOLUME = 26;

const val CHANNEL_BASS = 0;
const val CHANNEL_CHORDS = 1;
const val CHANNEL_LEAD = 2;

@RequiresApi(Build.VERSION_CODES.S)
class MainActivity : ComponentActivity() {
    private var bluetoothAdapter : BluetoothAdapter? = null
    // https://community.appinventor.mit.edu/t/bluetooth-classic-ble-uuid-and-chatgpt-answer/90767
    private val uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
    private var bluetoothSocket : BluetoothSocket? = null
    private val wavetables = arrayOf(
        "Custom","Sine","Sine o4","Sine o8","Square","PWM 20",
        "Triangle 50","Triangle 80","Triangle 95","Triangle 99","Sawtooth","Sawtooth o4","Sawtooth o8",
        "Additive1","Additive1 o4","Additive1 o8","Additive2","Additive2 o4","Additive2 o8","Flute",
        "Shepard Sine","Shepard Triangle","Shepard Sawtooth","Shepard Square"
    )
    private val registerForResult = registerForActivityResult(
        ActivityResultContracts.StartActivityForResult()
    ) { result ->
        if (result.resultCode == RESULT_OK) {
            Toast.makeText(this, "Bluetooth is now enabled", Toast.LENGTH_SHORT).show()
        }
    }

    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        enableEdgeToEdge()
        setContent {
            EKInterface()
        }
        checkBluetooth()
    }

    private fun checkBluetooth() {
        val bluetoothManager = getSystemService(Context.BLUETOOTH_SERVICE) as BluetoothManager
        bluetoothAdapter = bluetoothManager.adapter
        if (bluetoothAdapter == null) {
            Toast.makeText(this, "Bluetooth is not supported on this device", Toast.LENGTH_SHORT).show()
        } else if (!bluetoothAdapter!!.isEnabled) {
            Toast.makeText(this, "Bluetooth is not enabled", Toast.LENGTH_SHORT).show()
        } else {
            Toast.makeText(this, "Bluetooth is enabled", Toast.LENGTH_SHORT).show()
        }
    }

    private fun connectToDevice(address: String) {
        if (bluetoothAdapter==null || !bluetoothAdapter!!.isEnabled) {
            val enableBtIntent = Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE)
            registerForResult.launch(enableBtIntent)
            return
        }
        val device: BluetoothDevice = bluetoothAdapter!!.getRemoteDevice(address)
        if (ActivityCompat.checkSelfPermission(
                this,
                Manifest.permission.BLUETOOTH_CONNECT
            ) != PackageManager.PERMISSION_GRANTED
        ) {
            ActivityCompat.requestPermissions(this,arrayOf(
                Manifest.permission.BLUETOOTH,
                Manifest.permission.BLUETOOTH_ADMIN,
                Manifest.permission.BLUETOOTH_CONNECT,
                Manifest.permission.BLUETOOTH_SCAN
            ),0)
            return
        }
        bluetoothSocket = device.createRfcommSocketToServiceRecord(uuid)
        try {
            bluetoothSocket?.connect()
            Toast.makeText(this,"Connected", Toast.LENGTH_SHORT).show()
        } catch (e: IOException) {
            Toast.makeText(this, "Could not connect", Toast.LENGTH_LONG).show()
            return
        }
    }

    private fun stopConnection() {
        bluetoothSocket?.close()
    }

    @Composable
    private fun EKTitle(title: String) {
        Text(
            text = " $title",
            fontSize = 24.sp,
            fontWeight = FontWeight.Bold
        )
    }

    @Composable
    private fun EKSlider(title: String, id:Int, channel:Int, maxValue:Int, initialValue: Int) {
        var value by remember { mutableIntStateOf(initialValue) }
        Text(
            text = " $title",
            fontStyle = FontStyle.Italic
        )
        Slider(
            value = value.toFloat(),
            onValueChange = { newValue ->
                value = newValue.toInt()
                if (channel>=0) {
                    // Channel specific
                    bluetoothSocket?.outputStream?.write(byteArrayOf(id.toByte(),channel.toByte(),value.toByte()))
                }
                else {
                    // General message
                    bluetoothSocket?.outputStream?.write(byteArrayOf(id.toByte(),value.toByte()))
                }
            },
            valueRange = 0f..maxValue.toFloat(),
            steps = maxValue+1,
            modifier = Modifier.height(24.dp)
        )

    }

    @Composable
    private fun EKTablePicker(title: String, id:Int, channel:Int) {
        var value by remember { mutableIntStateOf(0) }
        Text(
            text = " $title : "+wavetables[value],
            fontStyle = FontStyle.Italic
        )
        Slider(
            value = value.toFloat(),
            onValueChange = { newValue ->
                value = newValue.toInt()
                if (channel>=0) {
                    // Channel specific
                    bluetoothSocket?.outputStream?.write(byteArrayOf(id.toByte(),channel.toByte(),value.toByte()))
                }
                else {
                    // General message
                    bluetoothSocket?.outputStream?.write(byteArrayOf(id.toByte(),value.toByte()))
                }
            },
            valueRange = 0f..(wavetables.size-1).toFloat(),
            steps = wavetables.size,
            modifier = Modifier.height(24.dp)
        )

    }

    @Composable
    private fun EKChannelInterface(title: String, channel: Int) {
        EKTitle(title)
        EKTablePicker(title = "Wavetable", id = CHANGE_WAVETABLE, channel = channel)
        EKSlider(title = "Resolution", id = CHANGE_RESOLUTION, channel = channel, maxValue = 15, initialValue = 0)
        EKSlider(title = "Downsampling", id = CHANGE_DOWNSAMPLING, channel = channel, maxValue = 31, initialValue = 0)
        EKSlider(title = "Octave", id = CHANGE_OCTAVE, channel = channel, maxValue = 6, initialValue = 3)
        EKSlider(title = "Arpeggio duration", id = CHANGE_ARPEGGIO_DURATION, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Arpeggiator", id = CHANGE_ARPEGGIATOR, channel = channel, maxValue = 7, initialValue = 0)
        EKSlider(title = "Vibrato", id = CHANGE_VIBRATO, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Envelope A", id = CHANGE_ENVELOPE_A, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Envelope D", id = CHANGE_ENVELOPE_D, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Envelope S", id = CHANGE_ENVELOPE_S, channel = channel, maxValue = 255, initialValue = 255)
        EKSlider(title = "Envelope R", id = CHANGE_ENVELOPE_R, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Filter low F", id = CHANGE_FILTER_LOW_F, channel = channel, maxValue = 255, initialValue = 255)
        EKSlider(title = "Filter high F", id = CHANGE_FILTER_HIGH_F, channel = channel, maxValue = 255, initialValue = 255)
        EKSlider(title = "Filter Q", id = CHANGE_FILTER_Q, channel = channel, maxValue = 255, initialValue = 50)
        EKSlider(title = "Filter mod A", id = CHANGE_FILTER_MOD_A, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Filter mod D", id = CHANGE_FILTER_MOD_D, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Filter mod S", id = CHANGE_FILTER_MOD_S, channel = channel, maxValue = 255, initialValue = 255)
        EKSlider(title = "Filter mod R", id = CHANGE_FILTER_MOD_R, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Tremolo", id = CHANGE_TREMOLO, channel = channel, maxValue = 255, initialValue = 0)
        EKSlider(title = "Dry volume", id = CHANGE_DRY_VOLUME, channel = channel, maxValue = 255, initialValue = 200)
        EKSlider(title = "Wet volume", id = CHANGE_WET_VOLUME, channel = channel, maxValue = 255, initialValue = 100)
    }

    @Composable
    private fun EKInterface() {
        val scrollState = rememberScrollState()

        Column(
                modifier = Modifier
                    .fillMaxSize()
                    .verticalScroll(scrollState)
                    .padding(end = 64.dp),
                verticalArrangement = Arrangement.Center,
                horizontalAlignment = Alignment.Start
        ) {
            Spacer(modifier = Modifier.height(64.dp))
            EKTitle("Bluetooth")
            Row {
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { checkBluetooth() },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("Check")
                }
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { connectToDevice("A8:42:E3:4B:09:DE") },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("ESP 1")
                }
                /*
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { connectToDevice("A8:42:E3:55:DE:5A") },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("ESP 2")
                }
                */
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { stopConnection() },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("Disconnect")
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            EKTitle("LFO")
            EKTablePicker(title = "Table", id = CHANGE_LFO_TABLE, channel = -1)
            EKSlider(title = "Frequency", id = CHANGE_LFO_FREQUENCY, channel = -1, maxValue = 255, initialValue = 50)
            EKTitle("Reverberation")
            EKSlider(title = "Feedback", id = CHANGE_REVERB_FEEDBACK, channel = -1, maxValue = 255, initialValue = 224)
            EKSlider(title = "Damping", id = CHANGE_REVERB_DAMPING, channel = -1, maxValue = 255, initialValue = 192)
            EKSlider(title = "Volume", id = CHANGE_REVERB_VOLUME, channel = -1, maxValue = 255, initialValue = 200)
            EKChannelInterface(title = "Bass", channel = CHANNEL_BASS)
            EKChannelInterface(title = "Chords", channel = CHANNEL_CHORDS)
            EKChannelInterface(title = "Lead", channel = CHANNEL_LEAD)
            Spacer(modifier = Modifier.height(64.dp))
        }
    }
}


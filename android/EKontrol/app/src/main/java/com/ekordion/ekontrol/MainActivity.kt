package com.ekordion.ekontrol
import android.Manifest
import android.bluetooth.BluetoothAdapter
import android.bluetooth.BluetoothDevice
import android.bluetooth.BluetoothManager
import android.bluetooth.BluetoothSocket
import android.content.Context
import android.content.pm.PackageManager
import android.os.Build
import android.os.Bundle
import android.widget.Toast
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.activity.enableEdgeToEdge
import androidx.annotation.RequiresApi
import androidx.compose.foundation.layout.Arrangement
import androidx.compose.foundation.layout.Column
import androidx.compose.foundation.layout.PaddingValues
import androidx.compose.foundation.layout.Row
import androidx.compose.foundation.layout.Spacer
import androidx.compose.foundation.layout.fillMaxSize
import androidx.compose.foundation.layout.height
import androidx.compose.foundation.layout.width
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

@RequiresApi(Build.VERSION_CODES.S)
class MainActivity : ComponentActivity() {
    private var bluetoothAdapter : BluetoothAdapter? = null
    // https://community.appinventor.mit.edu/t/bluetooth-classic-ble-uuid-and-chatgpt-answer/90767
    private val uuid = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB")
    private var bluetoothSocket : BluetoothSocket? = null

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
            Toast.makeText(this, "Check before", Toast.LENGTH_SHORT).show()
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
    private fun EKSlider(title: String, id:Int, initialValue: Int) {
        var value by remember { mutableIntStateOf(initialValue) }
        Text(
            text = title,
            fontStyle = FontStyle.Italic
        )
        Slider(
            value = value.toFloat(),
            onValueChange = { newValue ->
                value = newValue.toInt()
                bluetoothSocket?.outputStream?.write(byteArrayOf(id.toByte(),value.toByte()))
            },
            onValueChangeFinished = {
            },
            valueRange = 0f..255f,
            steps = 256, // This adds 4 steps in between the min and max values
            modifier = Modifier.height(24.dp)
        )

    }

    @Composable
    private fun EKInterface() {
        Column(
            modifier = Modifier.fillMaxSize(),
            verticalArrangement = Arrangement.Center,
            horizontalAlignment = Alignment.Start
        ) {
            Text(
                text = " Bluetooth",
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
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
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { connectToDevice("A8:42:E3:55:DE:5A") },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("ESP 2")
                }
                Spacer(modifier = Modifier.width(8.dp))
                Button(
                    onClick = { stopConnection() },
                    contentPadding = PaddingValues(12.dp)
                ) {
                    Text("Disconnect")
                }
            }
            Spacer(modifier = Modifier.height(16.dp))
            Text(
                text = " General",
                fontSize = 16.sp,
                fontWeight = FontWeight.Bold
            )
            Spacer(modifier = Modifier.height(4.dp))
            Text(text = " Reverberation",fontWeight = FontWeight.Bold)
            EKSlider(title = " feedback", id = 1, initialValue = 224)
            EKSlider(title = " damping", id = 2, initialValue = 192)
            Text(text = " Volumes",fontWeight = FontWeight.Bold)
            EKSlider(title = " bass", id = 3, initialValue = 200)
            EKSlider(title = " chords", id = 4, initialValue = 200)
            EKSlider(title = " lead", id = 5, initialValue = 200)
        }
    }
}


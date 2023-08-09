package com.example.w5300_mqtt_cam

import android.os.Build
import android.os.Bundle
import android.webkit.WebView
import android.widget.Button
import android.widget.EditText
import androidx.annotation.RequiresApi
import androidx.appcompat.app.AppCompatActivity
import com.hivemq.client.mqtt.MqttClient
import com.hivemq.client.mqtt.datatypes.MqttQos
import com.hivemq.client.mqtt.mqtt3.Mqtt3AsyncClient

class MainActivity : AppCompatActivity() {

    private lateinit var client: Mqtt3AsyncClient

    @RequiresApi(Build.VERSION_CODES.N)
    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)
        setContentView(R.layout.activity_main)

        var webView = findViewById<WebView>(R.id.webView)
        var editTextText = findViewById<EditText>(R.id.editTextText)
        val mqttBroker = "44.195.202.69"

        editTextText.setText(mqttBroker)

        client = MqttClient.builder()
            .useMqttVersion3()
            .identifier("AndroidClient-1231231234")
            .serverHost(mqttBroker)
            .buildAsync()

        client.connect()
            .whenComplete { _, throwable ->
                if (throwable != null) {
                    // Handle connection error
                } else {
                    // Successfully connected to the broker
                    client.subscribeWith()
                        .topicFilter("W5300-MQTT")
                        .qos(MqttQos.AT_LEAST_ONCE)
                        .callback { message ->
                            val imageUrl = message.payloadAsBytes.toString(Charsets.UTF_8)
                            //Log.d("W5300-MQTT", imageUrl)
                            if (imageUrl.startsWith("http:"))
                                runOnUiThread {
                                    webView.loadUrl(imageUrl)
                                }
                        }
                        .send()

                    findViewById<Button>(R.id.buttonSend).setOnClickListener {
                        client.publishWith()
                            .topic("MQTT-W5300")
                            .qos(MqttQos.AT_LEAST_ONCE)
                            .payload("cmd:capture".toByteArray())
                            .send()
                    }
                }
            }
    }

    override fun onDestroy() {
        client.disconnect()
        super.onDestroy()
    }
}

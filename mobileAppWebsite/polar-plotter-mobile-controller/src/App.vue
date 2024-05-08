<script setup lang="ts">
import { computed, reactive, shallowRef } from 'vue';
import CalibrationPhase1 from './components/CalibrationPhase1.vue'
import CalibrationPhase2 from './components/CalibrationPhase2.vue'
import CalibrationPhase3 from './components/CalibrationPhase3.vue'
import InitializingState from './components/InitializingState.vue'
import UnknownState from './components/UnknownState.vue'
import WorkingState from './components/WorkingState.vue'

const decoder = new TextDecoder()
const deviceProperties = reactive({
  maxRadius: 0,
  radiusStepSize: 0,
  azimuthStepSize: 0,
  marbleSizeInRadiusSteps: 0,
  currentDrawing: 'Unknown',
  currentStep: 'Unknown',
  position: 'Unknown',
  state: 'Initializing',
  status: ''
})

const data = reactive({
  device: shallowRef<undefined | BluetoothDevice>(undefined),
  supported: navigator && 'bluetooth' in navigator,
  message: '',
  error: ''
})

const isWorkingState = computed(() => {
  return deviceProperties.state === 'Wiping' || deviceProperties.state === 'Retrieving' || deviceProperties.state === 'Drawing' || deviceProperties.state === 'Paused'
})

function clearDevice(evt: Event) {
  data.device = undefined
}

function updateProperty(target: BluetoothRemoteGATTCharacteristic | undefined, setProperty: (value: DataView | undefined) => void) {
  setProperty(target?.value)
}

async function monitorProperty(service: BluetoothRemoteGATTService, uuid: BluetoothCharacteristicUUID, setProperty: (value: DataView | undefined) => void) {
  try {
    const chr = await service?.getCharacteristic(uuid)
    if (chr) {
      const dv = await chr.readValue()
      data.message += ', got prop ' + uuid + '/' + !!dv
      setProperty(dv)

      try {
        data.message += ', add notice ' + uuid
        const notify = await chr.startNotifications()
        data.message += ', add handler ' + uuid
        notify?.addEventListener('characteristicvaluechanged', (evt) => updateProperty(<BluetoothRemoteGATTCharacteristic>evt.target, setProperty))
      } catch (err) {
        // We can't use notifications, so we will poll
        data.message += ', add timeout ' + uuid
        setTimeout(() => updateProperty(chr, setProperty), 1000)
      }
      data.message += ', finished ' + uuid
    } else {
      data.message += ', no prop ' + uuid
      updateProperty(undefined, setProperty)
    }
  } catch (err) {
    data.error = (data.error ? data.error + ', ' : '') + 'CHARACTERISTIC (' + uuid + ') ERROR: ' + err
  }
}

async function requestDevice() {
  data.error = ''
  try {
    data.device = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'Dynamic_Sand_Arduino' }],
      optionalServices: ['45aa5c8f-c47e-42f6-af4a-66544b8aff17']
    })

    const dv = data.device
    data.message = 'Got device ' + dv.name + ' (' + dv.id + ')'

    dv.addEventListener('gattserverdisconnected', clearDevice)
    data.message += ', added disconnect'

    const server = await dv.gatt?.connect()
    if (!server) {
      data.error = 'Could not find the GATT server'
    }

    data.message += ', got server'
    const svc = await server?.getPrimaryService('45aa5c8f-c47e-42f6-af4a-66544b8aff17')

    if (svc) {
      data.message += ', got service'
      monitorProperty(svc, '52eb19a4-6421-4910-a8ca-7ff75ef2f56b', (dv) => deviceProperties.status = dv ? decoder.decode(dv) : '')
      monitorProperty(svc, 'fb65af7d-494f-4a45-8872-6e6ffbf0703c', (dv) => deviceProperties.maxRadius = dv ? dv.getFloat32(0) : 0)
      monitorProperty(svc, 'fd18cf65-85d7-4730-ad77-4cc3fabaab99', (dv) => deviceProperties.radiusStepSize = dv ? dv.getInt32(0) : 0)
      monitorProperty(svc, '4dc9c338-0004-4c05-bd26-bb13e55c3bb9', (dv) => deviceProperties.azimuthStepSize = dv ? dv.getInt32(0) : 0)
      monitorProperty(svc, '60af168a-b702-4d0b-8c1b-f35c7a436781', (dv) => deviceProperties.marbleSizeInRadiusSteps = dv ? dv.getInt32(0) : 0)
      monitorProperty(svc, 'fa95bee6-46f9-4898-913a-0575019d3d33', (dv) => deviceProperties.currentDrawing = dv ? decoder.decode(dv) : '')
      monitorProperty(svc, '54a63a69-90ce-4b14-a103-46152bb1da68', (dv) => deviceProperties.currentStep = dv ? decoder.decode(dv) : '')
      monitorProperty(svc, '7fcd311a-fafa-47ee-80b8-618616697a59', (dv) => deviceProperties.position = dv ? decoder.decode(dv) : '')
      monitorProperty(svc, 'ec314ea1-7426-47fb-825c-8fbd8b02f7fe', (dv) => deviceProperties.state = dv ? decoder.decode(dv) : '')
      data.message += ', added monitors'
    }
  } catch (err) {
    data.error = 'REQUEST ERROR: ' + err
  }
}

async function sendCommand(cmd: string) {
  data.error = ''
  data.message = ''
  try {
    data.message = 'Sending command: ' + cmd
  } catch (err) {
    data.error = 'SEND ERROR: ' + err
  }
}
</script>

<template>
  <div v-if="data.supported">
    <div v-if="data.device">
      <header><h1>{{ deviceProperties.state }}</h1></header>

      <main>
        <div>Max Radius: {{ deviceProperties.maxRadius }}</div>
        <div>Radius Step Size: {{ deviceProperties.radiusStepSize }}</div>
        <div>Azimuth Step Size: {{ deviceProperties.azimuthStepSize }}</div>
        <div>Marble Size: {{ deviceProperties.marbleSizeInRadiusSteps }}</div>
        <div>Drawing: {{ deviceProperties.currentDrawing }}</div>
        <div>Step: {{ deviceProperties.currentStep }}</div>
        <div>Position: {{ deviceProperties.position }}</div>
        <div>Status: {{ deviceProperties.status }}</div>
        <div v-if="isWorkingState">
          <WorkingState :state="deviceProperties.state" @send-command="sendCommand" />
        </div>
        <div v-else-if="deviceProperties.state === 'Initializing'">
          <InitializingState />
        </div>
        <div v-else-if="deviceProperties.state === 'Calibrating Center'">
          <CalibrationPhase1 />
        </div>
        <div v-else-if="deviceProperties.state === 'Calibrating Edge'">
          <CalibrationPhase2 />
        </div>
        <div v-else-if="deviceProperties.state === 'Calibrating Circle'">
          <CalibrationPhase3 />
        </div>
        <div v-else>
          <UnknownState />
        </div>
      </main>
    </div>
    <div v-else>
      <header><h1>Select a Device</h1></header>
      <main>
        <h2>Choose the Polar Plotter device to monitor</h2>
        <button @click="requestDevice">Select Device</button>
      </main>
    </div>
  </div>
  <div v-else>
    <header><h1>Unsupported</h1></header>
    <main><p>Your device does not support the Bluetooth Low Energy Web API.  Please try again from a different device.</p></main>
  </div>
  <div class="error">{{ data.error }}</div>
  <div class="message">{{ data.message }}</div>
</template>

<style scoped>
header {
  line-height: 1.5;
  text-align: center;
}

.message {
  color: darkgreen;
}

.error {
  color: red;
}
</style>

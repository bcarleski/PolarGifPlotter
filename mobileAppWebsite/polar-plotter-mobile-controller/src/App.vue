<script setup lang="ts">
import { computed, reactive } from 'vue';
import CalibrationPushing from './components/CalibrationPushing.vue'
import InitializingState from './components/InitializingState.vue'
import ManualMoving from './components/ManualMoving.vue'
import UnknownState from './components/UnknownState.vue'
import WorkingState from './components/WorkingState.vue'

const decoder = new TextDecoder()
const encoder = new TextEncoder()
const deviceProperties : {maxRadius:number, radiusStepSize:number, azimuthStepSize:number, marbleSizeInRadiusSteps:number, currentDrawing:string, currentStep:string, radius:number, azimuth:number, state:string, status:string, sendCmd:BluetoothRemoteGATTCharacteristic | undefined} = reactive({
  maxRadius: 0,
  radiusStepSize: 0,
  azimuthStepSize: 0,
  marbleSizeInRadiusSteps: 0,
  currentDrawing: 'Unknown',
  currentStep: 'Unknown',
  radius: 0,
  azimuth: 0,
  state: 'Initializing',
  status: '',
  sendCmd: undefined
})

const data : {device:undefined|BluetoothDevice, supported:boolean, disabled:boolean, message:string, error:string} = reactive({
  device: undefined,
  supported: navigator && 'bluetooth' in navigator,
  disabled: false,
  message: '',
  error: ''
})

const isWorkingState = computed(() => {
  return deviceProperties.state === 'Wiping' || deviceProperties.state === 'Retrieving Commands' || deviceProperties.state === 'Drawing' || deviceProperties.state === 'Paused' || deviceProperties.state === 'Resuming'
})

const isCalibrationState = computed(() => {
  return deviceProperties.state && deviceProperties.state.startsWith('Calibrating ')
})

const isManualState = computed(() => {
  return deviceProperties.state && deviceProperties.state.startsWith('Manual ')
})

const azimuthStepSizeDegrees = computed(() => {
  return deviceProperties.azimuthStepSize * 180 / Math.PI;
})

const azimuthDegrees = computed(() => {
  return deviceProperties.azimuth * 180 / Math.PI;
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

      deviceProperties.sendCmd = await svc.getCharacteristic('66af95bc-3dd1-4343-b4b5-ad328b33fda7')
      data.message += deviceProperties.sendCmd ? ', got send command' : ', could not get send command'

      await monitorProperty(svc, 'fb65af7d-494f-4a45-8872-6e6ffbf0703c', (dv) => deviceProperties.maxRadius = dv ? dv.getFloat64(0, true) : 0)
      await monitorProperty(svc, 'fd18cf65-85d7-4730-ad77-4cc3fabaab99', (dv) => deviceProperties.radiusStepSize = dv ? dv.getFloat64(0, true) : 0)
      await monitorProperty(svc, '4dc9c338-0004-4c05-bd26-bb13e55c3bb9', (dv) => deviceProperties.azimuthStepSize = dv ? dv.getFloat64(0, true) : 0)
      await monitorProperty(svc, '60af168a-b702-4d0b-8c1b-f35c7a436781', (dv) => deviceProperties.marbleSizeInRadiusSteps = dv ? dv.getInt32(0, true) : 0)
      await monitorProperty(svc, 'fa95bee6-46f9-4898-913a-0575019d3d33', (dv) => deviceProperties.currentDrawing = dv ? decoder.decode(dv) : '')
      await monitorProperty(svc, '54a63a69-90ce-4b14-a103-46152bb1da68', (dv) => deviceProperties.currentStep = dv ? decoder.decode(dv) : '')
      await monitorProperty(svc, '7fcd311a-fafa-47ee-80b8-618616697a59', (dv) => deviceProperties.radius = dv ? dv.getFloat64(0, true) : 0)
      await monitorProperty(svc, 'eb654acc-3430-45e3-8dc9-22c9fe982518', (dv) => deviceProperties.azimuth = dv ? dv.getFloat64(0, true) : 0)
      await monitorProperty(svc, '52eb19a4-6421-4910-a8ca-7ff75ef2f56b', (dv) => deviceProperties.status = dv ? decoder.decode(dv) : '')
      await monitorProperty(svc, 'ec314ea1-7426-47fb-825c-8fbd8b02f7fe', (dv) => deviceProperties.state = dv ? decoder.decode(dv) : '')
      data.message += ', added monitors'
    }
  } catch (err) {
    data.error = 'REQUEST ERROR: ' + err
  }
}

async function sendCommand(cmd: string) {
  data.message = ''

  if (!deviceProperties.sendCmd) {
    data.error = 'Send Command Not Found'
    return
  }

  data.error = ''
  data.disabled = true
  setTimeout(() => { data.disabled = false; data.message = ''; }, 1000)

  try {
    data.message = 'Sending: ' + cmd
    await deviceProperties.sendCmd.writeValue(encoder.encode(cmd))
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
        <div>Azimuth Step Size: {{ azimuthStepSizeDegrees }}&deg;</div>
        <div>Marble Size: {{ deviceProperties.marbleSizeInRadiusSteps }}</div>
        <div>Drawing: {{ deviceProperties.currentDrawing }}</div>
        <div>Step: {{ deviceProperties.currentStep }}</div>
        <div>Radius: {{ deviceProperties.radius }}</div>
        <div>Azimuth: {{ azimuthDegrees }}&deg;</div>
        <div>Status: {{ deviceProperties.status }}</div>
        <div v-if="deviceProperties.state === 'Initializing'">
          <InitializingState />
        </div>
        <div v-else-if="isWorkingState">
          <WorkingState :state="deviceProperties.state" :disabled="data.disabled" @send-command="sendCommand" />
        </div>
        <div v-else-if="isManualState">
          <ManualMoving :state="deviceProperties.state" :disabled="data.disabled" @send-command="sendCommand" />
        </div>
        <div v-else-if="isCalibrationState">
          <CalibrationPushing :state="deviceProperties.state" :disabled="data.disabled" @send-command="sendCommand" />
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
  position: fixed;
  top: 0;
  width: 100%;
  background-color: white;
}
main {
  margin-top: 120px;
}

.message {
  color: darkgreen;
}

.error {
  color: red;
}
</style>

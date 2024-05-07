<script setup lang="ts">
import { computed, reactive, shallowRef } from 'vue';
import CalibrationPhase1 from './components/CalibrationPhase1.vue'
import CalibrationPhase2 from './components/CalibrationPhase2.vue'
import CalibrationPhase3 from './components/CalibrationPhase3.vue'
import InitializingState from './components/InitializingState.vue'
import UnknownState from './components/UnknownState.vue'
import WorkingState from './components/WorkingState.vue'
const deviceProperties = reactive({
  maxRadius: 0,
  radiusStepSize: 0,
  azimuthStepSize: 0,
  marbleSizeInRadiusSteps: 0,
  currentDrawing: 'Unknown',
  currentStep: 'Unknown',
  position: 'Unknown',
  state: 'Initializing'
})

const data = reactive({
  device: shallowRef<undefined | BluetoothDevice>(undefined),
  supported: navigator && 'bluetooth' in navigator
})

const isWorkingState = computed(() => {
  return deviceProperties.state === 'Wiping' || deviceProperties.state === 'Retrieving' || deviceProperties.state === 'Drawing' || deviceProperties.state === 'Paused'
})

async function requestDevice() {
  try {
    data.device = await navigator.bluetooth.requestDevice({
      filters: [{ name: 'Dynamic_Sand_Arduino', services: ['45aa5c8f-c47e-42f6-af4a-66544b8aff17']}]
    })
  } catch (err) {
    console.log('Error: ' + err)
  }
}
</script>

<template>
  <div v-if="data.supported">
    <header><h1>Unsupported</h1></header>
    <main><p>Your device does not support the Bluetooth Low Energy Web API.  Please try again from a different device.</p></main>
  </div>
  <div v-else>
    <div v-if="data.device">
      <header><h1>Select a Device</h1></header>
      <main>
        <h2>Click the button below to choose the device</h2>
        <button @click="requestDevice">Select Device</button>
      </main>
    </div>
    <div v-else>
      <header><h1>{{ deviceProperties.state }}</h1></header>

      <main>
        <div v-if="isWorkingState">
          <WorkingState :state="deviceProperties.state" />
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
  </div>
</template>

<style scoped>
header {
  line-height: 1.5;
  text-align: center;
}
</style>

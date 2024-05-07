<script setup lang="ts">
import { computed, reactive } from 'vue';
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

const isWorkingState = computed(() => {
  return deviceProperties.state === 'Wiping' || deviceProperties.state === 'Retrieving' || deviceProperties.state === 'Drawing' || deviceProperties.state === 'Paused'
})
</script>

<template>
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
</template>

<style scoped>
header {
  line-height: 1.5;
  text-align: center;
}
</style>

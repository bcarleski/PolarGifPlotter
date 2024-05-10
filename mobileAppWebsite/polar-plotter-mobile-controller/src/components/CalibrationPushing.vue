<script setup lang="ts">
  import { ref } from 'vue';

  defineProps<{ state: string, disabled: boolean }>()
  const emit = defineEmits<{
    (e:'sendCommand', command: string): void
  }>()
  const radiusSteps = ref(10500)
  const azimuthSteps = ref(4810)
</script>

<template>
  <div>
    <button @click="emit('sendCommand', 'O-1000')" :disabled="disabled">&lt;&lt;&lt;</button>
    <button @click="emit('sendCommand', 'O-100')" :disabled="disabled">&lt;&lt;</button>
    <button @click="emit('sendCommand', 'O-10')" :disabled="disabled" class="middle-button">&lt;</button>
    <button @click="emit('sendCommand', 'O10')" :disabled="disabled">&gt;</button>
    <button @click="emit('sendCommand', 'O100')" :disabled="disabled">&gt;&gt;</button>
    <button @click="emit('sendCommand', 'O1000')" :disabled="disabled">&gt;&gt;&gt;</button>
  </div>
  <div>
    <button @click="emit('sendCommand', 'A')" :disabled="disabled">Accept</button>
  </div>
  <div v-if="state === 'Calibrating Center'">
    <div>Radius Steps: <input type="number" v-model="radiusSteps" /></div>
    <div>Azimuth Steps: <input type="number" v-model="azimuthSteps" /></div>
    <button @click="emit('sendCommand', 'E' + radiusSteps + ',' + azimuthSteps)" :disabled="disabled">Explicit Calibration</button>
  </div>
  <div>
    <button @click="emit('sendCommand', '.M')" :disabled="disabled">Manual</button>
  </div>
</template>

<style scoped>
button {
  margin: 14px 5px 14px 5px;
  padding: 16px;
}

.middle-button {
  margin-right: 24px;
}
</style>
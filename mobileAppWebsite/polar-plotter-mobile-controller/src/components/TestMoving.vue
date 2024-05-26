<script setup lang="ts">
import { reactive } from 'vue';
  defineProps<{ state: string, disabled: boolean }>()
  const emit = defineEmits<{
    (e:'sendCommand', command: string): void
  }>()
  const data = reactive({
    amount: 1000
  })
</script>

<template>
  <div>
    <button @click="emit('sendCommand', 'M0')" :disabled="disabled">M0</button>
    <button @click="emit('sendCommand', 'M1')" :disabled="disabled">M1</button>
    <button @click="emit('sendCommand', 'M2')" :disabled="disabled">M2</button>
    <button @click="emit('sendCommand', 'M3')" :disabled="disabled">M3</button>
  </div>
  <div>
    <input type="number" v-model="data.amount" />
    <button @click="emit('sendCommand', 'O' + data.amount)" :disabled="disabled">Update Amount</button>
  </div>
  <div>
    <button @click="emit('sendCommand', 'A')" v-if="state == 'Manual Radius'" :disabled="disabled">Switch to Azimuth</button>
    <button @click="emit('sendCommand', 'R')" v-else :disabled="disabled">Switch to Radius</button>
  </div>
  <div>
    <button @click="emit('sendCommand', 'X')" :disabled="disabled">Move to Center</button>
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
<script setup lang="ts">
  defineProps<{ state: string, disabled: boolean }>()
  const emit = defineEmits<{
    (e:'sendCommand', command: string): void
  }>()
</script>

<template>
  <button v-if="state === 'Paused'" @click="emit('sendCommand', '.R')" :disabled="disabled">Resume</button>
  <button v-else @click="emit('sendCommand', '.P')" :disabled="disabled">Pause</button>
  <button @click="emit('sendCommand', '.W')" :disabled="disabled">Wipe</button>
  <button @click="emit('sendCommand', '.C')" :disabled="disabled">Calibrate</button>
  <button @click="emit('sendCommand', '.M')" :disabled="disabled">Manual</button>
</template>

<style scoped>
button {
  margin: 14px 5px 14px 5px;
  padding: 16px;
  display: block;
}

.middle-button {
  margin-right: 24px;
}
</style>
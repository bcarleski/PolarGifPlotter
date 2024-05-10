<script setup lang="ts">
  defineProps<{ state: string, disabled: boolean }>()
  const emit = defineEmits<{
    (e:'sendCommand', command: string): void
  }>()
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
    <button @click="emit('sendCommand', 'A')" v-if="state == 'Manual Radius'" :disabled="disabled">Switch to Azimuth</button>
    <button @click="emit('sendCommand', 'R')" v-else :disabled="disabled">Switch to Radius</button>
  </div>
  <div>
    <button @click="emit('sendCommand', 'C')" :disabled="disabled">Move to Center</button>
  </div>
  <div>
    <button @click="emit('sendCommand', 'X')" :disabled="disabled">Exit Manual</button>
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
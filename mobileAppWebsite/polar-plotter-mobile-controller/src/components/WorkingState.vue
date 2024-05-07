<script setup lang="ts">
  defineProps<{ state: string }>()
  const emit = defineEmits<{
    (e:'sendCommand', command: string): void
  }>()

  const pause = function() {
    
  }
  const resume = function() {
    
  }
  const wipe = function() {
    emit('sendCommand', 'W')
  }
  const testDrawing1 = function() {
    
  }
  const testDrawing2 = function() {
    
  }
  const testDrawing3 = function() {
    
  }
  const calibrate = function() {
    
  }
</script>

<template>
  <button value="Pause" v-if="state === 'Paused'" @click="pause" />
  <button value="Resume" v-else @click="resume" />
  <button value="Wipe" @click="wipe" />
  <button value="Calibrate" @click="calibrate" />
  <button value="Test Drawing 1" @click="testDrawing1" />
  <button value="Test Drawing 2" @click="testDrawing2" />
  <button value="Test Drawing 3" @click="testDrawing3" />
</template>
